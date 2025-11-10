#pragma once

#include <cstdint>
#include <cstddef>
#include <string>

class Graph;

namespace socketcpp
{
class RouterServer
{
public:
    RouterServer(Graph &graph, uint16_t port);

    void run();

private:
    bool sendAll(int fd, const std::string &message) const;
    bool receiveLine(int fd, std::string &line) const;
    void handleClient(int clientFd);

    Graph &graph_;
    uint16_t port_;
    size_t pathCounter_;
};
}
