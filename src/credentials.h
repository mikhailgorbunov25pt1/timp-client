#pragma once

#include <string>

struct Credentials {
    std::string login;
    std::string password;
};

Credentials loadCredentials(const std::string& path);
