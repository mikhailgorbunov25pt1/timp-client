#include "user_interface.h"

#include "communicator.h"
#include "credentials.h"
#include "data_reader.h"
#include "data_writer.h"
#include "errors.h"

#include <cstdlib>
#include <iostream>
#include <sstream>

namespace po = boost::program_options;

namespace {

std::string expandHome(const std::string& path)
{
    if (path.compare(0, 2, "~/") != 0)
        return path;
    const char* home = std::getenv("HOME");
    if (home == nullptr)
        throw InterfaceError("UserInterface::parse",
                             "HOME is not set, cannot expand '" + path + "'");
    return home + path.substr(1);
}

}

UserInterface::UserInterface()
    : description("Allowed options")
{
    description.add_options()("help,h", "show this help")(
        "address,a", po::value<std::string>(&parsed.address)->required(),
        "server IPv4 address")(
        "port,p", po::value<uint16_t>(&parsed.port)->default_value(33333),
        "server port")("input,i",
                       po::value<std::string>(&parsed.inputFile)->required(),
                       "input data file")(
        "output,o", po::value<std::string>(&parsed.outputFile)->required(),
        "output results file")("config,c",
                               po::value<std::string>(&parsed.configFile)
                                   ->default_value("~/.config/vclient.conf"),
                               "file with LOGIN:PASSWORD");
}

bool UserInterface::parse(int argc, const char* const* argv)
{
    po::variables_map options;
    try {
        po::store(po::command_line_parser(argc, argv)
                      .options(description)
                      .positional({})
                      .run(),
                  options);
        if (argc == 1 || options.count("help"))
            return false;
        po::notify(options);
    } catch (const po::error& e) {
        throw InterfaceError("UserInterface::parse", e.what());
    }
    parsed.configFile = expandHome(parsed.configFile);
    return true;
}

const Params& UserInterface::params() const
{
    return parsed;
}

std::string UserInterface::help() const
{
    std::ostringstream out;
    out << "Usage: client -a ADDRESS -i INPUT -o OUTPUT [-p PORT] [-c CONFIG]\n"
        << description;
    return out.str();
}

void UserInterface::run(int argc, const char* const* argv)
{
    if (!parse(argc, argv)) {
        std::cout << help();
        return;
    }
    const Vectors vectors = DataReader(parsed.inputFile).read();
    const Credentials credentials = loadCredentials(parsed.configFile);
    const Results results = Communicator(parsed.address, parsed.port)
                                .exchange(credentials, vectors);
    DataWriter(parsed.outputFile).write(results);
}
