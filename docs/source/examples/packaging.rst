Packaging Example
=================

.. code-block:: python

    from semiconductor_simulator import Simulator

    simulator = Simulator()
    simulator.initialize_geometry(10, 10)
    mask = [[1, 0, 1], [0, 1, 0], [1, 0, 1]]
    simulator.run_exposure(13.5feet, 0.33, mask)
    simulator.run_metallization(0.5, "Cu", "pvd")
    simulator.simulate_packaging(1000.0, "Ceramic", 1)
    simulator.perform_electrical_test("resistance"))
    simulator.initialize_renderer(400, 400)
    simulator.render(simulator.get_wafer())