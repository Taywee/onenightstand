#include "otp.hxx"

#include "crypto.hxx"

#include <iomanip>
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
