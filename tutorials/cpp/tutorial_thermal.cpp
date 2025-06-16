#include "../../src/cpp/core/wafer.hpp"
#include "../../src/cpp/modules/geometry/geometry_manager.hpp"
#include "../../src/cpp/modules/photolithography/lithography_model.hpp"
#include "../../src/cpp/modules/metallization/metallization_model.hpp"
#include "../../src/cpp/modules/packaging/packaging_model.hpp"
#include "../../src/cpp/modules/thermal/thermal_model.hpp"
#include "../../src/cpp/renderer/vulkan_renderer.hpp"
#include <memory>
#include <iostream>

int main() {
    // Step 1: Create a wafer (300 mm diameter, 775 um thick, silicon)
    auto wafer = std::make_shared<Wafer>(300.0, 775.0, "silicon");
    GeometryManager geometry_manager;
    geometry_manager.initializeGrid(wafer, 10, 10);
    std::cout << "Initialized wafer with 10x10 grid\n";

    // Step 2: Define metal pads using photolithography
    LithographyModel lithography;
    std::vector<std::vector<int>> mask = {{1, 0, 1}, {0, 1, 0}, {1, 0, 1}};
    lithography.simulateExposure(wafer, 13.5, 0.33, mask);
    std::cout << "Applied photolithography with mask\n";

    // Step 3: Deposit copper metal layer
    MetallizationModel metallization;
    metallization.simulateMetallization(wafer, 0.5, "Cu", "pvd");
    std::cout << "Deposited 0.5 um copper layer\n";

    // Step 4: Perform electrical test to get resistance
    PackagingModel packaging;
    packaging.performElectricalTest(wafer, "resistance");
    for (const auto& prop : wafer->getElectricalProperties()) {
        std::cout << prop.first << ": " << prop.second << " Ohms\n";
    }

    // Step 5: Simulate thermal distribution
    ThermalSimulationModel thermal;
    thermal.simulateThermal(wafer, 300.0, 0.001); // 300 K ambient, 1 mA
    std::cout << "Thermal simulation completed. Max temperature: "
              << wafer->getTemperatureProfile().maxCoeff() << " K\n";

    // Step 6: Visualize results
    VulkanRenderer renderer(400, 400);
    renderer.initialize();
    renderer.render(wafer);
    std::cout << "Rendering temperature profile\n";

    return 0;
}