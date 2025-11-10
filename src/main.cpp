#include <cstdint>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_dense.h>

#include <pugixml.hpp>

#include "library.hpp"
#include "osmnode.hpp"
#include "osmway.hpp"
#include "graph.hpp"
#include "socket.hpp"

void readOSMFile(const std::string &filepath, ankerl::unordered_dense::map<u_int64_t, std::shared_ptr<OsmNode>> &nodes, ankerl::unordered_dense::map<uint64_t, std::unique_ptr<OsmWay>> &ways);
void createGraph(Graph &graph, ankerl::unordered_dense::map<u_int64_t, std::shared_ptr<OsmNode>> &nodes, ankerl::unordered_dense::map<uint64_t, std::unique_ptr<OsmWay>> &ways);

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

    {
        ankerl::unordered_dense::map<u_int64_t, std::shared_ptr<OsmNode>> nodes;
        ankerl::unordered_dense::map<uint64_t, std::unique_ptr<OsmWay>> ways;

        readOSMFile(argv[1], nodes, ways);

        createGraph(graph, nodes, ways);
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

void readOSMFile(const std::string &filepath, ankerl::unordered_dense::map<u_int64_t, std::shared_ptr<OsmNode>> &nodes, ankerl::unordered_dense::map<uint64_t, std::unique_ptr<OsmWay>> &ways)
{
    pugi::xml_document doc;
    if (doc.load_file(filepath.c_str()))
    {
        for (const auto &node : doc.select_nodes("/osm/node"))
        {
            u_int64_t id = std::stoull(node.node().attribute("id").value());
            double lat = std::stod(node.node().attribute("lat").value());
            double lon = std::stod(node.node().attribute("lon").value());
            nodes.emplace(id, std::make_shared<OsmNode>(id, lat, lon));
        }

        for(const auto &way : doc.select_nodes("/osm/way"))
        {
            for(const auto &tag : way.node().children("tag"))
            {
                // if current way describes a highway (find out by searching tags)
                if (std::string(tag.attribute("k").value()) == "highway")
                {
                    uint64_t id = std::stoull(way.node().attribute("id").value());
                    ways.emplace(id, std::make_unique<OsmWay>(id));
                    //std::cout << "Way id: " << way.node().attribute("id").value() << "\n";
                    
                    for (const auto &node : way.node().children("nd"))
                    {
                        try
                        {
                            auto nodeFromList = nodes.at(std::stoull(node.attribute("ref").value()));
                            nodeFromList->isVisited = true;
                            ways.at(id)->addNode(nodeFromList);
                        }
                        catch (const std::out_of_range &e)
                        {
                            std::cerr << "Node with ID " << node.attribute("ref").value() << " not found.\n";
                            continue;
                        }
                    }

                    if(ways.at(id)->getNodes().size() < 2)
                    {
                        //std::cout << "  Skipping way with less than 2 nodes.\n";
                        break;
                    }

                    ways.at(id)->getNodes().front()->isEdge = true;
                    ways.at(id)->getNodes().back()->isEdge = true;

                    //std::cout << "  Path Length: " << ways.at(id).calculateWayLength() << " m\n";

                    //std::cout << "  Tag: " << tag.attribute("k").value() << " = " << tag.attribute("v").value() << "\n";
                    break;
                }
            }
        }

        // Clean up empty nodes
        for(auto it = nodes.begin(); it != nodes.end(); /* */)
        {
            if(it->second->isVisited == false) 
            {
                it = nodes.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }
    else
    {
        std::cerr << "Error reading OSM file!\n"; 
        exit(1);
    }
}

void createGraph(Graph &graph, ankerl::unordered_dense::map<u_int64_t, std::shared_ptr<OsmNode>> &nodes, ankerl::unordered_dense::map<uint64_t, std::unique_ptr<OsmWay>> &ways)
{
    for(auto &node : nodes)
    {
        graph.addOsmNode(node.second);
    }

    for(auto &way : ways)
    {
        graph.addOsmWay(way.second.get());
    }
}
