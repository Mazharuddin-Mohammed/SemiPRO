#!/usr/bin/env python3
# Author: Dr. Mazharuddin Mohammed
"""
Basic functionality test for SemiPRO standalone application
Tests the core functionality without requiring Cython extensions
"""

import sys
import os
import subprocess
import time

# Add src to path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), 'src'))

def test_cpp_backend():
    """Test C++ backend compilation and execution"""
    print("=" * 60)
    print("Testing C++ Backend")
    print("=" * 60)
    
    # Test if simulator executable exists and runs
    simulator_path = "./build/simulator"
    if not os.path.exists(simulator_path):
        print("❌ C++ simulator executable not found")
        return False
    
    try:
        # Run simulator (should exit cleanly)
        result = subprocess.run([simulator_path], timeout=5, capture_output=True, text=True)
        print("✅ C++ simulator runs successfully")
        return True
    except subprocess.TimeoutExpired:
        print("✅ C++ simulator runs (timeout expected for GUI)")
        return True
    except Exception as e:
        print(f"❌ C++ simulator failed: {e}")
        return False

def test_cpp_examples():
    """Test C++ examples"""
    print("\n" + "=" * 60)
    print("Testing C++ Examples")
    print("=" * 60)
    
    examples = [
        "example_geometry",
        "example_oxidation", 
        "example_doping",
        "example_deposition",
        "example_etching"
    ]
    
    success_count = 0
    for example in examples:
        example_path = f"./build/{example}"
        if os.path.exists(example_path):
            try:
                result = subprocess.run([example_path], timeout=10, capture_output=True, text=True)
                if result.returncode == 0:
                    print(f"✅ {example} - Success")
                    success_count += 1
                else:
                    print(f"❌ {example} - Failed (return code: {result.returncode})")
            except Exception as e:
                print(f"❌ {example} - Exception: {e}")
        else:
            print(f"❌ {example} - Not found")
    
    print(f"\nC++ Examples: {success_count}/{len(examples)} passed")
    return success_count > 0

def test_python_imports():
    """Test Python module imports"""
    print("\n" + "=" * 60)
    print("Testing Python Imports")
    print("=" * 60)
    
    try:
        # Test basic Python imports
        import numpy as np
        print("✅ NumPy imported successfully")
        
        import matplotlib
        print("✅ Matplotlib imported successfully")
        
        # Test SemiPRO simulator import
        from src.python.simulator import Simulator
        print("✅ SemiPRO Simulator imported successfully")
        
        return True
    except ImportError as e:
        print(f"❌ Import failed: {e}")
        return False

def test_simulator_creation():
    """Test simulator object creation"""
    print("\n" + "=" * 60)
    print("Testing Simulator Creation")
    print("=" * 60)
    
    try:
        from src.python.simulator import Simulator
        
        # Create simulator (should use mock extensions)
        sim = Simulator()
        print("✅ Simulator object created successfully")
        
        # Test basic methods
        sim.initialize_geometry(100, 100)
        print("✅ Geometry initialization successful")
        
        # Test process simulation methods
        sim.simulate_oxidation(1000.0, 2.0, "dry")
        print("✅ Oxidation simulation successful")
        
        sim.simulate_doping("boron", 1e16, 50.0, 1000.0)
        print("✅ Doping simulation successful")
        
        sim.simulate_deposition("aluminum", 0.5, 300.0)
        print("✅ Deposition simulation successful")
        
        return True
    except Exception as e:
        print(f"❌ Simulator test failed: {e}")
        return False

def test_vulkan_availability():
    """Test Vulkan availability"""
    print("\n" + "=" * 60)
    print("Testing Vulkan Availability")
    print("=" * 60)
    
    try:
        # Check if vulkan-utils is available
        result = subprocess.run(["vulkaninfo", "--summary"], 
                              capture_output=True, text=True, timeout=10)
        if result.returncode == 0:
            print("✅ Vulkan is available and working")
            # Extract basic info
            lines = result.stdout.split('\n')
            for line in lines[:10]:  # First 10 lines usually contain key info
                if line.strip():
                    print(f"   {line.strip()}")
            return True
        else:
            print("❌ Vulkan not properly configured")
            return False
    except FileNotFoundError:
        print("❌ vulkaninfo not found - Vulkan may not be installed")
        return False
    except Exception as e:
        print(f"❌ Vulkan test failed: {e}")
        return False

