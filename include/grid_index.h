/**
 * @file grid_index.h
 * @brief High-performance 2D spatial indexing with O(1) box queries
 *
 * Grid Index divides 2D space into a regular grid for fast rectangular range queries.
 * Unlike R-Tree (O(log n)), grid index provides O(1) lookup per grid cell.
 *
 * @copyright MIT License
 */

#ifndef GRID_INDEX_H
#define GRID_INDEX_H

#include <vector>
#include <cmath>
#include <algorithm>
#include <stdexcept>

/**
 * @brief 2D spatial index using a regular grid structure
 *
 * @tparam T Coordinate type (typically float or double)
 *
 * The grid divides space into cells of uniform size. Each cell stores indices
 * of points that fall within its bounds. Box queries collect indices from all
 * cells that intersect the query rectangle.
 *
 * Example:
 * @code
 * GridIndex2D<float> grid(0.0f, 100.0f, 1.0f,  // x: [0,100] step 1
 *                         0.0f, 100.0f, 1.0f); // y: [0,100] step 1
 *
 * // Insert points
 * grid.insert(10.5f, 20.3f, 0);
 * grid.insert(10.8f, 20.7f, 1);
 *
 * // Query box
 * auto indices = grid.query_box(10.0f, 11.0f, 20.0f, 21.0f);
 * @endcode
 */
template<typename T>
class GridIndex2D {
public:
    /**
     * @brief Construct a new Grid Index 2D object
     *
     * @param x_start Minimum x coordinate of the grid
     * @param x_end Maximum x coordinate of the grid
     * @param x_step Cell width in x direction
     * @param y_start Minimum y coordinate of the grid
     * @param y_end Maximum y coordinate of the grid
     * @param y_step Cell height in y direction
     *
     * @throws std::invalid_argument if step values are <= 0 or if start >= end
     */
    GridIndex2D(T x_start, T x_end, T x_step,
                T y_start, T y_end, T y_step)
        : x_start_(x_start), x_end_(x_end), x_step_(x_step),
          y_start_(y_start), y_end_(y_end), y_step_(y_step)
    {
        if (x_step <= 0 || y_step <= 0) {
            throw std::invalid_argument("Step values must be positive");
        }
        if (x_start >= x_end || y_start >= y_end) {
            throw std::invalid_argument("Start must be less than end");
        }

        // Calculate number of cells in each dimension
        nx_ = static_cast<int>(std::ceil((x_end - x_start) / x_step));
        ny_ = static_cast<int>(std::ceil((y_end - y_start) / y_step));

        // Allocate grid cells
        grid_.resize(nx_ * ny_);
    }

    /**
     * @brief Insert a point index into the grid
     *
     * @param x X coordinate of the point
     * @param y Y coordinate of the point
     * @param index Index of the point in your data structure
     *
     * Points outside the grid bounds are clamped to the nearest edge cell.
     */
    void insert(T x, T y, size_t index) {
        int i = get_cell_x(x);
        int j = get_cell_y(y);
        int cell_id = get_cell_id(i, j);
        grid_[cell_id].push_back(index);
    }

    /**
     * @brief Query all point indices within a rectangular box
     *
     * @param x1 Minimum x coordinate of the query box
     * @param x2 Maximum x coordinate of the query box
     * @param y1 Minimum y coordinate of the query box
     * @param y2 Maximum y coordinate of the query box
     * @param include_min Include lower edges (default: true) - [x1, [y1 vs (x1, (y1
     * @param include_max Include upper edges (default: true) - x2], y2] vs x2), y2)
     * @return std::vector<size_t> Vector of all point indices in the box
     *
     * Edge inclusion examples:
     * - [x1, x2] × [y1, y2]: both true (default, fully inclusive)
     * - (x1, x2) × (y1, y2): both false (fully exclusive)
     * - [x1, x2) × [y1, y2): include_min=true, include_max=false (half-open)
     *
     * Note: Returns all points in grid cells that intersect with the query box.
     * This may include points outside the exact box boundaries (false positives).
     * For exact results, filter the returned indices against actual point coordinates.
     *
     * Complexity: O(k * m) where k is the number of cells in the box
     * and m is the average number of points per cell.
     */
    std::vector<size_t> query_box(T x1, T x2, T y1, T y2,
                                   bool include_min = true, bool include_max = true) const {
        std::vector<size_t> result;

        // Get cell ranges
        int i_min, i_max, j_min, j_max;
        get_cell_range(x1, x2, y1, y2, i_min, i_max, j_min, j_max,
                      include_min, include_max);

        // Collect indices from all cells in range
        for (int j = j_min; j <= j_max; ++j) {
            for (int i = i_min; i <= i_max; ++i) {
                int cell_id = get_cell_id(i, j);
                const auto& cell = grid_[cell_id];
                result.insert(result.end(), cell.begin(), cell.end());
            }
        }

        return result;
    }

