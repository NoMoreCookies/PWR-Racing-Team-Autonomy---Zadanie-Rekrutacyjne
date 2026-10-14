#pragma once
#include <vector>
#include <string>

struct Point {
    int id;
    double x;
    double y;
    std::vector<int> originalIds;
};

std::vector<Point> loadPointsFromFile(const std::string& filepath, double& outThreshold);