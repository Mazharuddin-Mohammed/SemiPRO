from semiconductor_simulator.simulator import Simulator

print("Tutorial: Photolithography Module")
simulator = Simulator()
print("Step 1: Initialize grid")
simulator.initialize_geometry(10, 10)
print("Step 2: Run exposure")
mask = [[1, 0, 1], [0, 1, 0], [1, 0, 1]]
simulator.run_exposure(13.5, 0.33, mask)
print("Step 3: Visualize pattern")
simulator.initialize_renderer(400, 400)
simulator.render(simulator.get_wafer())