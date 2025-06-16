#include "../../src/cpp/core/wafer.hpp"
#include "../../src/cpp/modules/reliability/reliability_model.hpp"
#include "../../src/cpp/modules/metallization/metallization_model.hpp"
#include "../../src/cpp/modules/thermal/thermal_model.hpp"
#include "../../src/cpp/modules/packaging/packaging_model.hpp"
#include <memory>
#include <cassert>
#include <iostream>

void test_reliability() {
    // Initialize wafer
    auto wafer = std::make_shared<Wafer>(300.0, 775.0, "silicon");
    wafer->initializeGrid(10, 10);

    // Add metal layer
    MetallizationModel metallization;
    Eigen::ArrayXXd photoresist = Eigen::ArrayXXd::Zero(10, 10);
    photoresist.block(2, 2, 3, 3).setConstant(0.1); // Metal pad
    wafer->setPhotoresistPattern(photoresist);
    metallization.simulateMetallization(wafer, 0.5, "Cu", "pvd");

    // Perform electrical test
    PackagingModel packaging;
    packaging.performElectricalTest(wafer, "resistance");

    // Run thermal simulation
    ThermalSimulationModel thermal;
    thermal.simulateThermal(wafer, 300.0, 0.001); // 300 K, 1 mA

    // Run reliability test
    ReliabilityModel reliability;
    reliability.performReliabilityTest(wafer, 0.001, 5.0); // 1 mA, 5 V

    // Check electromigration MTTF
    auto mttf = wafer->getElectromigrationMTTF();
    bool has_mttf = false;
    for (int i = 0; i < mttf.rows(); ++i) {
        for (int j = 0; j < mttf.cols(); ++j) {
            if (mttf(i, j) > 0) {
                has_mttf = true;
                assert(mttf(i, j) > 1e6); // MTTF > 1e6 s
            }
        }
    }
    assert(has_mttf);

    // Check thermal stress
    auto stress = wafer->getThermalStress();
    bool has_stress = false;
    for (int i = 0; i < stress.rows(); ++i) {
        for (int j = 0; j < stress.cols(); ++j) {
            if (stress(i, j) > 0) {
                has_stress = true;
                assert(stress(i, j) < 1000); // Stress < 1000 MPa
            }
        }
    }
    assert(has_stress);

    // Check dielectric field
    wafer->addFilmLayer(0.1, "SiO2");
    reliability.performReliabilityTest(wafer, 0.001, 5.0);
    auto field = wafer->getDielectricField();
    bool has_field = false;
    for (int i = 0; i < field.rows(); ++i) {
        for (int j = 0; j < field.cols(); ++j) {
            if (field(i, j) > 0) {
                has_field = true;
                assert(field(i, j) < 1e7); // Field < 10 MV/cm
            }
        }
    }
    assert(has_field);

    std::cout << "Reliability test passed\n";
}

int main() {
    test_reliability();
    return 0;
}