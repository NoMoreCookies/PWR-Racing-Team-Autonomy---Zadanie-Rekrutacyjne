#include <iostream>
#include <fstream>
#include <vector>
#include "Point.h"
#include "Greedy.h"
#include "SpatialHashReduction.h"
#include "ConvexHull.h"
#include "GrahamHull.h"

int main() {
    double threshold = 0.0;
    std::string inputPath = "data/DaneDoZadań/zad1/example_input.txt";

    auto points = loadPointsFromFile(inputPath, threshold);
    if (points.empty()) {
        std::cerr << "Błąd wczytywania pliku lub pusta lista!\n";
        return 1;
    }

    std::cout << "Wczytano punktów: " << points.size() << ", eps: " << threshold << "\n";

    // --- GREEDY ---
    GreedyClusterer greedy;
    std::vector<Point> greedyPoints = greedy.run(points, threshold);

    std::vector<Point> greedyMergedOnly;
    for (const auto& p : greedyPoints) {
        if (p.originalIds.size() > 1) {
            greedyMergedOnly.push_back(p);
        }
    }

    std::ofstream greedyFile("output_greedy.txt");
    greedyFile << greedyMergedOnly.size() << "\n";
    for (const auto& p : greedyMergedOnly) {
        greedyFile << p.id << " " << p.x << " " << p.y << " " << p.originalIds.size();
        for (int oldId : p.originalIds) {
            greedyFile << " " << oldId;
        }
        greedyFile << "\n";
    }
    greedyFile.close();

    // --- SPATIAL HASH ---
    SpatialHashReduction spatialReducer;
    std::vector<Point> spatialPoints = spatialReducer.reduce(points, threshold);

    std::vector<Point> spatialMergedOnly;
    for (const auto& p : spatialPoints) {
        if (p.originalIds.size() > 1) {
            spatialMergedOnly.push_back(p);
        }
    }

    std::ofstream spatialFile("output_spatial.txt");
    spatialFile << spatialMergedOnly.size() << "\n";
    for (const auto& p : spatialMergedOnly) {
        spatialFile << p.id << " " << p.x << " " << p.y << " " << p.originalIds.size();
        for (int oldId : p.originalIds) {
            spatialFile << " " << oldId;
        }
        spatialFile << "\n";
    }
    spatialFile.close();

    // --- CONVEX HULL dla Greedy (wszystkie punkty po redukcji: pojedyncze + klastry) ---
    std::vector<Point> hullGreedy = convexHull(greedyPoints);

    std::ofstream hullGreedyFile("output_hull_greedy.txt");
    hullGreedyFile << hullGreedy.size() << "\n";
    for (const auto& p : hullGreedy) {
        hullGreedyFile << p.id << " " << p.x << " " << p.y << "\n";
    }
    hullGreedyFile.close();

    // --- CONVEX HULL dla Spatial Hash (wszystkie punkty po redukcji) ---
    std::vector<Point> hullSpatial = convexHull(spatialPoints);

    std::ofstream hullSpatialFile("output_hull_spatial.txt");
    hullSpatialFile << hullSpatial.size() << "\n";
    for (const auto& p : hullSpatial) {
        hullSpatialFile << p.id << " " << p.x << " " << p.y << "\n";
    }
    hullSpatialFile.close();
    return 0;
}