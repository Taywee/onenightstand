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

// Encodings
extern std::vector<char> Base32Decode(const std::vector<char> &input);

// Crypto
extern std::vector<uint8_t> Sha1Sum(const std::vector<uint8_t> &input);

extern std::vector<uint8_t> Hmac(std::function<std::vector<uint8_t>(const std::vector<uint8_t> &)> hash, const unsigned int blockSize, std::vector<uint8_t> key, const std::vector<uint8_t> &message);

// Block Size is necessary here
// Count is the interval if timeBased, else it is the count to the HOTP
std::string PWGen(std::function<std::vector<uint8_t>(const std::vector<uint8_t> &)> fun, size_t blockSize, bool timeBased, time_t count, const unsigned int digits, const std::vector<char> &secret, const time_t time);

std::list<Account> GetAccounts();
void SaveAccounts(const std::list<Account> &accounts);
