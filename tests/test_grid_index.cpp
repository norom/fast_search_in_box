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

// Test points with same coordinates but different index values
TEST(test_same_coordinates_different_values) {
    GridIndex2D<float> grid(0.0f, 100.0f, 10.0f,
                            0.0f, 100.0f, 10.0f);

    // Insert multiple points at the exact same coordinates with different index values
    grid.insert(15.0f, 25.0f, 100);
    grid.insert(15.0f, 25.0f, 200);
    grid.insert(15.0f, 25.0f, 300);
    grid.insert(15.0f, 25.0f, 400);

    ASSERT_EQ(grid.get_num_points(), 4);

    // Query box that contains these points
    auto result = grid.query_box(10.0f, 20.0f, 20.0f, 30.0f);
    ASSERT_EQ(result.size(), 4);

    // Verify all index values are present
    ASSERT_TRUE(std::find(result.begin(), result.end(), 100) != result.end());
    ASSERT_TRUE(std::find(result.begin(), result.end(), 200) != result.end());
    ASSERT_TRUE(std::find(result.begin(), result.end(), 300) != result.end());
    ASSERT_TRUE(std::find(result.begin(), result.end(), 400) != result.end());
}

// Test same coordinates with callback
TEST(test_same_coordinates_callback) {
    GridIndex2D<float> grid(0.0f, 100.0f, 10.0f,
                            0.0f, 100.0f, 10.0f);

    // Insert points at same coordinates
    grid.insert(50.0f, 50.0f, 1);
    grid.insert(50.0f, 50.0f, 2);
    grid.insert(50.0f, 50.0f, 3);

    std::vector<size_t> collected;
    grid.query_box_callback(45.0f, 55.0f, 45.0f, 55.0f, [&](size_t idx) {
        collected.push_back(idx);
    });

    ASSERT_EQ(collected.size(), 3);
    ASSERT_TRUE(std::find(collected.begin(), collected.end(), 1) != collected.end());
    ASSERT_TRUE(std::find(collected.begin(), collected.end(), 2) != collected.end());
    ASSERT_TRUE(std::find(collected.begin(), collected.end(), 3) != collected.end());
}

// Test same coordinates at grid boundaries
TEST(test_same_coordinates_at_boundaries) {
    GridIndex2D<float> grid(0.0f, 100.0f, 10.0f,
                            0.0f, 100.0f, 10.0f);

    // Multiple points at boundary coordinates
    grid.insert(0.0f, 0.0f, 10);
    grid.insert(0.0f, 0.0f, 20);
    grid.insert(100.0f, 100.0f, 30);
    grid.insert(100.0f, 100.0f, 40);

    ASSERT_EQ(grid.get_num_points(), 4);

    // Query for points at origin
    auto result_origin = grid.query_box(0.0f, 1.0f, 0.0f, 1.0f);
    ASSERT_EQ(result_origin.size(), 2);
    ASSERT_TRUE(std::find(result_origin.begin(), result_origin.end(), 10) != result_origin.end());
    ASSERT_TRUE(std::find(result_origin.begin(), result_origin.end(), 20) != result_origin.end());

    // Query for points at far corner
    auto result_corner = grid.query_box(99.0f, 100.0f, 99.0f, 100.0f);
    ASSERT_EQ(result_corner.size(), 2);
    ASSERT_TRUE(std::find(result_corner.begin(), result_corner.end(), 30) != result_corner.end());
    ASSERT_TRUE(std::find(result_corner.begin(), result_corner.end(), 40) != result_corner.end());
}

