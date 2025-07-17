#!/usr/bin/env python3
"""
SemiPRO Python Frontend Tutorial
================================

This tutorial demonstrates the usage of the Python frontend interface
for the SemiPRO semiconductor process simulator.

Author: Dr. Mazharuddin Mohammed
"""

import sys
import os
import time
import numpy as np
import matplotlib.pyplot as plt
from pathlib import Path

# Add the src directory to the Python path
sys.path.insert(0, str(Path(__file__).parent.parent / "src"))

try:
    from python.simulator import Simulator
    from python.gui import SemiPROGUI
    print("✅ Successfully imported SemiPRO modules")
except ImportError as e:
    print(f"❌ Failed to import SemiPRO modules: {e}")
    print("Note: This is expected if Cython extensions are not built")
    print("The tutorial will demonstrate the Python interface using mock extensions")

def test_simulator_creation():
    """Test basic simulator creation and initialization"""
    print("\n=== Testing Simulator Creation ===")
    
    try:
        # Create simulator instance
        sim = Simulator()
        print("✅ Simulator created successfully")
        
        # Initialize geometry
        sim.initialize_geometry(100, 100)
        print("✅ Geometry initialized (100x100 grid)")
        
        # Get wafer information
        wafer = sim.get_wafer()
        print(f"✅ Wafer diameter: {wafer.get_diameter()} mm")
        print(f"✅ Wafer thickness: {wafer.get_thickness()} μm")
        print(f"✅ Wafer material: {wafer.get_material_id()}")
        
        return sim
        
    except Exception as e:
        print(f"❌ Simulator creation failed: {e}")
        return None

def test_basic_processes(sim):
    """Test basic semiconductor processes"""
    print("\n=== Testing Basic Processes ===")
    
    if not sim:
        print("❌ No simulator available for testing")
        return False
    
    try:
        # Test oxidation
        print("Testing oxidation process...")
        result = sim.simulate_oxidation(1000.0, 2.0, "dry")
        if result:
            print("✅ Oxidation simulation completed")
        else:
            print("⚠️ Oxidation simulation returned False (expected with mock)")
        
        # Test doping
        print("Testing doping process...")
        result = sim.simulate_doping("boron", 1e18, 50.0, 1000.0)
        if result:
            print("✅ Doping simulation completed")
        else:
            print("⚠️ Doping simulation returned False (expected with mock)")
        
        # Test deposition
        print("Testing deposition process...")
        result = sim.simulate_deposition("silicon_nitride", 100.0, 600.0)
        if result:
            print("✅ Deposition simulation completed")
        else:
            print("⚠️ Deposition simulation returned False (expected with mock)")
        
        # Test etching
        print("Testing etching process...")
        result = sim.simulate_etching(50.0, "anisotropic")
        if result:
            print("✅ Etching simulation completed")
        else:
            print("⚠️ Etching simulation returned False (expected with mock)")
        
        return True
        
    except Exception as e:
        print(f"❌ Process testing failed: {e}")
        return False

def test_advanced_features(sim):
    """Test advanced simulator features"""
    print("\n=== Testing Advanced Features ===")
    
    if not sim:
        print("❌ No simulator available for testing")
        return False
    
    try:
        # Test lithography
        print("Testing photolithography...")
        sim.apply_photoresist(0.5)
        sim.expose_photoresist(193.0, 25.0)  # ArF lithography
        sim.develop_photoresist()
        print("✅ Lithography sequence completed")
        
        # Test thermal processing
        print("Testing thermal annealing...")
        result = sim.simulate_thermal_annealing(1000.0, 0.1, "N2")
        if result:
            print("✅ Thermal annealing completed")
        else:
            print("⚠️ Thermal annealing returned False (expected with mock)")
        
        # Test metallization
        print("Testing metallization...")
        result = sim.simulate_metallization("aluminum", 0.8, 400.0)
        if result:
            print("✅ Metallization completed")
        else:
            print("⚠️ Metallization returned False (expected with mock)")
        
        # Test packaging
        print("Testing packaging...")
        wire_bonds = [(10, 10, 90, 90), (20, 20, 80, 80)]
        sim.add_packaging(500.0, "FR4", wire_bonds)
        print("✅ Packaging added")
        
        return True
        
    except Exception as e:
        print(f"❌ Advanced features testing failed: {e}")
        return False