    /**
     * @brief Query all point indices within a rectangular box (no allocation version)
     *
     * @param x1 Minimum x coordinate of the query box
     * @param x2 Maximum x coordinate of the query box
     * @param y1 Minimum y coordinate of the query box
     * @param y2 Maximum y coordinate of the query box
     * @param result Reference to vector to store results (will be cleared before use)
     * @param append_results If true, append to existing results instead of clearing
     * @param include_min Include lower edges (default: true) - [x1, [y1 vs (x1, (y1
     * @param include_max Include upper edges (default: true) - x2], y2] vs x2), y2)
     *
     * This version allows you to reuse the same result vector across multiple queries,
     * avoiding repeated allocations. The result vector is cleared at the start.
     *
     * Example:
     * @code
     * std::vector<size_t> result;
     * result.reserve(1000);  // Pre-allocate once
     *
     * // Reuse the same vector for multiple queries
     * grid.query_box_no_alloc(0, 10, 0, 10, result);
     * // process result...
     *
     * grid.query_box_no_alloc(20, 30, 20, 30, result);
     * // process result...
     * @endcode
     *
     * Note: Returns all points in grid cells that intersect with the query box.
     * This may include points outside the exact box boundaries (false positives).
     * For exact results, filter the returned indices against actual point coordinates.
     *
     * Complexity: O(k * m) where k is the number of cells in the box
     * and m is the average number of points per cell.
     */
    void query_box_no_alloc(T x1, T x2, T y1, T y2, std::vector<size_t>& result,
                            bool append_results = false,
                            bool include_min = true, bool include_max = true) const {
        if(!append_results)
            result.clear();

        // Get cell ranges
        int i_min, i_max, j_min, j_max;
        get_cell_range(x1, x2, y1, y2, i_min, i_max, j_min, j_max,
                      include_min, include_max);

        // Collect indices from all cells in range
        for (int j = j_min; j <= j_max; ++j) {
            for (int i = i_min; i <= i_max; ++i) {
                int cell_id = get_cell_id(i, j);
                const auto& cell = grid_[cell_id];
                result.insert(result.end(), cell.begin(), cell.end());
            }
        }
    }

    /**
     * @brief Query all point indices within a box using a callback
     *
     * @tparam Callback Function or lambda type: void(size_t index)
     * @param x1 Minimum x coordinate of the query box
     * @param x2 Maximum x coordinate of the query box
     * @param y1 Minimum y coordinate of the query box
     * @param y2 Maximum y coordinate of the query box
     * @param callback Function called for each point index in the box
     * @param include_min Include lower edges (default: true) - [x1, [y1 vs (x1, (y1
     * @param include_max Include upper edges (default: true) - x2], y2] vs x2), y2)
     *
     * More efficient than query_box() when you don't need to store results.
     *
     * Example:
     * @code
     * grid.query_box_callback(0, 10, 0, 10, [&](size_t idx) {
     *     std::cout << "Found point: " << idx << std::endl;
     * });
     * @endcode
     */
    template<typename Callback>
    void query_box_callback(T x1, T x2, T y1, T y2, Callback callback,
                           bool include_min = true, bool include_max = true) const {
        // Get cell ranges
        int i_min, i_max, j_min, j_max;
        get_cell_range(x1, x2, y1, y2, i_min, i_max, j_min, j_max,
                      include_min, include_max);

        // Call callback for each index in range
        for (int j = j_min; j <= j_max; ++j) {
            for (int i = i_min; i <= i_max; ++i) {
                int cell_id = get_cell_id(i, j);
                const auto& cell = grid_[cell_id];
                for (size_t idx : cell) {
                    callback(idx);
                }
            }
        }
    }

