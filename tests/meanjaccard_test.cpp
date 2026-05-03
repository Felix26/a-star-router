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
#include <set>

#include "router.hpp"
#include "gpxparser.hpp"
#include "routes.hpp"
#include "library.hpp"
#include "weights.hpp"
#include "parameters.hpp"

#define DISTANCE 10

struct CachedTrack {
    std::string filename;
    std::vector<uint64_t> ids;
    std::vector<Edge*> matchSet;
    Coordinates startPoint;
    Coordinates endPoint;
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
        // PHASE 1: PRE-COMPUTATION & PARAMETER SPACE
        // ---------------------------------------------------------
        std::cout << "Starte Pre-Computation...\n";
        std::vector<CachedTrack> trainingData;
        
        // Sammle alle einzigartigen Tag/Value-Kombinationen, die im Ground-Truth existieren
        std::set<std::pair<std::string, std::string>> activeParameters;

        for(auto &[filename, points] : parser.getTracks())
        {
            if(!router.getQuadtree().getBoundary().contains(points[0])) continue;
            std::vector<std::tuple<uint64_t, Coordinates>> matchedTrack = parser.mapMatching(router, points);
            
            Path coordinates;
            std::vector<uint64_t> ids;
            
            for(uint32_t i = 0; i < matchedTrack.size(); i++) {
                auto &[id, coords] = matchedTrack[i];
                coordinates.emplace_back(coords);
                if(id && id < 0x00FFFFFFFFFFFFFF) ids.emplace_back(id);
            }

            if(ids.empty()) continue;

            auto matchSet = routes.getEdgeSet(coordinates);
            auto tagsMatched = routes.getParameterLengthMap(matchSet);

            for(const auto& [key, valueMap] : tagsMatched) {
                for(const auto& [value, _] : valueMap) {
                    activeParameters.insert({key, value});
                }
            }

            trainingData.push_back({filename, ids, matchSet, points.front(), points.back(), tagsMatched});
        }
        std::cout << std::format("Gecacht: {} Tracks. Aktive Parameter zum Optimieren: {}\n\n", 
                                 trainingData.size(), activeParameters.size());

        // ---------------------------------------------------------
        // HELPER: EVALUATION FUNCTION
        // ---------------------------------------------------------
        // Berechnet die aktuelle Mean Jaccard über alle Trainingsdaten
        auto evaluateMeanJaccard = [&]() -> double {
            double jaccardSum = 0;
            uint32_t count = 0;
            
            for(const auto &track : trainingData) {
                std::vector<Edge *> edgeSet;
                for(uint32_t i = 0; i < track.ids.size(); i += DISTANCE) {
                    auto result = router.aStarEdges(track.ids[i], track.ids[std::min<uint32_t>(i + DISTANCE, track.ids.size() - 1)], true);
                    edgeSet.insert(edgeSet.end(), result.begin(), result.end());
                }
                edgeSet.emplace_back(router.getQuadtree().getClosestEdges(track.startPoint)[0].edge);
                edgeSet.emplace_back(router.getQuadtree().getClosestEdges(track.endPoint)[0].edge);
                routes.prepareEdgeSet(edgeSet);

                jaccardSum += routes.getJaccardCoefficient(edgeSet, track.matchSet);
                count++;
            }
            return count > 0 ? (jaccardSum / count) : 0.0;
        };

        // ---------------------------------------------------------
        // PHASE 2 & 3: TRAINING LOOP (Numeric Gradient Ascent)
        // ---------------------------------------------------------
        const int EPOCHS = 50; 
        const double LEARNING_RATE = 0.5; // Kann höher sein, da Gradient oft klein ist
        const double DELTA = 0.05;        // Die kleine Störung für die numerische Ableitung

        // Basis-Jaccard vor der ersten Epoche
        double currentBaselineJaccard = evaluateMeanJaccard();
        std::cout << std::format("Start Mean Jaccard (vor Training): {:.4f}\n\n", currentBaselineJaccard);

        for (int epoch = 1; epoch <= EPOCHS; ++epoch)
        {
            std::cout << std::format("=== Starte Epoche {}/{} ===\n", epoch, EPOCHS);

            // Log-Datei vorbereiten
            std::string logPath = std::format("logs/weights_epoch_{:03d}.csv", epoch);
            std::ofstream file(logPath);
            file << "#parameter,weight\n";

            for(const auto& [key, value] : activeParameters)
            {
                double originalWeight = router.getWeights().getWeight(key, value);
                
                // 1. Gewicht minimal verändern (stören)
                router.getWeights().setWeight(key, value, originalWeight + DELTA);
                
                // 2. Auswirkungen evaluieren (ACHTUNG: Das rechnet A* für alle Tracks neu!)
                double newJaccard = evaluateMeanJaccard();
                
                // 3. Numerischen Gradienten berechnen: (J_neu - J_alt) / Delta
                double gradient = (newJaccard - currentBaselineJaccard) / DELTA;
                
                // Gewicht zurücksetzen
                router.getWeights().setWeight(key, value, originalWeight);

                // 4. Update-Regel anwenden (Wir addieren, da Jaccard maximiert werden soll!)
                // Wenn Gradient positiv (höheres Gewicht = besseres Jaccard), erhöhen wir das Gewicht.
                double newWeight = std::max(0.0, originalWeight + (LEARNING_RATE * gradient));
                
                // 5. Neues Gewicht endgültig setzen
                router.getWeights().setWeight(key, value, newWeight);
                
                // 6. Neue Baseline für den nächsten Parameter setzen 
                // (Damit der nächste Parameter vom bereits verbesserten Zustand ausgeht)
                if (newWeight != originalWeight) {
                    currentBaselineJaccard = evaluateMeanJaccard();
                }

                file << std::format("{},{},{}\n", key, value, newWeight);
                
                // Optional: Output pro Parameter, um den Fortschritt zu sehen
                // std::cout << std::format("  -> {}: {} | Grad: {:.4f} | W: {:.2f} -> {:.2f}\n", 
                //                          key, value, gradient, originalWeight, newWeight);
            }
            
            file.close();
            std::cout << std::format("Epoche {} beendet. Aktueller Mean Jaccard: {:.4f}\n", epoch, currentBaselineJaccard);
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