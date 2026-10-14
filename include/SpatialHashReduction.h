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
        return std::hash<int>()(k.x) ^ (std::hash<int>()(k.y) << 1);
    }
};

class SpatialHashReduction {
private:
    double calculateDistance(const Point& p1, const Point& p2);

public:
    std::vector<Point> reduce(const std::vector<Point>& points, double eps);
};