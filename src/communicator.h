#pragma once

#include "credentials.h"
#include "types.h"

#include <netinet/in.h>

#include <cstddef>
#include <cstdint>
#include <string>

class Communicator {
public:
    Communicator(const std::string& address, uint16_t port);
    ~Communicator();
    Communicator(const Communicator&) = delete;
    Communicator& operator=(const Communicator&) = delete;

    Results exchange(const Credentials& credentials, const Vectors& vectors);

    static std::string makeSalt();
    static std::string makeHash(const std::string& salt,
                                const std::string& password);

private:
    sockaddr_in server;
    int socketFd;

    void connect();
    void authenticate(const Credentials& credentials);
    Results process(const Vectors& vectors);
    void sendBytes(const void* data, std::size_t size, const char* where);
    void receiveBytes(void* data, std::size_t size, const char* where);
    std::string receiveText(const char* where);
    std::string serverName() const;
};
