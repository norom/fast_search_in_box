# Grid Index

A high-performance 2D spatial indexing data structure for fast box queries in O(1) time.

## Overview

Grid Index is a header-only C++ library that provides efficient spatial indexing for 2D point data. It divides the 2D space into a regular grid and allows you to perform box queries (rectangular range queries) with constant-time complexity per grid cell.

## Problem Statement

Given:
- A collection of 2D points with coordinates (x, y) and associated data
- A regular grid defined by start, end, and step values for both X and Y axes
- Need to perform many box queries to find all points within rectangular regions

Traditional approaches like R-Tree have O(log(n)) complexity. Grid Index achieves O(1) lookup per grid cell by pre-organizing points into spatial buckets, making it significantly faster for regular grid queries.

## Key Features

- **O(1) Query Performance**: Fast box queries that return all point indices within a specified rectangular region
- **Header-Only**: Single header file, easy to integrate into any C++ project
- **Template-Based Coordinates**: Supports both `float` and `double` coordinate types
- **Regular Grid Structure**: Defined by start, end, and step parameters for both axes
- **Two Query Modes**:
  - `query_box`: Returns a vector of all indices in the query box
  - `query_box_callback`: Executes a callback function for each index in the query box
- **Memory Efficient**: Uses a flat grid structure with index vectors per cell

## Use Cases

- Geophysics 3D algorithms for prestack data
- Spatial databases and GIS applications
- Collision detection in games and simulations
- Neighbor finding in scientific computing
- Point-in-region queries for visualization
- Spatial filtering and clustering

## How It Works

1. **Grid Construction**: The 2D space is divided into cells based on the provided start, end, and step parameters
2. **Point Assignment**: Each point is assigned to a grid cell based on its coordinates
3. **Index Storage**: Each cell stores the indices of all points that fall within it
4. **Box Query**: When querying a box (x1, x2, y1, y2), the algorithm:
   - Determines which grid cells intersect with the query box
   - Collects all point indices from those cells
   - Returns the results

## Complexity

- **Build**: O(n) where n is the number of points
- **Query**: O(k) where k is the number of cells in the query box (typically O(1) for small boxes)
- **Space**: O(n + m) where m is the number of grid cells

## Installation

Simply copy the header file to your project:

```bash
git clone https://github.com/yourusername/grid_index.git
cp grid_index/include/grid_index.h your_project/
```

Or include it directly in your build system.

## Quick Start

```cpp
#include "grid_index.h"

// Define your points
struct Point {
    float x, y;
    // your data...
};

std::vector<Point> points = { /* your points */ };

// Create grid index with float coordinates
GridIndex2D<float> grid(
    x_start, x_end, x_step,
    y_start, y_end, y_step
);

// Fill the grid with points
for (size_t i = 0; i < points.size(); ++i) {
    grid.insert(points[i].x, points[i].y, i);
}

// Query a box
auto indices = grid.query_box(x1, x2, y1, y2);

// Or use callback
grid.query_box_callback(x1, x2, y1, y2, [](size_t idx) {
    // process index
});
```

## API Reference

### Template Parameters
```cpp
template<typename T>  // T = float or double
class GridIndex2D;
```

### Constructor
```cpp
GridIndex2D(T x_start, T x_end, T x_step,
            T y_start, T y_end, T y_step)
```

### Methods
- `void insert(T x, T y, size_t index)` - Insert a point index into the grid
- `std::vector<size_t> query_box(T x1, T x2, T y1, T y2) const` - Query all indices in a box
- `template<typename Callback> void query_box_callback(T x1, T x2, T y1, T y2, Callback cb) const` - Query with callback

## License

MIT License

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## Roadmap

- [ ] Basic 2D grid implementation
- [ ] Unit tests
- [ ] Benchmarks
- [ ] Python bindings
- [ ] 3D grid support
- [ ] Adaptive grid refinement
