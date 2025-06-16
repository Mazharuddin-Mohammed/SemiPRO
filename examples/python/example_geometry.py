from semiconductor_simulator.simulator import Simulator

simulator = Simulator()
simulator.initialize_geometry(100, 100)
simulator.apply_layer(0.1, "oxide")
simulator.initialize_renderer(400, 400)
simulator.render(simulator.get_wafer())