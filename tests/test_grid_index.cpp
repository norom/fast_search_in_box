/**
 * @file test_grid_index.cpp
 * @brief Unit tests for GridIndex2D
 *
 * Simple test suite without external dependencies
 */

#include <iostream>
#include <vector>
#include <cassert>
#include <cmath>
#include <algorithm>
#include "../include/grid_index.h"

#define TEST(name) void name()
#define ASSERT_EQ(a, b) assert((a) == (b))
#define ASSERT_TRUE(cond) assert(cond)
#define ASSERT_THROW(expr, exception) \
    do { \
        bool caught = false; \
        try { expr; } \
        catch (const exception&) { caught = true; } \
        assert(caught); \
    } while(0)

#define RUN_TEST(name) \
    do { \
        std::cout << "Running " << #name << "... "; \
        name(); \
        std::cout << "PASSED\n"; \
        passed++; \
    } while(0)

// Test basic construction
TEST(test_construction) {
    GridIndex2D<float> grid(0.0f, 100.0f, 10.0f,
                            0.0f, 100.0f, 10.0f);

    int nx, ny;
    grid.get_dimensions(nx, ny);
    ASSERT_EQ(nx, 10);
    ASSERT_EQ(ny, 10);
    ASSERT_EQ(grid.get_num_cells(), 100);
    ASSERT_EQ(grid.get_num_points(), 0);
}

// Test invalid parameters
TEST(test_invalid_params) {
    // Zero step
    ASSERT_THROW(
        GridIndex2D<float>(0.0f, 100.0f, 0.0f, 0.0f, 100.0f, 10.0f),
        std::invalid_argument
    );

    // Negative step
    ASSERT_THROW(
        GridIndex2D<float>(0.0f, 100.0f, -1.0f, 0.0f, 100.0f, 10.0f),
        std::invalid_argument
    );

    // Start >= end
    ASSERT_THROW(
        GridIndex2D<float>(100.0f, 0.0f, 10.0f, 0.0f, 100.0f, 10.0f),
        std::invalid_argument
    );
}

// Test single point insertion and query
TEST(test_single_point) {
    GridIndex2D<float> grid(0.0f, 100.0f, 10.0f,
                            0.0f, 100.0f, 10.0f);

    grid.insert(15.0f, 25.0f, 42);
    ASSERT_EQ(grid.get_num_points(), 1);

    // Query box that contains the point
    auto result = grid.query_box(10.0f, 20.0f, 20.0f, 30.0f);
    ASSERT_EQ(result.size(), 1);
    ASSERT_EQ(result[0], 42);

    // Query box that doesn't contain the point
    auto empty = grid.query_box(50.0f, 60.0f, 50.0f, 60.0f);
    ASSERT_EQ(empty.size(), 0);
}

// Test multiple points in same cell
TEST(test_multiple_points_same_cell) {
    GridIndex2D<float> grid(0.0f, 100.0f, 10.0f,
                            0.0f, 100.0f, 10.0f);

    // All points in cell [10, 20) x [20, 30)
    grid.insert(11.0f, 21.0f, 0);
    grid.insert(12.0f, 22.0f, 1);
    grid.insert(13.0f, 23.0f, 2);

    ASSERT_EQ(grid.get_num_points(), 3);

    auto result = grid.query_box(10.0f, 20.0f, 20.0f, 30.0f);
    ASSERT_EQ(result.size(), 3);
}

// Test multiple points in different cells
TEST(test_multiple_points_different_cells) {
    GridIndex2D<float> grid(0.0f, 100.0f, 10.0f,
                            0.0f, 100.0f, 10.0f);

    grid.insert(5.0f, 5.0f, 0);    // Cell (0, 0)
    grid.insert(15.0f, 15.0f, 1);   // Cell (1, 1)
    grid.insert(25.0f, 25.0f, 2);   // Cell (2, 2)
    grid.insert(35.0f, 35.0f, 3);   // Cell (3, 3)

    // Query box [0, 20] x [0, 20] intersects cells (0,0), (1,1), and (2,2)
    // Note: Cell (2,2) covers [20,30) x [20,30), so it's included even though
    // the point at (25,25) is outside the query box. This is expected behavior
    // for grid-based spatial indexing - it returns candidates that may need filtering.
    auto result = grid.query_box(0.0f, 20.0f, 0.0f, 20.0f);
    ASSERT_TRUE(result.size() >= 2);  // At least points 0 and 1

    // Query covering all cells
    auto all = grid.query_box(0.0f, 100.0f, 0.0f, 100.0f);
    ASSERT_EQ(all.size(), 4);
}

