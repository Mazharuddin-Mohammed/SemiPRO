Geometry Tutorial
================

This tutorial guides you through using the geometry module.

1. Initialize a wafer:

   .. code-block:: python

      from semiconductor_simulator.simulator import Simulator
      simulator = Simulator()
      simulator.initialize_geometry(100, 100)

2. Apply a layer:

   .. code-block:: python

      simulator.apply_layer(0.1, "oxide")