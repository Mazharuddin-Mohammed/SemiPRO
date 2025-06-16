#include "../../src/cpp/core/wafer.hpp"
#include "../../src/cpp/modules/geometry/geometry_manager.hpp"
#include "../../src/cpp/modules/photolithography/lithography_model.hpp"
#include "../../src/cpp/modules/deposition/deposition_model.hpp"
#include <memory>
#include <iostream>

int main() {
    auto wafer = std::make_shared<Wafer>(300.0, 775.0, "silicon");
    GeometryManager geometry_manager;
    geometry_manager.initializeGrid(wafer, 10, 10);
    LithographyModel lithography;
    std::vector<std::vector<int>> mask = {{1, 0, 1}, {0, 1, 0}, {1, 0, 1}};
    lithography.simulateExposure(wafer, 13.5, 0.33, mask);
    DepositionModel deposition;
    deposition.simulateDeposition(wafer, 0.1, "SiO2", "uniform");
    std::cout << "Film layers: " << wafer->getFilmLayers().size() << "\n";
    return 0;
}