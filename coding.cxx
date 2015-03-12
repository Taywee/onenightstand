#include "onenightstand.hxx"

#include <algorithm>
#include <iterator>
#include <array>
#include <iostream>
#include <stdexcept>

inline unsigned char Base32Reassign(char i)
{
    if (i == '0')
    {
        return static_cast<unsigned char>('O');
    } else if (i == '1')
    {
        return static_cast<unsigned char>('I');
    } else if (i == '8')
    {
        return static_cast<unsigned char>('B');
    } else if (i >= 'a' && i <= 'z')
    {
        return static_cast<unsigned char>(i - 32);
    } else
    {
        return static_cast<unsigned char>(i);
    }
}

inline unsigned char Base32Char(unsigned char i)
{
    if (i >= 'A' && i <= 'Z')
    {
        return static_cast<unsigned char>(i - 'A');
    } else if (i >= '2' && i <= '7')
    {
        return static_cast<unsigned char>(i - ('2' - 26));
    } else if (i == '=')
    {
        return static_cast<unsigned char>(0);
    } else
    {
        std::string exception("Character '");
        exception.append(1, static_cast<char>(i));
        exception.append("' is not a valid Base32 character.");
        throw std::invalid_argument(exception);
    }
}

std::vector<char> Base32Decode(const std::vector<char> &input)
{
    std::vector<unsigned char> message(input.size());
    std::transform(input.begin(), input.end(), message.begin(), Base32Reassign);
    auto newEnd = std::remove_if(message.begin(), message.end(), [](unsigned char c) -> bool {
            if (c == '\n' || c == '\t' || c == '\r' || c == ' ') return true;
            else return false;
            });
    message.erase(newEnd, message.end());

    if (message.size() % 8 != 0)
    {
        message.resize(message.size() + 8 - (message.size() % 8), '=');
    }

    std::vector<char> output;

    /* From RFC 4648:
     *       01234567 89012345 67890123 45678901 23456789
     *      +--------+--------+--------+--------+--------+
     *      |< 1 >< 2| >< 3 ><|.4 >< 5.|>< 6 ><.|7 >< 8 >|
     *      +--------+--------+--------+--------+--------+
     *                                              <===> 8th character
     *                                        <====> 7th character
     *                                   <===> 6th character
     *                             <====> 5th character
     *                       <====> 4th character
     *                  <===> 3rd character
     *            <====> 2nd character
     *       <===> 1st character
     */
    for (unsigned int index = 0; index < message.size(); index += 8)
    {
        std::array<unsigned char, 8> chunk;
        std::array<unsigned char, 5> outChunk;

        {
            auto begin = message.begin();
            std::advance(begin, index);
            auto end = begin;
            std::advance(end, 8);

            std::copy(begin, end, chunk.begin());
        }

        unsigned char piece = Base32Char(chunk[0]);
        outChunk[0] = piece << 3;

        piece = Base32Char(chunk[1]);
        outChunk[0] |= piece >> 2;
        outChunk[1] = piece << 6;

        piece = Base32Char(chunk[2]);
        outChunk[1] |= piece << 1;

        piece = Base32Char(chunk[3]);
        outChunk[1] |= piece >> 4;
        outChunk[2] = piece << 4;

        piece = Base32Char(chunk[4]);
        outChunk[2] |= piece >> 1;
        outChunk[3] = piece << 7;

        piece = Base32Char(chunk[5]);
        outChunk[3] |= piece << 2;

        piece = Base32Char(chunk[6]);
        outChunk[3] |= piece >> 3;
        outChunk[4] = piece << 5;

        piece = Base32Char(chunk[7]);
        outChunk[4] |= piece;

        unsigned char pads = 0;
        for (unsigned char piece: chunk)
        {
            if (piece == '=')
            {
                ++pads;
            }
        }

        auto end = outChunk.begin();

        switch (pads)
        {
            case 0:
                {
                    std::advance(end, 5);
                    break;
                }
            case 1:
                {
                    std::advance(end, 4);
                    break;
                }

            case 3:
                {
                    std::advance(end, 3);
                    break;
                }

            case 4:
                {
                    std::advance(end, 2);
                    break;
                }

            case 6:
                {
                    std::advance(end, 1);
                    break;
                }

            default:
                {
                    break;
                }
        }
        std::copy(outChunk.begin(), outChunk.end(), std::back_inserter(output));
    }

    return output;
}
