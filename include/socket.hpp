#pragma once

#include <cstdint>
#include <cstddef>
#include <string>

class Router;

namespace socketcpp
{
class RouterServer
{
public:
    RouterServer(Router &router, uint16_t port);

    void run();

private:
    bool sendAll(int fd, const std::string &message) const;
    bool receiveLine(int fd, std::string &line) const;
    void handleClient(int clientFd);

    Router &mRouter;
    uint16_t mPort;
    size_t mPathCounter;
};
}
