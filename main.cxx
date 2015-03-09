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
#include <sstream>
#include <iomanip>
#include <cstring>
#include <vector>
#include <array>
#include <chrono>
#include <cstdint>

#include "crypto.hxx"
#include "coding.hxx"

std::string GoogleAuthenticator(const std::string &secret);

int main(int argc, char **argv)
{
    std::string input;

    if (argc > 1)
    {
        input.assign(argv[1], argv[1] + strlen(argv[1]));
    }

    std::cout << GoogleAuthenticator(input) << std::endl;

    return 0;
}

std::string GoogleAuthenticator(const std::string &secret)
{
    std::vector<char> input(secret.begin(), secret.end());

    std::vector<char> key(Base32Decode(input));

    uint64_t message = std::chrono::duration_cast<std::chrono::duration<uint64_t, std::ratio<1, 1>>>(std::chrono::system_clock::now().time_since_epoch()).count() / 30;
    std::vector<char> value(8);

    for (unsigned int i = 8; i--; message >>= 8)
    {
        value[i] = static_cast<uint8_t>(message);
    }

    std::array<uint8_t, 20> hash(Hmac<20, 64>(Sha1Sum, key, value));

    uint8_t offset = hash.back() & 0x0F;

    uint32_t truncatedHash = 0;

    for (unsigned int i = 0; i < 4; ++i)
    {
        truncatedHash |= hash[i + offset] << (24 - (i * 8));
    }
    truncatedHash &= 0x7FFFFFFF;

    std::ostringstream output;
    output << std::setfill('0') << std::setw(6) << truncatedHash % 1000000;
    return output.str();
}
