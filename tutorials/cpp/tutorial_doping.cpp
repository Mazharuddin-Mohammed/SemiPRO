#include "../../src/cpp/core/wafer.hpp"
#include "../../src/cpp/modules/geometry/geometry_manager.hpp"
#include "../../src/cpp/modules/doping/doping_manager.hpp"
#include <memory>
#include <iostream>

int main() {
  std::cout << "Tutorial: Doping Module\n";
  auto wafer = std::make_shared<Wafer>(300.0, 775.0, "silicon");
  GeometryManager geometry;
  std::cout << "Step 1: Initialize grid\n";
  geometry.initializeGrid(wafer, 100, 100);
  DopingManager doping;
  std::cout << "Step 2: Run ion implantation\n";
  doping.simulateIonImplantation(wafer, 50.0, 1e15);
  std::cout << "Step 3: Run diffusion\n";
  doping.simulateDiffusion(wafer, 1000.0, 3600.0);
  std::cout << "Max dopant concentration: " << wafer->getDopantProfile().maxCoeff() << " cm^-3\n";
  return 0;
}