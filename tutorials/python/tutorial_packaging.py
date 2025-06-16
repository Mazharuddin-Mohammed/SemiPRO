from semiconductor_simulator import Simulator

print("Tutorial: Packaging Module")
simulator = Simulator()
print("Step 1: Initialize grid")
simulator.initialize_geometry(10, 10)
print("Step 2: Apply photoresist")
mask = [[1, 0, 1], [0, 1, 0], [1, 0, 1]]
simulator.run_exposure(13.5, 0.33, mask)
print("Step 3: Apply metallization")
simulator.run_metallization(0.5, "Cu", "pvd")
print("Step 4: Run packaging")
simulator.simulate_packaging(1000.0, "Ceramic", 1)
print("Step 5: Perform electrical test")
simulator.perform_electrical_test("resistance")
print("Step 6: Visualize result")
simulator.initialize_renderer(400, 400)
simulator.render(simulator.get_wafer())