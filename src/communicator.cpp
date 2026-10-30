#include "communicator.h"

#include "errors.h"

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#include <cryptopp/filters.h>
#include <cryptopp/hex.h>
#include <cryptopp/osrng.h>
#include <cryptopp/sha.h>

#include <cerrno>
#include <cstring>
#include <iomanip>
#include <sstream>

namespace {

const timeval receiveTimeout{30, 0};

std::string systemError()
{
    return std::strerror(errno);
}

}

Communicator::Communicator(const std::string& address, uint16_t port)
    : server{}
{
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    if (inet_pton(AF_INET, address.c_str(), &server.sin_addr) != 1)
        throw NetworkError("Communicator::Communicator",
                           "invalid IPv4 address '" + address + "'");
    socketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFd < 0)
        throw NetworkError("Communicator::Communicator",
                           "cannot create socket: " + systemError());
    if (setsockopt(socketFd, SOL_SOCKET, SO_RCVTIMEO, &receiveTimeout,
                   sizeof receiveTimeout) != 0) {
        const std::string reason = systemError();
        close(socketFd);
        throw NetworkError("Communicator::Communicator",
                           "cannot set receive timeout: " + reason);
    }
}

Communicator::~Communicator()
{
    close(socketFd);
}

Results Communicator::exchange(const Credentials& credentials,
                               const Vectors& vectors)
{
    connect();
    authenticate(credentials);
    return process(vectors);
}

std::string Communicator::makeSalt()
{
    uint64_t salt;
    CryptoPP::AutoSeededRandomPool().GenerateBlock(
        reinterpret_cast<CryptoPP::byte*>(&salt), sizeof salt);
    std::ostringstream hex;
    hex << std::uppercase << std::hex << std::setw(16) << std::setfill('0')
        << salt;
    return hex.str();
}

std::string Communicator::makeHash(const std::string& salt,
                                   const std::string& password)
{
    CryptoPP::SHA224 sha224;
    std::string digest;
    CryptoPP::StringSource(
        salt + password, true,
        new CryptoPP::HashFilter(
            sha224,
            new CryptoPP::HexEncoder(new CryptoPP::StringSink(digest))));
    return digest;
}

void Communicator::connect()
{
    if (::connect(socketFd, reinterpret_cast<const sockaddr*>(&server),
                  sizeof server) != 0)
        throw NetworkError("Communicator::connect",
                           "cannot connect to " + serverName() + ": " +
                               systemError());
}

void Communicator::authenticate(const Credentials& credentials)
{
    const std::string salt = makeSalt();
    const std::string message =
        credentials.login + salt + makeHash(salt, credentials.password);
    sendBytes(message.data(), message.size(), "Communicator::authenticate");
    const std::string answer = receiveText("Communicator::authenticate");
    if (answer != "OK")
        throw AuthError("Communicator::authenticate",
                        "server " + serverName() + " rejected login '" +
                            credentials.login + "' with '" + answer + "'");
}

Results Communicator::process(const Vectors& vectors)
{
    const uint32_t count = vectors.size();
    sendBytes(&count, sizeof count, "Communicator::process");
    Results results;
    for (const Vector& vector : vectors) {
        const uint32_t size = vector.size();
        sendBytes(&size, sizeof size, "Communicator::process");
        sendBytes(vector.data(), size * sizeof(Value), "Communicator::process");
        Value result;
        receiveBytes(&result, sizeof result, "Communicator::process");
        results.push_back(result);
    }
    return results;
}

void Communicator::sendBytes(const void* data, std::size_t size,
                             const char* where)
{
    if (send(socketFd, data, size, MSG_NOSIGNAL) != static_cast<ssize_t>(size))
        throw NetworkError(where,
                           "cannot send " + std::to_string(size) +
                               " bytes to " + serverName() + ": " +
                               systemError());
}

void Communicator::receiveBytes(void* data, std::size_t size, const char* where)
{
    const ssize_t received = recv(socketFd, data, size, MSG_WAITALL);
    if (received < 0)
        throw NetworkError(where,
                           "cannot receive data from " + serverName() + ": " +
                               systemError());
    if (received != static_cast<ssize_t>(size))
        throw NetworkError(where,
                           "expected " + std::to_string(size) + " bytes from " +
                               serverName() + ", received " +
                               std::to_string(received));
}

std::string Communicator::receiveText(const char* where)
{
    char buffer[64];
    const ssize_t received = recv(socketFd, buffer, sizeof buffer, 0);
    if (received < 0)
        throw NetworkError(where,
                           "cannot receive answer from " + serverName() + ": " +
                               systemError());
    if (received == 0)
        throw NetworkError(where,
                           "server " + serverName() + " closed the connection");
    return std::string(buffer, received);
}

std::string Communicator::serverName() const
{
    char address[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &server.sin_addr, address, sizeof address);
    return std::string(address) + ":" + std::to_string(ntohs(server.sin_port));
}
