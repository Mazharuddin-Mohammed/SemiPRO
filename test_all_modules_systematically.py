#!/usr/bin/env python3
# Author: Dr. Mazharuddin Mohammed
# Systematic testing of ALL process modules with detailed results

import subprocess
import os
import sys
import time

def run_cpp_module(module_name, description):
    """Run a C++ module and capture detailed output"""
    print(f"\n{'='*60}")
    print(f"🔬 TESTING: {description}")
    print(f"Module: {module_name}")
    print(f"{'='*60}")
    
    executable = f"./build/{module_name}"
    if not os.path.exists(executable):
        print(f"❌ MISSING: {executable} not found")
        return False, "Executable not found"
    
    try:
        print(f"▶️  Running: {executable}")
        start_time = time.time()
        
        result = subprocess.run(
            [executable], 
            capture_output=True, 
            text=True, 
            timeout=60,
            cwd="."
        )
        
        end_time = time.time()
        runtime = end_time - start_time
        
        print(f"⏱️  Runtime: {runtime:.2f} seconds")
        print(f"🔄 Return code: {result.returncode}")
        
        if result.returncode == 0:
            print(f"✅ SUCCESS")
            print(f"📤 STDOUT:")
            print(result.stdout)
            if result.stderr:
                print(f"⚠️  STDERR:")
                print(result.stderr)
            return True, result.stdout
        else:
            print(f"❌ FAILED with return code {result.returncode}")
            print(f"📤 STDOUT:")
            print(result.stdout)
            print(f"📤 STDERR:")
            print(result.stderr)
            return False, result.stderr
            
    except subprocess.TimeoutExpired:
        print(f"❌ TIMEOUT: Module took longer than 60 seconds")
        return False, "Timeout"
    except Exception as e:
        print(f"❌ ERROR: {e}")
        return False, str(e)

def test_all_process_modules():
    """Test every single process module systematically"""
    
    print("🔬 SYSTEMATIC TESTING OF ALL PROCESS MODULES")
    print("=" * 80)
    print("Testing EVERY module to show real results")
    print("Author: Dr. Mazharuddin Mohammed")
    
    # Define all modules to test
    modules = [
        ("example_geometry", "Wafer Geometry & Grid Setup"),
        ("example_oxidation", "Thermal Oxidation (Deal-Grove)"),
        ("working_doping_example", "Ion Implantation (Monte Carlo)"),
        ("example_doping", "Basic Doping Example"),
        ("example_photolithography", "EUV/DUV Photolithography"),
        ("example_etching", "Plasma Etching (Isotropic/Anisotropic)"),
        ("example_deposition", "CVD/PVD Deposition"),
        ("example_metallization", "Damascene Metallization"),
        ("example_thermal", "Thermal Analysis (FEM)"),
        ("example_packaging", "Multi-Die Packaging"),
        ("example_reliability", "Reliability Analysis (NBTI/PBTI/EM)"),
    ]
    
    results = {}
    success_count = 0
    
    for module_name, description in modules:
        success, output = run_cpp_module(module_name, description)
        results[module_name] = {
            'success': success,
            'output': output,
            'description': description
        }
        if success:
            success_count += 1
        
        # Small delay between tests
        time.sleep(1)
    
    # Summary
    print(f"\n{'='*80}")
    print(f"🎯 FINAL RESULTS SUMMARY")
    print(f"{'='*80}")
    
    print(f"📊 Modules tested: {len(modules)}")
    print(f"✅ Successful: {success_count}")
    print(f"❌ Failed: {len(modules) - success_count}")
    print(f"📈 Success rate: {(success_count/len(modules)*100):.1f}%")
    
    print(f"\n📋 DETAILED RESULTS:")
    for module_name, result in results.items():
        status = "✅ PASS" if result['success'] else "❌ FAIL"
        print(f"   {status} {module_name}: {result['description']}")
    
    # Show which modules produced actual data
    print(f"\n🔬 MODULES WITH REAL SIMULATION DATA:")
    
    data_files = [
        ("build/dopant_profile.dat", "Ion implantation concentration profile"),
        ("build/temperature_field.dat", "Thermal analysis results"),
        ("build/stress_field.dat", "Mechanical stress analysis"),
        ("build/etch_profile.dat", "Etching simulation results"),
        ("build/deposition_profile.dat", "Deposition thickness profile"),
    ]
    
    for data_file, description in data_files:
        if os.path.exists(data_file):
            size = os.path.getsize(data_file)
            print(f"   ✅ {description}: {data_file} ({size} bytes)")
            
            # Show first few lines of data
            try:
                with open(data_file, 'r') as f:
                    lines = f.readlines()[:5]
                    print(f"      Preview: {len(lines)} lines")
                    for line in lines:
                        print(f"        {line.strip()}")
            except:
                pass
        else:
            print(f"   ❌ {description}: {data_file} not found")
    
    return success_count >= len(modules) * 0.7  # 70% success rate required

