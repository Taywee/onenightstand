/*
    Copyright (c) 2015 Taylor C. Richberger

    The contents of this file are licensed MIT.
    The terms of this license are contained in LICENSE.MIT within this repository
*/
# pragma once

#include <string>

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
    unsigned int interval;
    std::string secret;

    Account(const std::string &name, const std::string &description, const Type type, const unsigned short digits, const Algorithm algorithm, const unsigned int interval, const std::string &secret);
    Account(const std::string &name, const std::string &description, const std::string &type, const std::string &digits, const std::string &algorithm, const std::string &interval, const std::string &secret);
};
