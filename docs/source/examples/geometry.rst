Geometry Example
===============

.. code-block:: python

   from semiconductor_simulator.simulator import Simulator
   simulator = Simulator()
   simulator.initialize_geometry(100, 100)
   simulator.apply_layer(0.1, "silicon")
   simulator.render(simulator.get_wafer())