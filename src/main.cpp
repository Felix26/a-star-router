#include <iostream>
#include <string>
#include <unordered_map>
#include <cstdint>


#include <pugixml.hpp>

#include "library.hpp"
#include "node.hpp"
#include "way.hpp"

std::unordered_map<u_int64_t, Node> nodes;
std::unordered_map<uint64_t, Way> ways;

int main()
{
    pugi::xml_document doc;
    if (doc.load_file("/home/felixm/Desktop/Studienarbeit/Router/testdata/karlsruhe_stadt.osm"))
    {
        for (const auto &node : doc.select_nodes("/osm/node"))
        {
            u_int64_t id = std::stoull(node.node().attribute("id").value());
            double lat = std::stod(node.node().attribute("lat").value());
            double lon = std::stod(node.node().attribute("lon").value());
            nodes.emplace(id, Node(id, lat, lon));
        }

        for(const auto &way : doc.select_nodes("/osm/way"))
        {
            for(const auto &tag : way.node().children("tag"))
            {
                // if current way describes a highway (find out by searching tags)
                if (std::string(tag.attribute("k").value()) == "highway")
                {
                    uint64_t id = std::stoull(way.node().attribute("id").value());
                    ways.emplace(id, Way(id));
                    std::cout << "Way id: " << way.node().attribute("id").value() << "\n";
                    
                    for (const auto &node : way.node().children("nd"))
                    {
                        try
                        {
                            auto &nodeFromList = nodes.at(std::stoull(node.attribute("ref").value()));
                            nodeFromList.trackcount++;
                            ways.at(id).addNode(nodeFromList);
                        }
                        catch (const std::out_of_range &e)
                        {
                            std::cerr << "Node with ID " << node.attribute("ref").value() << " not found.\n";
                            continue;
                        }
                    }

                    std::cout << "  Path Length: " << ways.at(id).calculateWayLength() << " m\n";

                    std::cout << "  Tag: " << tag.attribute("k").value() << " = " << tag.attribute("v").value() << "\n";
                    break;
                }
            }
        }

        // Clean up empty nodes
        for(auto it = nodes.begin(); it != nodes.end(); /* */)
        {
            if(it->second.trackcount == 0) 
            {
                it = nodes.erase(it);
            }
            else
            {
                ++it;
            }
        }

        for(const auto node : nodes)
        {
            std::cout << "Node id " << node.first << " has " << node.second.trackcount << " ways.\n"; 
        }
    }
}