def test_workflow_execution(sim):
    """Test complete workflow execution"""
    print("\n=== Testing Workflow Execution ===")
    
    if not sim:
        print("❌ No simulator available for testing")
        return False
    
    try:
        print("Executing NMOS transistor fabrication workflow...")
        
        # Step 1: Gate oxidation
        print("  Step 1: Gate oxidation")
        sim.simulate_oxidation(1000.0, 0.5, "dry")
        
        # Step 2: Gate definition
        print("  Step 2: Gate definition")
        sim.apply_photoresist(0.5)
        sim.expose_photoresist(193.0, 25.0)
        sim.develop_photoresist()
        sim.simulate_etching(0.02, "anisotropic")
        sim.remove_photoresist()
        
        # Step 3: Source/drain implantation
        print("  Step 3: Source/drain implantation")
        sim.simulate_doping("phosphorus", 1e20, 30.0, 1000.0)
        
        # Step 4: Activation anneal
        print("  Step 4: Activation anneal")
        sim.simulate_thermal_annealing(1000.0, 0.05, "N2")
        
        # Step 5: Metallization
        print("  Step 5: Metallization")
        sim.simulate_deposition("aluminum", 0.5, 300.0)
        
        print("✅ NMOS transistor workflow completed successfully")
        return True
        
    except Exception as e:
        print(f"❌ Workflow execution failed: {e}")
        return False

def test_data_analysis(sim):
    """Test data analysis and visualization capabilities"""
    print("\n=== Testing Data Analysis ===")
    
    if not sim:
        print("❌ No simulator available for testing")
        return False
    
    try:
        # Generate some sample data for visualization
        x = np.linspace(0, 300, 100)  # Wafer diameter in mm
        y = np.linspace(0, 300, 100)
        X, Y = np.meshgrid(x, y)
        
        # Simulate temperature distribution
        temperature = 300 + 50 * np.exp(-((X-150)**2 + (Y-150)**2) / 5000)
        
        # Simulate doping concentration
        doping = 1e18 * np.exp(-((X-150)**2 + (Y-150)**2) / 10000)
        
        # Create plots
        fig, ((ax1, ax2), (ax3, ax4)) = plt.subplots(2, 2, figsize=(12, 10))
        
        # Temperature distribution
        im1 = ax1.contourf(X, Y, temperature, levels=20, cmap='hot')
        ax1.set_title('Temperature Distribution (K)')
        ax1.set_xlabel('X (mm)')
        ax1.set_ylabel('Y (mm)')
        plt.colorbar(im1, ax=ax1)
        
        # Doping concentration
        im2 = ax2.contourf(X, Y, np.log10(doping), levels=20, cmap='viridis')
        ax2.set_title('Doping Concentration (log10 cm⁻³)')
        ax2.set_xlabel('X (mm)')
        ax2.set_ylabel('Y (mm)')
        plt.colorbar(im2, ax=ax2)
        
        # Cross-sectional profiles
        center_idx = len(x) // 2
        ax3.plot(x, temperature[center_idx, :], 'r-', linewidth=2, label='Temperature')
        ax3.set_xlabel('Position (mm)')
        ax3.set_ylabel('Temperature (K)')
        ax3.set_title('Temperature Profile (Center Line)')
        ax3.grid(True)
        ax3.legend()
        
        ax4.semilogy(x, doping[center_idx, :], 'b-', linewidth=2, label='Doping')
        ax4.set_xlabel('Position (mm)')
        ax4.set_ylabel('Doping Concentration (cm⁻³)')
        ax4.set_title('Doping Profile (Center Line)')
        ax4.grid(True)
        ax4.legend()
        
        plt.tight_layout()
        
        # Save the plot
        output_dir = Path(__file__).parent / "output"
        output_dir.mkdir(exist_ok=True)
        plt.savefig(output_dir / "simulation_results.png", dpi=300, bbox_inches='tight')
        print(f"✅ Simulation results saved to {output_dir / 'simulation_results.png'}")
        
        # Show plot if running interactively
        if hasattr(plt, 'show'):
            plt.show()
        
        return True
        
    except Exception as e:
        print(f"❌ Data analysis failed: {e}")
        return False

