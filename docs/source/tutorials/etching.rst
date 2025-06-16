Etching Tutorial
================

This tutorial demonstrates the etching module.

1. Initialize a wafer and apply photoresist:

   .. code-block:: python

      from semiconductor_simulator.simulator import Simulator
      simulator = Simulator()
      simulator.initialize_geometry(10, 10)
      mask = [[1, 0, 1], [0, 1, 0], [1, 0, 1]]
      simulator.run_exposure(13.5, 0.33, mask)

2. Run isotropic etching:

   .. code-block:: python

      simulator.run_etching(0.05, "isotropic")

3. Run anisotropic etching:

   .. code-block:: python

      simulator.run_etching(0.1, "anisotropic")

4. Visualize the result:

   .. code-block:: python

      simulator.initialize_renderer(400, 400)
      simulator.render(simulator.get_wafer())