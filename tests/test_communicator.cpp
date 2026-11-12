#include "communicator.h"
#include "errors.h"

#include <UnitTest++/UnitTest++.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <algorithm>
#include <string>
#include <thread>

namespace {

const Credentials valid{"user", "P@ssW0rd"};

int listenOnFreePort()
{
    const int fd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    bind(fd, reinterpret_cast<sockaddr*>(&address), sizeof address);
    listen(fd, 1);
    return fd;
}

uint16_t portOf(int fd)
{
    sockaddr_in address{};
    socklen_t size = sizeof address;
    getsockname(fd, reinterpret_cast<sockaddr*>(&address), &size);
    return ntohs(address.sin_port);
}

void sumVectors(int client)
{
    uint32_t count = 0;
    recv(client, &count, sizeof count, MSG_WAITALL);
    for (uint32_t i = 0; i < count; ++i) {
        uint32_t size = 0;
        recv(client, &size, sizeof size, MSG_WAITALL);
        Vector vector(size);
        recv(client, vector.data(), size * sizeof(Value), MSG_WAITALL);
        uint32_t sum = 0;
        for (const Value value : vector)
            sum += value;
        const Value result = std::min<uint32_t>(sum, 65535);
        send(client, &result, sizeof result, MSG_NOSIGNAL);
    }
}

enum class Behavior { Serve, HangUp };

class FakeServer {
public:
    explicit FakeServer(Behavior behavior)
        : listener(listenOnFreePort())
        , port(portOf(listener))
        , worker([this, behavior] { serve(behavior); })
    {
    }
    ~FakeServer()
    {
        shutdown(listener, SHUT_RDWR);
        worker.join();
        close(listener);
    }

    const int listener;
    const uint16_t port;

private:
    std::thread worker;

    void serve(Behavior behavior)
    {
        const int client = accept(listener, nullptr, nullptr);
        if (client < 0)
            return;
        char buffer[128];
        const ssize_t received = recv(client, buffer, sizeof buffer, 0);
        if (behavior == Behavior::Serve && received > 72) {
            const std::string message(buffer, received);
            const std::string login = message.substr(0, received - 72);
            const std::string salt = message.substr(login.size(), 16);
            const std::string hash = message.substr(login.size() + 16);
            const bool accepted =
                login == valid.login &&
                hash == Communicator::makeHash(salt, valid.password);
            const std::string answer = accepted ? "OK" : "ERR";
            send(client, answer.data(), answer.size(), MSG_NOSIGNAL);
            if (accepted)
                sumVectors(client);
        }
        close(client);
    }
};

Results exchangeWith(const FakeServer& server, const Credentials& credentials,
                     const Vectors& vectors)
{
    return Communicator("127.0.0.1", server.port)
        .exchange(credentials, vectors);
}

}

SUITE(Communicator)
{
    TEST(SaltIsSixteenHexDigits)
    {
        const std::string salt = Communicator::makeSalt();
        CHECK_EQUAL(16u, salt.size());
        CHECK(salt.find_first_not_of("0123456789ABCDEF") == std::string::npos);
    }

    TEST(SaltIsNewEveryTime)
    {
        CHECK(Communicator::makeSalt() != Communicator::makeSalt());
    }

    TEST(HashIsSha224OfSaltAndPassword)
    {
        CHECK_EQUAL("23097D223405D8228642A477BDA255B32AADBCE4BDA0B3F7E36C9DA7",
                    Communicator::makeHash("ab", "c"));
    }

    TEST(HashOfEmptyString)
    {
        CHECK_EQUAL("D14A028C2A3A2BC9476102BB288234C415A2B01F828EA62AC5B3E42F",
                    Communicator::makeHash("", ""));
    }

    TEST(InvalidAddress)
    {
        CHECK_THROW(Communicator("localhost", 33333), NetworkError);
    }

    TEST(ReturnsResultForEachVector)
    {
        FakeServer server(Behavior::Serve);
        const Results expected{3, 65535, 7};
        CHECK(exchangeWith(server, valid, {{1, 2}, {65535, 1}, {7}}) ==
              expected);
    }

    TEST(NoVectorsNoResults)
    {
        FakeServer server(Behavior::Serve);
        CHECK(exchangeWith(server, valid, {}).empty());
    }

    TEST(WrongPassword)
    {
        FakeServer server(Behavior::Serve);
        CHECK_THROW(exchangeWith(server, {"user", "wrong"}, {{1}}), AuthError);
    }

    TEST(UnknownLogin)
    {
        FakeServer server(Behavior::Serve);
        CHECK_THROW(exchangeWith(server, {"guest", valid.password}, {{1}}),
                    AuthError);
    }

    TEST(ServerClosesConnection)
    {
        FakeServer server(Behavior::HangUp);
        CHECK_THROW(exchangeWith(server, valid, {{1}}), NetworkError);
    }

    TEST(ServerNotListening)
    {
        const int fd = listenOnFreePort();
        const uint16_t port = portOf(fd);
        close(fd);
        CHECK_THROW(Communicator("127.0.0.1", port).exchange(valid, {{1}}),
                    NetworkError);
    }
}
