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
#include <cstring>
#include <vector>
#include <array>

#include "crypto.hxx"
#include "coding.hxx"

int main(int argc, char **argv)
{
    std::vector<char> input;

    if (argc > 1)
    {
        input.assign(argv[1], argv[1] + strlen(argv[1]));
    }

    std::vector<char> decoded(Base32Decode(input));
    std::cout.write(reinterpret_cast<char *>(decoded.data()), decoded.size());
    std::cout << std::endl;
    return 0;
}
