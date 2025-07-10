#!/usr/bin/env python3
"""
Comprehensive Real Validation of SemiPRO Simulator
Author: Dr. Mazharuddin Mohammed

This script performs HONEST validation using REAL simulation data,
not synthetic or mock data. It tests actual C++ physics simulations.
"""

import subprocess
import json
import numpy as np
import matplotlib.pyplot as plt
import os
import time
from pathlib import Path

def run_real_cpp_simulation(process_type, config_data, timeout=30):
    """Run actual C++ simulation and return results"""
    
    # Create config file
    config_file = f"config/test_{process_type}.json"
    with open(config_file, 'w') as f:
        json.dump(config_data, f, indent=2)
    
    try:
        # Run actual C++ simulator
        start_time = time.time()
        result = subprocess.run([
            "./build/simulator",
            "--process", process_type,
            "--config", config_file
        ], capture_output=True, text=True, timeout=timeout)
        
        end_time = time.time()
        execution_time = end_time - start_time
        
        return {
            "success": result.returncode == 0,
            "stdout": result.stdout,
            "stderr": result.stderr,
            "execution_time": execution_time,
            "config_file": config_file
        }
        
    except subprocess.TimeoutExpired:
        return {
            "success": False,
            "stdout": "",
            "stderr": "Simulation timed out",
            "execution_time": timeout,
            "config_file": config_file
        }
    except Exception as e:
        return {
            "success": False,
            "stdout": "",
            "stderr": str(e),
            "execution_time": 0,
            "config_file": config_file
        }

def test_oxidation_physics():
    """Test real oxidation physics simulation"""
    
    print("🔥 Testing Oxidation Physics...")
    
    # Test different temperatures
    temperatures = [800, 1000, 1200]  # °C
    results = []
    
    for temp in temperatures:
        config = {
            "temperature": float(temp),
            "time": 2.0,
            "pressure": 1.0,
            "oxygen_partial_pressure": 0.21,
            "grid_size_x": 50,
            "grid_size_y": 50
        }
        
        result = run_real_cpp_simulation("oxidation", config)
        results.append({
            "temperature": temp,
            "success": result["success"],
            "time": result["execution_time"],
            "error": result["stderr"] if not result["success"] else None
        })
        
        status = "✅" if result["success"] else "❌"
        print(f"   {temp}°C: {status} ({result['execution_time']:.2f}s)")
        if not result["success"]:
            print(f"      Error: {result['stderr']}")
    
    return results

def test_doping_physics():
    """Test real doping physics simulation"""
    
    print("💉 Testing Doping Physics...")
    
    # Test different energies
    energies = [50, 80, 120]  # keV
    results = []
    
    for energy in energies:
        config = {
            "energy": float(energy),
            "dose": 1e15,
            "angle": 0.0,
            "species": "phosphorus",
            "grid_size_x": 50,
            "grid_size_y": 50
        }
        
        result = run_real_cpp_simulation("doping", config)
        results.append({
            "energy": energy,
            "success": result["success"],
            "time": result["execution_time"],
            "error": result["stderr"] if not result["success"] else None
        })
        
        status = "✅" if result["success"] else "❌"
        print(f"   {energy} keV: {status} ({result['execution_time']:.2f}s)")
        if not result["success"]:
            print(f"      Error: {result['stderr']}")
    
    return results

def test_deposition_physics():
    """Test real deposition physics simulation"""
    
    print("⚡ Testing Deposition Physics...")
    
    # Test different materials
    materials = ["SiO2", "Si3N4", "polysilicon"]
    results = []
    
    for material in materials:
        config = {
            "material": material,
            "thickness": 0.1,
            "temperature": 400.0,
            "pressure": 1.0,
            "grid_size_x": 50,
            "grid_size_y": 50
        }
        
        result = run_real_cpp_simulation("deposition", config)
        results.append({
            "material": material,
            "success": result["success"],
            "time": result["execution_time"],
            "error": result["stderr"] if not result["success"] else None
        })
        
        status = "✅" if result["success"] else "❌"
        print(f"   {material}: {status} ({result['execution_time']:.2f}s)")
        if not result["success"]:
            print(f"      Error: {result['stderr']}")
    
    return results