// Test mixed: same and different coordinates
TEST(test_mixed_same_and_different_coordinates) {
    GridIndex2D<float> grid(0.0f, 100.0f, 10.0f,
                            0.0f, 100.0f, 10.0f);

    // Insert some points at same coordinates in cell (1,1): [10,20) x [10,20)
    grid.insert(15.0f, 15.0f, 1);
    grid.insert(15.0f, 15.0f, 2);
    grid.insert(15.0f, 15.0f, 3);

    // Insert some points at different coordinates in the same cell (1,1)
    grid.insert(16.0f, 16.0f, 4);
    grid.insert(17.0f, 17.0f, 5);

    // Insert a point in a different cell (2,2): [20,30) x [20,30)
    grid.insert(25.0f, 25.0f, 6);

    ASSERT_EQ(grid.get_num_points(), 6);

    // Query only cell (1,1) - use 19.9 to avoid boundary at 20.0
    auto result = grid.query_box(10.0f, 19.9f, 10.0f, 19.9f);
    ASSERT_EQ(result.size(), 5);

    // Verify all values from first cell
    for (size_t i = 1; i <= 5; ++i) {
        ASSERT_TRUE(std::find(result.begin(), result.end(), i) != result.end());
    }

    // Query the second cell
    auto result2 = grid.query_box(20.0f, 30.0f, 20.0f, 30.0f);
    ASSERT_EQ(result2.size(), 1);
    ASSERT_EQ(result2[0], 6);
}

// Test same coordinates with double precision
TEST(test_same_coordinates_double) {
    GridIndex2D<double> grid(0.0, 1000.0, 1.0,
                             0.0, 1000.0, 1.0);

    // Insert points at same precise coordinates
    grid.insert(123.456789, 987.654321, 11);
    grid.insert(123.456789, 987.654321, 22);
    grid.insert(123.456789, 987.654321, 33);

    auto result = grid.query_box(123.0, 124.0, 987.0, 988.0);
    ASSERT_EQ(result.size(), 3);
    ASSERT_TRUE(std::find(result.begin(), result.end(), 11) != result.end());
    ASSERT_TRUE(std::find(result.begin(), result.end(), 22) != result.end());
    ASSERT_TRUE(std::find(result.begin(), result.end(), 33) != result.end());
}

// Test query_box_no_alloc with vector& parameter (no allocation version)
TEST(test_query_box_with_vector_param) {
    GridIndex2D<float> grid(0.0f, 100.0f, 10.0f,
                            0.0f, 100.0f, 10.0f);

    grid.insert(15.0f, 25.0f, 100);
    grid.insert(16.0f, 26.0f, 200);
    grid.insert(17.0f, 27.0f, 300);

    std::vector<size_t> result;

    // First query
    grid.query_box_no_alloc(10.0f, 20.0f, 20.0f, 30.0f, result);
    ASSERT_EQ(result.size(), 3);
    ASSERT_TRUE(std::find(result.begin(), result.end(), 100) != result.end());
    ASSERT_TRUE(std::find(result.begin(), result.end(), 200) != result.end());
    ASSERT_TRUE(std::find(result.begin(), result.end(), 300) != result.end());

    // Reuse same vector for another query
    grid.query_box_no_alloc(50.0f, 60.0f, 50.0f, 60.0f, result);
    ASSERT_EQ(result.size(), 0);  // No points in this region
}

// Test vector reuse across multiple queries
TEST(test_query_box_vector_reuse) {
    GridIndex2D<float> grid(0.0f, 100.0f, 10.0f,
                            0.0f, 100.0f, 10.0f);

    // Insert points in different cells
    // Cell (0,0): [0,10) x [0,10)
    grid.insert(5.0f, 5.0f, 1);
    // Cell (1,1): [10,20) x [10,20)
    grid.insert(15.0f, 15.0f, 2);
    // Cell (2,2): [20,30) x [20,30)
    grid.insert(25.0f, 25.0f, 3);
    // Cell (3,3): [30,40) x [30,40)
    grid.insert(35.0f, 35.0f, 4);

    std::vector<size_t> result;
    result.reserve(100);  // Pre-allocate capacity

    // Query 1 - only cell (0,0)
    grid.query_box_no_alloc(0.0f, 9.9f, 0.0f, 9.9f, result);
    ASSERT_EQ(result.size(), 1);
    ASSERT_EQ(result[0], 1);

    // Query 2 - only cell (1,1), result should be cleared
    grid.query_box_no_alloc(10.0f, 19.9f, 10.0f, 19.9f, result);
    ASSERT_EQ(result.size(), 1);
    ASSERT_EQ(result[0], 2);

    // Query 3 - cells (0,0), (1,1), (2,2)
    grid.query_box_no_alloc(0.0f, 29.9f, 0.0f, 29.9f, result);
    ASSERT_EQ(result.size(), 3);
    ASSERT_TRUE(std::find(result.begin(), result.end(), 1) != result.end());
    ASSERT_TRUE(std::find(result.begin(), result.end(), 2) != result.end());
    ASSERT_TRUE(std::find(result.begin(), result.end(), 3) != result.end());
}

