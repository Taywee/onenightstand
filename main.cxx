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
#include <iomanip>
#include <vector>
#include <chrono>
#include <list>

#include "crypto.hxx"
#include "coding.hxx"
#include "account.hxx"
#include "otp.hxx"

int main(int argc, char **argv)
{
    time_t time = std::chrono::duration_cast<std::chrono::duration<time_t, std::ratio<1, 1>>>(std::chrono::system_clock::now().time_since_epoch()).count();

    std::list<Account> accounts(GetAccounts());
    for (Account &account: accounts)
    {
        std::cout << account.name << ":  " << account.description << '\n';

        if (account.type == Account::Type::TOTP)
        {
            std::cout << std::setw(4) << account.count - (time % account.count) << ": ";
        } else
        {
            std::cout << std::setw(4) << account.count << ": ";
        }

        std::cout << PWGen(Sha1Sum, 64, account.type == Account::Type::TOTP, account.count, account.digits, Base32Decode(std::vector<char>(account.secret.begin(), account.secret.end())), time);
        std::cout << std::endl;
        if (account.type == Account::Type::HOTP)
        {
            ++account.count;
        }
    }
    SaveAccounts(accounts);

    return 0;
}

