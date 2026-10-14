#include "ConvexHull.h"
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

std::vector<Point> convexHull(std::vector<Point> points) {
    std::sort(points.begin(), points.end(), [](const Point& a, const Point& b) {
        if (a.x != b.x) return a.x < b.x;
        return a.y < b.y;
    });

    points.erase(
        std::unique(points.begin(), points.end(), [](const Point& a, const Point& b) {
            return a.x == b.x && a.y == b.y;
        }),
        points.end()
    );

    if (points.size() <= 1) {
        return points;
    }

    std::vector<Point> lower, upper;

    for (const Point& p : points) {
        while (lower.size() >= 2 &&
               cross(lower[lower.size() - 2], lower.back(), p) <= 0) {
            lower.pop_back();
               }
        lower.push_back(p);
    }

    for (int i = static_cast<int>(points.size()) - 1; i >= 0; --i) {
        const Point& p = points[i];
        while (upper.size() >= 2 &&
               cross(upper[upper.size() - 2], upper.back(), p) <= 0) {
            upper.pop_back();
               }
        upper.push_back(p);
    }

    lower.pop_back();
    upper.pop_back();
    lower.insert(lower.end(), upper.begin(), upper.end());

    return lower;
}