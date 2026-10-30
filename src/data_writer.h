#pragma once

#include "types.h"

#include <fstream>
#include <string>

class DataWriter {
public:
    explicit DataWriter(const std::string& path);
    void write(const Results& results);

private:
    std::string path;
    std::ofstream file;
};
