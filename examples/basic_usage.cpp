/**
 * @file basic_usage.cpp
 * @brief Basic example of using GridIndex2D
 */

#include <iostream>
#include <vector>
#include <iomanip>
#include "../include/grid_index.h"

// Simple 2D point structure
struct Point {
    float x, y;
    int id;

    Point(float x_, float y_, int id_) : x(x_), y(y_), id(id_) {}
};

int main() {
    std::cout << "GridIndex2D Basic Usage Example\n";
    std::cout << "================================\n\n";

    // Create some sample points
    std::vector<Point> points = {
        Point(10.5f, 20.3f, 100),
        Point(10.8f, 20.7f, 101),
        Point(15.2f, 25.1f, 102),
        Point(30.0f, 40.0f, 103),
        Point(45.5f, 50.2f, 104),
        Point(11.2f, 21.5f, 105),
        Point(10.1f, 20.1f, 106),
        Point(50.0f, 50.0f, 107)
    };

    std::cout << "Sample points:\n";
    for (size_t i = 0; i < points.size(); ++i) {
        std::cout << "  [" << i << "] id=" << points[i].id
                  << " at (" << std::fixed << std::setprecision(1)
                  << points[i].x << ", " << points[i].y << ")\n";
    }
    std::cout << "\n";

    // Create grid index
    // Grid covers [0, 100] x [0, 100] with cell size 5x5
    GridIndex2D<float> grid(
        0.0f, 100.0f, 5.0f,  // x: [0, 100], step 5
        0.0f, 100.0f, 5.0f   // y: [0, 100], step 5
    );

    // Insert all points into the grid
    for (size_t i = 0; i < points.size(); ++i) {
        grid.insert(points[i].x, points[i].y, i);
    }

    int nx, ny;
    grid.get_dimensions(nx, ny);
    std::cout << "Grid created: " << nx << " x " << ny << " cells\n";
    std::cout << "Total cells: " << grid.get_num_cells() << "\n";
    std::cout << "Points indexed: " << grid.get_num_points() << "\n\n";

    // Example 1: Query a small box
    std::cout << "Example 1: Query box [10, 12] x [20, 22]\n";
    std::cout << "-------------------------------------------\n";
    auto indices1 = grid.query_box(10.0f, 12.0f, 20.0f, 22.0f);
    std::cout << "Found " << indices1.size() << " points:\n";
    for (size_t idx : indices1) {
        std::cout << "  [" << idx << "] id=" << points[idx].id
                  << " at (" << points[idx].x << ", " << points[idx].y << ")\n";
    }
    std::cout << "\n";

    // Example 2: Query a larger box
    std::cout << "Example 2: Query box [10, 50] x [20, 50]\n";
    std::cout << "-------------------------------------------\n";
    auto indices2 = grid.query_box(10.0f, 50.0f, 20.0f, 50.0f);
    std::cout << "Found " << indices2.size() << " points:\n";
    for (size_t idx : indices2) {
        std::cout << "  [" << idx << "] id=" << points[idx].id
                  << " at (" << points[idx].x << ", " << points[idx].y << ")\n";
    }
    std::cout << "\n";

    // Example 3: Query using callback
    std::cout << "Example 3: Query box [40, 60] x [40, 60] with callback\n";
    std::cout << "--------------------------------------------------------\n";
    int count = 0;
    grid.query_box_callback(40.0f, 60.0f, 40.0f, 60.0f, [&](size_t idx) {
        std::cout << "  Callback received: [" << idx << "] id=" << points[idx].id
                  << " at (" << points[idx].x << ", " << points[idx].y << ")\n";
        count++;
    });
    std::cout << "Total points processed: " << count << "\n\n";

    // Example 4: Empty query
    std::cout << "Example 4: Query empty region [0, 5] x [0, 5]\n";
    std::cout << "----------------------------------------------\n";
    auto indices4 = grid.query_box(0.0f, 5.0f, 0.0f, 5.0f);
    std::cout << "Found " << indices4.size() << " points\n\n";

    return 0;
}
