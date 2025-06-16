Photolithography Tutorial
========================

This tutorial guides you through using the photolithography module.

1. Initialize a wafer and grid:

   .. code-block:: python

      from semiconductor_simulator.simulator import Simulator
      simulator = Simulator()
      simulator.initialize_geometry(10, 10)

2. Run optical exposure:

   .. code-block:: python

      mask = [[1, 0, 1], [0, 1, 0], [1, 0, 1]]
      simulator.run_exposure(wavelength=13.5, na=0.33, mask=mask)

3. Run multi-patterning:

   .. code-block:: python

      masks = [mask, mask]
      simulator.run_multi_patterning(wavelength=13.5, na=0.33, masks=masks)

4. Visualize the pattern:

   .. code-block:: python

      simulator.initialize_renderer(400, 400)
      simulator.render(simulator.get_wafer())