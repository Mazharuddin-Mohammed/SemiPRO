from semiconductor_simulator.simulator import Simulator

print("Tutorial: Geometry Module")
simulator = Simulator()
print("Step 1: Initialize grid")
simulator.initialize_geometry(100, 100)
print("Step 2: Apply oxide layer")
simulator.apply_layer(0.1, "oxide")
print("Step 3: Visualize wafer")
simulator.initialize_renderer(400, 400)
simulator.render(simulator.get_wafer())