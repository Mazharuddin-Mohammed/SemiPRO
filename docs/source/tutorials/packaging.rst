Packaging Tutorial
=================

This tutorial demonstrates the packaging and electrical testing module.

1. Initialize a wafer, pattern it, and apply metal:

   .. code-block:: python

       from semiconductor_simulator import Simulator
       simulator = Simulator()
       simulator.initialize_geometry(10, 10)
       mask = [[1, 0, 1], [0, 1, 0], [1, 0, 1]]
       simulator.run_exposure(13.5, 0.33, mask)
       simulator.run_metallization(0.5, "Cu", "pvd")

2. Run packaging:

   .. code-block:: python

       simulator.simulate_packaging(1000.0, "Ceramic", 2)

3. Perform electrical testing:

   .. code-block:: python

       simulator.perform_electrical_test("resistance")
       simulator.perform_electrical_test("capacitance")

4. Visualize the result:

   .. code-block:: python

       simulator.initialize_geometry(400, renderer, 400)
       simulator.render(simulator.get_wafer())