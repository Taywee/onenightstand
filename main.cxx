#include <iostream>
#include <cstring>
#include <vector>
#include <array>

#include "crypto.hxx"

int main(int argc, char **argv)
{
    std::vector<char> key;
    std::vector<char> message;
    if (argc > 2)
    {
        key.assign(argv[1], argv[1] + strlen(argv[1]));
        message.assign(argv[2], argv[2] + strlen(argv[2]));
    }

    std::array<uint8_t, 20> hash(Hmac<20, 64>(Sha1Sum, key, message));
    std::cout.write(reinterpret_cast<char *>(hash.data()), hash.size());
    return 0;
}
