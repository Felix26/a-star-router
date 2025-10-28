#include <iostream>
#include <string>
#include <unordered_map>
#include <cstdint>
#include <cstdlib>


#include <pugixml.hpp>

#include "library.hpp"
#include "osmnode.hpp"
#include "osmway.hpp"

#include "graph.hpp"

std::unordered_map<u_int64_t, OsmNode> nodes;
std::unordered_map<uint64_t, OsmWay> ways;

Graph graph;

void readOSMFile(const std::string &filepath);
void createGraph();

int main()
{
    srand(240);
    readOSMFile("/home/felixm/Desktop/Studienarbeit/Router/testdata/bw_min.osm");

    createGraph();
    //graph.printGraph();

    auto &nodelist = graph.getNodes();
    auto nodeListSize = nodelist.size();

    std::vector<uint64_t> ids;

    for(auto &[id, _] : nodelist)
    {
        ids.push_back(id);
    }

    for(int i = 0; i < 10; i++)
    {
        uint64_t startId = ids.at(rand() % nodeListSize);
        uint64_t goalId = ids.at(rand() % nodeListSize);

        auto path = graph.aStar(startId, goalId);

        std::cout << "Path " << i << " from " << startId << " to " << goalId << "\n";
        HelperFunctions::exportPathToGeoJSON(path, "astar_path_" + std::to_string(i) + ".geojson");
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

void createGraph()
{
    for(auto &node : nodes)
    {
        graph.addOsmNode(node.second);
    }

    for(auto &way : ways)
    {
        graph.addOsmWay(way.second);
    }

    nodes = std::unordered_map<u_int64_t, OsmNode>(); // free memory
    ways = std::unordered_map<u_int64_t, OsmWay>(); // free memory
}