// Test boundary conditions
TEST(test_boundaries) {
    GridIndex2D<float> grid(0.0f, 100.0f, 10.0f,
                            0.0f, 100.0f, 10.0f);

    // Points at boundaries
    grid.insert(0.0f, 0.0f, 0);
    grid.insert(100.0f, 100.0f, 1);
    grid.insert(50.0f, 0.0f, 2);
    grid.insert(0.0f, 50.0f, 3);

    ASSERT_EQ(grid.get_num_points(), 4);
}

// Test out-of-bounds points (should be clamped)
TEST(test_out_of_bounds) {
    GridIndex2D<float> grid(0.0f, 100.0f, 10.0f,
                            0.0f, 100.0f, 10.0f);

    // Points outside grid (will be clamped)
    grid.insert(-10.0f, -10.0f, 0);
    grid.insert(200.0f, 200.0f, 1);

    ASSERT_EQ(grid.get_num_points(), 2);
}

// Test callback functionality
TEST(test_callback) {
    GridIndex2D<float> grid(0.0f, 100.0f, 10.0f,
                            0.0f, 100.0f, 10.0f);

    grid.insert(15.0f, 25.0f, 10);
    grid.insert(16.0f, 26.0f, 20);
    grid.insert(17.0f, 27.0f, 30);

    std::vector<size_t> collected;
    grid.query_box_callback(10.0f, 20.0f, 20.0f, 30.0f, [&](size_t idx) {
        collected.push_back(idx);
    });

    ASSERT_EQ(collected.size(), 3);
    ASSERT_TRUE(std::find(collected.begin(), collected.end(), 10) != collected.end());
    ASSERT_TRUE(std::find(collected.begin(), collected.end(), 20) != collected.end());
    ASSERT_TRUE(std::find(collected.begin(), collected.end(), 30) != collected.end());
}

// Test clear functionality
TEST(test_clear) {
    GridIndex2D<float> grid(0.0f, 100.0f, 10.0f,
                            0.0f, 100.0f, 10.0f);

    grid.insert(15.0f, 25.0f, 0);
    grid.insert(25.0f, 35.0f, 1);
    ASSERT_EQ(grid.get_num_points(), 2);

    grid.clear();
    ASSERT_EQ(grid.get_num_points(), 0);

    auto result = grid.query_box(0.0f, 100.0f, 0.0f, 100.0f);
    ASSERT_EQ(result.size(), 0);
}

// Test with double precision
TEST(test_double_precision) {
    GridIndex2D<double> grid(0.0, 1000.0, 1.0,
                             0.0, 1000.0, 1.0);

    grid.insert(123.456, 789.012, 100);

    auto result = grid.query_box(123.0, 124.0, 789.0, 790.0);
    ASSERT_EQ(result.size(), 1);
    ASSERT_EQ(result[0], 100);
}

// Test swapped box coordinates
TEST(test_swapped_box) {
    GridIndex2D<float> grid(0.0f, 100.0f, 10.0f,
                            0.0f, 100.0f, 10.0f);

    grid.insert(15.0f, 25.0f, 0);

    // Query with swapped coordinates (should work)
    auto result1 = grid.query_box(20.0f, 10.0f, 30.0f, 20.0f);
    auto result2 = grid.query_box(10.0f, 20.0f, 20.0f, 30.0f);

    ASSERT_EQ(result1.size(), result2.size());
}

// Test large number of points
TEST(test_many_points) {
    GridIndex2D<float> grid(0.0f, 100.0f, 1.0f,
                            0.0f, 100.0f, 1.0f);

    // Insert 10,000 points
    for (int i = 0; i < 100; ++i) {
        for (int j = 0; j < 100; ++j) {
            grid.insert(i + 0.5f, j + 0.5f, i * 100 + j);
        }
    }

    ASSERT_EQ(grid.get_num_points(), 10000);

    // Query a 10x10 box
    // Points are at 0.5, 1.5, ..., 99.5
    // Query [0, 9.9] will get cells 0-9, containing points 0.5-9.5 (10x10 = 100 points)
    auto result = grid.query_box(0.0f, 9.9f, 0.0f, 9.9f);
    ASSERT_EQ(result.size(), 100);
}

int main() {
    std::cout << "Running GridIndex2D Tests\n";
    std::cout << "=========================\n\n";

    int passed = 0;

    RUN_TEST(test_construction);
    RUN_TEST(test_invalid_params);
    RUN_TEST(test_single_point);
    RUN_TEST(test_multiple_points_same_cell);
    RUN_TEST(test_multiple_points_different_cells);
    RUN_TEST(test_boundaries);
    RUN_TEST(test_out_of_bounds);
    RUN_TEST(test_callback);
    RUN_TEST(test_clear);
    RUN_TEST(test_double_precision);
    RUN_TEST(test_swapped_box);
    RUN_TEST(test_many_points);

    std::cout << "\n=========================\n";
    std::cout << "All " << passed << " tests passed!\n";

    return 0;
}
