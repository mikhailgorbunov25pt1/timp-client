#pragma once

#include <stdexcept>
#include <string>

class ClientError : public std::runtime_error {
public:
    ClientError(const std::string& kind, const std::string& where,
                const std::string& what)
        : std::runtime_error(kind + " error in " + where + ": " + what)
    {
    }
};

class InterfaceError : public ClientError {
public:
    InterfaceError(const std::string& where, const std::string& what)
        : ClientError("Interface", where, what)
    {
    }
};

class IOError : public ClientError {
public:
    IOError(const std::string& where, const std::string& what)
        : ClientError("I/O", where, what)
    {
    }
};

class NetworkError : public ClientError {
public:
    NetworkError(const std::string& where, const std::string& what)
        : ClientError("Network", where, what)
    {
    }
};

class AuthError : public ClientError {
public:
    AuthError(const std::string& where, const std::string& what)
        : ClientError("Authentication", where, what)
    {
    }
};
