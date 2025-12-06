#include <cstdint>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <ankerl/unordered_dense.h>

#include "library.hpp"
#include "osmnode.hpp"
#include "osmway.hpp"
#include "graph.hpp"
#include "socket.hpp"

namespace
{
    constexpr uint16_t kDefaultPort = 5555;
}

int main(int argc, char *argv[])
{
    srand(0);
    
    if(argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <osm_file.osm> [port]\n";
        return 1;
    }
    
    uint16_t port = kDefaultPort;
    if (argc >= 3)
    {
        int parsedPort = std::stoi(argv[2]);
        if (parsedPort <= 0 || parsedPort > 65535)
        {
            std::cerr << "Ungueltiger Port: " << argv[2] << "\n";
            return 1;
        }
        port = static_cast<uint16_t>(parsedPort);
    }

    Graph graph;

    try
    {
        ankerl::unordered_dense::map<uint64_t, std::shared_ptr<OsmNode>> nodes;
        ankerl::unordered_dense::map<uint64_t, std::unique_ptr<OsmWay>> ways;

        HelperFunctions::readOSMFile(argv[1], nodes, ways);
        HelperFunctions::createGraph(graph, nodes, ways);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Fehler beim Laden der OSM-Datei: " << e.what() << "\n";
        return 1;
    }

    try
    {
        socketcpp::RouterServer server(graph, port);
        server.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Serverfehler: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
