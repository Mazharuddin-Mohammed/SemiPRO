#include "../../src/cpp/core/wafer.hpp"
#include "../../src/cpp/modules/geometry/geometry_manager.hpp"
#include "../../src/cpp/modules/photolithography/lithography_model.hpp"
#include "../../src/cpp/modules/metallization/metallization_model.hpp"
#include "../../src/cpp/modules/packaging/packaging_model.hpp"
#include <memory>
#include <iostream>

int main() {
    std::cout << "Tutorial: Packaging Module\n";
    auto wafer = std::make_shared<Wafer>(300.0, 775.0, "silicon");
    GeometryManager geometry_manager;
    std::cout << "Step 1: Initialize grid\n";
    geometry_manager.initializeGrid(wafer, 10, 10);
    LithographyModel lithography;
    std::vector<std::vector<int>> mask = {{1, 0, 1}, {0, 1, 0}, {1, 0, 1}};
    std::cout << "Step 2: Apply photoresist\n";
    lithography.simulateExposure(wafer, 13.5, 0.33, mask);
    MetallizationModel metallization;
    std::cout << "Step 3: Apply metallization\n";
    metallization.simulateMetallization(wafer, 0.5, "Cu", "pvd");
    PackagingModel packaging;
    std::cout << "Step 4: Run packaging\n";
    packaging.simulatePackaging(wafer, 1000.0, "Ceramic", 2);
    std::cout << "Step 5: Perform electrical test\n";
    packaging.performElectricalTest(wafer, "resistance");
    std::cout << "Substrate thickness: " << wafer->getPackagingSubstrate().first << " um\n";
    return 0;
}