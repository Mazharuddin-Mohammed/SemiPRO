Oxidation Tutorial
=================

This tutorial guides you through using the oxidation module.

1. Initialize a wafer and grid:

   .. code-block:: python

      from semiconductor_simulator.simulator import Simulator
      simulator = Simulator()
      simulator.initialize_geometry(100, 100)

2. Run thermal oxidation:

   .. code-block:: python

      simulator.run_oxidation(temperature=1000.0, time=3600.0)

3. Visualize the oxide layer:

   .. code-block:: python

      simulator.initialize_renderer(400, 400)
      simulator.render(simulator.get_wafer())