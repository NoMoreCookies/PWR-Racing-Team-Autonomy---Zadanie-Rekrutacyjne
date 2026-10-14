#include "Point.h"
#include <fstream>
#include <iostream>

std::vector<Point> loadPointsFromFile(const std::string& filepath, double& outThreshold) {
    std::vector<Point> points;
    std::ifstream file(filepath);

    if (!file.is_open()) {
        std::cerr << "Błąd: Nie można otworzyć pliku: " << filepath << "\n";
        return points;
    }

    int n = 0;
    file >> n;
    file >> outThreshold;
    points.reserve(n);

    for (int i = 0; i < n; ++i) {
        Point p;
        file >> p.id >> p.x >> p.y;
        p.originalIds = {p.id};
        points.push_back(p);
    }

    file.close();
    return points;
}