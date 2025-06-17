#include "../../src/cpp/core/wafer.hpp"
#include "../../src/cpp/modules/geometry/geometry_manager.hpp"
#include "../../src/cpp/modules/photolithography/lithography_model.hpp"
#include "../../src/cpp/modules/metallization/metallization_model.hpp"
#include "../../src/cpp/modules/deposition/deposition_model.hpp"
#include "../../src/cpp/modules/packaging/packaging_model.hpp"
#include "../../src/cpp/modules/thermal/thermal_model.hpp"
#include "../../src/cpp/modules/reliability/reliability_model.hpp"
#include "../../src/cpp/renderer/vulkan_renderer.hpp"
#include <memory>
#include <iostream>

int main() {
    std::cout << "Tutorial: Reliability Testing\n";

    // Step 1: Initialize wafer
    auto wafer = std::make_shared<Wafer>(300.0, 775.0, "silicon");
    GeometryManager geometry;
    geometry.initializeGrid(wafer, 10, 10);
    std::cout << "Step 1: Initialized 10x10 wafer\n";

    // Step 2: Pattern metal pads
    LithographyModel lithography;
    std::vector<std::vector<int>> mask(10, std::vector<int>(10, 0));
    for (int i = 2; i < 5; ++i)
        for (int j = 2; j < 5; ++j)
            mask[i][j] = 1;
    lithography.simulateExposure(wafer, 13.5, 0.33, mask);
    std::cout << "Step 2: Patterned metal pads\n";

    // Step 3: Deposit metal and dielectric
    MetallizationModel metallization;
    metallization.simulateMetallization(wafer, 0.5, "Cu", "pvd");
    DepositionModel deposition;
    deposition.simulateDeposition(wafer, 0.1, "SiO2", "cvd");
    std::cout << "Step 3: Deposited Cu and SiO2\n";

    // Step 4: Perform electrical test
    PackagingModel packaging;
    packaging.performElectricalTest(wafer, "resistance");
    std::cout << "Step 4: Measured resistance\n";

    // Step 5: Run thermal simulation
    ThermalSimulationModel thermal;
    thermal.simulateThermal(wafer, 300.0, 0.001);
    std::cout << "Step 5: Simulated thermal profile\n";

    // Step 6: Perform reliability test
    ReliabilityModel reliability;
    reliability.performReliabilityTest(wafer, 0.001, 5.0);
    auto mttf = wafer->getElectromigrationMTTF();
    auto stress = wafer->getThermalStress();
    auto field = wafer->getDielectricField();
    std::cout << "Step 6: Reliability results:\n";
    std::cout << "  Min MTTF: " << mttf.minCoeff() << " s\n";
    std::cout << "  Max Stress: " << stress.maxCoeff() << " MPa\n";
    std::cout << "  Max Field: " << field.maxCoeff() << " V/cm\n";

    // Step 7: Visualize
    VulkanRenderer renderer(400, 400);
    renderer.initialize();
    renderer.render(wafer, false, "stress");
    std::cout << "Step 7: Visualizing reliability risks\n";

    return 0;
}