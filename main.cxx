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
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <chrono>

#include "crypto.hxx"
#include "coding.hxx"
#include "account.hxx"
#include "otp.hxx"

inline std::vector<std::string> Split(const std::string &s, const char delim);

int main(int argc, char **argv)
{
    time_t time = std::chrono::duration_cast<std::chrono::duration<time_t, std::ratio<1, 1>>>(std::chrono::system_clock::now().time_since_epoch()).count();

    const char * const home = getenv("HOME");
    std::string dotFile;
    if (home)
    {
        dotFile.assign(home);
    } else
    {
        dotFile.assign(".");
    }
    dotFile.append("/.onenightstand");

    std::ifstream file(dotFile);
    if (file)
    {
        std::string line;
        while (std::getline(file, line))
        {
            // Remove comments
            size_t crunch = line.find('#');
            if (crunch < line.size())
            {
                line.erase(crunch);
            }
            // Remove trailing whitespace (since lines may contain comments)
            size_t whitespace = line.find_last_not_of(" \t\f\v\n\r");
            if ((whitespace + 1) < line.size())
            {
                line.erase(whitespace + 1);
            } else if (whitespace == line.npos)
            {
                continue;
            }

            std::vector<std::string> fields(Split(line, ':'));
            const Account account(fields[0], fields[1], fields[2], fields[3], fields[4], fields[5], fields[6]);

            std::cout << account.name << ":  " << account.description << '\n';
            std::cout << std::setw(4) << account.interval - (time % account.interval) << ": ";
            std::cout << PWGen(Sha1Sum, 64, account.type == Account::Type::TOTP, account.interval, account.digits, Base32Decode(std::vector<char>(account.secret.begin(), account.secret.end())), time);
            std::cout << std::endl;
        }
    }

    return 0;
}

std::vector<std::string> Split(const std::string &s, const char delim)
{
    std::vector<std::string> elems;
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}
