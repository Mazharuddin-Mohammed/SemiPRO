#include "../../src/cpp/core/wafer.hpp"
#include "../../src/cpp/modules/geometry/geometry_manager.hpp"
#include "../../src/cpp/modules/photolithography/lithography_model.hpp"
#include <memory>
#include <iostream>

int main() {
  std::cout << "Tutorial: Photolithography Module\n";
  auto wafer = std::make_shared<Wafer>(300.0, 775.0, "silicon");
  GeometryManager geometry;
  std::cout << "Step 1: Initialize grid\n";
  geometry.initializeGrid(wafer, 10, 10);
  LithographyModel lithography;
  std::cout << "Step 2: Run exposure\n";
  std::vector<std::vector<int>> mask = {{1, 0, 1}, {0, 1, 0}, {1, 0, 1}};
  lithography.simulateExposure(wafer, 13.5, 0.33, mask);
  std::cout << "Pattern density: " << wafer->getPhotoresistPattern().sum() / (10 * 10) << "\n";
  return 0;
}