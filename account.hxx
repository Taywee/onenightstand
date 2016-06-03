/*
    Copyright (C) 2015 Taylor C. Richberger

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    The entire text of the license is contained in LICENSE
*/
#pragma once

#include <string>
#include <vector>
#include <functional>
#include <cstdint>
#include <list>

// Account stuff

struct Account
{
    std::string name;
    std::string description;
    enum class Type
    {
        HOTP,
        TOTP
    } type;

    unsigned short digits;
    enum class Algorithm
    {
        SHA1,
        SHA256,
        SHA512,
        MD5
    } algorithm;
    unsigned long count;
    std::string secret;

    Account(const std::string &name, const std::string &description, const Type type, const unsigned short digits, const Algorithm algorithm, const unsigned int count, const std::string &secret);
    Account(const std::string &name, const std::string &description, const std::string &type, const std::string &digits, const std::string &algorithm, const std::string &count, const std::string &secret);
};

std::list<Account> GetAccounts();
void SaveAccounts(const std::list<Account> &accounts);
