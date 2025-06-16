Doping Example
=============

.. code-block:: python

   from semiconductor_simulator.simulator import Simulator
   simulator = Simulator()
   simulator.initialize_geometry(100, 100)
   simulator.run_ion_implantation(50.0, 1e15)
   simulator.run_diffusion(1000.0, 3600.0)
   simulator.initialize_renderer(400, 400)
   simulator.render(simulator.get_wafer())