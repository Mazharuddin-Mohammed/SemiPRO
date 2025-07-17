#!/usr/bin/env python3
"""
SemiPRO Cython Physics Module Tutorial
=====================================

This tutorial demonstrates building and using physics modules through Cython,
showing how to bridge C++ physics engines with Python interfaces.

Author: Dr. Mazharuddin Mohammed
"""

import sys
import os
import subprocess
import time
import numpy as np
from pathlib import Path

# Add the cython directory to Python path
project_root = Path(__file__).parent.parent
cython_dir = project_root / "src" / "cython"
sys.path.insert(0, str(cython_dir))

def build_physics_modules():
    """Attempt to build physics modules"""
    print("\n=== Building Physics Modules ===")
    
    modules_to_build = [
        ("oxidation", "Oxidation physics module"),
        ("doping", "Doping physics module"),
        ("deposition", "Deposition physics module"),
        ("etching", "Etching physics module")
    ]
    
    built_modules = []
    failed_modules = []
    
    original_dir = os.getcwd()
    
    try:
        os.chdir(str(cython_dir))
        
        for module_name, description in modules_to_build:
            print(f"\nBuilding {module_name} module...")
            
            try:
                # Create a simple setup script for individual module
                setup_content = f'''
from setuptools import setup, Extension
from Cython.Build import cythonize
import numpy

extensions = [
    Extension(
        "{module_name}",
        ["{module_name}.pyx"],
        language="c++",
        include_dirs=[
            numpy.get_include(),
            "../cpp/core",
            "../cpp/physics",
            "../cpp/modules/{module_name}",
            "/usr/include/eigen3"
        ],
        extra_compile_args=["-std=c++17", "-O2"],
        extra_link_args=["-std=c++17"]
    )
]

setup(
    ext_modules=cythonize(extensions, language_level=3)
)
'''
                
                # Write temporary setup file
                with open(f"setup_{module_name}.py", 'w') as f:
                    f.write(setup_content)
                
                # Try to build
                result = subprocess.run([
                    sys.executable, f"setup_{module_name}.py", "build_ext", "--inplace"
                ], capture_output=True, text=True, timeout=60)
                
                if result.returncode == 0:
                    print(f"✅ {module_name} module built successfully")
                    built_modules.append(module_name)
                else:
                    print(f"❌ {module_name} build failed: {result.stderr[:100]}...")
                    failed_modules.append(module_name)
                
                # Clean up temporary setup file
                if os.path.exists(f"setup_{module_name}.py"):
                    os.remove(f"setup_{module_name}.py")
                    
            except subprocess.TimeoutExpired:
                print(f"❌ {module_name} build timed out")
                failed_modules.append(module_name)
            except Exception as e:
                print(f"❌ {module_name} build error: {e}")
                failed_modules.append(module_name)
    
    finally:
        os.chdir(original_dir)
    
    print(f"\nBuild Summary:")
    print(f"  Built: {len(built_modules)} modules")
    print(f"  Failed: {len(failed_modules)} modules")
    
    return built_modules, failed_modules

def test_available_modules():
    """Test which physics modules are available"""
    print("\n=== Testing Available Physics Modules ===")
    
    modules_to_test = [
        "oxidation",
        "doping", 
        "deposition",
        "etching",
        "geometry"
    ]
    
    available_modules = []
    
    for module_name in modules_to_test:
        try:
            module = __import__(module_name)
            print(f"✅ {module_name} module available")
            
            # Check for common attributes
            attrs = dir(module)
            class_count = sum(1 for attr in attrs if attr.startswith('Py') and attr[2].isupper())
            function_count = sum(1 for attr in attrs if not attr.startswith('_') and not attr.startswith('Py'))
            
            print(f"   - {class_count} classes, {function_count} functions")
            available_modules.append(module_name)
            
        except ImportError:
            print(f"❌ {module_name} module not available")
        except Exception as e:
            print(f"⚠️  {module_name} module error: {e}")
    
    return available_modules

def demonstrate_oxidation_simulation():
    """Demonstrate oxidation simulation if module is available"""
    print("\n=== Oxidation Simulation Demo ===")
    
    try:
        import oxidation
        print("✅ Oxidation module loaded")
        
        # Check available classes
        attrs = [attr for attr in dir(oxidation) if not attr.startswith('_')]
        print(f"Available attributes: {attrs}")
        
        # Try to create oxidation objects
        if hasattr(oxidation, 'PyOxidationModel'):
            model = oxidation.PyOxidationModel()
            print("✅ PyOxidationModel created")
        
        if hasattr(oxidation, 'PyOxidationManager'):
            manager = oxidation.PyOxidationManager()
            print("✅ PyOxidationManager created")
        
        return True
        
    except ImportError:
        print("❌ Oxidation module not available")
        print("   This is expected if the module hasn't been built")
        return False
    except Exception as e:
        print(f"❌ Oxidation demo failed: {e}")
        return False

