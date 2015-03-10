/*
    Copyright (c) 2015 Taylor C. Richberger

    The contents of this file are licensed MIT.
    The terms of this license are contained in LICENSE.MIT within this repository
*/
# pragma once

#include <vector>
#include <functional>

// All hash functions take and return a std::vector<uint8_t>, so they can be used as generic input to Hmac

extern std::vector<uint8_t> Sha1Sum(const std::vector<uint8_t> &input);

extern std::vector<uint8_t> Hmac(std::function<std::vector<uint8_t>(const std::vector<uint8_t> &)> hash, const unsigned int blockSize, std::vector<uint8_t> key, const std::vector<uint8_t> &message);
