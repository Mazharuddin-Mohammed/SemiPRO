Reliability Testing Example
==========================

C++ Example
-----------

.. code-block:: cpp

    #include "core/wafer.hpp"
    #include "modules/reliability/reliability_model.hpp"
    #include "modules/thermal/thermal_model.hpp"
    #include "modules/metallization/metallization_model.hpp"
    #include "renderer/vulkan_renderer.hpp"
    #include <memory>

    int main() {
        auto wafer = std::make_shared<Wafer>(300.0, 775.0, "silicon");
        wafer->initializeGrid(10, 10);

        MetallizationModel metallization;
        Eigen::ArrayXXd photoresist = Eigen::ArrayXXd::Zero(10, 10);
        photoresist.block(2, 2, 3, 3).setConstant(0.1);
        wafer->setPhotoresistPattern(photoresist);
        metallization.simulateMetallization(wafer, 0.5, "Cu", "pvd");

        wafer->addFilmLayer(0.1, "SiO2");

        ThermalSimulationModel thermal;
        thermal.simulateThermal(wafer, 300.0, 0.001);

        ReliabilityModel reliability;
        reliability.performReliabilityTest(wafer, 0.001, 5.0);

        VulkanRenderer renderer(400, 400);
        renderer.initialize();
        renderer.render(wafer, false, true); // Show reliability risks

        return 0;
    }

Python Example
--------------

.. code-block:: python

    from semiconductor_simulator import Simulator

    def main():
        simulator = Simulator()
        simulator.initialize_geometry(10, 10)

        mask = [[0]*10 for _ in range(10)]
        for i in range(2, 5):
            for j in range(2, 5):
                mask[i][j] = 0.1
        simulator.run_exposure(13.5, 0.33, mask)

        simulator.run_metallization(0.5, "Cu", "pvd")
        simulator.run_deposition(0.1, "SiO2", "cvd")

        simulator.simulate_thermal(300.0, 0.001)
        simulator.perform_reliability_test(0.001, 5.0)

        simulator.initialize_renderer(400, 400)
        wafer = simulator.get_wafer()
        simulator.render(wafer)

    if __name__ == "__main__":
        main()