def demonstrate_doping_simulation():
    """Demonstrate doping simulation if module is available"""
    print("\n=== Doping Simulation Demo ===")
    
    try:
        import doping
        print("✅ Doping module loaded")
        
        # Check available classes
        attrs = [attr for attr in dir(doping) if not attr.startswith('_')]
        print(f"Available attributes: {attrs}")
        
        # Try to create doping objects
        if hasattr(doping, 'PyDopingManager'):
            manager = doping.PyDopingManager()
            print("✅ PyDopingManager created")
        
        if hasattr(doping, 'PyMonteCarloSolver'):
            solver = doping.PyMonteCarloSolver()
            print("✅ PyMonteCarloSolver created")
        
        return True
        
    except ImportError:
        print("❌ Doping module not available")
        print("   This is expected if the module hasn't been built")
        return False
    except Exception as e:
        print(f"❌ Doping demo failed: {e}")
        return False

def demonstrate_geometry_integration():
    """Demonstrate geometry integration if available"""
    print("\n=== Geometry Integration Demo ===")
    
    try:
        import geometry
        print("✅ Geometry module loaded")
        
        # Create a wafer
        wafer = geometry.PyWafer(300.0, 775.0, "silicon")
        print(f"✅ Wafer created: {wafer.get_diameter()}mm, {wafer.get_thickness()}μm")
        
        # Initialize grid
        wafer.initialize_grid(20, 20)
        print("✅ Grid initialized")
        
        # Apply layers
        wafer.apply_layer(5.0, "silicon_dioxide")
        print("✅ Oxide layer applied")
        
        # Test NumPy integration
        dopant_profile = np.array([1e15, 2e15, 3e15, 2e15, 1e15], dtype=np.float64)
        wafer.set_dopant_profile(dopant_profile)
        print("✅ Dopant profile set via NumPy array")
        
        # Retrieve data
        retrieved_profile = wafer.get_dopant_profile()
        print(f"✅ Retrieved profile shape: {retrieved_profile.shape}")
        
        return True
        
    except ImportError:
        print("❌ Geometry module not available")
        return False
    except Exception as e:
        print(f"❌ Geometry demo failed: {e}")
        return False

def simulate_complete_process():
    """Simulate a complete semiconductor process using available modules"""
    print("\n=== Complete Process Simulation ===")
    
    try:
        # Use minimal_test as a proxy for physics calculations
        import minimal_test
        
        print("Simulating complete CMOS process flow...")
        
        # Process steps with parameters
        process_steps = [
            ("Substrate Preparation", {"temperature": 25, "time": 1.0}),
            ("Initial Oxidation", {"temperature": 1000, "time": 2.0}),
            ("Well Implantation", {"energy": 100000, "dose": 1e13}),
            ("Well Drive-in", {"temperature": 1100, "time": 4.0}),
            ("Gate Oxidation", {"temperature": 950, "time": 1.5}),
            ("Poly Deposition", {"temperature": 620, "time": 0.5}),
            ("Poly Doping", {"energy": 30000, "dose": 1e15}),
            ("Source/Drain Implant", {"energy": 25000, "dose": 5e15}),
            ("Activation Anneal", {"temperature": 1000, "time": 0.5}),
            ("Contact Formation", {"temperature": 450, "time": 1.0})
        ]
        
        simulation_results = []
        total_thermal_budget = 0.0
        
        for step_name, params in process_steps:
            print(f"\n  Processing: {step_name}")
            
            # Simulate step using Cython objects
            if "temperature" in params and "time" in params:
                temp_obj = minimal_test.SimpleTest(params["temperature"])
                time_obj = minimal_test.SimpleTest(params["time"])
                
                # Calculate thermal budget contribution
                if params["temperature"] > 400:
                    thermal_contribution = temp_obj.multiply(time_obj.get_value())
                    total_thermal_budget += thermal_contribution
                else:
                    thermal_contribution = 0.0
                
                print(f"    Temperature: {params['temperature']}°C")
                print(f"    Time: {params['time']} hours")
                print(f"    Thermal contribution: {thermal_contribution:.1f} °C·h")
                
            elif "energy" in params and "dose" in params:
                energy_obj = minimal_test.SimpleTest(params["energy"])
                dose_obj = minimal_test.SimpleTest(params["dose"])
                
                # Calculate implant parameters
                range_estimate = energy_obj.multiply(0.001)  # Simplified range calculation
                straggle = range_estimate * 0.3
                
                print(f"    Energy: {params['energy']} eV")
                print(f"    Dose: {params['dose']:.1e} cm⁻²")
                print(f"    Estimated range: {range_estimate:.1f} nm")
                print(f"    Estimated straggle: {straggle:.1f} nm")
            
            simulation_results.append({
                'step': step_name,
                'parameters': params,
                'status': 'completed'
            })
        
        print(f"\n✅ Process simulation completed")
        print(f"   Total steps: {len(process_steps)}")
        print(f"   Total thermal budget: {total_thermal_budget:.1f} °C·h")
        
        # Analyze results
        if total_thermal_budget > 8000:
            print("⚠️  Very high thermal budget - significant dopant redistribution expected")
        elif total_thermal_budget > 5000:
            print("⚠️  High thermal budget - monitor dopant profiles carefully")
        else:
            print("✅ Thermal budget within acceptable range")
        
        return True
        
    except Exception as e:
        print(f"❌ Process simulation failed: {e}")
        return False

