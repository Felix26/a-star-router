#include "library.hpp"

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <string>
#include <vector>
#include <filesystem>

#include <libxml/xmlreader.h>

#include "graph.hpp"
#include "osmnode.hpp"
#include "osmway.hpp"

namespace HelperFunctions
{
    namespace
    {
        constexpr double DEG_TO_RAD = 3.14159265358979323846 / 180.0;

        struct XmlParserGuard
        {
            XmlParserGuard() { xmlInitParser(); }
            ~XmlParserGuard() { xmlCleanupParser(); }
        };

        struct XmlReaderGuard
        {
            explicit XmlReaderGuard(xmlTextReaderPtr readerPtr) : reader(readerPtr) {}
            ~XmlReaderGuard()
            {
                if (reader)
                {
                    xmlFreeTextReader(reader);
                }
            }

            xmlTextReaderPtr reader;
        };

        std::string getAttributeValue(xmlTextReaderPtr reader, const char *attributeName)
        {
            xmlChar *rawValue = xmlTextReaderGetAttribute(reader, BAD_CAST attributeName);
            if (!rawValue)
            {
                return {};
            }

            std::string value(reinterpret_cast<const char *>(rawValue));
            xmlFree(rawValue);
            return value;
        }

        void processNodeElement(xmlTextReaderPtr reader, ankerl::unordered_dense::map<uint64_t, std::shared_ptr<OsmNode>> &nodes)
        {
            const std::string idStr = getAttributeValue(reader, "id");
            const std::string latStr = getAttributeValue(reader, "lat");
            const std::string lonStr = getAttributeValue(reader, "lon");

            if (idStr.empty() || latStr.empty() || lonStr.empty())
            {
                return;
            }

            try
            {
                const uint64_t id = std::stoull(idStr);
                const double lat = std::stod(latStr);
                const double lon = std::stod(lonStr);
                nodes.try_emplace(id, std::make_shared<OsmNode>(id, lat, lon));
            }
            catch (const std::exception &)
            {
                // Ungültige Werte werden ignoriert.
            }
        }

        void processWayElement(xmlTextReaderPtr reader, ankerl::unordered_dense::map<uint64_t, std::shared_ptr<OsmNode>> &nodes, ankerl::unordered_dense::map<uint64_t, std::unique_ptr<OsmWay>> &ways)
        {
            const std::string idStr = getAttributeValue(reader, "id");
            if (idStr.empty())
            {
                return;
            }

            uint64_t wayId = 0;
            try
            {
                wayId = std::stoull(idStr);
            }
            catch (const std::exception &)
            {
                return;
            }

            bool isHighway = false;
            std::vector<uint64_t> nodeRefs;

            if (!xmlTextReaderIsEmptyElement(reader))
            {
                const int baseDepth = xmlTextReaderDepth(reader);
                while (xmlTextReaderRead(reader) == 1)
                {
                    const int nodeType = xmlTextReaderNodeType(reader);
                    if (nodeType == XML_READER_TYPE_ELEMENT)
                    {
                        const xmlChar *childName = xmlTextReaderConstName(reader);
                        if (!childName)
                        {
                            continue;
                        }

                        if (xmlStrcmp(childName, BAD_CAST "tag") == 0)
                        {
                            if (getAttributeValue(reader, "k") == "highway")
                            {
                                isHighway = true;
                            }
                        }
                        else if (xmlStrcmp(childName, BAD_CAST "nd") == 0)
                        {
                            const std::string refStr = getAttributeValue(reader, "ref");
                            if (!refStr.empty())
                            {
                                try
                                {
                                    nodeRefs.push_back(std::stoull(refStr));
                                }
                                catch (const std::exception &)
                                {
                                    // Ignorieren, falls ref ungültig ist.
                                }
                            }
                        }
                    }
                    else if (nodeType == XML_READER_TYPE_END_ELEMENT && xmlTextReaderDepth(reader) == baseDepth)
                    {
                        break;
                    }
                }
            }

            if (!isHighway)
            {
                return;
            }

            auto way = std::make_unique<OsmWay>(wayId);

            for (uint64_t nodeId : nodeRefs)
            {
                auto nodeIt = nodes.find(nodeId);
                if (nodeIt == nodes.end())
                {
                    std::cerr << "Node with ID " << nodeId << " not found.\n";
                    continue;
                }

                nodeIt->second->isVisited = true;
                way->addNode(nodeIt->second);
            }

            if (way->getNodes().size() < 2)
            {
                return;
            }

            way->getNodes().front()->isEdge = true;
            way->getNodes().back()->isEdge = true;

            ways[wayId] = std::move(way);
        }
    }

