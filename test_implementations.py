#!/usr/bin/env python3
"""
Test script to validate SemiPRO implementations
"""

import os
import sys
import time

def test_cpp_simulator():
    """Test the C++ simulator executable"""
    print("Testing C++ simulator...")
    
    # Test oxidation process
    cmd = "./build/simulator --process oxidation --config config/test_oxidation.json"
    print(f"Running: {cmd}")
    exit_code = os.system(cmd)
    if exit_code == 0:
        print("✅ Oxidation process test passed")
    else:
        print("❌ Oxidation process test failed")
    
    # Test etching process
    cmd = "./build/simulator --process etching --config config/test_etching.json"
    print(f"Running: {cmd}")
    exit_code = os.system(cmd)
    if exit_code == 0:
        print("✅ Etching process test passed")
    else:
        print("❌ Etching process test failed")
    
    # Test doping process with timeout
    cmd = "timeout 5s ./build/simulator --process doping --config config/test_doping.json"
    print(f"Running: {cmd}")
    exit_code = os.system(cmd)
    if exit_code == 0:
        print("✅ Doping process test passed")
    else:
        print("❌ Doping process test failed or timed out")
    
    # Test deposition process with timeout
    cmd = "timeout 5s ./build/simulator --process deposition --config config/test_deposition.json"
    print(f"Running: {cmd}")
    exit_code = os.system(cmd)
    if exit_code == 0:
        print("✅ Deposition process test passed")
    else:
        print("❌ Deposition process test failed or timed out")

def test_cython_modules():
    """Test the Cython module imports"""
    print("\nTesting Cython module imports...")
    
    try:
        sys.path.insert(0, os.path.abspath('.'))
        
        # Test geometry module
        try:
            from src.cython import geometry
            print("✅ Successfully imported geometry module")
        except ImportError as e:
            print(f"❌ Failed to import geometry module: {e}")
        
        # Test oxidation module
        try:
            from src.cython import oxidation
            print("✅ Successfully imported oxidation module")
        except ImportError as e:
            print(f"❌ Failed to import oxidation module: {e}")
        
        # Test doping module
        try:
            from src.cython import doping
            print("✅ Successfully imported doping module")
        except ImportError as e:
            print(f"❌ Failed to import doping module: {e}")
        
        # Test etching module
        try:
            from src.cython import etching
            print("✅ Successfully imported etching module")
        except ImportError as e:
            print(f"❌ Failed to import etching module: {e}")
        
        # Test deposition module
        try:
            from src.cython import deposition
            print("✅ Successfully imported deposition module")
        except ImportError as e:
            print(f"❌ Failed to import deposition module: {e}")
        
        # Test newly added modules
        try:
            from src.cython import defect_inspection
            print("✅ Successfully imported defect_inspection module")
        except ImportError as e:
            print(f"❌ Failed to import defect_inspection module: {e}")
        
        try:
            from src.cython import interconnect
            print("✅ Successfully imported interconnect module")
        except ImportError as e:
            print(f"❌ Failed to import interconnect module: {e}")
        
        try:
            from src.cython import metrology
            print("✅ Successfully imported metrology module")
        except ImportError as e:
            print(f"❌ Failed to import metrology module: {e}")
        
    except Exception as e:
        print(f"❌ Error during Cython module testing: {e}")

def test_python_frontend():
    """Test the Python frontend"""
    print("\nTesting Python frontend...")
    
    try:
        sys.path.insert(0, os.path.abspath('.'))
        
        # Test API client
        try:
            from src.python.api.client import SemiproClient
            print("✅ Successfully imported SemiproClient")
        except ImportError as e:
            print(f"❌ Failed to import SemiproClient: {e}")
        
        # Test GUI components
        try:
            from src.python.gui.main_window import MainWindow
            print("✅ Successfully imported MainWindow")
        except ImportError as e:
            print(f"❌ Failed to import MainWindow: {e}")
        
        # Test newly added GUI panels
        try:
            from src.python.gui.defect_inspection_panel import DefectInspectionPanel
            print("✅ Successfully imported DefectInspectionPanel")
        except ImportError as e:
            print(f"❌ Failed to import DefectInspectionPanel: {e}")
        
        try:
            from src.python.gui.interconnect_panel import InterconnectPanel
            print("✅ Successfully imported InterconnectPanel")
        except ImportError as e:
            print(f"❌ Failed to import InterconnectPanel: {e}")
        
        try:
            from src.python.gui.metrology_panel import MetrologyPanel
            print("✅ Successfully imported MetrologyPanel")
        except ImportError as e:
            print(f"❌ Failed to import MetrologyPanel: {e}")
        
    except Exception as e:
        print(f"❌ Error during Python frontend testing: {e}")

def test_tutorials():
    """Test that all tutorial files exist"""
    print("\nTesting tutorials...")
    
    tutorial_files = [
        "tutorials/cpp_core_tutorial.md",
        "tutorials/cython_integration_tutorial.md",
        "tutorials/python_frontend_tutorial.md"
    ]
    
    for file_path in tutorial_files:
        if os.path.exists(file_path):
            print(f"✅ Tutorial file exists: {file_path}")
        else:
            print(f"❌ Tutorial file missing: {file_path}")

def main():
    """Main test function"""
    print("=== SemiPRO Implementation Validation Tests ===\n")
    
    # Test C++ simulator
    test_cpp_simulator()
    
    # Test Cython modules
    test_cython_modules()
    
    # Test Python frontend
    test_python_frontend()
    
    # Test tutorials
    test_tutorials()
    
    print("\n=== Test Summary ===")
    print("C++ simulator: Tested core process modules")
    print("Cython modules: Tested module imports")
    print("Python frontend: Tested API and GUI components")
    print("Tutorials: Verified tutorial files exist")
    
    print("\nNote: These are basic validation tests. For comprehensive testing,")
    print("run the full test suite with 'python scripts/test_all_modules_systematically.py'")

if __name__ == "__main__":
    main()
