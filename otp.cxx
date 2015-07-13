/*
    Copyright (C) 2015 Taylor C. Richberger

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    The entire text of the license is contained in LICENSE
*/
#include "otp.hxx"

#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cmath>

#include "cryptpp/crypto.hxx"

// Generate a HMAC-based OTP
std::string PWGen(std::function<void (std::ostream &, std::istream &)> hash, const size_t blockSize, const bool timeBased, const time_t count, const unsigned int digits, const std::string &secret, const time_t time)
{
    uint64_t message;

    if (timeBased)
    {
        message = time / count;
    } else
    {
        message = count;
    }

    std::stringstream value;

    for (unsigned int i = 0; i < 8; ++i)
    {
        value.put(static_cast<char>(message >> ((7 - i) * 8)));
    }

    std::ostringstream hashss;
    Hmac(hash, blockSize, hashss, secret, value);

    const std::string hashOut(hashss.str());

    uint8_t offset = hashOut.back() & 0x0F;

    uint32_t truncatedHash = 0;

    for (unsigned int i = 0; i < 4; ++i)
    {
        truncatedHash |= static_cast<unsigned char>(hashOut[i + offset]) << (24 - (i * 8));
    }
    truncatedHash &= 0x7FFFFFFF;

    std::ostringstream output;
    output << std::setfill('0') << std::setw(digits) << truncatedHash % static_cast<unsigned long>(pow(10, digits));
    return output.str();
}
