#include "Greedy.h"
#include <iostream>
#include <cmath>
#include <algorithm>

static double calculateDistance(const Point& p1, const Point& p2) {
    std::cout << "DEBUG Greedy: p1=(" << p1.x << "," << p1.y << "), p2=(" << p2.x << "," << p2.y << ")\n";
    return std::sqrt(std::pow(p1.x - p2.x, 2) +
                     std::pow(p1.y - p2.y, 2));
}

std::vector<Point> GreedyClusterer::run(const std::vector<Point>& points, double threshold) {
    std::vector<Point> currentPoints = points;

    // Inicjalizacja: każdy punkt początkowo zawiera sam siebie w originalIds
    for (auto& pt : currentPoints) {
        if (pt.originalIds.empty()) {
            pt.originalIds = {pt.id};
        }
    }

    bool mergedAny = true;

    while (mergedAny) {
        mergedAny = false;
        std::vector<Point> nextPoints;
        std::vector<bool> merged(currentPoints.size(), false);

        for (size_t i = 0; i < currentPoints.size(); ++i) {
            if (merged[i]) continue;

            Point current = currentPoints[i];

            for (size_t j = i + 1; j < currentPoints.size(); ++j) {
                if (!merged[j]) {
                    double dist = calculateDistance(current, currentPoints[j]);
                    if (dist <= threshold) {
                        // Uśredniamy pozycję
                        double totalCount = current.originalIds.size() + currentPoints[j].originalIds.size();
                        current.x = (current.x * current.originalIds.size() + currentPoints[j].x * currentPoints[j].originalIds.size()) / totalCount;
                        current.y = (current.y * current.originalIds.size() + currentPoints[j].y * currentPoints[j].originalIds.size()) / totalCount;

                        // Łączymy listy oryginalnych ID
                        current.originalIds.insert(
                            current.originalIds.end(),
                            currentPoints[j].originalIds.begin(),
                            currentPoints[j].originalIds.end()
                        );

                        merged[j] = true;
                        mergedAny = true;
                    }
                }
            }
            nextPoints.push_back(current);
        }
        currentPoints = nextPoints;
    }

    // Znajdujemy maksymalne oryginalne ID w całym zbiorze wejściowym, aby nowe ID zaczynały się od niego
    int maxOriginalId = 0;
    for (const auto& pt : points) {
        for (int oid : pt.originalIds) {
            if (oid > maxOriginalId) maxOriginalId = oid;
        }
    }

    int nextNewId = maxOriginalId + 1;

    // Przypisanie ID zgodnie z zasadą: pojedyncze zachowują stare ID, połączone dostają nowe
    for (auto& pt : currentPoints) {
        if (pt.originalIds.size() == 1) {
            pt.id = pt.originalIds[0]; // Zachowaj oryginalne ID
        } else {
            pt.id = nextNewId++;       // Nowe ID dla połączonych
        }
    }

    std::cout << "Greedy zakończony. Przed: " << points.size() << ", Po: " << currentPoints.size() << "\n";
    return currentPoints;
}