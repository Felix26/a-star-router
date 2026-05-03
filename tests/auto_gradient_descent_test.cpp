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

#define DISTANCE 2

// Struktur zum Cachen der unveränderlichen Map-Matching-Daten pro Track
struct CachedTrack {
    std::string filename;
    std::vector<uint64_t> ids;
    std::vector<Edge*> matchSet;
    Coordinates startPoint;
    Coordinates endPoint;
    
    // Die berechneten Tags/Längen für das matchSet ändern sich ebenfalls nie
    std::unordered_map<std::string, std::unordered_map<std::string, double>> tagsMatched;
};

int main()
{
    try
    {
        const std::string osmPath = std::string(PROJECT_SOURCE_DIR) + "/testdata/Karlsruhe_Region.osm";
        Router router(osmPath, "weightsnew.csv");
        GPXParser parser;
        Routes routes(router);

        parser.loadGPXFiles(std::string(PROJECT_SOURCE_DIR) + "/testdata/gpxdata");

        // ---------------------------------------------------------
        // PHASE 1: PRE-COMPUTATION (Map-Matching cachen)
        // ---------------------------------------------------------
        std::cout << "Starte Pre-Computation (Map-Matching)...\n";
        std::vector<CachedTrack> trainingData;

        for(auto &[filename, points] : parser.getTracks())
        {
            if(!router.getQuadtree().getBoundary().contains(points[0])) continue;
            
            std::vector<std::tuple<uint64_t, Coordinates>> matchedTrack = parser.mapMatching(router, points);
            
            Path coordinates;
            std::vector<uint64_t> ids;
            
            for(uint32_t i = 0; i < matchedTrack.size(); i++)
            {
                auto &[id, coords] = matchedTrack[i];
                coordinates.emplace_back(coords);
                if(id && id < 0x00FFFFFFFFFFFFFF)
                {
                    ids.emplace_back(id);
                }
            }

            if(ids.empty()) continue;

            auto matchSet = routes.getEdgeSet(coordinates);
            auto tagsMatched = routes.getParameterLengthMap(matchSet);

            trainingData.push_back({
                filename, 
                ids, 
                matchSet, 
                points.front(), 
                points.back(),
                tagsMatched
            });
        }
        std::cout << "Pre-Computation abgeschlossen. " << trainingData.size() << " Tracks gecacht.\n\n";

        // ---------------------------------------------------------
        // PHASE 2 & 3: TRAINING LOOP (Gradientenabstieg)
        // ---------------------------------------------------------
        const int EPOCHS = 250; 
        const double LEARNING_RATE = 0.05; // Hyperparameter für kleinere, stabilere Schritte

        for (int epoch = 1; epoch <= EPOCHS; ++epoch)
        {
            std::cout << std::format("=== Starte Epoche {}/{} ===\n", epoch, EPOCHS);

            double jaccardSum = 0;
            uint8_t jaccardCount = 0;

            // Aggregierte Maps für diese Epoche
            std::unordered_map<std::string, std::unordered_map<std::string, double>> globalTagsMatched;
            std::unordered_map<std::string, std::unordered_map<std::string, double>> globalTagsGuessed;

            // 1. A* Routing mit AKTUELLEN Gewichten für alle Tracks
            for(const auto &track : trainingData)
            {
                std::vector<Edge *> edgeSet;

                for(uint32_t i = 0; i < track.ids.size(); i += DISTANCE)
                {
                    auto result = router.aStarEdges(track.ids[i], track.ids[std::min<uint32_t>(i + DISTANCE, track.ids.size() - 1)], true);
                    edgeSet.insert(edgeSet.end(), result.begin(), result.end());
                }

                edgeSet.emplace_back(router.getQuadtree().getClosestEdges(track.startPoint)[0].edge);
                edgeSet.emplace_back(router.getQuadtree().getClosestEdges(track.endPoint)[0].edge);
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
            }

            // 2. Kosten und Differenzen auswerten
            std::vector<std::tuple<std::string, std::string, double>> tagsDifferenceVector;
            double lengthMatched = 0, lengthGuessed = 0;
            double costMatched = 0, costGuessed = 0;

            for(const auto &[key, valueMap] : globalTagsMatched)
            {
                for(const auto &[value, length] : valueMap)
                {
                    double tagsGuessedLength = 0;
                    double tagCost = router.getWeights().getWeight(key, value);
                    
                    if(globalTagsGuessed[key].find(value) != globalTagsGuessed[key].end()) {
                        tagsGuessedLength = globalTagsGuessed[key][value];
                        globalTagsGuessed[key].erase(value); // Entfernen, um Reste später leichter zu iterieren
                    }
                    
                    tagsDifferenceVector.emplace_back(key, value, length - tagsGuessedLength);

                    lengthMatched += length;
                    lengthGuessed += tagsGuessedLength;
                    costMatched += length + (length * tagCost);
                    costGuessed += tagsGuessedLength + (tagsGuessedLength * tagCost);
                }
                
                // Reste aus Guessed, die in Matched nicht vorkamen
                for(const auto &[value, length] : globalTagsGuessed[key])
                {
                    tagsDifferenceVector.emplace_back(key, value, -length);
                    lengthGuessed += length;
                    costGuessed += length + (length * router.getWeights().getWeight(key, value));
                }
            }

            double meanJaccard = (jaccardCount > 0) ? (jaccardSum / jaccardCount) : 0;
            double costDifference = costMatched - costGuessed;

            std::cout << std::format("Mean Jaccard: {:.4f} | Cost Diff: {:6.1f} | Length Diff: {:6.1f} km\n", 
                                     meanJaccard, costDifference / 1000, (lengthMatched - lengthGuessed) / 1000);

            // 3. Gradientenabstieg & Gewichte anpassen
            double norm = 0;
            for(auto &[key, value, difference] : tagsDifferenceVector) {
                norm += difference * difference;
            }

            // Log-Datei für diese Epoche öffnen
            std::string logPath = std::format("logs/weights_epoch_{:03d}.csv", epoch);
            std::ofstream file(logPath);
            file << "#parameter,weight\n";

            for(auto &[key, value, difference] : tagsDifferenceVector)
            {
                // Gradient berechnen
                double normedgradient = (norm > 0) ? (difference / norm * costDifference) : 0;
                
                double currentWeight = router.getWeights().getWeight(key, value);
                
                // Standard Machine Learning Update-Regel: w_neu = w_alt - (learning_rate * gradient)
                // Bei deinem vorherigen Code hast du die Lernrate weggelassen (=1.0). 
                // Ein Faktor entschärft Oszillationen.
                double newWeight = std::max(0.0, currentWeight - (LEARNING_RATE * normedgradient));
                
                // --- WICHTIG: Gewicht im RAM updaten! ---
                // Du benötigst vermutlich eine Setter-Funktion in deiner Weights-Klasse.
                router.getWeights().setWeight(key, value, newWeight);
                
                file << std::format("{},{},{}\n", key, value, newWeight);
            }
            
            file.close();
            std::cout << "Gewichte in " << logPath << " gespeichert.\n\n";
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Fehler im Test: " << e.what() << "\n";
        return 1;
    }
    return 0;
}