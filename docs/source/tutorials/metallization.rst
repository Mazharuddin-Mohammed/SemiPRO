Metallization Tutorial
=====================

This tutorial demonstrates the metallization module.

1. Initialize a wafer and apply photoresist:

   .. code-block:: python

       from semiconductor_simulator import Simulator
       simulator = Simulator()
       simulator.initialize_geometry(10, 10)
       mask = [[1, 0, 1], [0, 1, 0], [1, 0, 1]]
       simulator.run_exposure(13.5, 0.33, mask)

2. Run PVD metallization:

   .. code-block:: python

       simulator.run_metallization(0.5, "Cu", "pvd")

3. Run electroplating:

   .. code-block:: python

       simulator.run_metallization(1, "Al", "electroplating")

4. Visualize the result:

   .. code-block:: python

       simulator.initialize_geometry(400, renderer, 400)
       simulator.render(simulator.get_wafer())