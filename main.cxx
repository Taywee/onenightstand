#include <iostream>
#include <cstring>
#include <vector>
#include <array>

#include "crypto.hxx"

int main(int argc, char **argv)
{
    std::vector<char> message;
    if (argc > 1)
    {
        message.assign(argv[1], argv[1] + strlen(argv[1]));
    }

    std::array<uint8_t, 20> hash(Sha1Sum(message));
    std::cout.write(reinterpret_cast<char *>(hash.data()), hash.size());
    return 0;
}
