#include "../../src/cpp/core/wafer.hpp"
#include "../../src/cpp/modules/geometry/geometry_manager.hpp"
#include "../../src/cpp/modules/oxidation/oxidation_model.hpp"
#include <memory>
#include <iostream>

int main() {
  std::cout << "Tutorial: Oxidation Module\n";
  auto wafer = std::make_shared<Wafer>(300.0, 775.0, "silicon");
  GeometryManager geometry;
  std::cout << "Step 1: Initialize grid\n";
  geometry.initializeGrid(wafer, 100, 100);
  OxidationModel oxidation;
  std::cout << "Step 2: Run thermal oxidation\n";
  oxidation.simulateOxidation(wafer, 1000.0, 3600.0);
  std::cout << "Final thickness: " << wafer->getThickness() << " um\n";
  return 0;
}