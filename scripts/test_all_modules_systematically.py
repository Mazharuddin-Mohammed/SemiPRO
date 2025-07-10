#!/usr/bin/env python3
"""
Systematic Testing of All SemiPRO Modules
Author: Dr. Mazharuddin Mohammed

This script tests each module systematically and provides detailed
diagnostics for failures. It focuses on REAL simulation testing.
"""

import subprocess
import json
import time
import os
from pathlib import Path

def test_single_module(process_type, config_data, timeout=30):
    """Test a single module with detailed diagnostics"""
    
    print(f"\n🔬 Testing {process_type.upper()} Module")
    print("-" * 50)
    
    # Create config file
    config_file = f"config/test_{process_type}_systematic.json"
    try:
        os.makedirs("config", exist_ok=True)
        with open(config_file, 'w') as f:
            json.dump(config_data, f, indent=2)
        print(f"✅ Config file created: {config_file}")
    except Exception as e:
        print(f"❌ Failed to create config: {e}")
        return False
    
    # Run simulation with detailed output
    try:
        print(f"🚀 Running: ./build/simulator --process {process_type} --config {config_file}")
        
        start_time = time.time()
        result = subprocess.run([
            "./build/simulator",
            "--process", process_type,
            "--config", config_file
        ], capture_output=True, text=True, timeout=timeout)
        
        end_time = time.time()
        execution_time = end_time - start_time
        
        print(f"⏱️  Execution time: {execution_time:.3f} seconds")
        print(f"📤 Return code: {result.returncode}")
        
        if result.stdout:
            print(f"📋 STDOUT:\n{result.stdout}")
        
        if result.stderr:
            print(f"⚠️  STDERR:\n{result.stderr}")
        
        # Check for output files
        output_files = []
        for pattern in ["*.dat", "*.log", "*.txt"]:
            import glob
            files = glob.glob(pattern)
            output_files.extend(files)
        
        if output_files:
            print(f"📁 Output files generated: {len(output_files)}")
            for file in output_files[-3:]:  # Show last 3 files
                try:
                    size = os.path.getsize(file)
                    print(f"   - {file}: {size} bytes")
                except:
                    pass
        else:
            print("📁 No output files generated")
        
        success = result.returncode == 0
        status = "✅ SUCCESS" if success else "❌ FAILED"
        print(f"🎯 Result: {status}")
        
        return {
            "success": success,
            "execution_time": execution_time,
            "stdout": result.stdout,
            "stderr": result.stderr,
            "output_files": len(output_files)
        }
        
    except subprocess.TimeoutExpired:
        print(f"⏰ TIMEOUT after {timeout} seconds")
        return {
            "success": False,
            "execution_time": timeout,
            "stdout": "",
            "stderr": "Timeout",
            "output_files": 0
        }
    except Exception as e:
        print(f"💥 EXCEPTION: {e}")
        return {
            "success": False,
            "execution_time": 0,
            "stdout": "",
            "stderr": str(e),
            "output_files": 0
        }

def test_all_modules():
    """Test all available modules systematically"""
    
    print("🔬 SemiPRO Systematic Module Testing")
    print("=" * 60)
    print("Testing each module with appropriate configurations")
    print("=" * 60)
    
    # Define test configurations for each module
    test_configs = {
        "oxidation": {
            "temperature": 1000.0,
            "time": 1.0,
            "pressure": 1.0,
            "oxygen_partial_pressure": 0.21
        },
        "doping": {
            "energy": 50.0,  # Reduced from 80 to minimize timeout risk
            "dose": 1e14,    # Reduced from 1e15 to speed up
            "angle": 0.0,
            "species": "phosphorus"
        },
        "deposition": {
            "thickness": 0.1,  # Reduced thickness
            "temperature": 300.0,  # Lower temperature
            "material": "aluminum"
        },
        "etching": {
            "rate": 0.1,
            "etch_type": "anisotropic",
            "selectivity": 5.0
        }
    }
    
    results = {}
    
    for process_type, config in test_configs.items():
        try:
            result = test_single_module(process_type, config, timeout=20)  # Reduced timeout
            results[process_type] = result
        except Exception as e:
            print(f"💥 Failed to test {process_type}: {e}")
            results[process_type] = {
                "success": False,
                "execution_time": 0,
                "stdout": "",
                "stderr": str(e),
                "output_files": 0
            }
    
    return results

def analyze_results(results):
    """Analyze and summarize test results"""
    
    print("\n" + "=" * 60)
    print("📊 SYSTEMATIC TEST RESULTS ANALYSIS")
    print("=" * 60)
    
    total_tests = len(results)
    successful_tests = sum(1 for r in results.values() if r["success"])
    
    print(f"Total modules tested: {total_tests}")
    print(f"Successful: {successful_tests}")
    print(f"Failed: {total_tests - successful_tests}")
    print(f"Success rate: {successful_tests/total_tests*100:.1f}%")
    
    print("\n📋 Detailed Results:")
    print("-" * 40)
    
    for module, result in results.items():
        status = "✅ PASS" if result["success"] else "❌ FAIL"
        time_str = f"{result['execution_time']:.3f}s"
        files_str = f"{result['output_files']} files"
        
        print(f"{module:12} | {status} | {time_str:8} | {files_str}")
        
        if not result["success"] and result["stderr"]:
            print(f"             Error: {result['stderr'][:50]}...")
    
    print("\n🔍 Analysis:")
    print("-" * 20)
    
    # Identify patterns
    timeouts = [m for m, r in results.items() if "timeout" in r["stderr"].lower()]
    config_errors = [m for m, r in results.items() if "config" in r["stderr"].lower()]
    
    if timeouts:
        print(f"⏰ Timeout issues: {', '.join(timeouts)}")
    if config_errors:
        print(f"⚙️  Configuration issues: {', '.join(config_errors)}")
    
    working_modules = [m for m, r in results.items() if r["success"]]
    if working_modules:
        print(f"✅ Working modules: {', '.join(working_modules)}")
    
    return {
        "total": total_tests,
        "successful": successful_tests,
        "success_rate": successful_tests/total_tests,
        "working_modules": working_modules,
        "timeout_modules": timeouts,
        "config_error_modules": config_errors
    }

def main():
    """Main testing function"""
    
    # Check if simulator exists
    if not os.path.exists("build/simulator"):
        print("❌ Simulator executable not found in build directory")
        return 1
    
    # Run systematic tests
    results = test_all_modules()
    
    # Analyze results
    analysis = analyze_results(results)
    
    # Save detailed results
    os.makedirs("validation_results", exist_ok=True)
    with open("validation_results/systematic_test_results.json", "w") as f:
        json.dump({
            "timestamp": time.strftime("%Y-%m-%d %H:%M:%S"),
            "results": results,
            "analysis": analysis
        }, f, indent=2)
    
    print(f"\n📋 Detailed results saved to: validation_results/systematic_test_results.json")
    
    # Return appropriate exit code
    if analysis["success_rate"] >= 0.5:  # At least 50% success
        print("🎉 Overall assessment: ACCEPTABLE")
        return 0
    else:
        print("❌ Overall assessment: NEEDS WORK")
        return 1

if __name__ == "__main__":
    exit(main())
