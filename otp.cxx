#include "otp.hxx"

#include "crypto.hxx"

#include <iomanip>
#include <fstream>
#include <sstream>
#include <cmath>

std::string PWGen(std::function<std::vector<uint8_t>(const std::vector<uint8_t> &)> fun, size_t blockSize, bool timeBased, time_t count, const unsigned int digits, const std::vector<char> &secret, const time_t time)
{
    std::vector<uint8_t> key(secret.begin(), secret.end());

    uint64_t message;

    if (timeBased)
    {
        message = time / count;
    } else
    {
        message = count;
    }

    std::vector<uint8_t> value(8);

    for (unsigned int i = 8; i--; message >>= 8)
    {
        value[i] = static_cast<uint8_t>(message);
    }

    std::vector<uint8_t> hash(Hmac(fun, blockSize, key, value));

    uint8_t offset = hash.back() & 0x0F;

    uint32_t truncatedHash = 0;

    for (unsigned int i = 0; i < 4; ++i)
    {
        truncatedHash |= hash[i + offset] << (24 - (i * 8));
    }
    truncatedHash &= 0x7FFFFFFF;

    std::ostringstream output;
    output << std::setfill('0') << std::setw(digits) << truncatedHash % static_cast<unsigned long>(pow(10, digits));
    return output.str();
}

inline std::vector<std::string> Split(const std::string &s, const char delim)
{
    std::vector<std::string> elems;
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

static const std::string GetAccountFile()
{
    const char * const home = getenv("HOME");
    std::string file;
    if (home)
    {
        file.assign(home);
    } else
    {
        file.assign(".");
    }
    file.append("/.onenightstand");

    return file;
}

std::list<Account> GetAccounts()
{
    const std::string fileName(GetAccountFile());
    std::list<Account> accounts;

    std::ifstream file(fileName);
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
            accounts.emplace_back(fields[0], fields[1], fields[2], fields[3], fields[4], fields[5], fields[6]);
        }
    }
    return accounts;
}
void SaveAccounts(const std::list<Account> &accounts)
{
    const std::string fileName(GetAccountFile());

    std::ofstream file(fileName);
    if (file)
    {
        for (const Account &account: accounts)
        {
            file << account.name << ':'
                 << account.description << ':'
                 << (account.type == Account::Type::HOTP ? "HOTP" : "TOTP") << ':'
                 << account.digits << ':';
            switch (account.algorithm)
            {
                case Account::Algorithm::MD5:
                    {
                        file << "MD5";
                        break;
                    }

                case Account::Algorithm::SHA1:
                    {
                        file << "SHA1";
                        break;
                    }

                case Account::Algorithm::SHA256:
                    {
                        file << "SHA256";
                        break;
                    }

                case Account::Algorithm::SHA512:
                    {
                        file << "SHA512";
                        break;
                    }

                default:
                    {
                        file << "SHA1";
                        break;
                    }
            }
            file << ':'
                 << account.count << ':'
                 << account.secret << '\n';
        }
    }
}
