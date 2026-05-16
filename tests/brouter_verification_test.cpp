#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <cassert>
#include <fstream>
#include <format>
#include <vector>
#include <unordered_map>
#include <tuple>

#include "router.hpp"
#include "gpxparser.hpp"
#include "routes.hpp"
#include "library.hpp"
#include "weights.hpp"
#include "parameters.hpp"

// Struktur zum Cachen der unveränderlichen Map-Matching-Daten pro Track
struct CachedTrack {
    std::string filename;
    std::vector<Edge*> matchSet;
    Coordinates startPoint;
    Coordinates endPoint;
    
    // Die berechneten Tags/Längen für das matchSet ändern sich ebenfalls nie
    std::unordered_map<std::string, std::unordered_map<std::string, double>> tagsMatched;
};

int main(int argc, char **argv)
{
    try
    {
        const std::string osmPath = std::string(PROJECT_SOURCE_DIR) + "/testdata/Karlsruhe_Region.osm";
        std::string weightsPath = argc > 1 ? "weights_empty.csv" : "brouter_weights.csv";
        Router router(osmPath, weightsPath);
        Weights checkWeights(weightsPath);
        GPXParser parser;
        Routes routes(router);

        parser.loadGPXFiles(std::string(PROJECT_SOURCE_DIR) + "/testdata/brouterdata_validation");

        // ---------------------------------------------------------
        // PHASE 1: PRE-COMPUTATION (Map-Matching cachen)
        // ---------------------------------------------------------
        std::cout << "Starte Pre-Computation (Map-Matching)...\n";
        std::vector<CachedTrack> trainingData;

        for(auto &[filename, points] : parser.getTracks())
        {
            if(!router.getQuadtree().getBoundary().contains(points[0])) continue;

            std::cout << "Matching " << filename << std::endl;

            auto matchSet = routes.getEdgeSet(points);
            auto tagsMatched = routes.getParameterLengthMap(matchSet);

            trainingData.push_back({
                filename, 
                matchSet, 
                points.front(), 
                points.back(),
                tagsMatched
            });
        }
        std::cout << "Pre-Computation abgeschlossen. " << trainingData.size() << " Tracks gecacht.\n\n";


        double jaccardSum = 0;
        uint8_t jaccardCount = 0;

        // Aggregierte Maps für diese Epoche
        std::unordered_map<std::string, std::unordered_map<std::string, double>> globalTagsMatched;
        std::unordered_map<std::string, std::unordered_map<std::string, double>> globalTagsGuessed;

        double lengthMatched = 0;
        double lengthGuessed = 0;

        // 1. A* Routing mit AKTUELLEN Gewichten für alle Tracks
        for(const auto &track : trainingData)
        {
            std::vector<Edge *> edgeSet = router.aStarEdges(track.startPoint, track.endPoint, true);
            routes.prepareEdgeSet(edgeSet);

            double jaccard = routes.getJaccardCoefficient(edgeSet, track.matchSet);
            jaccardSum += jaccard;
            jaccardCount++;

            // Tags des generierten (guessed) Pfades berechnen
            auto tagsGuessed = routes.getParameterLengthMap(edgeSet);

            // Aggregieren für den Gradienten
            for(const auto &[key, valueMap] : track.tagsMatched) {
                for(const auto &[value, length] : valueMap) {
                    globalTagsMatched[key][value] += length;
                }
            }
            for(const auto &[key, valueMap] : tagsGuessed) {
                for(const auto &[value, length] : valueMap) {
                    globalTagsGuessed[key][value] += length;
                }
            }

            double routeLengthMatched = routes.getLength(track.matchSet);
            double routeLengthGuessed = routes.getLength(edgeSet);

            double routeCostMatched = routes.getCost(track.matchSet, checkWeights);
            double routeCostGuessed = routes.getCost(edgeSet, checkWeights);

            lengthMatched += routeLengthMatched;
            lengthGuessed += routeLengthGuessed;

            double lengthDifference = routeLengthMatched - routeLengthGuessed;
            double weightDifference = routeCostMatched - routeCostGuessed;

            std::cout << std::format("filename: {}, jaccard: {}, cost difference: {}, length difference: {}, cost difference per km in m: {}\n", track.filename, jaccard, weightDifference / 1000, lengthDifference / 1000, weightDifference / routeLengthMatched * 1000);

            //HelperFunctions::saveEdgesAsGeoJSON(edgeSet, weightsPath);
        }

        // 2. Kosten und Differenzen auswerten
        std::vector<std::tuple<std::string, std::string, double>> tagsDifferenceVector;
        double costMatched = 0, costGuessed = 0;

        for(const auto &[key, valueMap] : globalTagsMatched)
        {
            for(const auto &[value, length] : valueMap)
            {
                double tagsGuessedLength = 0;
                double tagCost = checkWeights.getWeight(key, value);
                
                if(globalTagsGuessed[key].find(value) != globalTagsGuessed[key].end()) {
                    tagsGuessedLength = globalTagsGuessed[key][value];
                    globalTagsGuessed[key].erase(value); // Entfernen, um Reste später leichter zu iterieren
                }
                
                tagsDifferenceVector.emplace_back(key, value, length - tagsGuessedLength);
                costMatched += (length * tagCost);
                costGuessed += (tagsGuessedLength * tagCost);
            }
            
            // Reste aus Guessed, die in Matched nicht vorkamen
            for(const auto &[value, length] : globalTagsGuessed[key])
            {
                tagsDifferenceVector.emplace_back(key, value, -length);
                costGuessed += (length * checkWeights.getWeight(key, value));
            }
        }

        double meanJaccard = (jaccardCount > 0) ? (jaccardSum / jaccardCount) : 0;
        costMatched += lengthMatched;
        costGuessed += lengthGuessed;
        double costDifference = costMatched - costGuessed;

        std::cout << std::format("Mean Jaccard: {:.4f} | Cost Diff: {:6.1f} | Length Diff: {:6.1f} km\n", 
                                    meanJaccard, costDifference / 1000, (lengthMatched - lengthGuessed) / 1000);

        std::cout << "Cost difference per km in m: " << costDifference / costMatched * 1000 << std::endl;
        std::cout << std::format("cost matched: {:.4f}, cost guessed: {:.4f}\n", costMatched / 1000, costGuessed / 1000);
        std::cout << std::format("length matched: {:.4f}, length guessed: {:.4f}\n", lengthMatched / 1000, lengthGuessed / 1000);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Fehler im Test: " << e.what() << "\n";
        return 1;
    }
    return 0;
}