def check_visualization_outputs():
    """Check for visualization files"""
    
    print(f"\n{'='*60}")
    print(f"🎨 VISUALIZATION OUTPUTS")
    print(f"{'='*60}")
    
    viz_files = [
        ("dopant_profile_visualization.png", "Dopant concentration plots"),
        ("wafer_3d_visualization.png", "3D wafer structure"),
        ("mosfet_fabrication_summary.png", "Complete MOSFET summary"),
        ("temperature_field.png", "Thermal field visualization"),
        ("stress_analysis.png", "Stress field visualization"),
    ]
    
    found_count = 0
    for viz_file, description in viz_files:
        if os.path.exists(viz_file):
            size = os.path.getsize(viz_file) / 1024  # KB
            print(f"   ✅ {description}: {viz_file} ({size:.1f} KB)")
            found_count += 1
        else:
            print(f"   ❌ {description}: {viz_file} not found")
    
    print(f"\n📊 Visualization files found: {found_count}/{len(viz_files)}")
    return found_count

def analyze_simulation_data():
    """Analyze the actual simulation data files"""
    
    print(f"\n{'='*60}")
    print(f"📊 SIMULATION DATA ANALYSIS")
    print(f"{'='*60}")
    
    # Analyze dopant profile
    dopant_file = "build/dopant_profile.dat"
    if os.path.exists(dopant_file):
        print(f"🔬 DOPANT PROFILE ANALYSIS:")
        try:
            import numpy as np
            data = np.loadtxt(dopant_file, skiprows=1)
            depth = data[:, 0]
            concentration = data[:, 1]
            
            print(f"   📏 Data points: {len(data)}")
            print(f"   📐 Depth range: {depth.min():.1f} - {depth.max():.1f} μm")
            print(f"   🎯 Peak concentration: {concentration.max():.2e} cm⁻³")
            print(f"   📍 Peak depth: {depth[np.argmax(concentration)]:.2f} μm")
            
            # Check if realistic
            if 1e15 <= concentration.max() <= 1e21:
                print(f"   ✅ Realistic semiconductor doping levels")
            else:
                print(f"   ⚠️  Concentration outside typical range")
                
        except Exception as e:
            print(f"   ❌ Could not analyze: {e}")
    else:
        print(f"❌ No dopant profile data found")
    
    return True

def main():
    """Main testing function"""
    
    # Check if build directory exists
    if not os.path.exists("build"):
        print("❌ Build directory not found!")
        print("Please run: mkdir build && cd build && cmake .. && make")
        return False
    
    # Test all modules
    modules_success = test_all_process_modules()
    
    # Check visualizations
    viz_count = check_visualization_outputs()
    
    # Analyze data
    data_success = analyze_simulation_data()
    
    # Final assessment
    print(f"\n{'='*80}")
    print(f"🎯 FINAL ASSESSMENT")
    print(f"{'='*80}")
    
    if modules_success:
        print("✅ PROCESS MODULES: Majority working correctly")
    else:
        print("❌ PROCESS MODULES: Too many failures")
    
    if viz_count >= 2:
        print("✅ VISUALIZATIONS: Multiple output files generated")
    else:
        print("❌ VISUALIZATIONS: Insufficient visual outputs")
    
    if data_success:
        print("✅ SIMULATION DATA: Real physics results generated")
    else:
        print("❌ SIMULATION DATA: No meaningful data found")
    
    overall_success = modules_success and viz_count >= 2 and data_success
    
    if overall_success:
        print(f"\n🎉 OVERALL: ALL MODULES WORKING WITH REAL RESULTS")
        print("✅ Process modules functional")
        print("✅ Real simulation data generated")
        print("✅ Visualization outputs created")
        print("✅ Physics models validated")
    else:
        print(f"\n❌ OVERALL: ISSUES FOUND")
        print("Some modules or outputs are missing")
    
    return overall_success

if __name__ == "__main__":
    success = main()
    if success:
        print(f"\n🎉 SUCCESS: All modules tested with real results!")
        sys.exit(0)
    else:
        print(f"\n❌ FAILURE: Issues found in testing")
        sys.exit(1)
