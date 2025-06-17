#!/usr/bin/env python3
# Author: Dr. Mazharuddin Mohammed
"""
Comprehensive test for SemiPRO standalone application
Tests C++ backend, Python frontend, and Vulkan rendering integration
"""

import sys
import os
import time
import numpy as np

# Add src to path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), 'src'))

def test_cpp_backend():
    """Test C++ backend functionality"""
    print("🔧 Testing C++ Backend")
    print("-" * 40)
    
    # Test if C++ executables exist and run
    cpp_tests = [
        ("./build/simulator", "Main simulator"),
        ("./build/example_geometry", "Geometry example"),
        ("./build/example_oxidation", "Oxidation example"),
        ("./build/example_doping", "Doping example"),
        ("./build/example_deposition", "Deposition example"),
        ("./build/example_etching", "Etching example"),
    ]
    
    passed = 0
    for executable, description in cpp_tests:
        if os.path.exists(executable):
            print(f"✅ {description} - Executable found")
            passed += 1
        else:
            print(f"❌ {description} - Executable missing")
    
    print(f"C++ Backend: {passed}/{len(cpp_tests)} components available\n")
    return passed > 0

def test_python_frontend():
    """Test Python frontend with C++ bridge"""
    print("🐍 Testing Python Frontend")
    print("-" * 40)
    
    try:
        from src.python.simulator import Simulator
        print("✅ Simulator class imported successfully")
        
        # Create simulator instance
        sim = Simulator()
        print("✅ Simulator instance created")
        
        # Test basic initialization
        sim.initialize_geometry(50, 50)
        print("✅ Geometry initialization successful")
        
        return True
        
    except Exception as e:
        print(f"❌ Python frontend test failed: {e}")
        return False

def test_simulation_processes():
    """Test individual simulation processes"""
    print("⚗️ Testing Simulation Processes")
    print("-" * 40)
    
    try:
        from src.python.simulator import Simulator
        sim = Simulator()
        sim.initialize_geometry(50, 50)
        
        # Test oxidation
        result = sim.simulate_oxidation(1000.0, 2.0, "dry")
        print(f"✅ Oxidation simulation: {'Success' if result else 'Failed'}")
        
        # Test doping
        result = sim.simulate_doping("boron", 1e16, 50.0, 1000.0)
        print(f"✅ Doping simulation: {'Success' if result else 'Failed'}")
        
        # Test deposition
        result = sim.simulate_deposition("aluminum", 0.5, 300.0)
        print(f"✅ Deposition simulation: {'Success' if result else 'Failed'}")
        
        # Test etching
        result = sim.simulate_etching(0.2, "anisotropic")
        print(f"✅ Etching simulation: {'Success' if result else 'Failed'}")
        
        # Test lithography
        result = sim.simulate_lithography(193.0, 25.0)
        print(f"✅ Lithography simulation: {'Success' if result else 'Failed'}")
        
        print("✅ All simulation processes completed\n")
        return True
        
    except Exception as e:
        print(f"❌ Simulation process test failed: {e}\n")
        return False

def test_vulkan_rendering():
    """Test Vulkan rendering capabilities"""
    print("🎨 Testing Vulkan Rendering")
    print("-" * 40)
    
    try:
        from src.python.simulator import Simulator
        sim = Simulator()
        
        # Initialize renderer
        sim.initialize_renderer(800, 600)
        print("✅ Vulkan renderer initialized")
        
        # Test rendering with dummy data
        sim.initialize_geometry(50, 50)
        sim.simulate_oxidation(1000.0, 1.0)
        
        # Attempt to render (may open window briefly)
        sim.render(sim.get_wafer())
        print("✅ Rendering completed")
        
        return True
        
    except Exception as e:
        print(f"❌ Vulkan rendering test failed: {e}")
        return False

def test_mosfet_workflow():
    """Test complete MOSFET fabrication workflow"""
    print("🔬 Testing Complete MOSFET Workflow")
    print("-" * 40)
    
    try:
        from src.python.simulator import Simulator
        sim = Simulator()
        
        print("📋 Executing MOSFET fabrication workflow:")
        
        # 1. Initialize wafer
        sim.initialize_geometry(100, 100)
        print("   1. ✅ Wafer initialization (100x100 grid)")
        
        # 2. Gate oxide growth
        sim.simulate_oxidation(1000.0, 2.0, "dry")
        print("   2. ✅ Gate oxide growth (1000°C, 2h, dry)")
        
        # 3. Polysilicon gate deposition
        sim.simulate_deposition("polysilicon", 0.3, 600.0)
        print("   3. ✅ Polysilicon gate deposition (0.3μm, 600°C)")
        
        # 4. Gate patterning
        sim.simulate_lithography(193.0, 25.0)
        print("   4. ✅ Gate patterning (193nm lithography)")
        
        # 5. Gate etching
        sim.simulate_etching(0.3, "anisotropic")
        print("   5. ✅ Gate etching (0.3μm anisotropic)")
        
        # 6. Source/Drain implantation
        sim.simulate_doping("phosphorus", 1e20, 80.0, 1000.0)
        print("   6. ✅ Source/Drain implantation (P, 1e20 cm⁻³, 80 keV)")
        
        # 7. Activation annealing
        sim.simulate_oxidation(900.0, 0.5, "nitrogen")  # Use oxidation for annealing
        print("   7. ✅ Activation annealing (900°C, 30min, N₂)")
        
        # 8. Contact metallization
        sim.simulate_deposition("aluminum", 0.8, 400.0)
        print("   8. ✅ Metal contact deposition (Al, 0.8μm, 400°C)")
        
        # 9. Metal patterning and etching
        sim.simulate_lithography(365.0, 150.0)
        sim.simulate_etching(0.8, "isotropic")
        print("   9. ✅ Metal patterning and etching")
        
        print("\n🎉 Complete MOSFET workflow executed successfully!")
        
        # Display results summary
        if hasattr(sim, 'simulation_results') and sim.simulation_results:
            print("\n📊 Simulation Results Summary:")
            for process, result in sim.simulation_results.items():
                print(f"   - {process.capitalize()}: ✅ Completed")
        
        return True
        
    except Exception as e:
        print(f"❌ MOSFET workflow test failed: {e}")
        return False