// Test vector parameter with same coordinates
TEST(test_query_box_vector_param_same_coords) {
    GridIndex2D<float> grid(0.0f, 100.0f, 10.0f,
                            0.0f, 100.0f, 10.0f);

    // Insert multiple points at same coordinates
    grid.insert(15.0f, 25.0f, 10);
    grid.insert(15.0f, 25.0f, 20);
    grid.insert(15.0f, 25.0f, 30);
    grid.insert(15.0f, 25.0f, 40);

    std::vector<size_t> result;
    grid.query_box_no_alloc(10.0f, 20.0f, 20.0f, 30.0f, result);

    ASSERT_EQ(result.size(), 4);
    ASSERT_TRUE(std::find(result.begin(), result.end(), 10) != result.end());
    ASSERT_TRUE(std::find(result.begin(), result.end(), 20) != result.end());
    ASSERT_TRUE(std::find(result.begin(), result.end(), 30) != result.end());
    ASSERT_TRUE(std::find(result.begin(), result.end(), 40) != result.end());
}

// Test that both query_box versions return same results
TEST(test_query_box_versions_consistency) {
    GridIndex2D<float> grid(0.0f, 100.0f, 10.0f,
                            0.0f, 100.0f, 10.0f);

    // Insert various points
    for (int i = 0; i < 50; ++i) {
        grid.insert(i + 0.5f, i + 0.5f, i);
    }

    // Query using both versions
    auto result1 = grid.query_box(0.0f, 25.0f, 0.0f, 25.0f);

    std::vector<size_t> result2;
    grid.query_box_no_alloc(0.0f, 25.0f, 0.0f, 25.0f, result2);

    // Both should return same results
    ASSERT_EQ(result1.size(), result2.size());

    // Sort both to compare
    std::sort(result1.begin(), result1.end());
    std::sort(result2.begin(), result2.end());

    ASSERT_TRUE(result1 == result2);
}

// Test edge inclusion parameters - fully inclusive [x1, x2] × [y1, y2]
TEST(test_edge_handling_fully_inclusive) {
    GridIndex2D<float> grid(0.0f, 10.0f, 1.0f,
                            0.0f, 10.0f, 1.0f);

    // Insert points at exact cell boundaries
    grid.insert(5.0f, 5.0f, 0);  // Lower-left corner of cell (5,5)
    grid.insert(6.0f, 6.0f, 1);  // Upper-right corner of cell (5,5) / Lower-left of (6,6)
    grid.insert(5.5f, 5.5f, 2);  // Middle of cell (5,5)

    // Query [5.0, 6.0] × [5.0, 6.0] with fully inclusive edges
    auto result = grid.query_box(5.0f, 6.0f, 5.0f, 6.0f, true, true);

    // Should include all points on and within boundaries
    ASSERT_EQ(result.size(), 3);
}

// Test edge inclusion parameters - fully exclusive (x1, x2) × (y1, y2)
TEST(test_edge_handling_fully_exclusive) {
    GridIndex2D<float> grid(0.0f, 10.0f, 1.0f,
                            0.0f, 10.0f, 1.0f);

    // Insert points at exact boundaries
    grid.insert(5.0f, 5.0f, 0);  // In cell (5,5)
    grid.insert(6.0f, 6.0f, 1);  // In cell (6,6)
    grid.insert(5.5f, 5.5f, 2);  // In cell (5,5)

    // Query (5.0, 6.0) × (5.0, 6.0) with exclusive edges
    // When both boundaries align with cell boundaries and we exclude them:
    // i_min starts at 5, gets incremented to 6 (exclude lower)
    // i_max starts at 6, gets decremented to 5 (exclude upper)
    // Result: no cells are queried (i_min > i_max)
    auto result = grid.query_box(5.0f, 6.0f, 5.0f, 6.0f, false, false);

    // No cells included when both boundaries are excluded
    ASSERT_EQ(result.size(), 0);
}

