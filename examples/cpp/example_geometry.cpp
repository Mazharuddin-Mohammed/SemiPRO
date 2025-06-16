#include "../../src/cpp/core/wafer.hpp"
#include "../../src/cpp/modules/geometry/geometry_manager.hpp"
#include <memory>
#include <iostream>

int main() {
  auto wafer = std::make_shared<Wafer>(300.0, 775.0, "silicon");
  GeometryManager geometry;
  geometry.initializeGrid(wafer, 100, 100);
  geometry.applyLayer(wafer, 0.1, "oxide");
  std::cout << "Wafer thickness: " << wafer->getThickness() << " um\n";
  return 0;
}