#include "../../src/cpp/core/wafer.hpp"
#include "../../src/cpp/modules/geometry/geometry_manager.hpp"
#include "../../src/cpp/modules/oxidation/oxidation_model.hpp"
#include <memory>
#include <iostream>

int main() {
  auto wafer = std::make_shared<Wafer>(300.0, 775.0, "silicon");
  GeometryManager geometry;
  geometry.initializeGrid(wafer, 100, 100);
  OxidationModel oxidation;
  oxidation.simulateOxidation(wafer, 1000.0, 3600.0);
  std::cout << "Wafer thickness after oxidation: " << wafer->getThickness() << " um\n";
  return 0;
}