from semiconductor_simulator.simulator import Simulator

print("Tutorial: Doping Module")
simulator = Simulator()
print("Step 1: Initialize grid")
simulator.initialize_geometry(100, 100)
print("Step 2: Run ion implantation")
simulator.run_ion_implantation(50.0, 1e15)
print("Step 3: Run diffusion")
simulator.run_diffusion(1000.0, 3600.0)
print("Step 4: Visualize dopant profile")
simulator.initialize_renderer(400, 400)
simulator.render(simulator.get_wafer())