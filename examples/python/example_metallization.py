from semiconductor_simulator import Simulator

simulator = Simulator()
simulator.initialize_geometry(10, 10)
mask = [[1, 0, 1], [0, 1, 0], [1, 0, 1]]
simulator.run_exposure(13.5, 0.33, mask)
simulator.run_metallization(0.5, "Cu", "pvd")
simulator.initialize_renderer(400, 400)
simulator.render(simulator.get_wafer())