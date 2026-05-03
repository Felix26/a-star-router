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
        // PHASE 2 & 3: GREEDY HILL CLIMBING (Coordinate Ascent)
        // ---------------------------------------------------------
        const int EPOCHS = 50; 
        const double DELTA = 0.05; // Feste, kleine Schrittweite (keine Lernrate mehr nötig!)

        double bestJaccard = evaluateMeanJaccard();
        std::cout << std::format("Start Mean Jaccard (vor Training): {:.4f}\n\n", bestJaccard);

        for (int epoch = 1; epoch <= EPOCHS; ++epoch)
        {
            std::cout << std::format("=== Starte Epoche {}/{} ===\n", epoch, EPOCHS);
            bool improvedInEpoch = false;

            // Log-Datei vorbereiten
            std::string logPath = std::format("logs/weights_epoch_{:03d}.csv", epoch);
            
            // Wir iterieren über alle Parameter
            for(const auto& [key, value] : activeParameters)
            {
                double originalWeight = router.getWeights().getWeight(key, value);
                
                // --- TEST 1: Gewicht ERHÖHEN ---
                router.getWeights().setWeight(key, value, originalWeight + DELTA);
                double jaccardUp = evaluateMeanJaccard();
                
                // --- TEST 2: Gewicht VERRINGERN ---
                double jaccardDown = 0.0;
                if (originalWeight - DELTA >= 0) { // Gewichte dürfen nicht negativ werden
                    router.getWeights().setWeight(key, value, originalWeight - DELTA);
                    jaccardDown = evaluateMeanJaccard();
                }

                // --- AUSWERTUNG ---
                // Welche der beiden Richtungen war am besten (und besser als unser bisheriges Maximum)?
                if (jaccardUp > bestJaccard && jaccardUp >= jaccardDown) {
                    // Erhöhen war am besten! Wir belassen das Gewicht bei original + DELTA.
                    router.getWeights().setWeight(key, value, originalWeight + DELTA);
                    bestJaccard = jaccardUp;
                    improvedInEpoch = true;
                    // Optional: std::cout << std::format("  [+] {}:{} -> Jaccard: {:.4f}\n", key, value, bestJaccard);
                } 
                else if (jaccardDown > bestJaccard) {
                    // Verringern war am besten! Wir belassen das Gewicht bei original - DELTA.
                    router.getWeights().setWeight(key, value, originalWeight - DELTA);
                    bestJaccard = jaccardDown;
                    improvedInEpoch = true;
                    // Optional: std::cout << std::format("  [-] {}:{} -> Jaccard: {:.4f}\n", key, value, bestJaccard);
                } 
                else {
                    // Keine Richtung hat eine Verbesserung gebracht. Wir setzen das Gewicht zurück.
                    router.getWeights().setWeight(key, value, originalWeight);
                }
            }
            
            // Gewichte am Ende der Epoche speichern
            std::ofstream file(logPath);
            file << "#parameter,weight\n";
            for(const auto& [key, value] : activeParameters) {
                file << std::format("{},{},{}\n", key, value, router.getWeights().getWeight(key, value));
            }
            file.close();

            std::cout << std::format("Epoche {} beendet. Aktueller Mean Jaccard: {:.4f}\n", epoch, bestJaccard);
            
            // Early Stopping: Wenn in einer kompletten Epoche KEIN Parameter mehr eine 
            // Verbesserung gebracht hat, sind wir in einem lokalen Maximum angekommen.
            if (!improvedInEpoch) {
                std::cout << "Keine Verbesserung mehr möglich. Training beendet (Lokales Maximum erreicht).\n";
                break;
            }
            
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