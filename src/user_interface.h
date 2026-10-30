#pragma once

#include <boost/program_options.hpp>
#include <cstdint>
#include <string>

struct Params {
    std::string address;
    uint16_t port;
    std::string inputFile;
    std::string outputFile;
    std::string configFile;
};

class UserInterface {
public:
    UserInterface();
    bool parse(int argc, const char* const* argv);
    const Params& params() const;
    std::string help() const;
    void run(int argc, const char* const* argv);

private:
    boost::program_options::options_description description;
    Params parsed;
};
