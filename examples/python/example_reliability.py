from semiconductor_simulator import Simulator
import numpy as np

def main():
    simulator = Simulator()
    simulator.initialize_geometry(10, 10)
    
    mask = np.zeros((10, 10))
    mask[2:5, 2:5] = 0.1
    simulator.run_exposure(13.5, 0.33, mask.tolist())
    simulator.run_metallization(0.5, "Cu", "pvd")
    simulator.run_deposition(0.1, "SiO2", "cvd")
    simulator.simulate_thermal(300.0, 0.001)
    simulator.perform_reliability_test(0.001, 5.0)
    
    wafer = simulator.get_wafer()
    print(f"Min MTTF: {wafer.get_electromigration_mttf().min():.2e} s")
    print(f"Max Stress: {wafer.get_thermal_stress().max():.2f} MPa")
    print(f"Max Field: {wafer.get_dielectric_field().max():.2e} V/cm")
    
    simulator.initialize_renderer(400, 400)
    simulator.renderer.render(wafer, False, "Electromigration")  # Visualize electromigration risks

if __name__ == "__main__":
    main()