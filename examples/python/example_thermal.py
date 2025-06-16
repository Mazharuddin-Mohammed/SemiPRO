from semiconductor_simulator import Simulator

def main():
    # Initialize simulator
    simulator = Simulator()
    
    # Step 1: Set up wafer geometry
    simulator.initialize_geometry(10, 10)
    
    # Step 2: Apply photolithography to define metal pads
    mask = [[1, 0, 1], [0, 1, 0], [1, 0, 1]]
    simulator.run_exposure(13.5, 0.33, mask)
    
    # Step 3: Deposit metal layer
    simulator.run_metallization(0.5, "Cu", "pvd")
    
    # Step 4: Perform electrical test to compute resistance
    simulator.perform_electrical_test("resistance")
    
    # Step 5: Run thermal simulation
    simulator.simulate_thermal(300.0, 0.001)  # Ambient: 300 K, Current: 1 mA
    
    # Step 6: Visualize results
    simulator.initialize_renderer(400, 400)
    wafer = simulator.get_wafer()
    temp_profile = wafer.get_temperature_profile()
    print(f"Maximum temperature: {temp_profile.max():.2f} K")
    simulator.render(wafer)

if __name__ == "__main__":
    main()