#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>
#include <limits>

struct Point { double x, y; };
struct Triangle { Point a, b, c; };

bool isConvex(Point prev, Point curr, Point next) {
    return (curr.x - prev.x) * (next.y - curr.y) - (curr.y - prev.y) * (next.x - curr.x) > 0;
}

bool pointInTriangle(Point p, Point a, Point b, Point c) {
    auto sign = [](Point p1, Point p2, Point p3) {
        return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
    };
    bool b1 = sign(p, a, b) < 0.0f;
    bool b2 = sign(p, b, c) < 0.0f;
    bool b3 = sign(p, c, a) < 0.0f;
    return ((b1 == b2) && (b2 == b3));
}

bool isCounterClockwise(const std::vector<Point>& polygon) {
    double sum = 0.0;
    for (size_t i = 0; i < polygon.size(); i++) {
        size_t next = (i + 1) % polygon.size();
        sum += (polygon[next].x - polygon[i].x) * (polygon[next].y + polygon[i].y);
    }
    return sum < 0;
}

std::vector<Triangle> triangulate(std::vector<Point> polygon) {
    std::vector<Triangle> triangles;
    if (polygon.size() < 3) return triangles;

    while (polygon.size() > 3) {
        bool earFound = false;
        int n = polygon.size();
        
        for (int i = 0; i < n; i++) {
            int prevIdx = (i - 1 + n) % n;
            int nextIdx = (i + 1) % n;
            
            if (isConvex(polygon[prevIdx], polygon[i], polygon[nextIdx])) {
                bool isEar = true;
                for (int j = 0; j < n; j++) {
                    if (j == prevIdx || j == i || j == nextIdx) continue;
                    if (pointInTriangle(polygon[j], polygon[prevIdx], polygon[i], polygon[nextIdx])) {
                        isEar = false;
                        break;
                    }
                }
                
                if (isEar) {
                    triangles.push_back({polygon[prevIdx], polygon[i], polygon[nextIdx]});
                    polygon.erase(polygon.begin() + i);
                    earFound = true;
                    break;
                }
            }
        }
        if (!earFound) break;
    }
    
    if (polygon.size() == 3) {
        triangles.push_back({polygon[0], polygon[1], polygon[2]});
    }
    return triangles;
}

void exportSVG(const std::vector<Point>& polygon, const std::vector<Triangle>& triangles, const std::string& filename) {
    if (polygon.empty()) return;

    double minX = polygon[0].x, maxX = polygon[0].x;
    double minY = polygon[0].y, maxY = polygon[0].y;
    for (const auto& p : polygon) {
        minX = std::min(minX, p.x); maxX = std::max(maxX, p.x);
        minY = std::min(minY, p.y); maxY = std::max(maxY, p.y);
    }

    double width = maxX - minX;
    double height = maxY - minY;
    double padX = width * 0.1;
    double padY = height * 0.1;

    std::ofstream file(filename);
    file << "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"" 
         << (minX - padX) << " " << (minY - padY) << " " 
         << (width + 2 * padX) << " " << (height + 2 * padY) << "\">\n";

    file << "<rect width=\"100%\" height=\"100%\" fill=\"white\"/>\n";

    double strokeW = std::max(width, height) * 0.005;

    for (const auto& t : triangles) {
        file << "<polygon points=\"" << t.a.x << "," << t.a.y << " " 
             << t.b.x << "," << t.b.y << " " << t.c.x << "," << t.c.y
             << "\" style=\"fill:#ADD8E6;stroke:navy;stroke-width:" << strokeW << "\" />\n";
    }

    file << "<polygon points=\"";
    for (const auto& p : polygon) file << p.x << "," << p.y << " ";
    file << "\" style=\"fill:none;stroke:red;stroke-width:" << (strokeW * 2) << "\" />\n";

    file << "</svg>\n";
    file.close();
    std::cout << "\nSuccess! Visual output saved to: " << filename << std::endl;
}

int main() {
    int numVertices;
    std::cout << "Enter the number of vertices in your polygon: ";
    if (!(std::cin >> numVertices) || numVertices < 3) {
        std::cerr << "A polygon must have at least 3 vertices.\n";
        return 1;
    }

    std::vector<Point> polygon(numVertices);
    std::cout << "Enter the X and Y coordinates for each vertex in order (separated by space):\n";
    for (int i = 0; i < numVertices; i++) {
        std::cout << "Vertex " << (i + 1) << ": ";
        std::cin >> polygon[i].x >> polygon[i].y;
    }

    if (!isCounterClockwise(polygon)) {
        std::cout << "[System] Clockwise input detected. Reversing point order to Counter-Clockwise for valid triangulation...\n";
        std::reverse(polygon.begin(), polygon.end());
    }

    std::vector<Triangle> resultingTriangles = triangulate(polygon);
    exportSVG(polygon, resultingTriangles, "dynamic_triangulation.svg");

    return 0;
}