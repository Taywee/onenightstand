/*
    Copyright (c) 2015 Taylor C. Richberger

    The contents of this file are licensed GPLv3.
    The terms of this license are contained in LICENSE.GPLv3 within this repository
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
        MD5,
        SHA1,
        SHA256,
        SHA512
    } algorithm;
    unsigned long count;
    std::string secret;

    Account(const std::string &name, const std::string &description, const Type type, const unsigned short digits, const Algorithm algorithm, const unsigned int count, const std::string &secret);
    Account(const std::string &name, const std::string &description, const std::string &type, const std::string &digits, const std::string &algorithm, const std::string &count, const std::string &secret);
};

std::list<Account> GetAccounts();
void SaveAccounts(const std::list<Account> &accounts);
