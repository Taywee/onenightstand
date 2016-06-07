/*
    onenightstand - OTP generator system
    Copyright (C) 2015 Taylor C. Richberger

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    The entire text of the license is contained in LICENSE
*/
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <list>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <cstdio>

#include <unistd.h>
#ifdef GETTEXT
#include <libintl.h>
#else
#define gettext(x) x
#define textdomain(x)
#endif

#include <openssl/hmac.h>
#include <openssl/evp.h>

#include <args.hxx>
#include <cppcodec/base32_default_rfc4648.hpp>

#include "account.hxx"

inline void OTP(Account &account);

template <typename ...Types>
inline std::string gettextf(const std::string &format, Types ...args);

static void ToLowerReader(const std::string &name, const std::string &value, std::string &destination);

int main(int argc, char **argv)
{
    setlocale(LC_ALL,"");
    textdomain("onenightstand");

    const std::unordered_map<std::string, Account::Type> typemap{
        {"totp", Account::Type::TOTP},
        {"hotp", Account::Type::HOTP}};
    const std::unordered_map<std::string, Account::Algorithm> algorithmmap{
        {"sha1", Account::Algorithm::SHA1},
        {"sha256", Account::Algorithm::SHA256},
        {"sha512", Account::Algorithm::SHA512},
        {"md5", Account::Algorithm::MD5}};
    args::ArgumentParser parser(gettext("This program generates OTPs, particularly Google's flavor"));
    args::HelpFlag help(parser, gettext("help"), gettext("Display this help menu"), {'h', "help"});
    args::Group args(parser, gettext("Only one of the following may be specified:"), args::Group::Validators::AtMostOne);
    args::ValueFlagList<std::string, std::unordered_set<std::string>> deletes(args, gettext("account"), gettext("Delete accounts given as arguments"), {'d', "delete"});
    args::CounterFlag list(args, gettext("list"), gettext("List accounts.  Specify twice to increase verbosity"), {'l', "list"});
    args::Flag print(args, gettext("print"), gettext("Print OTPs (default)"), {'p', "print"});
    args::ValueFlag<std::string> set(args, gettext("name"), gettext("Set an OTP account"), {'s', "set"});
    args::Group dontcare(parser);
    args::Group setgroup(dontcare, gettext("If --set is specified for a new account, all of the following must be specified"), args::Group::Validators::All);
    args::ValueFlag<std::string> description(setgroup, gettext("description"), gettext("The new account description"), {'D', "description"});
    args::MapFlag<std::string, Account::Type, ToLowerReader> type(setgroup, gettext("type"), gettext("The account type (TOTP or HOTP)"), {'t', "type"}, typemap);
    args::ValueFlag<int> digits(setgroup, gettext("digits"), gettext("The number of digits in the OTP"), {'n', "digits"});
    args::MapFlag<std::string, Account::Algorithm, ToLowerReader> algorithm(setgroup, gettext("algorithm"), gettext("The algorithm type (md5, sha1, sha256, sha512), defaults sha1"), {'a', "algorithm"}, algorithmmap);
    args::ValueFlag<int> count(setgroup, gettext("count"), gettext("Interval (TOTP) or count number (HOTP)"), {'i', "interval", 'c', "count"});
    args::ValueFlag<std::string> secret(setgroup, gettext("secret"), gettext("Secret key"), {'S', "secret"});

    try
    {
        parser.ParseCLI(argc, argv);

        std::list<Account> accounts(GetAccounts());

        if (deletes)
        {
            for (auto it = accounts.begin(); it != accounts.end();)
            {
                if (args::get(deletes).find(it->name) != args::get(deletes).end())
                {
                    std::cout << gettextf("Deleting account %s: %s", it->name.c_str(), it->description.c_str()) << std::endl;
                    it = accounts.erase(it);
                } else
                {
                    ++it;
                }
            }
            SaveAccounts(accounts);
        }
        else if (list)
        {
            for (const Account &account: accounts)
            {
                std::cout << "### " << account.name << '\n';
                std::string algorithm;

                switch (account.algorithm)
                {
                    case Account::Algorithm::MD5:
                        {
                            algorithm = "MD5";
                            break;
                        }

                    case Account::Algorithm::SHA1:
                        {
                            algorithm = "SHA1";
                            break;
                        }

                    case Account::Algorithm::SHA256:
                        {
                            algorithm = "SHA256";
                            break;
                        }

                    case Account::Algorithm::SHA512:
                        {
                            algorithm = "SHA512";
                            break;
                        }

                    default:
                        {
                            algorithm = "SHA1";
                            break;
                        }
                }

                std::cout << gettext("description") << ":\n    " << account.description << '\n'
                    << gettext("type") << ":\n    " << (account.type == Account::Type::HOTP ? "HOTP" : "TOTP") << '\n'
                    << gettext("digits") << ":\n    " << std::to_string(account.digits) << '\n'
                    << gettext("algorithm") << ":\n    " << algorithm << '\n'
                    << (account.type == Account::Type::HOTP ? gettext("count") : gettext("interval")) << ":\n    " << account.count << '\n';
                if (args::get(list) > 1)
                {
                    std::cout << gettext("secret") << ":\n    " << account.secret << '\n';
                }
                std::cout << "==================" << std::endl;
            }
        }
        else if (set)
        {
            auto it = std::find_if(std::begin(accounts), std::end(accounts), [&set](const Account &a) -> bool { return a.name == args::get(set); });
            if (it == std::end(accounts))
            {
                if (!setgroup)
                    throw args::Error("When trying to --set a new account, all of the set options are mandatory");
                const Account newAccount(args::get(set), args::get(description), args::get(type), args::get(digits), args::get(algorithm), args::get(count), args::get(secret));
                std::cout << gettextf("Creating new account %s", args::get(set).c_str()) << std::endl;

                accounts.emplace_back(newAccount);
            } else
            {
                std::cout << gettextf("Updating account %s", args::get(set).c_str()) << std::endl;

                Account &account = *it;
                if (description)
                    account.description = args::get(description);
                if (type)
                    account.type = args::get(type);
                if (digits)
                    account.digits = args::get(digits);
                if (algorithm)
                    account.algorithm = args::get(algorithm);
                if (count)
                    account.count = args::get(count);
                if (secret)
                    account.secret = args::get(secret);
            }
            SaveAccounts(accounts);
        } else
        {
            if (!accounts.empty())
            {
                std::for_each(accounts.begin(), --(accounts.end()), [](Account &account){OTP(account); std::cout << std::endl;});
                OTP(accounts.back());
            }
        }

        return 0;
    }
    catch (args::Help)
    {
        std::cout << parser;
        return 0;
    }
    catch (args::Error e)
    {
        std::cerr << e.what() << std::endl;
        std::cerr << parser;
        return 1;
    }
}

