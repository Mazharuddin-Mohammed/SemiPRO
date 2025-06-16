#include "../../src/cpp/core/wafer.hpp"
#include "../../src/cpp/modules/geometry/geometry_manager.hpp"
#include "../../src/cpp/modules/photolithography/lithography_model.hpp"
#include "../../src/cpp/modules/metallization/metallization_model.hpp"
#include <memory>
#include <iostream>

int main() {
    auto wafer = std::make_shared<Wafer>(300.0, 775.0, "silicon");
    GeometryManager geometry_manager;
    geometry_manager.initializeGrid(wafer, 10, 10);
    LithographyModel lithography;
    std::vector<std::vector<int>> mask = {{1, 0, 1}, {0, 1, 0}, {1, 0, 1}};
    lithography.simulateExposure(wafer, 13.5, 0.33, mask);
    MetallizationModel metallization;
    metallization.simulateMetallization(wafer, 0.5, "Cu", "pvd");
    std::cout << "Metal layers: " << wafer->getMetalLayers().size() << "\n";
    return 0;
}