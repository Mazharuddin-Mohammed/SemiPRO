from semiconductor_simulator.simulator import Simulator

print("Tutorial: Oxidation Module")
simulator = Simulator()
print("Step 1: Initialize grid")
simulator.initialize_geometry(100, 100)
print("Step 2: Run thermal oxidation")
simulator.run_oxidation(1000.0, 3600.0)
print("Step 3: Visualize oxide layer")
simulator.initialize_renderer(400, 400)
simulator.render(simulator.get_wafer())