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
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <list>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <cstdio>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
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

#include "account.pb.h"

inline void OTP(Account &account);

template <typename ...Types>
inline std::string gettextf(const std::string &format, Types ...args);

static void ToLowerReader(const std::string &name, const std::string &value, std::string &destination);

static const std::string GetAccountFile();
static Accounts GetAccounts();
static void SaveAccounts(const Accounts &accounts);

int main(int argc, char **argv)
{
    setlocale(LC_ALL,"");
    textdomain("onenightstand");

    const std::unordered_map<std::string, Account::Type> typemap{
        {"totp", Account::TOTP},
        {"hotp", Account::HOTP}};
    const std::unordered_map<std::string, Account::Algorithm> algorithmmap{
        {"sha1", Account::SHA1},
        {"sha256", Account::SHA256},
        {"sha512", Account::SHA512},
        {"md5", Account::MD5}};
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

        Accounts accounts = GetAccounts();

        if (deletes)
        {
            // Move erasure to the end
            std::remove_reference<decltype(accounts.account())>::type::const_iterator it = std::remove_if(std::begin(*accounts.mutable_account()), std::end(*accounts.mutable_account()), [&deletes](const Account &account) -> bool { return (args::get(deletes).find(account.name()) != args::get(deletes).end()); });
            // Print the massage
            for (auto jt = it; jt != accounts.account().end(); ++jt)
            {
                    std::cout << gettextf("Deleting account %s: %s", jt->name().c_str(), jt->description().c_str()) << std::endl;
            }
            accounts.mutable_account()->erase(it, accounts.account().end());
            SaveAccounts(accounts);
        }
        else if (list)
        {
            for (const Account &account: accounts.account())
            {
                std::cout << "### " << account.name() << '\n';
                std::string algorithm;

                switch (account.algorithm())
                {
                    case Account::MD5:
                        {
                            algorithm = "MD5";
                            break;
                        }

                    case Account::SHA1:
                        {
                            algorithm = "SHA1";
                            break;
                        }

                    case Account::SHA256:
                        {
                            algorithm = "SHA256";
                            break;
                        }

                    case Account::SHA512:
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

                std::cout << gettext("description") << ":\n    " << account.description() << '\n'
                    << gettext("type") << ":\n    " << (account.type() == Account::HOTP ? "HOTP" : "TOTP") << '\n'
                    << gettext("digits") << ":\n    " << account.digits() << '\n'
                    << gettext("algorithm") << ":\n    " << algorithm << '\n'
                    << (account.type() == Account::HOTP ? gettext("count") : gettext("interval")) << ":\n    " << account.count() << '\n';
                if (args::get(list) > 1)
                {
                    std::cout << gettext("secret") << ":\n    " << account.secret() << '\n';
                }
                std::cout << "==================" << std::endl;
            }
        }
        else if (set)
        {
            auto it = std::find_if(std::begin(*accounts.mutable_account()), std::end(*accounts.mutable_account()), [&set](const Account &a) -> bool { return a.name() == args::get(set); });
            if (it == std::end(*accounts.mutable_account()))
            {
                if (!setgroup)
                    throw args::Error("When trying to --set a new account, all of the set options are mandatory");
                std::cout << gettextf("Creating new account %s", args::get(set).c_str()) << std::endl;

                Account &newAccount = *accounts.add_account();
                newAccount.set_name(args::get(set));
                newAccount.set_description(args::get(description));
                newAccount.set_type(args::get(type));
                newAccount.set_digits(args::get(digits));
                newAccount.set_algorithm(args::get(algorithm));
                newAccount.set_count(args::get(count));
                newAccount.set_secret(args::get(secret));
            } else
            {
                std::cout << gettextf("Updating account %s", args::get(set).c_str()) << std::endl;

                Account &account = *it;
                if (description)
                    account.set_description(args::get(description));
                if (type)
                    account.set_type(args::get(type));
                if (digits)
                    account.set_digits(args::get(digits));
                if (algorithm)
                    account.set_algorithm(args::get(algorithm));
                if (count)
                    account.set_count(args::get(count));
                if (secret)
                    account.set_secret(args::get(secret));
            }
            SaveAccounts(accounts);
        } else
        {
            if (accounts.account_size() > 0)
            {
                std::for_each(std::begin(*accounts.mutable_account()), --(accounts.mutable_account()->end()), [](Account &account){OTP(account); std::cout << std::endl;});
                OTP(*(--(accounts.mutable_account()->end())));
            }
            SaveAccounts(accounts);
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

    std::cout << account.name() << ":  " << account.description() << '\n';

    if (account.type() == Account::TOTP)
    {
        std::cout << std::setw(6) << account.count() - (time % account.count()) << ": ";
    } else
    {
        std::cout << std::setw(6) << account.count() << ": ";
    }

    const uint64_t rmessage = (account.type() == Account::TOTP ? time / account.count() : account.count());
    const std::vector<unsigned char> secret{base32::decode(account.secret())};
    std::stringstream value;
    for (unsigned int i = 0; i < 8; ++i)
    {
        value.put(static_cast<char>(rmessage >> ((7 - i) * 8)));
    }
    const std::string smessage{value.str()};
    const std::vector<unsigned char> message(std::begin(smessage), std::end(smessage));

    std::vector<unsigned char> digest;
    const EVP_MD *evp;
    switch (account.algorithm())
    {
        case Account::MD5:
            {
                evp = EVP_md5();
                break;
            }

        case Account::SHA1:
            {
                evp = EVP_sha1();
                break;
            }

        case Account::SHA256:
            {
                evp = EVP_sha256();
                break;
            }

        case Account::SHA512:
            {
                evp = EVP_sha512();
                break;
            }

        default:
            {
                evp = EVP_sha1();
                break;
            }
    }
    digest.resize(EVP_MD_size(evp));
    HMAC(evp, secret.data(), secret.size(), message.data(), message.size(), digest.data(), nullptr);

    const uint8_t offset = digest.back() & 0x0F;

    const uint32_t truncatedHash = 
        ((digest[offset] & 0x7f) << 24) |
        ((digest[offset + 1] & 0xff) << 16) |
        ((digest[offset + 2] & 0xff) << 8) |
        (digest[offset + 3] & 0xff);

    std::ostringstream output;
    output <<  std::setfill('0') << std::setw(account.digits()) << truncatedHash % static_cast<unsigned long>(pow(10, account.digits()));
    std::cout << output.str() << std::endl;

    if (account.type() == Account::HOTP)
    {
        account.set_count(account.count() + 1);
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

static bool makepath(const std::string &path, const mode_t mode = S_IRWXU)
{
    std::string::size_type pos = path.find('/');
    while (pos != path.npos)
    {
        if (pos != 0)
        {
            if (mkdir(path.substr(0, pos).c_str(), mode) == -1)
            {
                if (errno != EEXIST)
                {
                    return false;
                }
            }
        }
        pos = path.find('/', pos + 1);
    }
    if (mkdir(path.c_str(), mode) == -1)
    {
        if (errno != EEXIST)
        {
            return false;
        }
    }
    return true;
}

static const std::string GetAccountFile()
{
    const char * const datahome = getenv("XDG_DATA_HOME");
    std::string home;
    if (datahome)
    {
        home.assign(datahome);
    } else
    {
        const char * const homeenv = getenv("HOME");
        if (homeenv)
        {
            home.assign(homeenv);
        } else
        {
            const struct passwd *const pw = getpwuid(getuid());

            home.assign(pw->pw_dir);
        }
        home.append("/.local/share");
        if (!makepath(home))
        {
            std::ostringstream problem;

            problem << "Could not create directory \"" << home << '"';
            throw std::runtime_error(problem.str());
        }
    }
    return home + "/onenightstand.dat";
}

static Accounts GetAccounts()
{
    Accounts accounts;
    const std::string accountsFile(GetAccountFile());
    std::ifstream file(accountsFile, std::ios_base::binary);
    accounts.ParseFromIstream(&file);
    return accounts;
}

static void SaveAccounts(const Accounts &accounts)
{
    const std::string accountsFile(GetAccountFile());
    std::ofstream file(accountsFile, std::ios_base::binary);
    accounts.SerializeToOstream(&file);
}
