/**
 * @file performance_example.cpp
 * @brief Performance demonstration of GridIndex2D vs naive search
 */

#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include <iomanip>
#include "../include/grid_index.h"

struct Point {
    double x, y;
    int data;
};

// Naive O(n) search for comparison
std::vector<size_t> naive_box_search(const std::vector<Point>& points,
                                      double x1, double x2,
                                      double y1, double y2) {
    std::vector<size_t> result;
    for (size_t i = 0; i < points.size(); ++i) {
        if (points[i].x >= x1 && points[i].x <= x2 &&
            points[i].y >= y1 && points[i].y <= y2) {
            result.push_back(i);
        }
    }
    return result;
}

int main() {
    std::cout << "GridIndex2D Performance Test\n";
    std::cout << "=============================\n\n";

    // Generate random points
    const size_t NUM_POINTS = 100000;
    const double GRID_SIZE = 1000.0;

    std::cout << "Generating " << NUM_POINTS << " random points...\n";

    std::random_device rd;
    std::mt19937 gen(42);  // Fixed seed for reproducibility
    std::uniform_real_distribution<double> dist(0.0, GRID_SIZE);

    std::vector<Point> points;
    points.reserve(NUM_POINTS);
    for (size_t i = 0; i < NUM_POINTS; ++i) {
        points.push_back({dist(gen), dist(gen), static_cast<int>(i)});
    }
    std::cout << "Done!\n\n";

    // Build grid index
    std::cout << "Building grid index...\n";
    auto build_start = std::chrono::high_resolution_clock::now();

    GridIndex2D<double> grid(0.0, GRID_SIZE, 10.0,   // 100 cells per dimension
                             0.0, GRID_SIZE, 10.0);

    for (size_t i = 0; i < points.size(); ++i) {
        grid.insert(points[i].x, points[i].y, i);
    }

    auto build_end = std::chrono::high_resolution_clock::now();
    auto build_time = std::chrono::duration_cast<std::chrono::milliseconds>(
        build_end - build_start).count();

    int nx, ny;
    grid.get_dimensions(nx, ny);
    std::cout << "Grid built: " << nx << " x " << ny << " cells\n";
    std::cout << "Build time: " << build_time << " ms\n\n";

    // Test queries
    const int NUM_QUERIES = 1000;
    std::cout << "Running " << NUM_QUERIES << " queries...\n\n";

    // Small box queries (10x10)
    std::cout << "Small box queries (10x10):\n";
    std::cout << "--------------------------\n";

    auto grid_start = std::chrono::high_resolution_clock::now();
    size_t total_found_grid = 0;
    for (int i = 0; i < NUM_QUERIES; ++i) {
        double x = dist(gen);
        double y = dist(gen);
        auto result = grid.query_box(x, x + 10.0, y, y + 10.0);
        total_found_grid += result.size();
    }
    auto grid_end = std::chrono::high_resolution_clock::now();
    auto grid_time = std::chrono::duration_cast<std::chrono::microseconds>(
        grid_end - grid_start).count();

    auto naive_start = std::chrono::high_resolution_clock::now();
    size_t total_found_naive = 0;
    for (int i = 0; i < NUM_QUERIES; ++i) {
        double x = dist(gen);
        double y = dist(gen);
        auto result = naive_box_search(points, x, x + 10.0, y, y + 10.0);
        total_found_naive += result.size();
    }
    auto naive_end = std::chrono::high_resolution_clock::now();
    auto naive_time = std::chrono::duration_cast<std::chrono::microseconds>(
        naive_end - naive_start).count();

    std::cout << "Grid Index: " << grid_time << " μs ("
              << std::fixed << std::setprecision(2)
              << (grid_time / (double)NUM_QUERIES) << " μs/query)\n";
    std::cout << "Naive Search: " << naive_time << " μs ("
              << (naive_time / (double)NUM_QUERIES) << " μs/query)\n";
    std::cout << "Speedup: " << std::setprecision(1)
              << (naive_time / (double)grid_time) << "x\n";
    std::cout << "Avg points found: " << (total_found_grid / NUM_QUERIES) << "\n\n";

    // Medium box queries (50x50)
    std::cout << "Medium box queries (50x50):\n";
    std::cout << "----------------------------\n";

    grid_start = std::chrono::high_resolution_clock::now();
    total_found_grid = 0;
    for (int i = 0; i < NUM_QUERIES; ++i) {
        double x = dist(gen);
        double y = dist(gen);
        auto result = grid.query_box(x, x + 50.0, y, y + 50.0);
        total_found_grid += result.size();
    }
    grid_end = std::chrono::high_resolution_clock::now();
    grid_time = std::chrono::duration_cast<std::chrono::microseconds>(
        grid_end - grid_start).count();

    naive_start = std::chrono::high_resolution_clock::now();
    total_found_naive = 0;
    for (int i = 0; i < NUM_QUERIES; ++i) {
        double x = dist(gen);
        double y = dist(gen);
        auto result = naive_box_search(points, x, x + 50.0, y, y + 50.0);
        total_found_naive += result.size();
    }
    naive_end = std::chrono::high_resolution_clock::now();
    naive_time = std::chrono::duration_cast<std::chrono::microseconds>(
        naive_end - naive_start).count();

    std::cout << "Grid Index: " << grid_time << " μs ("
              << std::fixed << std::setprecision(2)
              << (grid_time / (double)NUM_QUERIES) << " μs/query)\n";
    std::cout << "Naive Search: " << naive_time << " μs ("
              << (naive_time / (double)NUM_QUERIES) << " μs/query)\n";
    std::cout << "Speedup: " << std::setprecision(1)
              << (naive_time / (double)grid_time) << "x\n";
    std::cout << "Avg points found: " << (total_found_grid / NUM_QUERIES) << "\n\n";

    // Callback performance test
    std::cout << "Callback vs vector return (small box):\n";
    std::cout << "---------------------------------------\n";

    auto callback_start = std::chrono::high_resolution_clock::now();
    size_t callback_count = 0;
    for (int i = 0; i < NUM_QUERIES; ++i) {
        double x = dist(gen);
        double y = dist(gen);
        grid.query_box_callback(x, x + 10.0, y, y + 10.0, [&](size_t idx) {
            callback_count++;
        });
    }
    auto callback_end = std::chrono::high_resolution_clock::now();
    auto callback_time = std::chrono::duration_cast<std::chrono::microseconds>(
        callback_end - callback_start).count();

    auto vector_start = std::chrono::high_resolution_clock::now();
    size_t vector_count = 0;
    for (int i = 0; i < NUM_QUERIES; ++i) {
        double x = dist(gen);
        double y = dist(gen);
        auto result = grid.query_box(x, x + 10.0, y, y + 10.0);
        vector_count += result.size();
    }
    auto vector_end = std::chrono::high_resolution_clock::now();
    auto vector_time = std::chrono::duration_cast<std::chrono::microseconds>(
        vector_end - vector_start).count();

    std::cout << "Callback: " << callback_time << " μs ("
              << std::fixed << std::setprecision(2)
              << (callback_time / (double)NUM_QUERIES) << " μs/query)\n";
    std::cout << "Vector: " << vector_time << " μs ("
              << (vector_time / (double)NUM_QUERIES) << " μs/query)\n";
    std::cout << "Callback advantage: " << std::setprecision(1)
              << (vector_time / (double)callback_time) << "x\n\n";

    return 0;
}
