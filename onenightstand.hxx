/*
    Copyright (c) 2015 Taylor C. Richberger

    The contents of this file are licensed MIT.
    The terms of this license are contained in LICENSE.MIT within this repository
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