// Test edge inclusion parameters - half-open [x1, x2) × [y1, y2)
TEST(test_edge_handling_half_open) {
    GridIndex2D<float> grid(0.0f, 10.0f, 1.0f,
                            0.0f, 10.0f, 1.0f);

    grid.insert(5.0f, 5.0f, 0);  // In cell (5,5)
    grid.insert(6.0f, 6.0f, 1);  // In cell (6,6)
    grid.insert(5.5f, 5.5f, 2);  // In cell (5,5)

    // Query [5.0, 6.0) × [5.0, 6.0) - include lower, exclude upper
    // i_min = 5 (include lower boundary)
    // i_max = 6, then decremented to 5 (exclude upper boundary)
    // Only cell (5,5) is queried
    auto result = grid.query_box(5.0f, 6.0f, 5.0f, 6.0f, true, false);

    // Should include all points from cell (5,5): points 0 and 2
    ASSERT_EQ(result.size(), 2);
    ASSERT_TRUE(std::find(result.begin(), result.end(), 0) != result.end());
    ASSERT_TRUE(std::find(result.begin(), result.end(), 2) != result.end());
}

// Test edge inclusion parameters - reverse half-open (x1, x2] × (y1, y2]
TEST(test_edge_handling_reverse_half_open) {
    GridIndex2D<float> grid(0.0f, 10.0f, 1.0f,
                            0.0f, 10.0f, 1.0f);

    grid.insert(5.0f, 5.0f, 0);  // In cell (5,5)
    grid.insert(6.0f, 6.0f, 1);  // In cell (6,6)
    grid.insert(5.5f, 5.5f, 2);  // In cell (5,5)

    // Query (5.0, 6.0] × (5.0, 6.0] - exclude lower, include upper
    // i_min = 5, then incremented to 6 (exclude lower boundary)
    // i_max = 6 (include upper boundary)
    // Only cell (6,6) is queried
    auto result = grid.query_box(5.0f, 6.0f, 5.0f, 6.0f, false, true);

    // Should include all points from cell (6,6): point 1
    ASSERT_EQ(result.size(), 1);
    ASSERT_TRUE(std::find(result.begin(), result.end(), 1) != result.end());
}

// Test edge inclusion with callback
TEST(test_edge_handling_callback) {
    GridIndex2D<float> grid(0.0f, 10.0f, 1.0f,
                            0.0f, 10.0f, 1.0f);

    grid.insert(5.0f, 5.0f, 0);  // In cell (5,5)
    grid.insert(6.0f, 6.0f, 1);  // In cell (6,6)
    grid.insert(5.5f, 5.5f, 2);  // In cell (5,5)

    std::vector<size_t> collected;

    // Half-open query with callback [5.0, 6.0) × [5.0, 6.0)
    // Only cell (5,5) is queried
    grid.query_box_callback(5.0f, 6.0f, 5.0f, 6.0f, [&](size_t idx) {
        collected.push_back(idx);
    }, true, false);  // include_min=true, include_max=false

    ASSERT_EQ(collected.size(), 2);
    ASSERT_TRUE(std::find(collected.begin(), collected.end(), 0) != collected.end());
    ASSERT_TRUE(std::find(collected.begin(), collected.end(), 2) != collected.end());
}

// Test edge inclusion with vector parameter
TEST(test_edge_handling_vector_param) {
    GridIndex2D<float> grid(0.0f, 10.0f, 1.0f,
                            0.0f, 10.0f, 1.0f);

    grid.insert(3.0f, 3.0f, 0);  // In cell (3,3)
    grid.insert(4.0f, 4.0f, 1);  // In cell (4,4)
    grid.insert(5.0f, 5.0f, 2);  // In cell (5,5)

    std::vector<size_t> result;

    // Test with half-open interval [3.0, 5.0) × [3.0, 5.0)
    // Cells 3 and 4 are queried, cell 5 is excluded
    grid.query_box_no_alloc(3.0f, 5.0f, 3.0f, 5.0f, result, false, true, false);

    ASSERT_EQ(result.size(), 2);
    ASSERT_TRUE(std::find(result.begin(), result.end(), 0) != result.end());
    ASSERT_TRUE(std::find(result.begin(), result.end(), 1) != result.end());
}

