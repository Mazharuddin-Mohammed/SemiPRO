from semiconductor_simulator import Simulator

def main():
    print("Tutorial: Thermal Simulation in Semiconductor Simulator")
    
    # Step 1: Initialize simulator and wafer
    simulator = Simulator()
    simulator.initialize_geometry(10, 10)
    print("Step 1: Initialized 10x10 wafer grid")

    # Step 2: Pattern metal pads
    mask = [[1, 0, 1], [0, 1, 0], [1, 0, 1]]
    simulator.run_exposure(13.5, 0.33, mask)
    print("Step 2: Applied photolithography mask")

    # Step 3: Deposit metal
    simulator.run_metallization(0.5, "Cu", "pvd")
    print("Step 3: Deposited 0.5 um copper layer")

    # Step 4: Measure resistance
    simulator.perform_electrical_test("resistance")
    wafer = simulator.get_wafer()
    props = wafer.get_electrical_properties()
    for name, value in props:
        print(f"Step 4: {name}: {value} Ohms")

    # Step 5: Run thermal simulation
    simulator.simulate_thermal(300.0, 0.001)  # 300 K, 1 mA
    temp_profile = wafer.get_temperature_profile()
    print(f"Step 5: Thermal simulation complete. Max temperature: {temp_profile.max():.2f} K")

    # Step 6: Visualize using GUI
    simulator.initialize_renderer(400, 400)
    simulator.render(wafer)
    print("Step 6: Visualizing temperature profile (close window to exit)")

if __name__ == "__main__":
    main()