    // Haversine formula to calculate the great-circle distance between two points in meters
    double haversine(double lat1_deg, double lon1_deg, double lat2_deg, double lon2_deg)
    {
        const double lat1 = lat1_deg * DEG_TO_RAD;
        const double lon1 = lon1_deg * DEG_TO_RAD;
        const double lat2 = lat2_deg * DEG_TO_RAD;
        const double lon2 = lon2_deg * DEG_TO_RAD;

        const double dlat = lat2 - lat1;
        const double dlon = lon2 - lon1;

        const double sin_lat = std::sin(dlat / 2.0);
        const double sin_lon = std::sin(dlon / 2.0);

        const double a = (sin_lat * sin_lat) +
                         std::cos(lat1) * std::cos(lat2) * (sin_lon * sin_lon);

        const double c = 2.0 * std::atan2(std::sqrt(a), std::sqrt(1.0 - a));

        return EARTH_RADIUS_KM * c * 1000.0; // return distance in meters
    }

    double haversine(const Coordinates &coord1, const Coordinates &coord2)
    {
        return haversine(coord1.getLatitude(), coord1.getLongitude(),
                         coord2.getLatitude(), coord2.getLongitude());
    }

    double euclideanDistanceSquared(const Coordinates &c1, const Coordinates &c2)
    {
        // magic numbers:
        // 0.017453292519943295 = pi / 180
        // 6371000.0           = mean Earth radius [m]
        // 0.739942111693848   = cos(48.5°)  (reference latitude for BW)

        const double dy =
            6371000.0 *
            (c2.getLatitude() - c1.getLatitude()) *
            0.017453292519943295;

        const double dx =
            6371000.0 *
            (c2.getLongitude() - c1.getLongitude()) *
            0.017453292519943295 *
            0.739942111693848;

        return dx * dx + dy * dy; // m²
    }
    

    double calculatePathLength(const std::vector<Coordinates> &path)
    {
        double totalLength = 0.0;

        for (std::size_t i = 1; i < path.size(); ++i)
        {
            totalLength += haversine(path[i - 1], path[i]);
        }

        return totalLength;
    }

    double distancePointToSegment(const Coordinates &point, const Coordinates &segStart, const Coordinates &segEnd)
    {
        Coordinates projection = getProjectionOnSegment(point, segStart, segEnd);
        return haversine(point, projection);
    }

    Coordinates getProjectionOnSegment(const Coordinates &point, const Coordinates &segStart, const Coordinates &segEnd)
    {
        double lat1 = segStart.getLatitude() * DEG_TO_RAD;
        double lon1 = segStart.getLongitude() * DEG_TO_RAD;
        double lat2 = segEnd.getLatitude() * DEG_TO_RAD;
        double lon2 = segEnd.getLongitude() * DEG_TO_RAD;
        double latP = point.getLatitude() * DEG_TO_RAD;
        double lonP = point.getLongitude() * DEG_TO_RAD;

        double dLat = lat2 - lat1;
        double dLon = lon2 - lon1;

        if (dLat == 0 && dLon == 0)
        {
            return segStart;
        }

        double t = ((latP - lat1) * dLat + (lonP - lon1) * dLon) / (dLat * dLat + dLon * dLon);
        t = std::max(0.0, std::min(1.0, t));

        double projLat = lat1 + t * dLat;
        double projLon = lon1 + t * dLon;

        return Coordinates(projLat / DEG_TO_RAD, projLon / DEG_TO_RAD);
    }