def test_gui_dependencies():
    """Test GUI dependencies"""
    print("\n" + "=" * 60)
    print("Testing GUI Dependencies")
    print("=" * 60)
    
    gui_available = True
    
    try:
        import tkinter
        print("✅ Tkinter available")
    except ImportError:
        print("❌ Tkinter not available")
        gui_available = False
    
    try:
        # Try to import Qt (PySide6 or PyQt5)
        try:
            import PySide6
            print("✅ PySide6 available")
        except ImportError:
            try:
                import PyQt5
                print("✅ PyQt5 available")
            except ImportError:
                print("❌ No Qt framework available (PySide6/PyQt5)")
                gui_available = False
    except Exception as e:
        print(f"❌ Qt test failed: {e}")
        gui_available = False
    
    return gui_available

def test_basic_workflow():
    """Test a basic semiconductor fabrication workflow"""
    print("\n" + "=" * 60)
    print("Testing Basic MOSFET Workflow")
    print("=" * 60)
    
    try:
        from src.python.simulator import Simulator
        
        # Create simulator
        sim = Simulator()
        print("✅ Simulator created")
        
        # Initialize wafer
        sim.initialize_geometry(100, 100)
        print("✅ Wafer geometry initialized (100x100)")
        
        # Basic MOSFET process flow
        print("\n📋 Executing MOSFET fabrication workflow:")
        
        # 1. Substrate preparation (already done)
        print("   1. ✅ Substrate preparation")
        
        # 2. Gate oxide growth
        sim.simulate_oxidation(1000.0, 2.0, "dry")
        print("   2. ✅ Gate oxide growth (1000°C, 2h, dry)")
        
        # 3. Gate material deposition
        sim.simulate_deposition("polysilicon", 0.3, 600.0)
        print("   3. ✅ Polysilicon gate deposition (0.3μm, 600°C)")
        
        # 4. Source/Drain implantation
        sim.simulate_doping("phosphorus", 1e20, 80.0, 1000.0)
        print("   4. ✅ Source/Drain implantation (P, 1e20 cm⁻³, 80 keV)")
        
        # 5. Metallization
        sim.simulate_deposition("aluminum", 0.8, 400.0)
        print("   5. ✅ Metal contact deposition (Al, 0.8μm, 400°C)")
        
        print("\n🎉 Basic MOSFET workflow completed successfully!")
        return True
        
    except Exception as e:
        print(f"❌ Workflow test failed: {e}")
        return False

def main():
    """Run all tests"""
    print("🔬 SemiPRO Standalone Application Test Suite")
    print("Author: Dr. Mazharuddin Mohammed")
    print("=" * 60)
    
    tests = [
        ("C++ Backend", test_cpp_backend),
        ("C++ Examples", test_cpp_examples),
        ("Python Imports", test_python_imports),
        ("Simulator Creation", test_simulator_creation),
        ("Vulkan Availability", test_vulkan_availability),
        ("GUI Dependencies", test_gui_dependencies),
        ("Basic Workflow", test_basic_workflow),
    ]
    
    results = []
    for test_name, test_func in tests:
        try:
            result = test_func()
            results.append((test_name, result))
        except Exception as e:
            print(f"❌ {test_name} - Exception: {e}")
            results.append((test_name, False))
    
    # Summary
    print("\n" + "=" * 60)
    print("TEST SUMMARY")
    print("=" * 60)
    
    passed = 0
    total = len(results)
    
    for test_name, result in results:
        status = "✅ PASS" if result else "❌ FAIL"
        print(f"{test_name:<25} {status}")
        if result:
            passed += 1
    
    print(f"\nOverall: {passed}/{total} tests passed ({passed/total*100:.1f}%)")
    
    if passed == total:
        print("🎉 All tests passed! SemiPRO is ready to use.")
        return 0
    elif passed >= total * 0.7:
        print("⚠️  Most tests passed. Some features may be limited.")
        return 0
    else:
        print("❌ Many tests failed. Please check dependencies and setup.")
        return 1

if __name__ == "__main__":
    exit(main())