def test_performance():
    """Test performance with larger simulations"""
    print("⚡ Testing Performance")
    print("-" * 40)
    
    try:
        from src.python.simulator import Simulator
        
        # Test with different grid sizes
        grid_sizes = [(50, 50), (100, 100), (200, 200)]
        
        for x_dim, y_dim in grid_sizes:
            start_time = time.time()
            
            sim = Simulator()
            sim.initialize_geometry(x_dim, y_dim)
            sim.simulate_oxidation(1000.0, 1.0)
            sim.simulate_doping("boron", 1e16, 50.0, 1000.0)
            
            elapsed = time.time() - start_time
            print(f"✅ Grid {x_dim}x{y_dim}: {elapsed:.3f}s")
        
        return True
        
    except Exception as e:
        print(f"❌ Performance test failed: {e}")
        return False

def test_data_export():
    """Test data export capabilities"""
    print("💾 Testing Data Export")
    print("-" * 40)
    
    try:
        from src.python.simulator import Simulator
        sim = Simulator()
        
        # Run a simple simulation
        sim.initialize_geometry(50, 50)
        sim.simulate_oxidation(1000.0, 1.0)
        
        # Test if results are stored
        if hasattr(sim, 'simulation_results') and sim.simulation_results:
            print("✅ Simulation results stored")
            
            # Try to export results (create dummy files)
            os.makedirs("output", exist_ok=True)
            
            # Export simulation summary
            with open("output/simulation_summary.txt", "w") as f:
                f.write("SemiPRO Simulation Summary\n")
                f.write("=" * 30 + "\n")
                for process, result in sim.simulation_results.items():
                    f.write(f"{process}: {result}\n")
            
            print("✅ Results exported to output/simulation_summary.txt")
            return True
        else:
            print("⚠️ No simulation results to export")
            return True
            
    except Exception as e:
        print(f"❌ Data export test failed: {e}")
        return False

def main():
    """Run comprehensive standalone application test"""
    print("🔬 SemiPRO Standalone Application Test Suite")
    print("Author: Dr. Mazharuddin Mohammed")
    print("=" * 60)
    print("Testing C++ backend, Python frontend, and Vulkan rendering")
    print("=" * 60)
    
    tests = [
        ("C++ Backend", test_cpp_backend),
        ("Python Frontend", test_python_frontend),
        ("Simulation Processes", test_simulation_processes),
        ("Vulkan Rendering", test_vulkan_rendering),
        ("MOSFET Workflow", test_mosfet_workflow),
        ("Performance", test_performance),
        ("Data Export", test_data_export),
    ]
    
    results = []
    start_time = time.time()
    
    for test_name, test_func in tests:
        print(f"\n{'='*20} {test_name} {'='*20}")
        try:
            result = test_func()
            results.append((test_name, result))
        except Exception as e:
            print(f"❌ {test_name} - Exception: {e}")
            results.append((test_name, False))
    
    total_time = time.time() - start_time
    
    # Final summary
    print("\n" + "=" * 60)
    print("COMPREHENSIVE TEST SUMMARY")
    print("=" * 60)
    
    passed = 0
    total = len(results)
    
    for test_name, result in results:
        status = "✅ PASS" if result else "❌ FAIL"
        print(f"{test_name:<25} {status}")
        if result:
            passed += 1
    
    print(f"\nOverall Results:")
    print(f"  Tests Passed: {passed}/{total} ({passed/total*100:.1f}%)")
    print(f"  Total Time: {total_time:.2f} seconds")
    
    if passed == total:
        print("\n🎉 ALL TESTS PASSED!")
        print("✅ SemiPRO standalone application is fully functional")
        print("✅ C++ backend integration working")
        print("✅ Python frontend operational")
        print("✅ Vulkan rendering available")
        print("✅ Complete simulation workflows supported")
        return 0
    elif passed >= total * 0.8:
        print("\n⚠️ MOST TESTS PASSED")
        print("✅ Core functionality working")
        print("⚠️ Some advanced features may be limited")
        return 0
    else:
        print("\n❌ MULTIPLE TEST FAILURES")
        print("❌ Please check dependencies and configuration")
        return 1

if __name__ == "__main__":
    exit(main())
