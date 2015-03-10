/*
    Copyright (c) 2015 Taylor C. Richberger

    The contents of this file are licensed MIT.
    The terms of this license are contained in LICENSE.MIT within this repository
*/

#include <string>
#include <cstdint>
#include <functional>
#include <vector>

// Block Size is necessary here
// Count is the interval if timeBased, else it is the count to the HOTP
std::string PWGen(std::function<std::vector<uint8_t>(const std::vector<uint8_t> &)> fun, size_t blockSize, bool timeBased, time_t count, const unsigned int digits, const std::vector<char> &secret, const time_t time);

# pragma once