    /**
     * @brief Clear all data from the grid
     */
    void clear() {
        for (auto& cell : grid_) {
            cell.clear();
        }
    }

    /**
     * @brief Get the total number of cells in the grid
     * @return size_t Total number of cells (nx * ny)
     */
    size_t get_num_cells() const {
        return grid_.size();
    }

    /**
     * @brief Get the total number of points stored in the grid
     * @return size_t Total number of point indices
     */
    size_t get_num_points() const {
        size_t count = 0;
        for (const auto& cell : grid_) {
            count += cell.size();
        }
        return count;
    }

    /**
     * @brief Get grid dimensions
     * @param nx Output: number of cells in x direction
     * @param ny Output: number of cells in y direction
     */
    void get_dimensions(int& nx, int& ny) const {
        nx = nx_;
        ny = ny_;
    }

    /**
     * @brief Get grid bounds
     */
    void get_bounds(T& x_start, T& x_end, T& y_start, T& y_end) const {
        x_start = x_start_;
        x_end = x_end_;
        y_start = y_start_;
        y_end = y_end_;
    }

private:
    T x_start_, x_end_, x_step_;
    T y_start_, y_end_, y_step_;
    int nx_, ny_;  // Number of cells in each dimension
    std::vector<std::vector<size_t>> grid_;  // Flat grid: grid_[j*nx + i]

    /**
     * @brief Convert x coordinate to cell index (clamped to valid range)
     */
    int get_cell_x(T x) const {
        int i = static_cast<int>(std::floor((x - x_start_) / x_step_));
        return std::max(0, std::min(i, nx_ - 1));
    }

    /**
     * @brief Convert y coordinate to cell index (clamped to valid range)
     */
    int get_cell_y(T y) const {
        int j = static_cast<int>(std::floor((y - y_start_) / y_step_));
        return std::max(0, std::min(j, ny_ - 1));
    }

    /**
     * @brief Convert 2D cell indices to linear cell ID
     */
    int get_cell_id(int i, int j) const {
        return j * nx_ + i;
    }

    /**
     * @brief Get range of cells that intersect with a box query
     */
    void get_cell_range(T x1, T x2, T y1, T y2,
                       int& i_min, int& i_max,
                       int& j_min, int& j_max,
                       bool include_min = true, bool include_max = true) const {
        // Ensure x1 <= x2 and y1 <= y2
        if (x1 > x2) std::swap(x1, x2);
        if (y1 > y2) std::swap(y1, y2);

        // Convert coordinates to cell indices
        i_min = static_cast<int>(std::floor((x1 - x_start_) / x_step_));
        i_max = static_cast<int>(std::floor((x2 - x_start_) / x_step_));
        j_min = static_cast<int>(std::floor((y1 - y_start_) / y_step_));
        j_max = static_cast<int>(std::floor((y2 - y_start_) / y_step_));

        // Adjust for edge inclusion/exclusion
        // If we exclude the minimum edge and x1/y1 is exactly on a cell boundary, skip that cell
        if (!include_min) {
            // Check if x1 is exactly on a cell boundary
            T x1_normalized = (x1 - x_start_) / x_step_;
            if (x1_normalized == std::floor(x1_normalized)) {
                i_min++;
            }
            // Check if y1 is exactly on a cell boundary
            T y1_normalized = (y1 - y_start_) / y_step_;
            if (y1_normalized == std::floor(y1_normalized)) {
                j_min++;
            }
        }

        // If we exclude the maximum edge and x2/y2 is exactly on a cell boundary, skip that cell
        if (!include_max) {
            // Check if x2 is exactly on a cell boundary
            T x2_normalized = (x2 - x_start_) / x_step_;
            if (x2_normalized == std::floor(x2_normalized)) {
                i_max--;
            }
            // Check if y2 is exactly on a cell boundary
            T y2_normalized = (y2 - y_start_) / y_step_;
            if (y2_normalized == std::floor(y2_normalized)) {
                j_max--;
            }
        }

        // Clamp to valid range
        i_min = std::max(0, std::min(i_min, nx_ - 1));
        i_max = std::max(0, std::min(i_max, nx_ - 1));
        j_min = std::max(0, std::min(j_min, ny_ - 1));
        j_max = std::max(0, std::min(j_max, ny_ - 1));
    }
};

#endif // GRID_INDEX_H