// Test edge handling with points not on boundaries
TEST(test_edge_handling_non_boundary_points) {
    GridIndex2D<float> grid(0.0f, 10.0f, 1.0f,
                            0.0f, 10.0f, 1.0f);

    // Points not on exact cell boundaries - all in cell (5,5)
    grid.insert(5.3f, 5.7f, 0);
    grid.insert(5.8f, 5.2f, 1);

    // Test different edge settings with query boundaries on cell boundaries
    auto r1 = grid.query_box(5.0f, 6.0f, 5.0f, 6.0f, true, true);   // [5,6] - cells 5,6
    auto r2 = grid.query_box(5.0f, 6.0f, 5.0f, 6.0f, false, false); // (5,6) - no cells
    auto r3 = grid.query_box(5.0f, 6.0f, 5.0f, 6.0f, true, false);  // [5,6) - cell 5 only
    auto r4 = grid.query_box(5.0f, 6.0f, 5.0f, 6.0f, false, true);  // (5,6] - cell 6 only

    ASSERT_EQ(r1.size(), 2);  // Cell 5 has both points
    ASSERT_EQ(r2.size(), 0);  // No cells when both boundaries excluded
    ASSERT_EQ(r3.size(), 2);  // Cell 5 has both points
    ASSERT_EQ(r4.size(), 0);  // Cell 6 is empty
}

// Test edge handling across multiple cells
TEST(test_edge_handling_multiple_cells) {
    GridIndex2D<float> grid(0.0f, 10.0f, 1.0f,
                            0.0f, 10.0f, 1.0f);

    // Points at various boundaries
    grid.insert(3.0f, 3.0f, 0);
    grid.insert(4.0f, 4.0f, 1);
    grid.insert(5.0f, 5.0f, 2);
    grid.insert(6.0f, 6.0f, 3);
    grid.insert(7.0f, 7.0f, 4);

    // Half-open [3.0, 7.0) × [3.0, 7.0)
    auto result = grid.query_box(3.0f, 7.0f, 3.0f, 7.0f, true, false);

    // Should include 0,1,2,3 but not 4
    ASSERT_EQ(result.size(), 4);
    ASSERT_TRUE(std::find(result.begin(), result.end(), 0) != result.end());
    ASSERT_TRUE(std::find(result.begin(), result.end(), 1) != result.end());
    ASSERT_TRUE(std::find(result.begin(), result.end(), 2) != result.end());
    ASSERT_TRUE(std::find(result.begin(), result.end(), 3) != result.end());
}

// Test default parameters still work (backward compatibility)
TEST(test_edge_handling_default_params) {
    GridIndex2D<float> grid(0.0f, 10.0f, 1.0f,
                            0.0f, 10.0f, 1.0f);

    grid.insert(5.0f, 5.0f, 0);
    grid.insert(6.0f, 6.0f, 1);

    // Default should be fully inclusive
    auto result1 = grid.query_box(5.0f, 6.0f, 5.0f, 6.0f);
    auto result2 = grid.query_box(5.0f, 6.0f, 5.0f, 6.0f, true, true);

    ASSERT_EQ(result1.size(), result2.size());
    ASSERT_TRUE(result1 == result2);
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
    RUN_TEST(test_same_coordinates_different_values);
    RUN_TEST(test_same_coordinates_callback);
    RUN_TEST(test_same_coordinates_at_boundaries);
    RUN_TEST(test_mixed_same_and_different_coordinates);
    RUN_TEST(test_same_coordinates_double);
    RUN_TEST(test_query_box_with_vector_param);
    RUN_TEST(test_query_box_vector_reuse);
    RUN_TEST(test_query_box_vector_param_same_coords);
    RUN_TEST(test_query_box_versions_consistency);
    RUN_TEST(test_edge_handling_fully_inclusive);
    RUN_TEST(test_edge_handling_fully_exclusive);
    RUN_TEST(test_edge_handling_half_open);
    RUN_TEST(test_edge_handling_reverse_half_open);
    RUN_TEST(test_edge_handling_callback);
    RUN_TEST(test_edge_handling_vector_param);
    RUN_TEST(test_edge_handling_non_boundary_points);
    RUN_TEST(test_edge_handling_multiple_cells);
    RUN_TEST(test_edge_handling_default_params);

    std::cout << "\n=========================\n";
    std::cout << "All " << passed << " tests passed!\n";

    return 0;
}
