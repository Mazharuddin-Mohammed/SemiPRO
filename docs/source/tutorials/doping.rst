Doping Tutorial
==============

This tutorial guides you through using the doping module.

1. Initialize a wafer and grid:

   .. code-block:: python

      from semiconductor_simulator.simulator import Simulator
      simulator = Simulator()
      simulator.initialize_geometry(100, 100)

2. Run ion implantation:

   .. code-block:: python

      simulator.run_ion_implantation(energy=50.0, dose=1e15)

3. Run diffusion:

   .. code-block:: python

      simulator.run_diffusion(temperature=1000.0, time=3600.0)

4. Visualize the dopant profile:

   .. code-block:: python

      simulator.initialize_renderer(400, 400)
      simulator.render(simulator.get_wafer())