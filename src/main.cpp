#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <cmath>
#include "Point.h"
#include "Greedy.h"
#include "SpatialHashReduction.h"
#include "ConvexHull.h"
#include "GrahamHull.h"

// Funkcja pomocnicza do obliczania obwodu otoczki dla wpisu ENCLOSURE
double calculatePerimeter(const std::vector<Point>& hull) {
    if (hull.size() < 2) return 0.0;
    double perimeter = 0.0;
    for (size_t i = 0; i < hull.size(); ++i) {
        const auto& p1 = hull[i];
        const auto& p2 = hull[(i + 1) % hull.size()];
        perimeter += std::hypot(p2.x - p1.x, p2.y - p1.y);
    }
    return perimeter;
}

// Funkcja pomocnicza do zapisu w formacie CONE + ENCLOSURE
void saveResultsToFile(const std::string& filename, const std::vector<Point>& reducedPoints, const std::vector<Point>& hull) {
    std::ofstream outfile(filename);
    outfile << std::fixed << std::setprecision(3);

    // 1. Zapis pachołków
    for (const auto& p : reducedPoints) {
        outfile << "CONE " << p.id << " " << p.x << " " << p.y << " [";
        for (size_t i = 0; i < p.originalIds.size(); ++i) {
            outfile << p.originalIds[i];
            if (i + 1 < p.originalIds.size()) {
                outfile << ",";
            }
        }
        outfile << "]\n";
    }

    // 2. Zapis otoczki wypukłej
    double perimeter = calculatePerimeter(hull);
    outfile << "ENCLOSURE " << perimeter << " [";
    for (size_t i = 0; i < hull.size(); ++i) {
        outfile << hull[i].id;
        if (i + 1 < hull.size()) {
            outfile << ",";
        }
    }
    outfile << "]\n";

    outfile.close();
}

int main() {
    double threshold = 0.5; // Próg (eps) dla redukcji
    std::string inputPath = "data/DaneDoZadan/zad1/example_input.txt";

    // Wczytanie danych z pliku (threshold jest aktualizowany wewnątrz przez loadPointsFromFile, jeśli taka jest jego logika)
    auto points = loadPointsFromFile(inputPath, threshold);
    if (points.empty()) {
        std::cerr << "Błąd wczytywania pliku lub pusta lista!\n";
        return 1;
    }

    // METODĄ GREEDY
    GreedyClusterer greedy;
    std::vector<Point> greedyPoints = greedy.run(points, threshold);

    // METODĄ SPATIAL HASH
    SpatialHashReduction spatialReducer;
    std::vector<Point> spatialPoints = spatialReducer.reduce(points, threshold);


    //OBLICZANIE OTOCZEK

    // Greedy + Convex Hull
    std::vector<Point> hullGreedy = convexHull(greedyPoints);
    saveResultsToFile("output_greedy_convexhull.txt", greedyPoints, hullGreedy);

    // Spatial Hash + Convex Hull
    std::vector<Point> hullSpatial = convexHull(spatialPoints);
    saveResultsToFile("output_spatial_convexhull.txt", spatialPoints, hullSpatial);

    // Greedy + Graham Scan
    std::vector<Point> grahamGreedy = grahamScanHull(greedyPoints);
    saveResultsToFile("output_greedy_graham.txt", greedyPoints, grahamGreedy);

    // Spatial Hash + Graham Scan
    std::vector<Point> grahamSpatial = grahamScanHull(spatialPoints);
    saveResultsToFile("output_spatial_graham.txt", spatialPoints, grahamSpatial);

    std::cout << "Wszystkie 4 metody wykonane pomyślnie! Wyniki zapisano w wymaganym formacie do plików.\n";

    return 0;
}