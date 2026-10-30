#include "credentials.h"

#include "errors.h"

#include <fstream>

Credentials loadCredentials(const std::string& path)
{
    std::ifstream file(path);
    if (!file)
        throw IOError("loadCredentials",
                      "cannot open credentials file '" + path + "'");
    std::string line;
    std::getline(file, line);
    const std::size_t separator = line.find(':');
    if (separator == std::string::npos || separator == 0 ||
        separator + 1 == line.size())
        throw IOError("loadCredentials",
                      "file '" + path + "' must contain a line LOGIN:PASSWORD");
    return {line.substr(0, separator), line.substr(separator + 1)};
}
