#include "crypto.hxx"

#include <iterator>
#include <iostream>

inline uint32_t LRot32(const uint32_t number, uint8_t places)
{
    places = places % 32;
    if (places != 0)
    {
        return (number << places) | (number >> (32 - places));
    } else
    {
        return number;
    }
}

inline uint32_t Pack32(const uint8_t a, const uint8_t b, const uint8_t c, const uint8_t d)
{
    return (static_cast<uint32_t>(a) << 24)
            | (static_cast<uint32_t>(b) << 16)
            | (static_cast<uint32_t>(c) << 8)
            | static_cast<uint32_t>(d);
}

inline void Unpack32(const uint32_t input, uint8_t &a, uint8_t &b, uint8_t &c, uint8_t &d)
{
    a = static_cast<uint8_t>(input >> 24);
    b = static_cast<uint8_t>(input >> 16);
    c = static_cast<uint8_t>(input >> 8);
    d = static_cast<uint8_t>(input);
}

inline uint32_t Sha1F1(const uint32_t a, const uint32_t b, const uint32_t c)
{
    return (a & b) | ((~a) & c);
}

inline uint32_t Sha1F2(const uint32_t a, const uint32_t b, const uint32_t c)
{
    return a ^ b ^ c;
}

inline uint32_t Sha1F3(const uint32_t a, const uint32_t b, const uint32_t c)
{
    return (a & b) | (a & c) | (b & c);
}

std::array<uint8_t, 20> Sha1Sum(const std::vector<char> &input)
{
    std::vector<uint8_t> message(input.begin(), input.end());

    uint64_t size = message.size() * 8;

    message.push_back(0x80);

    // Make message congruent to 448 (mod 512) bits, by padding with 0s
    if ((message.size() - 56) % 64 != 0)
    {
        message.resize(message.size() + 64 - ((message.size() - 56) % 64));
    }


    // Attach input size to message (big-endian order), making message a multiple of 512 bits
    // This way avoids endienness issues
    for (int i = 7; i >= 0; --i)
    {
        message.push_back(static_cast<uint8_t>(size >> (i * 8)));
    }

    std::array<uint32_t, 5> h;
    h[0] = 0x67452301u;
    h[1] = 0xEFCDAB89u;
    h[2] = 0x98BADCFEu;
    h[3] = 0x10325476u;
    h[4] = 0xC3D2E1F0u;
    for (unsigned int index = 0; index < message.size(); index += 64)
    {
        std::array<uint8_t, 64> chunk;
        {
            auto begin = message.begin();
            std::advance(begin, index);
            auto end = begin;
            std::advance(end, 64);

            std::copy(begin, end, chunk.begin());
        }

        std::array<uint32_t, 80> words;

        // break the 512-bit chunk into 16 32-bit pieces
        for (unsigned int i = 0; i < 16; ++i)
        {
            unsigned int startChunk = 4 * i;

            words[i] = Pack32(chunk[startChunk], chunk[startChunk + 1], chunk[startChunk + 2], chunk[startChunk + 3]);
        }


        // Do some weird calculations to fill up to 80 pieces
        for (unsigned int i = 16; i < 80; ++i)
        {
            words[i] = words[i - 3] ^ words[i - 8] ^ words[i - 14] ^ words[i - 16];
            words[i] = LRot32(words[i], 1);
        }

        uint32_t a = h[0];
        uint32_t b = h[1];
        uint32_t c = h[2];
        uint32_t d = h[3];
        uint32_t e = h[4];
        uint32_t f;
        uint32_t k;

        for (unsigned int i = 0; i < 80; ++i)
        {
            if (i < 20)
            {
                f = Sha1F1(b, c, d);
                k = 0x5A827999u;
            } else if (i < 40)
            {
                f = Sha1F2(b, c, d);
                k = 0x6ED9EBA1u;
            } else if (i < 60)
            {
                f = Sha1F3(b, c, d);
                k = 0x8F1BBCDCu;
            } else
            {
                f = Sha1F2(b, c, d);
                k = 0xCA62C1D6u;
            }
            uint32_t temp = LRot32(a, 5) + f + e + k + words[i];
            e = d;
            d = c;
            c = LRot32(b, 30);
            b = a;
            a = temp;
        }
        h[0] += a;
        h[1] += b;
        h[2] += c;
        h[3] += d;
        h[4] += e;
    }

    std::array<uint8_t, 20> output;
    for (unsigned int i = 0; i < 5; ++i)
    {
        unsigned int start = i * 4;
        Unpack32(h[i], output[start], output[start + 1], output[start + 2], output[start + 3]);
    }
    return output;
}
