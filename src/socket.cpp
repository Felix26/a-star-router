#include "socket.hpp"

#include "graph.hpp"
#include "library.hpp"

#ifdef _WIN32
  #include <winsock2.h>
  #include <ws2tcpip.h>
#else
  #include <arpa/inet.h>
  #include <netinet/in.h>
  #include <sys/socket.h>
#endif

#include <cerrno>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <string>
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
                      "Geben Sie \"<startId> <zielId>\", \"-c <startLat>,<startLon> <zielLat>,<zielLon>\", "
                      "leer fuer Zufall oder \"exit\" ein.\n");

    std::string line;
    while (true)
    {
        if (!sendAll(clientFd, "Bitte '-i <startId> <zielId>' oder '-c <lat>,<lon> <lat>,<lon>' eingeben (Enter fuer Zufall): "))
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
        double startLat = 0.0;
        double startLon = 0.0;
        double goalLat = 0.0;
        double goalLon = 0.0;
        bool useCoordinates = false;

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

            if (!line.empty() && line[0] == '-')
            {
                std::string flag;
                iss >> flag;
                if (flag == "-c" || flag == "--coords" || flag == "--coordinates")
                {
                    useCoordinates = true;
                }
                else if (flag == "-i" || flag == "--ids")
                {
                    useCoordinates = false;
                }
                else
                {
                    sendAll(clientFd, "Unbekannte Option. Nutzen Sie '-i' fuer IDs oder '-c' fuer Koordinaten.\n");
                    continue;
                }
            }
            else
            {
                iss.clear();
                iss.str(line);
            }

            if (useCoordinates)
            {
                auto readCoordinate = [&iss](double &value) -> bool {
                    if (!(iss >> value))
                    {
                        return false;
                    }
                    iss >> std::ws;
                    if (iss.peek() == ',')
                    {
                        iss.ignore();
                        iss >> std::ws;
                    }
                    return true;
                };

                if (!(readCoordinate(startLat) && readCoordinate(startLon) && readCoordinate(goalLat) && readCoordinate(goalLon)))
                {
                    sendAll(clientFd, "Ungueltige Koordinaten. Beispiel: -c 49.04878,8.41707 49.05339,8.43972\n");
                    continue;
                }
            }
            else
            {
                if (!(iss >> startId >> goalId))
                {
                    sendAll(clientFd, "Ungueltige Eingabe. Bitte '-i <startId> <zielId>' oder zwei numerische IDs senden.\n");
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
        }

        std::vector<std::tuple<uint64_t, Coordinates>> path;
        if (useCoordinates)
        {
            Coordinates startCoords(startLat, startLon);
            Coordinates goalCoords(goalLat, goalLon);
            path = graph_.aStar(startCoords, goalCoords);
        }
        else
        {
            path = graph_.aStar(startId, goalId);
        }

        if (path.empty())
        {
            sendAll(clientFd, "Kein Pfad gefunden.\n");
            continue;
        }

        const std::string fileName = "astar_path_" + std::to_string(pathCounter_++) + ".geojson";
        pathCounter_ %= 10;
        std::string filepath = HelperFunctions::exportPathToGeoJSON(path, fileName);

        std::ostringstream response;

        response << "Pfad von ";
        if (useCoordinates)
        {
            response << "(" << Coordinates(startLat, startLon) << ") nach ("
                     << Coordinates(goalLat, goalLon) << ")";
        }
        else
        {
            response << startId << " nach " << goalId;
        }
        response << " mit " << path.size() << " Punkten exportiert nach " << filepath << "\n";
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
    #ifdef _WIN32
    if (setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt)) < 0)
    #else
    if (setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    #endif
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