    std::string exportPathToGeoJSON(const std::vector<std::tuple<uint64_t, Coordinates>> &path, const std::string &filename)
    {
        std::ofstream file(filename);
        if (!file.is_open())
        {
            std::cerr << "Error opening file for writing: " << filename << "\n";
            return "";
        }

        file << "{\n";
        file << "  \"type\": \"FeatureCollection\",\n";
        file << "  \"features\": [\n";
        file << "    {\n";
        file << "      \"type\": \"Feature\",\n";
        file << "      \"geometry\": {\n";
        file << "        \"type\": \"LineString\",\n";
        file << "        \"coordinates\": [\n";

        for (size_t i = 0; i < path.size(); ++i)
        {
            const auto &[id, coord] = path[i];
            file << "          [" << std::fixed << std::setprecision(7) << coord.getLongitude() << ", " << coord.getLatitude() << "]";
            if (i < path.size() - 1)
                file << ",";
            file << "\n";
        }

        file << "        ]\n";
        file << "      },\n";
        file << "      \"properties\": {}\n";
        file << "    }\n";
        file << "  ]\n";
        file << "}\n";

        file.close();

        try
        {
            // Konvertiert den (relativen) Dateinamen in einen absoluten Pfad
            std::filesystem::path absolute_path = std::filesystem::absolute(filename);
            return absolute_path.string();
        }

        catch (const std::exception& e)
        {
            std::cerr << "Error determining absolute path: " << e.what() << "\n";
            return filename; // Rückgabe des ursprünglichen Pfades als Fallback
        }
    }

    void readOSMFile(const std::string &filepath, ankerl::unordered_dense::map<uint64_t, std::shared_ptr<OsmNode>> &nodes, ankerl::unordered_dense::map<uint64_t, std::unique_ptr<OsmWay>> &ways)
    {
        XmlParserGuard parserGuard;
        xmlTextReaderPtr readerPtr = xmlReaderForFile(filepath.c_str(), nullptr, XML_PARSE_NOBLANKS | XML_PARSE_NOERROR | XML_PARSE_NOWARNING);
        if (!readerPtr)
        {
            throw std::runtime_error("Error reading OSM file: " + filepath);
        }

        XmlReaderGuard reader(readerPtr);

        int ret = 0;
        while ((ret = xmlTextReaderRead(reader.reader)) == 1)
        {
            if (xmlTextReaderNodeType(reader.reader) != XML_READER_TYPE_ELEMENT)
            {
                continue;
            }

            const xmlChar *nodeName = xmlTextReaderConstName(reader.reader);
            if (!nodeName)
            {
                continue;
            }

            if (xmlStrcmp(nodeName, BAD_CAST "node") == 0)
            {
                processNodeElement(reader.reader, nodes);
            }
            else if (xmlStrcmp(nodeName, BAD_CAST "way") == 0)
            {
                processWayElement(reader.reader, nodes, ways);
            }
        }

        if (ret < 0)
        {
            throw std::runtime_error("XML parsing error in file: " + filepath);
        }

        for (auto it = nodes.begin(); it != nodes.end();)
        {
            if (!it->second->isVisited)
            {
                it = nodes.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

    const Box createGraph(Graph &graph, ankerl::unordered_dense::map<uint64_t, std::shared_ptr<OsmNode>> &nodes, ankerl::unordered_dense::map<uint64_t, std::unique_ptr<OsmWay>> &ways)
    {
        double minLat = std::numeric_limits<double>::max();
        double minLon = std::numeric_limits<double>::max();
        double maxLat = std::numeric_limits<double>::lowest();
        double maxLon = std::numeric_limits<double>::lowest();

        for (auto &node : nodes)
        {
            graph.addOsmNode(node.second);
            const double lat = node.second->getCoordinates().getLatitude();
            const double lon = node.second->getCoordinates().getLongitude();

            if (lat < minLat) minLat = lat;
            if (lat > maxLat) maxLat = lat;
            if (lon < minLon) minLon = lon;
            if (lon > maxLon) maxLon = lon;
        }

        for (auto &way : ways)
        {
            graph.addOsmWay(way.second.get());
        }

        return Box(Coordinates(minLat, minLon), Coordinates(maxLat, maxLon));
    }
} // namespace HelperFunctions
