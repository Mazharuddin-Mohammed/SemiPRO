#include "../../src/cpp/core/wafer.hpp"
#include "../../src/cpp/modules/geometry/geometry_manager.hpp"
#include <memory>
#include <iostream>

int main() {
  std::cout << "Tutorial: Geometry Module\n";
  auto wafer = std::make_shared<Wafer>(300.0, 775.0, "silicon");
  GeometryManager geometry;
  std::cout << "Step 1: Initialize grid\n";
  geometry.initializeGrid(wafer, 100, 100);
  std::cout << "Step 2: Apply oxide layer\n";
  geometry.applyLayer(wafer, 0.1, "oxide");
  std::cout << "Final thickness: " << wafer->getThickness() << " um\n";
  return 0;
}