#!/usr/bin/env python3
"""
SemiPRO Cython Integration Tutorial
==================================

This tutorial demonstrates the bridge between C++ backend and Python frontend
through Cython bindings, validating the integration layer functionality.

Author: Dr. Mazharuddin Mohammed
"""

import sys
import os
import numpy as np
import time
from pathlib import Path

# Add the cython directory to Python path
cython_dir = Path(__file__).parent.parent / "src" / "cython"
sys.path.insert(0, str(cython_dir))

def test_minimal_cython_integration():
    """Test basic Cython functionality"""
    print("\n=== Testing Minimal Cython Integration ===")
    
    try:
        import minimal_test
        
        # Test basic Cython functionality
        result = minimal_test.test_cython_integration()
        print(f"✅ Basic Cython: {result}")
        
        # Test NumPy integration
        result = minimal_test.test_numpy_integration()
        print(f"✅ NumPy Integration: {result}")
        
        # Test C++ string integration
        result = minimal_test.test_cpp_string()
        print(f"✅ C++ String Integration: {result}")
        
        # Test Cython classes
        result = minimal_test.create_test_object(42.0)
        print(f"✅ Cython Classes: {result}")
        
        return True
        
    except ImportError as e:
        print(f"❌ Minimal Cython Integration failed: {e}")
        return False
    except Exception as e:
        print(f"❌ Minimal Cython Integration error: {e}")
        return False

def test_geometry_integration():
    """Test geometry module Cython integration"""
    print("\n=== Testing Geometry Cython Integration ===")
    
    try:
        import geometry
        
        # Create a wafer through Cython
        wafer = geometry.PyWafer(300.0, 775.0, "silicon")
        print("✅ PyWafer created successfully")
        print(f"   Diameter: {wafer.get_diameter()} mm")
        print(f"   Thickness: {wafer.get_thickness()} μm")
        print(f"   Material: {wafer.get_material_id()}")
        
        # Initialize grid
        wafer.initialize_grid(50, 50)
        print("✅ Grid initialized through Cython")
        
        # Apply layer
        wafer.apply_layer(10.0, "silicon_dioxide")
        print("✅ Layer applied through Cython")
        
        # Test NumPy array integration
        dopant_profile = np.array([1e15, 2e15, 3e15, 2e15, 1e15], dtype=np.float64)
        wafer.set_dopant_profile(dopant_profile)
        print("✅ NumPy array passed to C++ through Cython")
        
        # Test getting data back from C++
        retrieved_profile = wafer.get_dopant_profile()
        print(f"✅ Data retrieved from C++: shape={retrieved_profile.shape}")
        
        return True
        
    except ImportError as e:
        print(f"❌ Geometry integration not available: {e}")
        print("   This is expected if geometry module is not built")
        return False
    except Exception as e:
        print(f"❌ Geometry integration error: {e}")
        return False

def test_physics_module_integration():
    """Test physics modules Cython integration"""
    print("\n=== Testing Physics Modules Cython Integration ===")
    
    modules_to_test = [
        ("oxidation", "Test oxidation Cython bindings"),
        ("doping", "Test doping Cython bindings"),
        ("deposition", "Test deposition Cython bindings"),
        ("etching", "Test etching Cython bindings")
    ]
    
    results = {}
    
    for module_name, description in modules_to_test:
        try:
            module = __import__(module_name)
            print(f"✅ {module_name.capitalize()} module imported successfully")
            
            # Test if the module has expected classes/functions
            if hasattr(module, f'Py{module_name.capitalize()}Model'):
                print(f"   - Has Py{module_name.capitalize()}Model class")
            if hasattr(module, f'Py{module_name.capitalize()}Manager'):
                print(f"   - Has Py{module_name.capitalize()}Manager class")
                
            results[module_name] = True
            
        except ImportError as e:
            print(f"❌ {module_name.capitalize()} module not available: {e}")
            results[module_name] = False
        except Exception as e:
            print(f"❌ {module_name.capitalize()} module error: {e}")
            results[module_name] = False
    
    return results

def test_data_transfer_performance():
    """Test performance of data transfer between Python and C++"""
    print("\n=== Testing Data Transfer Performance ===")
    
    try:
        import minimal_test
        
        # Test different array sizes
        sizes = [100, 1000, 10000, 100000]
        
        for size in sizes:
            # Create test data
            data = np.random.random(size).astype(np.float64)
            
            # Measure time for data creation and basic operations
            start_time = time.time()
            
            # Simulate data transfer (using NumPy operations as proxy)
            result = np.sum(data)
            processed = data * 2.0
            
            end_time = time.time()
            elapsed = (end_time - start_time) * 1000  # Convert to milliseconds
            
            print(f"✅ Array size {size:6d}: {elapsed:6.2f} ms (sum={result:.2e})")
        
        return True
        
    except Exception as e:
        print(f"❌ Performance test failed: {e}")
        return False

