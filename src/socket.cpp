#include "socket.hpp"

#include "graph.hpp"
#include "library.hpp"

#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <netinet/in.h>
#include <sstream>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

namespace socketcpp
{

RouterServer::RouterServer(Graph &graph, uint16_t port)
    : graph_(graph), port_(port), pathCounter_(0)
{
}

bool RouterServer::sendAll(int fd, const std::string &message) const
{
    size_t totalSent = 0;
    while (totalSent < message.size())
    {
        const ssize_t sent = ::send(fd, message.data() + totalSent, message.size() - totalSent, 0);
        if (sent <= 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
            return false;
        }
        totalSent += static_cast<size_t>(sent);
    }
    return true;
}

bool RouterServer::receiveLine(int fd, std::string &line) const
{
    line.clear();
    char ch = 0;
    while (true)
    {
        const ssize_t received = ::recv(fd, &ch, 1, 0);
        if (received == 0)
        {
            return !line.empty();
        }
        if (received < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
            return false;
        }
        if (ch == '\r')
        {
            continue;
        }
        if (ch == '\n')
        {
            break;
        }
        line.push_back(ch);
        if (line.size() > 4096)
        {
            break;
        }
    }
    return true;
}

void RouterServer::handleClient(int clientFd)
{
    auto &nodelist = graph_.getNodes();
    sendAll(clientFd, "Verbunden mit Router-Server. "
                      "Geben Sie \"<startId> <zielId>\", leer fuer Zufall oder \"exit\" ein.\n");

    std::string line;
    while (true)
    {
        if (!sendAll(clientFd, "Bitte Start- und Zielknoten-ID eingeben: "))
        {
            break;
        }

        if (!receiveLine(clientFd, line))
        {
            break;
        }

        std::istringstream iss(line);
        uint64_t startId = 0;
        uint64_t goalId = 0;

        if (line.empty())
        {
            auto it = nodelist.begin();
            std::advance(it, rand() % nodelist.size());
            startId = it->first;

            it = nodelist.begin();
            std::advance(it, rand() % nodelist.size());
            goalId = it->first;

            std::string message = "Zufaellige IDs ausgewaehlt: " + std::to_string(startId) + " -> " + std::to_string(goalId) + "\n";
            sendAll(clientFd, message);
        }
        else
        {
            if (line == "exit" || line == "quit")
            {
                sendAll(clientFd, "Verbindung wird beendet.\n");
                break;
            }

            if (!(iss >> startId >> goalId))
            {
                sendAll(clientFd, "Ungueltige Eingabe. Bitte zwei numerische IDs senden.\n");
                continue;
            }

            if (nodelist.find(startId) == nodelist.end())
            {
                sendAll(clientFd, "Startknoten nicht gefunden.\n");
                continue;
            }

            if (nodelist.find(goalId) == nodelist.end())
            {
                sendAll(clientFd, "Zielknoten nicht gefunden.\n");
                continue;
            }
        }

        auto path = graph_.aStar(startId, goalId);
        if (path.empty())
        {
            sendAll(clientFd, "Kein Pfad gefunden.\n");
            continue;
        }

        const std::string fileName = "astar_path_" + std::to_string(pathCounter_++) + ".geojson";
        HelperFunctions::exportPathToGeoJSON(path, fileName);

        std::ostringstream response;
        response << "Pfad von " << startId << " nach " << goalId << " mit " << path.size()
                 << " Punkten exportiert nach " << fileName << "\n";
        sendAll(clientFd, response.str());
    }
}

void RouterServer::run()
{
    int serverFd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (serverFd < 0)
    {
        throw std::runtime_error("Kann Socket nicht erzeugen: " + std::string(std::strerror(errno)));
    }

    int opt = 1;
    if (setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        ::close(serverFd);
        throw std::runtime_error("Kann Socketoption nicht setzen: " + std::string(std::strerror(errno)));
    }

    sockaddr_in address {};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port_);

    if (bind(serverFd, reinterpret_cast<sockaddr *>(&address), sizeof(address)) < 0)
    {
        ::close(serverFd);
        throw std::runtime_error("Kann nicht an Port binden: " + std::string(std::strerror(errno)));
    }

    if (listen(serverFd, 3) < 0)
    {
        ::close(serverFd);
        throw std::runtime_error("Kann nicht auf Verbindungen warten: " + std::string(std::strerror(errno)));
    }

    std::cout << "Router-Server lauscht auf Port " << port_ << ".\n";

    while (true)
    {
        sockaddr_in clientAddr {};
        socklen_t clientLen = sizeof(clientAddr);
        int clientFd = accept(serverFd, reinterpret_cast<sockaddr *>(&clientAddr), &clientLen);
        if (clientFd < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
            std::cerr << "Fehler bei accept: " << std::strerror(errno) << "\n";
            continue;
        }

        std::cout << "Client verbunden: " << inet_ntoa(clientAddr.sin_addr) << ":" << ntohs(clientAddr.sin_port) << "\n";
        handleClient(clientFd);
        ::close(clientFd);
        std::cout << "Client getrennt.\n";
    }

    ::close(serverFd);
}

}
