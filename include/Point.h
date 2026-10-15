#pragma once
#include <vector>
#include <string>

struct Point {
    int id;
    double x;
    double y;
    std::vector<int> originalIds;
};

// Deklaracja funkcji wczytującej punkty z pliku
std::vector<Point> loadPointsFromFile(const std::string& filepath, double& outThreshold);

// Wolna funkcja licząca kwadrat odległości
inline double calculateDistanceSquared(const Point& p1, const Point& p2) {
    double dx = p1.x - p2.x;
    double dy = p1.y - p2.y;
    return (dx * dx) + (dy * dy);
}

// Globalny iloczyn wektorowy
inline long long cross(const Point& A, const Point& B, const Point& C) {
    long long ax = static_cast<long long>(A.x);
    long long ay = static_cast<long long>(A.y);
    long long bx = static_cast<long long>(B.x);
    long long by = static_cast<long long>(B.y);
    long long cx = static_cast<long long>(C.x);
    long long cy = static_cast<long long>(C.y);

    return (bx - ax) * (cy - ay) - (by - ay) * (cx - ax);
}