Reliability Testing Tutorial
===========================

This tutorial guides you through performing reliability testing using the Semiconductor Simulator.

C++ Tutorial
------------

.. code-block:: cpp

    #include "core/wafer.hpp"
    #include "modules/reliability/reliability_model.hpp"
    #include "modules/thermal/thermal_model.hpp"
    #include "modules/metallization/metallization_model.hpp"
    #include <memory>
    #include <iostream>

    int main() {
        // Initialize wafer
        auto wafer = std::make_shared<Wafer>(300.0, 775.0, "silicon");
        wafer->initializeGrid(10, 10);

        // Define metal pads
        MetallizationModel metallization;
        Eigen::ArrayXXd photoresist = Eigen::ArrayXXd::Zero(10, 10);
        photoresist.block(2, 2, 3, 3).setConstant(0.1);
        wafer->setPhotoresistPattern(photoresist);
        metallization.simulateMetallization(wafer, 0.5, "Cu", "pvd");

        // Add dielectric
        wafer->addFilmLayer(0.1, "SiO2");

        // Run thermal simulation
        ThermalSimulationModel thermal;
        thermal.simulateThermal(wafer, 300.0, 0.001);

        // Perform reliability test
        ReliabilityModel reliability;
        reliability.performReliabilityTest(wafer, 0.001, 5.0);
        auto mttf = wafer->getElectromigrationMTTF();
        auto stress = wafer->getThermalStress();
        auto field = wafer->getDielectricField();
        std::cout << "Min MTTF: " << mttf.minCoeff() << " s\n";
        std::cout << "Max Stress: " << stress.maxCoeff() << " MPa\n";
        std::cout << "Max Field: " << field.maxCoeff() << " V/cm\n";

        return 0;
    }

Python Tutorial
---------------

.. code-block:: python

    from semiconductor_simulator import Simulator

    def main():
        simulator = Simulator()
        simulator.initialize_geometry(10, 10)

        # Define metal pads
        mask = [[0]*10 for _ in range(10)]
        for i in range(2, 5):
            for j in range(2, 5):
                mask[i][j] = 0.1
        simulator.run_exposure(13.5, 0.33, mask)

        # Deposit metal and dielectric
        simulator.run_metallization(0.5, "Cu", "pvd")
        simulator.run_deposition(0.1, "SiO2", "cvd")

        # Run thermal simulation
        simulator.simulate_thermal(300.0, 0.001)

        # Perform reliability test
        simulator.perform_reliability_test(0.001, 5.0)
        wafer = simulator.get_wafer()
        print(f"Min MTTF: {wafer.get_electromigration_mttf().min():.2e} s")
        print(f"Max Stress: {wafer.get_thermal_stress().max():.2f} MPa")
        print(f"Max Field: {wafer.get_dielectric_field().max():.2e} V/cm")

    if __name__ == "__main__":
        main()