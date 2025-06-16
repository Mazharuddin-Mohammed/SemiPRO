Thermal Simulation Tutorial
==========================

This tutorial demonstrates the thermal simulation module.

1. Initialize a wafer, pattern it, and apply metal:

   .. code-block:: python

       from semiconductor_simulator import Simulator
       simulator = Simulator()
       simulator.initialize_geometry(10, 10)
       mask = [[1, 0, 1], [0, 1, 0], [1, 0, 1]]
       simulator.run_exposure(13.5, 0.33, mask)
       simulator.run_metallization(0.5, "Cu", "pvd")

2. Perform electrical test for resistance:

   .. code-block:: python

       simulator.perform_electrical_test("resistance")

3. Run thermal simulation:

   .. code-block:: python

       simulator.simulate_thermal(300.0, 0.001)

4. Visualize the temperature profile:

   .. code-block:: python

       simulator.initialize_renderer(400, 400)
       simulator.render(simulator.get_wafer())