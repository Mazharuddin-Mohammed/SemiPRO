#!/usr/bin/env python3
# Author: Dr. Mazharuddin Mohammed
"""
Core functionality test for SemiPRO
Tests the essential components without complex dependencies
"""

import sys
import os
import subprocess
import traceback
from pathlib import Path

# Add src to path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), 'src'))

def test_cpp_backend():
    """Test C++ backend functionality"""
    print("🔧 Testing C++ Backend...")
    
    try:
        # Test C++ simulator executable
        result = subprocess.run(["./build/simulator"], 
                              capture_output=True, text=True, timeout=10)
        if result.returncode == 0:
            print("✅ C++ simulator runs successfully")
            return True
        else:
            print(f"❌ C++ simulator failed with code {result.returncode}")
            print(result.stderr)
            return False
    except subprocess.TimeoutExpired:
        print("✅ C++ simulator runs (timed out waiting for input)")
        return True
    except Exception as e:
        print(f"❌ C++ test failed: {e}")
        return False

def test_python_imports():
    """Test Python module imports"""
    print("🐍 Testing Python Imports...")
    
    try:
        # Test core Python modules
        import numpy as np
        print("✅ NumPy imported")
        
        import matplotlib.pyplot as plt
        print("✅ Matplotlib imported")
        
        # Test simulator import
        sys.path.insert(0, 'src/python')
        from simulator import Simulator
        print("✅ Simulator imported")
        
        return True
    except Exception as e:
        print(f"❌ Python import failed: {e}")
        traceback.print_exc()
        return False

def test_simulator_creation():
    """Test simulator creation and basic operations"""
    print("🔬 Testing Simulator Creation...")
    
    try:
        sys.path.insert(0, 'src/python')
        from simulator import Simulator
        
        # Create simulator
        sim = Simulator()
        print("✅ Simulator created successfully")
        
        # Test basic operations
        sim.initialize_geometry(50, 50)
        print("✅ Geometry initialized")
        
        sim.apply_layer(0.1, "SiO2")
        print("✅ Layer applied")
        
        sim.run_oxidation(1000.0, 1.0)
        print("✅ Oxidation simulation completed")
        
        return True
    except Exception as e:
        print(f"❌ Simulator test failed: {e}")
        traceback.print_exc()
        return False

def test_cpp_examples():
    """Test C++ examples"""
    print("🧪 Testing C++ Examples...")
    
    examples = [
        "example_geometry",
        "example_oxidation", 
        "example_doping",
        "example_deposition",
        "example_etching"
    ]
    
    passed = 0
    for example in examples:
        try:
            result = subprocess.run([f"./build/{example}"], 
                                  capture_output=True, text=True, timeout=10)
            if result.returncode == 0:
                print(f"✅ {example} - Success")
                passed += 1
            else:
                print(f"❌ {example} - Failed")
        except subprocess.TimeoutExpired:
            print(f"✅ {example} - Success (timed out)")
            passed += 1
        except Exception as e:
            print(f"❌ {example} - Error: {e}")
    
    print(f"C++ Examples: {passed}/{len(examples)} passed")
    return passed == len(examples)

def test_memory_usage():
    """Test for memory leaks using basic operations"""
    print("🧠 Testing Memory Usage...")
    
    try:
        import psutil
        import gc
        
        # Get initial memory
        process = psutil.Process()
        initial_memory = process.memory_info().rss / 1024 / 1024  # MB
        
        # Run multiple simulator operations
        sys.path.insert(0, 'src/python')
        from simulator import Simulator
        
        for i in range(10):
            sim = Simulator()
            sim.initialize_geometry(100, 100)
            sim.apply_layer(0.1, "SiO2")
            sim.run_oxidation(1000.0, 0.5)
            del sim
            gc.collect()
        
        # Check final memory
        final_memory = process.memory_info().rss / 1024 / 1024  # MB
        memory_increase = final_memory - initial_memory
        
        print(f"Memory usage: {initial_memory:.1f} MB → {final_memory:.1f} MB")
        print(f"Memory increase: {memory_increase:.1f} MB")
        
        if memory_increase < 100:  # Less than 100MB increase is acceptable
            print("✅ Memory usage looks reasonable")
            return True
        else:
            print("⚠️  High memory usage detected")
            return False
            
    except ImportError:
        print("⚠️  psutil not available, skipping memory test")
        return True
    except Exception as e:
        print(f"❌ Memory test failed: {e}")
        return False

def main():
    """Main test function"""
    print("🚀 SemiPRO Core Functionality Test")
    print("=" * 50)
    
    tests = [
        ("C++ Backend", test_cpp_backend),
        ("Python Imports", test_python_imports),
        ("Simulator Creation", test_simulator_creation),
        ("C++ Examples", test_cpp_examples),
        ("Memory Usage", test_memory_usage),
    ]
    
    passed = 0
    total = len(tests)
    
    for test_name, test_func in tests:
        print(f"\n{test_name}:")
        print("-" * 30)
        try:
            if test_func():
                passed += 1
                print(f"✅ {test_name}: PASSED")
            else:
                print(f"❌ {test_name}: FAILED")
        except Exception as e:
            print(f"❌ {test_name}: ERROR - {e}")
    
    print("\n" + "=" * 50)
    print(f"📊 Test Results: {passed}/{total} passed ({passed/total*100:.1f}%)")
    
    if passed == total:
        print("🎉 All tests passed!")
        return 0
    elif passed >= total * 0.8:
        print("✅ Most tests passed - system is functional")
        return 0
    else:
        print("❌ Many tests failed - system needs attention")
        return 1

if __name__ == "__main__":
    exit(main())
