#include "../../src/cpp/core/wafer.hpp"
#include "../../src/cpp/modules/geometry/geometry_manager.hpp"
#include "../../src/cpp/modules/photolithography/lithography_model.hpp"
#include "../../src/cpp/modules/metallization/metallization_model.hpp"
#include "../../src/cpp/modules/deposition/deposition_model.hpp"
#include "../../src/cpp/modules/thermal/thermal_model.hpp"
#include "../../src/cpp/modules/reliability/reliability_model.hpp"
#include "../../src/cpp/renderer/vulkan_renderer.hpp"
#include <memory>
#include <iostream>

int main() {
    // Initialize wafer
    auto wafer = std::make_shared<Wafer>(300.0, 775.0, "silicon");
    GeometryManager geometry;
    geometry.initializeGrid(wafer, 10, 10);

    // Pattern metal pads
    LithographyModel lithography;
    std::vector<std::vector<int>> mask(10, std::vector<int>(10, 0));
    for (int i = 2; i < 5; ++i)
        for (int j = 2; j < 5; ++j)
            mask[i][j] = 1;
    lithography.simulateExposure(wafer, 13.5, 0.33, mask);

    // Deposit metal and dielectric
    MetallizationModel metallization;
    metallization.simulateMetallization(wafer, 0.5, "Cu", "pvd");
    DepositionModel deposition;
    deposition.simulateDeposition(wafer, 0.1, "SiO2", "cvd");

    // Run thermal simulation
    ThermalSimulationModel thermal;
    thermal.simulateThermal(wafer, 300.0, 0.001);

    // Perform reliability test
    ReliabilityModel reliability;
    reliability.performReliabilityTest(wafer, 0.001, 5.0);

    // Output results
    auto mttf = wafer->getElectromigrationMTTF();
    auto stress = wafer->getThermalStress();
    auto field = wafer->getDielectricField();
    std::cout << "Min MTTF: " << mttf.minCoeff() << " s\n";
    std::cout << "Max Stress: " << stress.maxCoeff() << " MPa\n";
    std::cout << "Max Field: " << field.maxCoeff() << " V/cm\n";

    // Visualize
    VulkanRenderer renderer(400, 400);
    renderer.initialize();
    renderer.render(wafer, false, "stress");

    return 0;
}