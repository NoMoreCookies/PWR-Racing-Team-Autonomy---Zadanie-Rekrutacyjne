#pragma once

#include <vector>
#include <unordered_map>
#include <cmath>
#include <numeric>
#include "Point.h" // Założenie: Point ma pola id, x, y

class DisjointSet {
    std::vector<int> parent;
public:
    DisjointSet(int n);
    int find(int i);
    void unite(int i, int j);
};

// Klucz do komórki w 2D
struct CellKey {
    int x, y;

    bool operator==(const CellKey& other) const {
        return x == other.x && y == other.y;
    }
};

// Funkcja mieszająca dla 2D
struct CellHash {
    std::size_t operator()(const CellKey& k) const {
        std::size_t h1 = std::hash<int>{}(k.x);
        std::size_t h2 = std::hash<int>{}(k.y);

        // Wzór znaleziony w necie na optymalizację funkcji hashującej (minimalizację kolizji)
        return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
    }
};

class SpatialHashReduction {
private:
    double calculateDistance(const Point& p1, const Point& p2);

public:
    std::vector<Point> reduce(const std::vector<Point>& points, double eps);
};