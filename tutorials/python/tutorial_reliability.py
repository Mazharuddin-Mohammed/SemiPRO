from semiconductor_simulator import Simulator
import numpy as np

def main():
    print("Tutorial: Reliability Testing")
    simulator = Simulator()
    
    # Step 1: Initialize wafer
    simulator.initialize_geometry(10, 10)
    print("Step 1: Initialized 10x10 wafer")
    
    # Step 2: Pattern metal pads
    mask = np.zeros((10, 10))
    mask[2:5, 2:5] = 0.1
    simulator.run_exposure(13.5, 0.33, mask.tolist())
    print("Step 2: Patterned metal pads")
    
    # Step 3: Deposit metal and dielectric
    simulator.run_metallization(0.5, "Cu", "pvd")
    simulator.run_deposition(0.1, "SiO2", "cvd")
    print("Step 3: Deposited Cu and SiO2")
    
    # Step 4: Perform electrical test
    simulator.perform_electrical_test("resistance")
    print("Step 4: Measured resistance")
    
    # Step 5: Run thermal simulation
    simulator.simulate_thermal(300.0, 0.001)
    print("Step 5: Simulated thermal profile")
    
    # Step 6: Perform reliability test
    simulator.perform_reliability_test(0.001, 5.0)
    wafer = simulator.get_wafer()
    print("Step 6: Reliability results:")
    print(f"  Min MTTF: {wafer.get_electromigration_mttf().min():.2e} s")
    print(f"  Max Stress: {wafer.get_thermal_stress().max():.2f} MPa")
    print(f"  Max Field: {wafer.get_dielectric_field().max():.2e} V/cm")
    
    # Step 7: Visualize thermal stress
    simulator.initialize_renderer(400, 400)
    simulator.renderer.render(wafer, False, "Thermal Stress")
    print("Step 7: Visualizing thermal stress risks")

if __name__ == "__main__":
    main()