#include <iostream>
#include <string>
#include <unordered_map>
#include <cstdint>
#include <sstream>
#include <memory>


#include <pugixml.hpp>

#include "library.hpp"
#include "osmnode.hpp"
#include "osmway.hpp"

#include "graph.hpp"

std::unordered_map<u_int64_t, std::shared_ptr<OsmNode>> nodes;
std::unordered_map<uint64_t, std::unique_ptr<OsmWay>> ways;

Graph graph;

void readOSMFile(const std::string &filepath);
void createGraph();

int main(int argc, char *argv[])
{
    if(argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <osm_file.osm>\n";
        return 1;
    }

    readOSMFile(argv[1]);

    createGraph();
    //graph.printGraph();

    auto &nodelist = graph.getNodes();

    size_t pathCounter = 0;
    std::string line;
    while (true)
    {
        std::cout << "Bitte Start- und Zielknoten-ID eingeben (oder 'exit'): ";
        if (!std::getline(std::cin, line))
        {
            std::cout << "Eingabe beendet.\n";
            break;
        }

        if (line.empty())
            continue;

        if (line == "exit" || line == "quit")
            break;

        std::istringstream iss(line);
        uint64_t startId = 0;
        uint64_t goalId = 0;
        if (!(iss >> startId >> goalId))
        {
            std::cerr << "Ungültige Eingabe. Bitte zwei numerische IDs eingeben.\n";
            continue;
        }

        if (nodelist.find(startId) == nodelist.end())
        {
            std::cerr << "Startknoten " << startId << " existiert nicht im Graphen.\n";
            continue;
        }

        if (nodelist.find(goalId) == nodelist.end())
        {
            std::cerr << "Zielknoten " << goalId << " existiert nicht im Graphen.\n";
            continue;
        }

        auto path = graph.aStar(startId, goalId);
        if (path.empty())
        {
            std::cerr << "Kein Pfad gefunden zwischen " << startId << " und " << goalId << ".\n";
            continue;
        }

        std::cout << "Pfad " << pathCounter << " von " << startId << " nach " << goalId << " mit " << path.size() << " Punkten.\n";
        HelperFunctions::exportPathToGeoJSON(path, "astar_path_" + std::to_string(pathCounter++) + ".geojson");
    }

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

void createGraph()
{
    for(auto &node : nodes)
    {
        graph.addOsmNode(node.second);
    }

    for(auto &way : ways)
    {
        graph.addOsmWay(way.second.get());
    }

    nodes = std::unordered_map<u_int64_t, std::shared_ptr<OsmNode>>(); // free memory
    ways = std::unordered_map<u_int64_t, std::unique_ptr<OsmWay>>(); // free memory
}
