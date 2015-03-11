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

#include "crypto.hxx"
#include "coding.hxx"
#include "account.hxx"
#include "otp.hxx"

enum class Mode
{
    OTP,
    Set,
    List,
    Delete
};

inline void Usage(const std::string &progName);
inline void OTP(Account &account);

int main(int argc, char **argv)
{
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

    for (unsigned int i = optind; i < argc; ++i)
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
                        std::cout << "Deleting account " << it->name << ": " << it->description << std::endl;
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
                        std::cout << account.name << ':' << '\n'
                            << std::setw(20) << "description: " << account.description << '\n'
                            << std::setw(20) << "type: " << (account.type == Account::Type::HOTP ? "HOTP" : "TOTP") << '\n'
                            << std::setw(20) << "digits: " << account.digits << '\n'
                            << std::setw(20) << "algorithm: ";

                        switch (account.algorithm)
                        {
                            case Account::Algorithm::MD5:
                                {
                                    std::cout << "MD5";
                                    break;
                                }

                            case Account::Algorithm::SHA1:
                                {
                                    std::cout << "SHA1";
                                    break;
                                }

                            case Account::Algorithm::SHA256:
                                {
                                    std::cout << "SHA256";
                                    break;
                                }

                            case Account::Algorithm::SHA512:
                                {
                                    std::cout << "SHA512";
                                    break;
                                }

                            default:
                                {
                                    std::cout << "SHA1";
                                    break;
                                }
                        }
                        std::cout << '\n'
                            << std::setw(20) << (account.type == Account::Type::HOTP ? "count: " : "interval: ") << account.count << '\n';
                        if (listCount >= 2)
                        {
                            std::cout << std::setw(20) << "secret: " << account.secret << '\n';
                        }
                        std::cout << '\n';
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
                        std::cout << "Updating account " << newAccount.name << std::endl;
                        *it = newAccount;
                        found = true;
                        break;
                    }
                }
                if (!found)
                {
                    std::cout << "Creating new account " << newAccount.name << std::endl;
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
    std::cout << "USAGE:" << '\n'
              << "\t" << progName << " [-d] [-h] [-l[l]] [-p] [-s] [ Args... ]" << '\n'
              << "\t" << "When multiple options are listed, the latest takes precedence." << '\n'
              << '\n'
              << "\t\t" << "-d" << "\t" << "Delete accounts named by Args list." << '\n'
              << "\t\t" << "-h" << "\t" << "Show this help menu and exit." << '\n'
              << "\t\t" << "-l" << "\t" << "List accounts and information.  List twice to also show secret pass.  If accounts" << '\n'
              << "\t\t\t" <<               "are specified in arguments, only show (and increment for HOTP) those." << '\n'
              << "\t\t" << "-p" << "\t" << "Generate OTPs for all accounts.  If accounts are specified in arguments, only show those. (default)" << '\n'
              << "\t\t" << "-s" << "\t" << "Create or set information for one account.  Arguments are, in order, " << '\n'
              << "\t\t\t" <<               "name, description, TOTP or HOTP, digits, algorithm, count or interval number, secret " << '\n'
              << '\n'
              << "\t\t" << "Ex:" << '\t' << progName << "-s name 'Account description' TOTP 6 SHA1 30 ABCDEFGHIJ2345" << '\n';
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

    std::cout << PWGen(Sha1Sum, 64, account.type == Account::Type::TOTP, account.count, account.digits, Base32Decode(std::vector<char>(account.secret.begin(), account.secret.end())), time);
    std::cout << std::endl;
    if (account.type == Account::Type::HOTP)
    {
        ++account.count;
    }
}
