Deposition Tutorial
==================

This tutorial demonstrates the deposition module.

1. Initialize a wafer and apply photoresist:

   .. code-block:: python

      from semiconductor_simulator.simulator import Simulator
      simulator = Simulator()
      simulator.initialize_geometry(10, 10)
      mask = [[1, 0, 1], [0, 1, 0], [1, 0, 1]]
      simulator.run_exposure(13.5, 0.33, mask)

2. Run uniform deposition:

   .. code-block:: python

      simulator.run_deposition(0.1, "SiO2", "uniform")

3. Run conformal deposition:

   .. code-block:: python

      simulator.run_deposition(0.2, "SiN", "conformal")

4. Visualize the result:

   .. code-block:: python

      simulator.initialize_renderer(400, 400)
      simulator.render(simulator.get_wafer())