void OTP(Account &account)
{
    const time_t time = std::chrono::duration_cast<std::chrono::duration<time_t, std::ratio<1, 1>>>(std::chrono::system_clock::now().time_since_epoch()).count();

    std::cout << account.name << ":  " << account.description << '\n';

    if (account.type == Account::Type::TOTP)
    {
        std::cout << std::setw(4) << account.count - (time % account.count) << ": ";
    } else
    {
        std::cout << std::setw(4) << account.count << ": ";
    }

    const uint64_t rmessage = (account.type == Account::Type::TOTP ? time / account.count : account.count);
    const std::vector<unsigned char> secret{base32::decode(account.secret)};
    std::stringstream value;
    for (unsigned int i = 0; i < 8; ++i)
    {
        value.put(static_cast<char>(rmessage >> ((7 - i) * 8)));
    }
    const std::string smessage{value.str()};
    const std::vector<unsigned char> message(std::begin(smessage), std::end(smessage));

    std::array<unsigned char, 20> digest;
    unsigned int dummy = digest.size();
    HMAC(EVP_sha1(), secret.data(), secret.size(), message.data(), message.size(), digest.data(), &dummy);

    const uint8_t offset = digest.back() & 0x0F;

    uint32_t truncatedHash = 0;

    for (unsigned int i = 0; i < 4; ++i)
    {
        truncatedHash |= static_cast<unsigned char>(digest[i + offset]) << (24 - (i * 8));
    }
    truncatedHash &= 0x7FFFFFFF;

    std::ostringstream output;
    output <<  std::setfill('0') << std::setw(account.digits) << truncatedHash % static_cast<unsigned long>(pow(10, account.digits));
    std::cout << output.str() << std::endl;

    if (account.type == Account::Type::HOTP)
    {
        ++account.count;
    }
}

template <typename ...Types>
std::string gettextf(const std::string &format, Types ...args)
{
    const char *translated = gettext(format.c_str());

    std::vector<char> buffer;
    buffer.resize(snprintf(NULL, 0, translated, args...) + 1);
    snprintf(buffer.data(), buffer.size(), translated, args...);
    while (buffer.back() == '\0')
    {
        buffer.pop_back();
    }

    return std::string(buffer.data(), buffer.size());
}

void ToLowerReader(const std::string &name, const std::string &value, std::string &destination)
{
    destination = value;
    std::transform(destination.begin(), destination.end(), destination.begin(), ::tolower);
}
