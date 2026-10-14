#include "GrahamHull.h"
#include <algorithm>

static long long cross(const Point& A, const Point& B, const Point& C) {
    long long ax = static_cast<long long>(A.x);
    long long ay = static_cast<long long>(A.y);
    long long bx = static_cast<long long>(B.x);
    long long by = static_cast<long long>(B.y);
    long long cx = static_cast<long long>(C.x);
    long long cy = static_cast<long long>(C.y);

    return (bx - ax) * (cy - ay) - (by - ay) * (cx - ax);
}

static long long distSq(const Point& p1, const Point& p2) {
    long long dx = p1.x - p2.x;
    long long dy = p1.y - p2.y;
    return dx * dx + dy * dy;
}

static Point anchorPoint;

static bool comparePoints(const Point& p1, const Point& p2) {
    long long cp = cross(anchorPoint, p1, p2);
    if (cp == 0) {
        // Jeśli punkty współliniowe, bliższy jest "mniejszy"
        return distSq(anchorPoint, p1) < distSq(anchorPoint, p2);
    }
    return cp > 0; // Kierunek przeciwny do wskazówek zegara
}

std::vector<Point> grahamScanHull(std::vector<Point> points) {
    if (points.size() < 3) return points;

    // 1. Znajdź punkt wysunięty najbardziej w dół (a w przypadku remisu najbardziej w lewo)
    int minYIdx = 0;
    for (size_t i = 1; i < points.size(); ++i) {
        if (points[i].y < points[minYIdx].y || 
            (points[i].y == points[minYIdx].y && points[i].x < points[minYIdx].x)) {
            minYIdx = i;
        }
    }

    // Przenieś punkt początkowy na początek wektora
    std::swap(points[0], points[minYIdx]);
    anchorPoint = points[0];

    // 2. Posortuj pozostałe punkty według kąta biegunowego względem punktu bazowego
    std::sort(points.begin() + 1, points.end(), comparePoints);

    // 3. Usuń punkty o tym samym kącie (zostawiając najdalej położony)
    std::vector<Point> filtered;
    filtered.push_back(points[0]);
    for (size_t i = 1; i < points.size(); ++i) {
        while (i < points.size() - 1 && cross(anchorPoint, points[i], points[i + 1]) == 0) {
            i++;
        }
        filtered.push_back(points[i]);
    }

    if (filtered.size() < 3) return filtered;

    // 4. Algorytm Grahama oparty na stosie
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