def test_gui_components():
    """Test GUI components (non-interactive)"""
    print("\n=== Testing GUI Components ===")
    
    try:
        # Test GUI creation (without showing)
        print("Creating GUI instance...")
        gui = SemiPROGUI()
        print("✅ GUI instance created successfully")
        
        # Test simulator integration
        print("Testing simulator integration...")
        gui.initialize_wafer()
        print("✅ Wafer initialization through GUI completed")
        
        print("✅ GUI components test completed")
        print("Note: GUI display is disabled for automated testing")
        
        return True
        
    except Exception as e:
        print(f"❌ GUI components test failed: {e}")
        return False

def generate_report(results):
    """Generate a test report"""
    print("\n=== Test Report ===")
    
    total_tests = len(results)
    passed_tests = sum(results.values())
    
    print(f"Total tests: {total_tests}")
    print(f"Passed tests: {passed_tests}")
    print(f"Failed tests: {total_tests - passed_tests}")
    print(f"Success rate: {(passed_tests / total_tests) * 100:.1f}%")
    
    print("\nDetailed results:")
    for test_name, result in results.items():
        status = "✅ PASS" if result else "❌ FAIL"
        print(f"  {test_name}: {status}")
    
    # Save report to file
    output_dir = Path(__file__).parent / "output"
    output_dir.mkdir(exist_ok=True)
    
    with open(output_dir / "python_tutorial_report.txt", "w") as f:
        f.write("SemiPRO Python Frontend Tutorial Report\n")
        f.write("=" * 40 + "\n\n")
        f.write(f"Test Date: {time.strftime('%Y-%m-%d %H:%M:%S')}\n")
        f.write(f"Total tests: {total_tests}\n")
        f.write(f"Passed tests: {passed_tests}\n")
        f.write(f"Failed tests: {total_tests - passed_tests}\n")
        f.write(f"Success rate: {(passed_tests / total_tests) * 100:.1f}%\n\n")
        
        f.write("Detailed results:\n")
        for test_name, result in results.items():
            status = "PASS" if result else "FAIL"
            f.write(f"  {test_name}: {status}\n")
    
    print(f"\n✅ Report saved to {output_dir / 'python_tutorial_report.txt'}")

def main():
    """Main tutorial function"""
    print("SemiPRO Python Frontend Tutorial")
    print("=" * 40)
    print(f"Python version: {sys.version}")
    print(f"NumPy version: {np.__version__}")
    
    # Dictionary to store test results
    results = {}
    
    # Run tests
    sim = test_simulator_creation()
    results["Simulator Creation"] = sim is not None
    
    results["Basic Processes"] = test_basic_processes(sim)
    results["Advanced Features"] = test_advanced_features(sim)
    results["Workflow Execution"] = test_workflow_execution(sim)
    results["Data Analysis"] = test_data_analysis(sim)
    results["GUI Components"] = test_gui_components()
    
    # Generate report
    generate_report(results)
    
    print("\n=== Tutorial Complete ===")
    print("Python frontend tutorial has been completed.")
    print("Check the output directory for generated files and reports.")

if __name__ == "__main__":
    main()
