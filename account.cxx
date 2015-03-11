#include "account.hxx"

#include <cctype>
#include <sstream>

Account::Account(const std::string &name, const std::string &description, const Type type, const unsigned short digits, const Algorithm algorithm, const unsigned int count, const std::string &secret) : name(name), description(description), type(type), digits(digits), algorithm(algorithm), count(count), secret(secret)
{
}

Account::Account(const std::string &name, const std::string &description, const std::string &type, const std::string &digits, const std::string &algorithm, const std::string &count, const std::string &secret) : name(name), description(description), secret(secret)
{
    if (toupper(type.front()) == 'T')
    {
        this->type = Type::TOTP;
    } else
    {
        this->type = Type::HOTP;
    }
    {
        std::stringstream conv;
        conv << digits;
        conv >> this->digits;
    }

    if (algorithm.substr(0, 3) == "sha" || algorithm.substr(0, 3) == "SHA")
    {
        size_t numberIndex = algorithm.find_first_of("0123456789");
        if (numberIndex != algorithm.npos)
        {
            std::string numbers(algorithm.substr(numberIndex));
            if (numbers == "256")
            {
                this->algorithm = Algorithm::SHA256;
            } else if (numbers == "512")
            {
                this->algorithm = Algorithm::SHA512;
            } else
            {
                this->algorithm = Algorithm::SHA1;
            }
        } else
        {
            this->algorithm = Algorithm::SHA1;
        }
    } else
    {
        this->algorithm = Algorithm::MD5;
    }
    {
        std::stringstream conv;
        conv << count;
        conv >> this->count;
    }
}
