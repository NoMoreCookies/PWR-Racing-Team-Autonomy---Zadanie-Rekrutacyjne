#include "GrahamHull.h"
#include "Point.h"
#include <algorithm>

std::vector<Point> grahamScanHull(std::vector<Point> points) {
    if (points.size() < 3) return points;

    // punkt wysunięty najbardziej w dół
    int minYIdx = 0;
    for (size_t i = 1; i < points.size(); ++i) {
        if (points[i].y < points[minYIdx].y ||
            (points[i].y == points[minYIdx].y && points[i].x < points[minYIdx].x)) {
            minYIdx = i;
            }
    }

    // Punkt początkowy
    std::swap(points[0], points[minYIdx]);
    Point anchor = points[0];

    // Sortowanie
    std::sort(points.begin() + 1, points.end(), [anchor](const Point& p1, const Point& p2) {
        long long cp = cross(anchor, p1, p2);
        if (cp == 0) {
            return calculateDistanceSquared(anchor, p1) < calculateDistanceSquared(anchor, p2);
        }
        return cp > 0;
    });

    // 3. Usuwam punkty o tym samym kącie (zostawiając najdalej położony - bo tylko on może leżeć na otoczce)
    std::vector<Point> filtered;
    filtered.push_back(points[0]);
    for (size_t i = 1; i < points.size(); ++i) {
        while (i < points.size() - 1 && cross(anchor, points[i], points[i + 1]) == 0) {
            i++;
        }
        filtered.push_back(points[i]);
    }

    if (filtered.size() < 3) return filtered;

    // Algorytm Graham oparty na stosie
    std::vector<Point> hull;
    hull.push_back(filtered[0]);
    hull.push_back(filtered[1]);
    hull.push_back(filtered[2]);

    for (size_t i = 3; i < filtered.size(); ++i) {
        while (hull.size() >= 2 && cross(hull[hull.size() - 2], hull.back(), filtered[i]) <= 0) {
            hull.pop_back();
        }
        hull.push_back(filtered[i]);
    }

    return hull;
}