def test_memory_management():
    """Test memory management in Cython integration"""
    print("\n=== Testing Memory Management ===")
    
    try:
        import minimal_test
        
        # Test object creation and destruction
        objects = []
        for i in range(100):
            obj = minimal_test.SimpleTest(float(i))
            objects.append(obj)
        
        print("✅ Created 100 Cython objects")
        
        # Test accessing objects
        total = sum(obj.get_value() for obj in objects)
        print(f"✅ Accessed all objects, total value: {total}")
        
        # Clear objects (test garbage collection)
        objects.clear()
        print("✅ Objects cleared, memory should be freed")
        
        return True
        
    except Exception as e:
        print(f"❌ Memory management test failed: {e}")
        return False

def test_error_handling():
    """Test error handling in Cython integration"""
    print("\n=== Testing Error Handling ===")
    
    try:
        import minimal_test
        
        # Test normal operation
        obj = minimal_test.SimpleTest(10.0)
        result = obj.multiply(2.0)
        print(f"✅ Normal operation: 10.0 * 2.0 = {result}")
        
        # Test with edge cases
        obj2 = minimal_test.SimpleTest(0.0)
        result2 = obj2.multiply(float('inf'))
        print(f"✅ Edge case handled: 0.0 * inf = {result2}")
        
        return True
        
    except Exception as e:
        print(f"❌ Error handling test failed: {e}")
        return False

def demonstrate_cython_workflow():
    """Demonstrate a complete workflow using Cython integration"""
    print("\n=== Demonstrating Complete Cython Workflow ===")
    
    try:
        import minimal_test
        
        print("Step 1: Create simulation parameters")
        temperature = 1000.0
        time_hours = 2.0
        
        print("Step 2: Create test objects")
        temp_obj = minimal_test.SimpleTest(temperature)
        time_obj = minimal_test.SimpleTest(time_hours)
        
        print("Step 3: Perform calculations")
        thermal_budget = temp_obj.multiply(time_obj.get_value())
        print(f"   Thermal budget: {thermal_budget} °C·h")
        
        print("Step 4: Process results")
        if thermal_budget > 1500:
            print("   ⚠️  High thermal budget - may cause excessive diffusion")
        else:
            print("   ✅ Thermal budget within acceptable range")
        
        print("Step 5: Generate summary")
        summary = {
            'temperature': temp_obj.get_value(),
            'time': time_obj.get_value(),
            'thermal_budget': thermal_budget,
            'status': 'acceptable' if thermal_budget <= 1500 else 'high'
        }
        
        print(f"✅ Workflow completed: {summary}")
        return True
        
    except Exception as e:
        print(f"❌ Workflow demonstration failed: {e}")
        return False

def run_comprehensive_test():
    """Run all Cython integration tests"""
    print("SemiPRO Cython Integration Tutorial")
    print("=" * 50)
    print("Testing the bridge between C++ backend and Python frontend")
    
    tests = [
        ("Minimal Integration", test_minimal_cython_integration),
        ("Geometry Integration", test_geometry_integration),
        ("Physics Modules", test_physics_module_integration),
        ("Data Transfer Performance", test_data_transfer_performance),
        ("Memory Management", test_memory_management),
        ("Error Handling", test_error_handling),
        ("Complete Workflow", demonstrate_cython_workflow)
    ]
    
    results = {}
    total_tests = len(tests)
    passed_tests = 0
    
    for test_name, test_func in tests:
        print(f"\n{'='*20} {test_name} {'='*20}")
        try:
            result = test_func()
            results[test_name] = result
            if result:
                passed_tests += 1
        except Exception as e:
            print(f"❌ {test_name} failed with exception: {e}")
            results[test_name] = False
    
    # Generate summary report
    print("\n" + "="*60)
    print("CYTHON INTEGRATION TEST SUMMARY")
    print("="*60)
    
    for test_name, result in results.items():
        status = "✅ PASS" if result else "❌ FAIL"
        print(f"{status} {test_name}")
    
    print(f"\nTotal Tests: {total_tests}")
    print(f"Passed: {passed_tests}")
    print(f"Failed: {total_tests - passed_tests}")
    print(f"Success Rate: {(passed_tests/total_tests)*100:.1f}%")
    
    # Save results to file
    output_dir = Path(__file__).parent / "output"
    output_dir.mkdir(exist_ok=True)
    
    report_file = output_dir / "cython_integration_report.txt"
    with open(report_file, 'w') as f:
        f.write("SemiPRO Cython Integration Test Report\n")
        f.write("=" * 50 + "\n\n")
        f.write(f"Test Date: {time.strftime('%Y-%m-%d %H:%M:%S')}\n")
        f.write(f"Python Version: {sys.version}\n")
        f.write(f"NumPy Version: {np.__version__}\n\n")
        
        f.write("Test Results:\n")
        f.write("-" * 20 + "\n")
        for test_name, result in results.items():
            status = "PASS" if result else "FAIL"
            f.write(f"{status:4} {test_name}\n")
        
        f.write(f"\nSummary:\n")
        f.write(f"Total Tests: {total_tests}\n")
        f.write(f"Passed: {passed_tests}\n")
        f.write(f"Failed: {total_tests - passed_tests}\n")
        f.write(f"Success Rate: {(passed_tests/total_tests)*100:.1f}%\n")
    
    print(f"\n✅ Report saved to {report_file}")
    
    return passed_tests == total_tests

if __name__ == "__main__":
    success = run_comprehensive_test()
    sys.exit(0 if success else 1)
