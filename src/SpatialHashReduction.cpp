#include "SpatialHashReduction.h"
#include <numeric>
#include <cmath>
#include <unordered_map>
#include <algorithm>

DisjointSet::DisjointSet(int n) {
    parent.resize(n);
    std::iota(parent.begin(), parent.end(), 0);
}

int DisjointSet::find(int i) {
    if (parent[i] == i)
        return i;
    return parent[i] = find(parent[i]);
}

void DisjointSet::unite(int i, int j) {
    int root_i = find(i);
    int root_j = find(j);
    if (root_i != root_j) {
        parent[root_i] = root_j;

    }
}

    std::vector<Point> SpatialHashReduction::reduce(const std::vector<Point>& points, double eps) {
        if (points.empty()) return {};

        int n = points.size();
        DisjointSet dsu(n);
        std::unordered_map<CellKey, std::vector<int>, CellHash> grid;

        // Przypisanie do siatki
        for (int i = 0; i < n; ++i) {
            int cx = static_cast<int>(std::floor(points[i].x / eps));
            int cy = static_cast<int>(std::floor(points[i].y / eps));
            grid[{cx, cy}].push_back(i);
        }

        // Sprawdzanie sąsiedztwa i DSU
        for (const auto& pair : grid) {
            const CellKey& cell = pair.first;
            const auto& currentCellPoints = pair.second;

            for (int dx = -1; dx <= 1; ++dx) {
                for (int dy = -1; dy <= 1; ++dy) {
                    CellKey neighborCell = {cell.x + dx, cell.y + dy};
                    auto it = grid.find(neighborCell);
                    if (it != grid.end()) {
                        const auto& neighborPoints = it->second;

                        for (int p1Idx : currentCellPoints) {
                            for (int p2Idx : neighborPoints) {
                                if (p1Idx >= p2Idx) continue;

                                double distSq = calculateDistanceSquared(points[p1Idx], points[p2Idx]);

                                // Porównujemy kwadrat odległości z kwadratem progu
                                if (distSq <= eps * eps) {
                                    dsu.unite(p1Idx, p2Idx);
                                }
                            }
                        }
                    }
                }
            }
        }

        // Grupowanie klastrów z DSU
        std::unordered_map<int, std::vector<int>> clusters;
        for (int i = 0; i < n; ++i) {
            int root = dsu.find(i);
            clusters[root].push_back(i);
        }

        // Maksymalne oryginalne ID
        int maxOriginalId = 0;
        for (const auto& p : points) {
            int oid = p.originalIds.empty() ? p.id : p.originalIds[0];
            if (oid > maxOriginalId) maxOriginalId = oid;
        }

        // 4. Obliczanie centroidów i przypisanie ID
        std::vector<Point> reducedPoints;
        int nextNewId = maxOriginalId + 1;

        for (const auto& pair : clusters) {
            const auto& indices = pair.second;
            double sumX = 0.0, sumY = 0.0;
            std::vector<int> combinedOriginalIds;

            for (int idx : indices) {
                sumX += points[idx].x;
                sumY += points[idx].y;
                int origId = (points[idx].originalIds.empty()) ? points[idx].id : points[idx].originalIds[0];
                combinedOriginalIds.push_back(origId);
            }

            // Sortujemy oryginalne ID
            std::sort(combinedOriginalIds.begin(), combinedOriginalIds.end());

            size_t count = indices.size();
            int assignedId;
            if (count == 1) {
                assignedId = combinedOriginalIds[0];
                reducedPoints.push_back({
                assignedId,
                points[indices[0]].x,
                points[indices[0]].y,
                combinedOriginalIds
            });
            } else {
                assignedId = nextNewId++;

                reducedPoints.push_back({
                    assignedId,
                    sumX / count,
                    sumY / count,
                    combinedOriginalIds
                });
            }
        }

        return reducedPoints;
    }