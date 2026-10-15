#include "Greedy.h"
#include <iostream>
#include <cmath>
#include <algorithm>

std::vector<Point> GreedyClusterer::run(const std::vector<Point>& points, double threshold) const {
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

                    double distSq = calculateDistanceSquared(current, currentPoints[j]);

                    if (distSq <= threshold * threshold) {

                        // Średnia ważona
                        double sizeCurrent = static_cast<double>(current.originalIds.size());
                        double sizeNeighbor = static_cast<double>(currentPoints[j].originalIds.size());
                        double totalCount = sizeCurrent + sizeNeighbor;

                        current.x = (current.x * sizeCurrent + currentPoints[j].x * sizeNeighbor) / totalCount;
                        current.y = (current.y * sizeCurrent + currentPoints[j].y * sizeNeighbor) / totalCount;

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

    // Maksymalne oryginalne ID w całym zbiorze wejściowym, żeby nowe ID zaczynały się od niego
    int maxOriginalId = 0;
    for (const auto& pt : points) {
        for (int oid : pt.originalIds) {
            if (oid > maxOriginalId) maxOriginalId = oid;
        }
    }

    int nextNewId = maxOriginalId + 1;

    // Połączone punkty dostają nowe ID, pojedyncze zachowują stare
    for (auto& pt : currentPoints) {
        if (pt.originalIds.size() == 1) {
            pt.id = pt.originalIds[0];
        } else {
            pt.id = nextNewId++;
        }
    }

    std::cout << "Greedy zakończony. Przed: " << points.size() << ", Po: " << currentPoints.size() << "\n";
    return currentPoints;
}