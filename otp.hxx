/*
    Copyright (c) 2015 Taylor C. Richberger

    The contents of this file are licensed GPLv3.
    The terms of this license are contained in LICENSE.GPLv3 within this repository
*/
#include <string>
#include <functional>
#include <iostream>
#include <ctime>

std::string PWGen(std::function<void (std::ostream &, std::istream &)> hash, const size_t blockSize, const bool timeBased, const time_t count, const unsigned int digits, const std::string &secret, const time_t time);
