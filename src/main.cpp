#include <iostream>
#include <string>
#include <unordered_map>
#include <cstdint>


#include <pugixml.hpp>

#include "library.hpp"
#include "node.hpp"

std::unordered_map<u_int64_t, Node> nodes;

int main()
{
    pugi::xml_document doc;
    if (doc.load_file("/home/felixm/Desktop/Studienarbeit/Router/testdata/neureut.osm"))
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
                    std::cout << "Way id: " << way.node().attribute("id").value() << "\n";

                    std::vector<Coordinates> path;
                    
                    for (const auto &node : way.node().children("nd"))
                    {
                        //std::cout << "  Node ref: " << node.attribute("ref").value() << "\n";
                        //std::cout << "    Coordinates: " << nodes.at(std::stoull(node.attribute("ref").value())).getCoordinates() << "\n";
                        path.push_back(nodes.at(std::stoull(node.attribute("ref").value())).getCoordinates());
                    }

                    std::cout << "  Path Length: " << HelperFunctions::calculatePathLength(path) << " m\n";

                    std::cout << "  Tag: " << tag.attribute("k").value() << " = " << tag.attribute("v").value() << "\n";
                    break;
                }
            }
        }
    }
}
