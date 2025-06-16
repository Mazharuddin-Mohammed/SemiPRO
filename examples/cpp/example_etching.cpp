#include "../../src/cpp/core/wafer.hpp"
#include "../../src/cpp/modules/geometry/geometry_manager.hpp"
#include "../../src/cpp/modules/photolithography/lithography_model.hpp"
#include "../../src/cpp/modules/etching/etching_model.hpp"
#include <memory>
#include <iostream>

int main() {
    auto wafer = std::make_shared<Wafer>(300.0, 775.0, "silicon");
    GeometryManager geometry_manager;
    geometry_manager.initializeGrid(wafer, 10, 10);
    LithographyModel lithography;
    std::vector<std::vector<int>> mask = {{1, 0, 1}, {0, 1, 0}, {1, 0, 1}};
    lithography.simulateExposure(wafer, 13.5, 0.33, mask);
    EtchingModel etching;
    etching.simulateEtching(wafer, 0.05, "anisotropic");
    std::cout << "Grid sum after etching: " << wafer->getGrid().sum() << "\n";
    return 0;
}