#include "data_reader.h"

#include "errors.h"

#include <cerrno>
#include <cstdlib>
#include <limits>
#include <utility>

DataReader::DataReader(const std::string& path)
    : path(path)
    , file(path)
{
    if (!file)
        throw IOError("DataReader::DataReader",
                      "cannot open input file '" + path + "'");
}

Vectors DataReader::read()
{
    const uint32_t count = readNumber(0, std::numeric_limits<uint32_t>::max(),
                                      "number of vectors");
    Vectors vectors;
    for (uint32_t i = 1; i <= count; ++i) {
        const std::string vectorName = "vector " + std::to_string(i);
        const uint32_t size = readNumber(
            1, std::numeric_limits<uint32_t>::max(), "size of " + vectorName);
        Vector vector;
        for (uint32_t j = 1; j <= size; ++j)
            vector.push_back(
                readNumber(0, std::numeric_limits<Value>::max(),
                           "value " + std::to_string(j) + " of " + vectorName));
        vectors.push_back(std::move(vector));
    }
    if (!(file >> std::ws).eof())
        throw IOError("DataReader::read",
                      "file '" + path + "': unexpected data after vector " +
                          std::to_string(count));
    return vectors;
}

unsigned long long DataReader::readNumber(unsigned long long min,
                                          unsigned long long max,
                                          const std::string& what)
{
    std::string token;
    if (!(file >> token))
        throw IOError("DataReader::read",
                      "file '" + path + "': " + what + " is missing");
    errno = 0;
    const unsigned long long number = std::strtoull(token.c_str(), nullptr, 10);
    const bool digitsOnly =
        token.find_first_not_of("0123456789") == std::string::npos;
    if (!digitsOnly || errno == ERANGE || number < min || number > max)
        throw IOError("DataReader::read",
                      "file '" + path + "': " + what + " '" + token +
                          "' is not an integer in range " +
                          std::to_string(min) + ".." + std::to_string(max));
    return number;
}