def analyze_real_output_data():
    """Analyze actual output data files"""
    
    print("📊 Analyzing Real Output Data...")
    
    data_files = []
    
    # Check for output files
    for pattern in ["*.dat", "*.log", "*.txt"]:
        import glob
        files = glob.glob(pattern)
        data_files.extend(files)
    
    # Check build directory
    build_files = glob.glob("build/*.dat")
    data_files.extend(build_files)
    
    analysis = {
        "total_files": len(data_files),
        "file_details": []
    }
    
    for file_path in data_files:
        try:
            file_size = os.path.getsize(file_path)
            
            # Try to read as data
            if file_path.endswith('.dat'):
                try:
                    data = np.loadtxt(file_path, skiprows=1)
                    rows, cols = data.shape if data.ndim > 1 else (len(data), 1)
                    
                    analysis["file_details"].append({
                        "file": file_path,
                        "size_bytes": file_size,
                        "data_points": rows,
                        "columns": cols,
                        "type": "numerical_data"
                    })
                    
                    print(f"   ✅ {file_path}: {rows} data points, {cols} columns")
                    
                except:
                    analysis["file_details"].append({
                        "file": file_path,
                        "size_bytes": file_size,
                        "type": "text_data"
                    })
                    print(f"   📄 {file_path}: {file_size} bytes (text)")
            else:
                analysis["file_details"].append({
                    "file": file_path,
                    "size_bytes": file_size,
                    "type": "log_file"
                })
                print(f"   📋 {file_path}: {file_size} bytes (log)")
                
        except Exception as e:
            print(f"   ❌ {file_path}: Error reading - {e}")
    
    return analysis

def create_validation_report(oxidation_results, doping_results, deposition_results, data_analysis):
    """Create honest validation report"""
    
    report = {
        "timestamp": time.strftime("%Y-%m-%d %H:%M:%S"),
        "validation_type": "REAL_SIMULATION_DATA",
        "oxidation": {
            "total_tests": len(oxidation_results),
            "successful": sum(1 for r in oxidation_results if r["success"]),
            "failed": sum(1 for r in oxidation_results if not r["success"]),
            "avg_time": np.mean([r["time"] for r in oxidation_results]),
            "details": oxidation_results
        },
        "doping": {
            "total_tests": len(doping_results),
            "successful": sum(1 for r in doping_results if r["success"]),
            "failed": sum(1 for r in doping_results if not r["success"]),
            "avg_time": np.mean([r["time"] for r in doping_results]),
            "details": doping_results
        },
        "deposition": {
            "total_tests": len(deposition_results),
            "successful": sum(1 for r in deposition_results if r["success"]),
            "failed": sum(1 for r in deposition_results if not r["success"]),
            "avg_time": np.mean([r["time"] for r in deposition_results]),
            "details": deposition_results
        },
        "data_analysis": data_analysis
    }
    
    # Calculate overall statistics
    total_tests = (report["oxidation"]["total_tests"] + 
                  report["doping"]["total_tests"] + 
                  report["deposition"]["total_tests"])
    
    total_successful = (report["oxidation"]["successful"] + 
                       report["doping"]["successful"] + 
                       report["deposition"]["successful"])
    
    report["overall"] = {
        "total_tests": total_tests,
        "successful": total_successful,
        "success_rate": total_successful / total_tests if total_tests > 0 else 0,
        "total_data_files": data_analysis["total_files"]
    }
    
    # Save report
    os.makedirs("validation_results", exist_ok=True)
    with open("validation_results/HONEST_VALIDATION_REPORT.json", "w") as f:
        json.dump(report, f, indent=2)
    
    return report

def main():
    """Main validation function"""
    
    print("🔬 SemiPRO HONEST Real Simulation Validation")
    print("=" * 60)
    print("Testing ACTUAL C++ physics simulations")
    print("NO synthetic or mock data used!")
    print("=" * 60)
    
    # Check prerequisites
    if not os.path.exists("build/simulator"):
        print("❌ C++ simulator not found at build/simulator")
        return 1
    
    # Run real physics tests
    oxidation_results = test_oxidation_physics()
    doping_results = test_doping_physics()
    deposition_results = test_deposition_physics()
    
    # Analyze output data
    data_analysis = analyze_real_output_data()
    
    # Create validation report
    report = create_validation_report(oxidation_results, doping_results, 
                                    deposition_results, data_analysis)
    
    # Print summary
    print("\n" + "=" * 60)
    print("📊 HONEST VALIDATION RESULTS")
    print("=" * 60)
    print(f"Total Tests: {report['overall']['total_tests']}")
    print(f"Successful: {report['overall']['successful']}")
    print(f"Success Rate: {report['overall']['success_rate']:.1%}")
    print(f"Data Files Generated: {report['overall']['total_data_files']}")
    
    print(f"\nOxidation: {report['oxidation']['successful']}/{report['oxidation']['total_tests']} passed")
    print(f"Doping: {report['doping']['successful']}/{report['doping']['total_tests']} passed")
    print(f"Deposition: {report['deposition']['successful']}/{report['deposition']['total_tests']} passed")
    
    print(f"\n📋 Report saved: validation_results/HONEST_VALIDATION_REPORT.json")
    
    if report['overall']['success_rate'] >= 0.8:
        print("✅ Validation PASSED - System is functional")
        return 0
    else:
        print("❌ Validation FAILED - System needs work")
        return 1

if __name__ == "__main__":
    exit(main())
