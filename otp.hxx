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
#include <functional>
#include <iostream>
#include <ctime>

std::string PWGen(std::function<void (std::ostream &, std::istream &)> hash, const size_t blockSize, const bool timeBased, const time_t count, const unsigned int digits, const std::string &secret, const time_t time);
