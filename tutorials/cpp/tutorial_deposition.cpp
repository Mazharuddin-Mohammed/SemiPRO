#include "../../src/cpp/core/wafer.hpp"
#include "../../src/cpp/modules/geometry/geometry_manager.hpp"
#include "../../src/cpp/modules/photolithography/lithography_model.hpp"
#include "../../src/cpp/modules/deposition/deposition_model.hpp"
#include <memory>
#include <iostream>

int main() {
    std::cout << "Tutorial: Deposition Module\n";
    auto wafer = std::make_shared<Wafer>(300.0, 775.0, "silicon");
    GeometryManager geometry_manager;
    std::cout << "Step 1: Initialize grid\n";
    geometry_manager.initializeGrid(wafer, 10, 10);
    LithographyModel lithography;
    std::vector<std::vector<int>> mask = {{1, 0, 1}, {0, 1, 0}, {1, 0, 1}};
    std::cout << "Step 2: Apply photoresist pattern\n";
    lithography.simulateExposure(wafer, 13.5, 0.33, mask);
    DepositionModel deposition;
    std::cout << "Step 3: Run uniform deposition\n";
    deposition.simulateDeposition(wafer, 0.1, "SiO2", "uniform");
    std::cout << "Film layers: " << wafer->getFilmLayers().size() << "\n";
    return 0;
}