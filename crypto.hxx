#include <cstdint>
#include <array>
#include <vector>
#include <functional>

std::array<uint8_t, 20> Sha1Sum(const std::vector<char> &input);

template <size_t N, size_t B>
std::array<uint8_t, N> Hmac(std::function<std::array<uint8_t, N>(const std::vector<char> &)> hash, std::vector<char> key, const std::vector<char> &message);
