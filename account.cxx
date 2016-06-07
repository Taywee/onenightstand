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

#include <json-c/json.h>

Account::Account(const std::string &name, const std::string &description, const Type type, const unsigned short digits, const Algorithm algorithm, const unsigned int count, const std::string &secret) : name(name), description(description), type(type), digits(digits), algorithm(algorithm), count(count), secret(secret)
{
    this->name.erase(std::remove(this->name.begin(), this->name.end(), ':'), this->name.end());
    this->description.erase(std::remove(this->description.begin(), this->description.end(), ':'), this->description.end());
    this->secret.erase(std::remove(this->secret.begin(), this->secret.end(), ':'), this->secret.end());
}

Account::Account(const std::string &name, const std::string &description, const std::string &type, const unsigned short digits, const std::string &algorithm, const unsigned int count, const std::string &secret) : name(name), description(description), digits(digits), count(count), secret(secret)
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

#include <iostream>
std::list<Account> GetAccounts()
{
    const std::string fileName(GetAccountFile());
    std::list<Account> accounts;

    std::ifstream file(fileName);
    if (file)
    {
        std::ostringstream buffer;
        buffer << file.rdbuf();
        json_object *jaccounts = json_tokener_parse(buffer.str().c_str());
        int arraylen = json_object_array_length(jaccounts);
        for (int i = 0; i < arraylen; ++i)
        {
            json_object *account = json_object_array_get_idx(jaccounts, i);
            json_object *name;
            json_object *description;
            json_object *type;
            json_object *digits;
            json_object *algorithm;
            json_object *count;
            json_object *secret;
            json_object_object_get_ex(account, "name", &name);
            json_object_object_get_ex(account, "description", &description);
            json_object_object_get_ex(account, "type", &type);
            json_object_object_get_ex(account, "digits", &digits);
            json_object_object_get_ex(account, "algorithm", &algorithm);
            json_object_object_get_ex(account, "count", &count);
            json_object_object_get_ex(account, "secret", &secret);
            const std::string sname(json_object_get_string(name), json_object_get_string_len(name));
            const std::string sdescription(json_object_get_string(description), json_object_get_string_len(description));
            const std::string stype(json_object_get_string(type), json_object_get_string_len(type));
            const unsigned short sdigits = json_object_get_int(digits);
            const std::string salgorithm(json_object_get_string(algorithm), json_object_get_string_len(algorithm));
            const unsigned int scount = json_object_get_int(count);
            const std::string ssecret(json_object_get_string(secret), json_object_get_string_len(secret));
            accounts.emplace_back(sname, sdescription, stype, sdigits, salgorithm, scount, ssecret);
        }
        json_object_put(jaccounts);
    }
    return accounts;
}

void SaveAccounts(const std::list<Account> &accounts)
{
    const std::string fileName(GetAccountFile());

    std::ofstream file(fileName);
    if (file)
    {
        json_object *jaccounts = json_object_new_array();

        for (const Account &account: accounts)
        {
            json_object *jaccount = json_object_new_object();
            const std::string type(account.type == Account::Type::HOTP ? "HOTP" : "TOTP");
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
            json_object_object_add(jaccount, "name", json_object_new_string_len(account.name.data(), account.name.size()));
            json_object_object_add(jaccount, "description", json_object_new_string_len(account.description.data(), account.description.size()));
            json_object_object_add(jaccount, "type", json_object_new_string_len(type.data(), type.size()));
            json_object_object_add(jaccount, "digits", json_object_new_int(account.digits));
            json_object_object_add(jaccount, "algorithm", json_object_new_string_len(algorithm.data(), algorithm.size()));
            json_object_object_add(jaccount, "count", json_object_new_int(account.count));
            json_object_object_add(jaccount, "secret", json_object_new_string_len(account.secret.data(), account.secret.size()));
            json_object_array_add(jaccounts, jaccount);
        }
        const std::string json(json_object_to_json_string_ext(jaccounts, JSON_C_TO_STRING_PLAIN));
        file << json << '\n';
        json_object_put(jaccounts);
    }
}
