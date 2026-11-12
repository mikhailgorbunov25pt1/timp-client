#pragma once

#include <unistd.h>

#include <cstdio>
#include <fstream>
#include <iterator>
#include <string>

class TempFile {
public:
    explicit TempFile(const std::string& content = "")
        : path("/tmp/timp_test_" + std::to_string(getpid()) + "_" +
               std::to_string(counter++))
    {
        std::ofstream(path) << content;
    }
    ~TempFile()
    {
        std::remove(path.c_str());
    }
    TempFile(const TempFile&) = delete;
    TempFile& operator=(const TempFile&) = delete;

    std::string content() const
    {
        std::ifstream file(path);
        return {std::istreambuf_iterator<char>(file), {}};
    }

    const std::string path;

private:
    static inline int counter = 0;
};
