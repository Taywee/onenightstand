/*
    onenightstand - OTP generator system
    Copyright (C) 2015 Taylor C. Richberger

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    The entire text of the license is contained in LICENSE.GPLv3
*/
#include <iostream>
#include <iomanip>
#include <vector>
#include <chrono>
#include <list>
#include <algorithm>
#include <unordered_set>
#include <unistd.h>
#ifdef GETTEXT
#include <libintl.h>
#else
#define gettext(x) x
#define textdomain(x) x
#endif
#include <cstdio>

#include "account.hxx"
#include "otp.hxx"
#include "cryptpp/crypto.hxx"
#include "cryptpp/coding.hxx"
#include "clipp/table.hxx"

enum class Mode
{
    OTP,
    Set,
    List,
    Delete
};

inline void Usage(const std::string &progName);
inline void OTP(Account &account);

template <typename ...Types>
inline std::string gettextf(const std::string &format, Types ...args);


int main(int argc, char **argv)
{
    setlocale(LC_ALL,"");
    textdomain("onenightstand");

    Mode mode = Mode::OTP;

    int opt;
    unsigned int listCount = 0;
    while ((opt = getopt(argc, argv, "dhlps")) != -1)
    {
        switch (opt)
        {
            case 'd':
                {
                    mode = Mode::Delete;
                    break;
                }

            case 'h':
                {
                    Usage(argv[0]);
                    return 0;
                    break;
                }

            case 'l':
                {
                    ++listCount;
                    mode = Mode::List;
                    break;
                }

            case 'p':
                {
                    mode = Mode::OTP;
                    break;
                }

            case 's':
                {
                    mode = Mode::Set;
                    break;
                }

            case '?':
                {
                    Usage(argv[0]);
                    return (1);
                }
        }
    }

    std::vector<std::string> arguments;

    for (int i = optind; i < argc; ++i)
    {
        arguments.emplace_back(argv[i]);
    }

    switch (mode)
    {
        case Mode::OTP:
            {
                std::list<Account> accounts(GetAccounts());
                std::unordered_set<std::string> set(arguments.begin(), arguments.end());
                for (Account &account: accounts)
                {
                    // If no accounts were given, or the current account is in the list of accounts given
                    if (arguments.empty() || set.find(account.name) != set.end())
                    {
                        OTP(account);
                        std::cout << std::endl;
                    }
                }
                SaveAccounts(accounts);
                break;
            }

        case Mode::Delete:
            {
                std::list<Account> accounts(GetAccounts());
                std::unordered_set<std::string> set(arguments.begin(), arguments.end());
                auto it = accounts.begin();
                while (it != accounts.end())
                {
                    if (set.find(it->name) != set.end())
                    {
                        std::cout << gettextf("Deleting account %s: %s", it->name.c_str(), it->description.c_str()) << std::endl;
                        it = accounts.erase(it);
                    } else
                    {
                        ++it;
                    }
                }
                SaveAccounts(accounts);
                break;
            }

        case Mode::List:
            {
                std::list<Account> accounts(GetAccounts());
                std::unordered_set<std::string> set(arguments.begin(), arguments.end());
                for (const Account &account: accounts)
                {
                    if (arguments.empty() || set.find(account.name) != set.end())
                    {
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

                        std::list<std::vector<std::string>> table = {
                            {gettext("description"), account.description},
                            {gettext("type"), account.type == Account::Type::HOTP ? "HOTP" : "TOTP"},
                            {gettext("digits"), std::to_string(account.digits)},
                            {gettext("algorithm"), algorithm},
                            {account.type == Account::Type::HOTP ? gettext("count") : gettext("interval"), std::to_string(account.count)},
                            {gettext("secret"), account.secret}};

                        const std::vector<unsigned int> lengths = {22, 50};

                        if (listCount < 2)
                        {
                            table.pop_back();
                        }
                        for (const std::vector<std::string> &item: table)
                        {
                            std::cout << Table::Row(item, lengths, "", "", "") << '\n';
                        }
                    }
                }
                break;
            }

        case Mode::Set:
            {
                if (arguments.size() != 7)
                {
                    Usage(argv[0]);
                    return 1;
                }

                std::list<Account> accounts(GetAccounts());
                Account newAccount(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6]);

                bool found = false;
                for (auto it = accounts.begin(); it != accounts.end(); ++it)
                {
                    if (it->name == newAccount.name)
                    {
                        std::cout << gettextf("Updating account %s", newAccount.name.c_str()) << std::endl;

                        *it = newAccount;
                        found = true;
                        break;
                    }
                }
                if (!found)
                {
                    std::cout << gettextf("Creating new account %s", newAccount.name.c_str()) << std::endl;

                    accounts.emplace_back(newAccount);
                }
                SaveAccounts(accounts);
                break;
            }
    }

    return 0;
}

void Usage(const std::string &progName)
{
    const std::list<std::vector<std::string>> options = {
        {"", "-d", gettext("Delete accounts named by Args list.")},
        {"", "-h", gettext("Show this help menu and exit.")},
        {"", "-l", gettext("List accounts and information.  List twice to also show secret pass.  If accounts are specified in arguments, only show (and increment for HOTP) those.")},
        {"", "-p", gettext("Generate OTPs for all accounts.  If accounts are specified in arguments, only show those. (default)")},
        {"", "-s", gettext("Create or set information for one account.  Arguments are, in order, name, description, TOTP or HOTP, digits, algorithm, count or interval number, secret")}};
    const std::vector<unsigned int> lengths = {8, 8, 56};

    std::cout << "USAGE:" << '\n'
              << "\t" << progName << " [-d] [-h] [-l[l]] [-p] [-s] [ Args... ]" << '\n'
              << "\t" << gettext("When multiple options are listed, the latest takes precedence.") << '\n'
              << '\n';
    for (const std::vector<std::string> &option: options)
    {
        std::cout << Table::Row(option, lengths, "", "", "") << '\n';
    }
    std::cout << "\n\t\t" << gettext("Ex:") << '\t' << progName << gettext(" -s name 'Account description' TOTP 6 SHA1 30 ABCDEFGHIJ2345") << '\n';
}

void OTP(Account &account)
{
    time_t time = std::chrono::duration_cast<std::chrono::duration<time_t, std::ratio<1, 1>>>(std::chrono::system_clock::now().time_since_epoch()).count();

    std::cout << account.name << ":  " << account.description << '\n';

    if (account.type == Account::Type::TOTP)
    {
        std::cout << std::setw(4) << account.count - (time % account.count) << ": ";
    } else
    {
        std::cout << std::setw(4) << account.count << ": ";
    }

    const std::string secret(Base32::IterDecode(account.secret.begin(), account.secret.end()));

    std::cout << PWGen(Sha1::Sum, Sha1::BlockSize, account.type == Account::Type::TOTP, account.count, account.digits, secret, time);
    std::cout << std::endl;
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
    buffer.resize(snprintf(NULL, 0, translated, args...));
    snprintf(buffer.data(), buffer.size(), translated, args...);
    if (buffer.back() == '\0')
    {
        buffer.pop_back();
    }

    return std::string(buffer.data(), buffer.size());
}
