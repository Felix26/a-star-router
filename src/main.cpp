#include <iostream>
#include <string>
#include <unordered_map>
#include <cstdint>


#include <pugixml.hpp>

#include "library.hpp"
#include "osmnode.hpp"
#include "osmway.hpp"

#include "graph.hpp"

std::unordered_map<u_int64_t, OsmNode> nodes;
std::unordered_map<uint64_t, OsmWay> ways;

Graph graph;

void readOSMFile(const std::string &filepath);

int main()
{
    readOSMFile("/home/felixm/Desktop/Studienarbeit/Router/testdata/neureut.osm");

    for(auto &node : nodes)
    {
        graph.addOsmNode(node.second);
    }

    for(auto &way : ways)
    {
        graph.addOsmWay(way.second);
    }

    graph.printGraph();

    std::cout << "Original Way Count: " << ways.size() << "\n";
    std::cout << "Original Node Count: " << nodes.size() << "\n";

    return 0;
}

void readOSMFile(const std::string &filepath)
{
    pugi::xml_document doc;
    if (doc.load_file(filepath.c_str()))
    {
        for (const auto &node : doc.select_nodes("/osm/node"))
        {
            u_int64_t id = std::stoull(node.node().attribute("id").value());
            double lat = std::stod(node.node().attribute("lat").value());
            double lon = std::stod(node.node().attribute("lon").value());
            nodes.emplace(id, OsmNode(id, lat, lon));
        }

        for(const auto &way : doc.select_nodes("/osm/way"))
        {
            for(const auto &tag : way.node().children("tag"))
            {
                // if current way describes a highway (find out by searching tags)
                if (std::string(tag.attribute("k").value()) == "highway")
                {
                    uint64_t id = std::stoull(way.node().attribute("id").value());
                    ways.emplace(id, OsmWay(id));
                    //std::cout << "Way id: " << way.node().attribute("id").value() << "\n";
                    
                    for (const auto &node : way.node().children("nd"))
                    {
                        try
                        {
                            auto &nodeFromList = nodes.at(std::stoull(node.attribute("ref").value()));
                            nodeFromList.isVisited = true;
                            ways.at(id).addNode(nodeFromList);
                        }
                        catch (const std::out_of_range &e)
                        {
                            std::cerr << "Node with ID " << node.attribute("ref").value() << " not found.\n";
                            continue;
                        }
                    }

                    if(ways.at(id).getNodes().size() < 2)
                    {
                        //std::cout << "  Skipping way with less than 2 nodes.\n";
                        break;
                    }

                    ways.at(id).getNodes().front().get().isEdge = true;
                    ways.at(id).getNodes().back().get().isEdge = true;

                    //std::cout << "  Path Length: " << ways.at(id).calculateWayLength() << " m\n";

                    //std::cout << "  Tag: " << tag.attribute("k").value() << " = " << tag.attribute("v").value() << "\n";
                    break;
                }
            }
        }

        // Clean up empty nodes
        for(auto it = nodes.begin(); it != nodes.end(); /* */)
        {
            if(it->second.isVisited == false) 
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
