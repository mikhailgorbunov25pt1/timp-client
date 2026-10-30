#pragma once

#include "types.h"

#include <fstream>
#include <string>

class DataReader {
public:
    explicit DataReader(const std::string& path);
    Vectors read();

private:
    std::string path;
    std::ifstream file;

    unsigned long long readNumber(unsigned long long min,
                                  unsigned long long max,
                                  const std::string& what);
};
