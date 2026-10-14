#pragma once
#include "Point.h"
#include <vector>

class GreedyClusterer {
public:
    // Uruchamia algorytm zachłanny i zwraca zredukowaną listę punktów
    std::vector<Point> run(const std::vector<Point>& points, double threshold);
};