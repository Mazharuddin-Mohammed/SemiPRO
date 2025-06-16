from semiconductor_simulator import Simulator

print("Tutorial: Deposition Module")
simulator = Simulator()
print("Step 1: Initialize grid")
simulator.initialize_geometry(10, 10)
print("Step 2: Apply photoresist pattern")
mask = [[1, 0, 1], [0, 1, 0], [1, 0, 1]]
simulator.run_exposure(13.5, 0.33, mask)
print("Step 3: Run uniform deposition")
simulator.run_deposition(0.1, "SiO2", "uniform")
print("Step 4: Visualize result")
simulator.initialize_renderer(400, 400)
simulator.render(simulator.get_wafer())