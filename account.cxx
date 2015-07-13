/*
    Copyright (C) 2015 Taylor C. Richberger

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    The entire text of the license is contained in LICENSE
*/
#include "account.hxx"

#include <cctype>
#include <sstream>
#include <algorithm>
#include <fstream>

Account::Account(const std::string &name, const std::string &description, const Type type, const unsigned short digits, const Algorithm algorithm, const unsigned int count, const std::string &secret) : name(name), description(description), type(type), digits(digits), algorithm(algorithm), count(count), secret(secret)
{
    this->name.erase(std::remove(this->name.begin(), this->name.end(), ':'), this->name.end());
    this->description.erase(std::remove(this->description.begin(), this->description.end(), ':'), this->description.end());
    this->secret.erase(std::remove(this->secret.begin(), this->secret.end(), ':'), this->secret.end());
}

Account::Account(const std::string &name, const std::string &description, const std::string &type, const std::string &digits, const std::string &algorithm, const std::string &count, const std::string &secret) : name(name), description(description), secret(secret)
{
    this->name.erase(std::remove(this->name.begin(), this->name.end(), ':'), this->name.end());
    this->description.erase(std::remove(this->description.begin(), this->description.end(), ':'), this->description.end());
    this->secret.erase(std::remove(this->secret.begin(), this->secret.end(), ':'), this->secret.end());

    if (toupper(type.front()) == 'T')
    {
        this->type = Type::TOTP;
    } else
    {
        this->type = Type::HOTP;
    }
    {
        std::stringstream conv;
        conv << digits;
        conv >> this->digits;
    }

    if (algorithm.substr(0, 3) == "sha" || algorithm.substr(0, 3) == "SHA")
    {
        size_t numberIndex = algorithm.find_first_of("0123456789");
        if (numberIndex != algorithm.npos)
        {
            std::string numbers(algorithm.substr(numberIndex));
            if (numbers == "256")
            {
                this->algorithm = Algorithm::SHA256;
            } else if (numbers == "512")
            {
                this->algorithm = Algorithm::SHA512;
            } else
            {
                this->algorithm = Algorithm::SHA1;
            }
        } else
        {
            this->algorithm = Algorithm::SHA1;
        }
    } else
    {
        this->algorithm = Algorithm::MD5;
    }
    {
        std::stringstream conv;
        conv << count;
        conv >> this->count;
    }
}

inline std::vector<std::string> Split(const std::string &s, const char delim)
{
    std::vector<std::string> elems;
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

static const std::string GetAccountFile()
{
    const char * const home = getenv("HOME");
    std::string file;
    if (home)
    {
        file.assign(home);
    } else
    {
        file.assign(".");
    }
    file.append("/.onenightstand");

    return file;
}

std::list<Account> GetAccounts()
{
    const std::string fileName(GetAccountFile());
    std::list<Account> accounts;

    std::ifstream file(fileName);
    if (file)
    {
        std::string line;
        while (std::getline(file, line))
        {
            // Remove comments
            size_t crunch = line.find('#');
            if (crunch < line.size())
            {
                line.erase(crunch);
            }
            // Remove trailing whitespace (since lines may contain comments)
            size_t whitespace = line.find_last_not_of(" \t\f\v\n\r");
            if ((whitespace + 1) < line.size())
            {
                line.erase(whitespace + 1);
            } else if (whitespace == line.npos)
            {
                continue;
            }

            std::vector<std::string> fields(Split(line, ':'));
            accounts.emplace_back(fields[0], fields[1], fields[2], fields[3], fields[4], fields[5], fields[6]);
        }
    }
    return accounts;
}
void SaveAccounts(const std::list<Account> &accounts)
{
    const std::string fileName(GetAccountFile());

    std::ofstream file(fileName);
    if (file)
    {
        for (const Account &account: accounts)
        {
            file << account.name << ':'
                 << account.description << ':'
                 << (account.type == Account::Type::HOTP ? "HOTP" : "TOTP") << ':'
                 << account.digits << ':';
            switch (account.algorithm)
            {
                case Account::Algorithm::MD5:
                    {
                        file << "MD5";
                        break;
                    }

                case Account::Algorithm::SHA1:
                    {
                        file << "SHA1";
                        break;
                    }

                case Account::Algorithm::SHA256:
                    {
                        file << "SHA256";
                        break;
                    }

                case Account::Algorithm::SHA512:
                    {
                        file << "SHA512";
                        break;
                    }

                default:
                    {
                        file << "SHA1";
                        break;
                    }
            }
            file << ':'
                 << account.count << ':'
                 << account.secret << '\n';
        }
    }
}