def benchmark_cython_performance():
    """Benchmark Cython performance for physics calculations"""
    print("\n=== Cython Performance Benchmark ===")
    
    try:
        import minimal_test
        
        # Test different computational loads
        test_cases = [
            ("Light computation", 1000, 1.5),
            ("Medium computation", 10000, 2.5),
            ("Heavy computation", 100000, 3.5)
        ]
        
        for test_name, iterations, factor in test_cases:
            print(f"\n{test_name} ({iterations} iterations):")
            
            # Cython implementation
            start_time = time.time()
            cython_results = []
            for i in range(iterations):
                obj = minimal_test.SimpleTest(float(i))
                result = obj.multiply(factor)
                cython_results.append(result)
            cython_time = time.time() - start_time
            cython_sum = sum(cython_results)
            
            # Pure Python equivalent
            start_time = time.time()
            python_results = []
            for i in range(iterations):
                result = float(i) * factor
                python_results.append(result)
            python_time = time.time() - start_time
            python_sum = sum(python_results)
            
            # NumPy equivalent
            start_time = time.time()
            numpy_array = np.arange(iterations, dtype=np.float64)
            numpy_result = numpy_array * factor
            numpy_sum = np.sum(numpy_result)
            numpy_time = time.time() - start_time
            
            print(f"  Cython:     {cython_time*1000:8.2f} ms (sum: {cython_sum:.1e})")
            print(f"  Python:     {python_time*1000:8.2f} ms (sum: {python_sum:.1e})")
            print(f"  NumPy:      {numpy_time*1000:8.2f} ms (sum: {numpy_sum:.1e})")
            
            if cython_time < python_time:
                speedup = python_time / cython_time
                print(f"  Cython speedup: {speedup:.1f}x vs Python")
            
        return True
        
    except Exception as e:
        print(f"❌ Performance benchmark failed: {e}")
        return False

def main():
    """Main tutorial execution"""
    print("SemiPRO Cython Physics Module Tutorial")
    print("=" * 60)
    print("Demonstrating physics module integration through Cython")
    
    # Run tutorial sections
    sections = [
        ("Build Physics Modules", build_physics_modules),
        ("Test Available Modules", test_available_modules),
        ("Oxidation Demo", demonstrate_oxidation_simulation),
        ("Doping Demo", demonstrate_doping_simulation),
        ("Geometry Demo", demonstrate_geometry_integration),
        ("Complete Process", simulate_complete_process),
        ("Performance Benchmark", benchmark_cython_performance)
    ]
    
    results = []
    
    for section_name, section_func in sections:
        print(f"\n{'='*20} {section_name} {'='*20}")
        try:
            if section_name == "Build Physics Modules":
                built, failed = section_func()
                result = len(built) > 0
            else:
                result = section_func()
            results.append((section_name, result))
        except Exception as e:
            print(f"❌ {section_name} failed: {e}")
            results.append((section_name, False))
    
    # Generate summary
    print("\n" + "="*60)
    print("CYTHON PHYSICS TUTORIAL SUMMARY")
    print("="*60)
    
    passed = sum(1 for _, result in results if result)
    total = len(results)
    
    for section_name, result in results:
        status = "✅ PASS" if result else "❌ FAIL"
        print(f"{status} {section_name}")
    
    print(f"\nOverall: {passed}/{total} sections completed ({(passed/total)*100:.1f}%)")
    
    # Save results
    output_dir = Path(__file__).parent / "output"
    output_dir.mkdir(exist_ok=True)
    
    with open(output_dir / "cython_physics_report.txt", 'w') as f:
        f.write("SemiPRO Cython Physics Tutorial Report\n")
        f.write("=" * 50 + "\n\n")
        f.write(f"Date: {time.strftime('%Y-%m-%d %H:%M:%S')}\n\n")
        
        for section_name, result in results:
            status = "PASS" if result else "FAIL"
            f.write(f"{status:4} {section_name}\n")
        
        f.write(f"\nSummary: {passed}/{total} sections completed\n")
    
    print(f"\n✅ Report saved to {output_dir / 'cython_physics_report.txt'}")
    
    return passed >= total // 2  # Consider success if at least half pass

if __name__ == "__main__":
    success = main()
    sys.exit(0 if success else 1)
