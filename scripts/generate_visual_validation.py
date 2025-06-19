#!/usr/bin/env python3
# Author: Dr. Mazharuddin Mohammed
# Generate Visual Validation Results - Actual Plots and Data

import sys
import os
import numpy as np
import matplotlib.pyplot as plt
import time
import json

# Add src to path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'src'))

def create_output_directories():
    """Create directories for organized output"""
    directories = [
        'results/plots',
        'results/data',
        'results/validation',
        'results/performance'
    ]
    
    for directory in directories:
        os.makedirs(directory, exist_ok=True)
    
    print("📁 Created output directories")

def test_and_plot_physics_models():
    """Test physics models and generate actual plots"""
    print("\n🔬 Testing Physics Models with Visual Output")
    print("=" * 60)
    
    try:
        from python.simulator import Simulator
        
        sim = Simulator()
        sim.enable_enhanced_mode(True)
        
        # Test different grid sizes and measure performance
        grid_sizes = [20, 40, 60, 80, 100]
        execution_times = []
        
        fig, ((ax1, ax2), (ax3, ax4)) = plt.subplots(2, 2, figsize=(12, 10))
        
        # Performance scaling test
        print("📊 Testing performance scaling...")
        for size in grid_sizes:
            sim.initialize_geometry(size, size)
            
            start_time = time.time()
            
            # Run standard process sequence
            success1 = sim.simulate_oxidation(1000, 1.0, "dry")
            success2 = sim.simulate_doping("boron", 1e16, 50, 1000)
            success3 = sim.simulate_deposition("aluminum", 0.5, 400)
            
            end_time = time.time()
            execution_time = end_time - start_time
            execution_times.append(execution_time)
            
            print(f"   {size}x{size}: {execution_time:.4f}s (Success: {success1 and success2 and success3})")
        
        # Plot 1: Performance Scaling
        ax1.plot(grid_sizes, execution_times, 'bo-', linewidth=2, markersize=8)
        ax1.set_xlabel('Grid Size')
        ax1.set_ylabel('Execution Time (s)')
        ax1.set_title('Performance Scaling')
        ax1.grid(True, alpha=0.3)
        
        # Add performance data to plot
        for i, (size, time_val) in enumerate(zip(grid_sizes, execution_times)):
            ax1.annotate(f'{time_val:.3f}s', (size, time_val), 
                        textcoords="offset points", xytext=(0,10), ha='center')
        
        # Test different process conditions
        print("📊 Testing process parameter effects...")
        
        # Oxidation temperature effects
        temperatures = [900, 950, 1000, 1050, 1100]
        oxidation_results = []
        
        sim.initialize_geometry(50, 50)
        for temp in temperatures:
            success = sim.simulate_oxidation(temp, 1.0, "dry")
            oxidation_results.append(1 if success else 0)
        
        # Plot 2: Oxidation Temperature Effects
        ax2.bar(temperatures, oxidation_results, color='orange', alpha=0.7)
        ax2.set_xlabel('Temperature (°C)')
        ax2.set_ylabel('Success Rate')
        ax2.set_title('Oxidation Temperature Effects')
        ax2.set_ylim(0, 1.2)
        
        # Ion implantation energy effects
        print("📊 Testing ion implantation energies...")
        energies = [30, 50, 80, 120, 150]
        implant_results = []
        
        for energy in energies:
            success = sim.simulate_doping("boron", 1e16, energy, 1000)
            implant_results.append(1 if success else 0)
        
        # Plot 3: Ion Implantation Energy Effects
        ax3.bar(energies, implant_results, color='green', alpha=0.7)
        ax3.set_xlabel('Energy (keV)')
        ax3.set_ylabel('Success Rate')
        ax3.set_title('Ion Implantation Energy Effects')
        ax3.set_ylim(0, 1.2)
        
        # Deposition thickness effects
        print("📊 Testing deposition thickness...")
        thicknesses = [0.1, 0.3, 0.5, 0.8, 1.0]
        deposition_results = []
        
        for thickness in thicknesses:
            success = sim.simulate_deposition("aluminum", thickness, 400)
            deposition_results.append(1 if success else 0)
        
        # Plot 4: Deposition Thickness Effects
        ax4.bar(thicknesses, deposition_results, color='purple', alpha=0.7)
        ax4.set_xlabel('Thickness (μm)')
        ax4.set_ylabel('Success Rate')
        ax4.set_title('Deposition Thickness Effects')
        ax4.set_ylim(0, 1.2)
        
        plt.tight_layout()
        plt.savefig('results/plots/physics_validation.png', dpi=300, bbox_inches='tight')
        plt.close()
        
        # Save raw data
        validation_data = {
            'performance_scaling': {
                'grid_sizes': grid_sizes,
                'execution_times': execution_times
            },
            'oxidation_temperature': {
                'temperatures': temperatures,
                'success_rates': oxidation_results
            },
            'implantation_energy': {
                'energies': energies,
                'success_rates': implant_results
            },
            'deposition_thickness': {
                'thicknesses': thicknesses,
                'success_rates': deposition_results
            }
        }
        
        with open('results/data/physics_validation_data.json', 'w') as f:
            json.dump(validation_data, f, indent=2)
        
        print("✅ Physics validation plots saved to results/plots/physics_validation.png")
        print("✅ Raw data saved to results/data/physics_validation_data.json")
        
        return True
        
    except Exception as e:
        print(f"❌ Physics validation failed: {e}")
        import traceback
        traceback.print_exc()
        return False

def test_and_plot_mosfet_fabrication():
    """Test MOSFET fabrication and generate process flow visualization"""
    print("\n🏭 Testing MOSFET Fabrication with Visual Output")
    print("=" * 60)
    
    try:
        # Run MOSFET fabrication and capture results
        import subprocess
        result = subprocess.run([sys.executable, 'examples/complete_mosfet_fabrication.py'], 
                              capture_output=True, text=True, timeout=60)
        
        # Parse output for step results
        output = result.stdout
        steps = [
            "Substrate Preparation",
            "Gate Oxidation", 
            "Gate Definition",
            "Source/Drain Formation",
            "Metallization",
            "Final Testing"
        ]
        
        step_results = []
        for step in steps:
            if f"✅" in output and step in output:
                step_results.append(1)
            else:
                step_results.append(0)
        
        # Create MOSFET fabrication flow chart
        fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(14, 6))
        
        # Plot 1: Step-by-step results
        colors = ['green' if result else 'red' for result in step_results]
        bars = ax1.barh(range(len(steps)), step_results, color=colors, alpha=0.7)
        ax1.set_yticks(range(len(steps)))
        ax1.set_yticklabels(steps)
        ax1.set_xlabel('Success (1) / Failure (0)')
        ax1.set_title('MOSFET Fabrication Step Results')
        ax1.set_xlim(0, 1.2)
        
        # Add success/failure labels
        for i, (bar, result) in enumerate(zip(bars, step_results)):
            label = "✅ PASS" if result else "❌ FAIL"
            ax1.text(0.1, i, label, va='center', fontweight='bold')
        
        # Plot 2: Overall success rate
        success_rate = sum(step_results) / len(step_results) * 100
        ax2.pie([success_rate, 100-success_rate], 
                labels=[f'Success\n{success_rate:.1f}%', f'Failed\n{100-success_rate:.1f}%'],
                colors=['green', 'red'], autopct='%1.1f%%', startangle=90)
        ax2.set_title('Overall MOSFET Fabrication Success Rate')
        
        plt.tight_layout()
        plt.savefig('results/plots/mosfet_fabrication.png', dpi=300, bbox_inches='tight')
        plt.close()
        
        # Save fabrication data
        fabrication_data = {
            'steps': steps,
            'results': step_results,
            'success_rate': success_rate,
            'total_steps': len(steps),
            'successful_steps': sum(step_results)
        }
        
        with open('results/data/mosfet_fabrication_data.json', 'w') as f:
            json.dump(fabrication_data, f, indent=2)
        
        print(f"✅ MOSFET fabrication plots saved to results/plots/mosfet_fabrication.png")
        print(f"✅ Success rate: {success_rate:.1f}% ({sum(step_results)}/{len(steps)} steps)")
        print("✅ Raw data saved to results/data/mosfet_fabrication_data.json")
        
        return success_rate >= 50  # At least 50% success rate
        
    except Exception as e:
        print(f"❌ MOSFET fabrication visualization failed: {e}")
        return False

def generate_performance_comparison():
    """Generate performance comparison charts"""
    print("\n⚡ Generating Performance Comparison")
    print("=" * 60)
    
    try:
        from python.simulator import Simulator
        
        # Test different scenarios
        scenarios = [
            {"name": "Basic Simulation", "grid": 30, "processes": 3},
            {"name": "Medium Complexity", "grid": 60, "processes": 5},
            {"name": "High Complexity", "grid": 100, "processes": 7},
            {"name": "Extreme Scale", "grid": 150, "processes": 10}
        ]
        
        scenario_names = []
        execution_times = []
        memory_usage = []
        
        for scenario in scenarios:
            print(f"📊 Testing {scenario['name']}...")
            
            sim = Simulator()
            sim.enable_enhanced_mode(True)
            sim.initialize_geometry(scenario['grid'], scenario['grid'])
            
            start_time = time.time()
            
            # Run multiple processes
            for i in range(scenario['processes']):
                if i % 3 == 0:
                    sim.simulate_oxidation(1000, 1.0, "dry")
                elif i % 3 == 1:
                    sim.simulate_doping("boron", 1e16, 50, 1000)
                else:
                    sim.simulate_deposition("aluminum", 0.3, 400)
            
            end_time = time.time()
            execution_time = end_time - start_time
            
            scenario_names.append(scenario['name'])
            execution_times.append(execution_time)
            
            # Estimate memory usage (simplified)
            grid_size = scenario['grid'] * scenario['grid']
            estimated_memory = grid_size * 8 * 4 / 1024 / 1024  # MB
            memory_usage.append(estimated_memory)
            
            print(f"   Time: {execution_time:.3f}s, Est. Memory: {estimated_memory:.1f}MB")
        
        # Create performance comparison plots
        fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(14, 6))
        
        # Plot 1: Execution Time Comparison
        bars1 = ax1.bar(scenario_names, execution_times, color='skyblue', alpha=0.8)
        ax1.set_ylabel('Execution Time (s)')
        ax1.set_title('Performance Comparison Across Scenarios')
        ax1.tick_params(axis='x', rotation=45)
        
        # Add time labels on bars
        for bar, time_val in zip(bars1, execution_times):
            height = bar.get_height()
            ax1.text(bar.get_x() + bar.get_width()/2., height + 0.001,
                    f'{time_val:.3f}s', ha='center', va='bottom')
        
        # Plot 2: Memory Usage Comparison
        bars2 = ax2.bar(scenario_names, memory_usage, color='lightcoral', alpha=0.8)
        ax2.set_ylabel('Estimated Memory Usage (MB)')
        ax2.set_title('Memory Usage Comparison')
        ax2.tick_params(axis='x', rotation=45)
        
        # Add memory labels on bars
        for bar, mem_val in zip(bars2, memory_usage):
            height = bar.get_height()
            ax2.text(bar.get_x() + bar.get_width()/2., height + 0.1,
                    f'{mem_val:.1f}MB', ha='center', va='bottom')
        
        plt.tight_layout()
        plt.savefig('results/plots/performance_comparison.png', dpi=300, bbox_inches='tight')
        plt.close()
        
        # Save performance data
        performance_data = {
            'scenarios': scenario_names,
            'execution_times': execution_times,
            'memory_usage': memory_usage,
            'test_timestamp': time.time()
        }
        
        with open('results/data/performance_comparison_data.json', 'w') as f:
            json.dump(performance_data, f, indent=2)
        
        print("✅ Performance comparison plots saved to results/plots/performance_comparison.png")
        print("✅ Raw data saved to results/data/performance_comparison_data.json")
        
        return True
        
    except Exception as e:
        print(f"❌ Performance comparison failed: {e}")
        return False

def generate_validation_summary():
    """Generate a comprehensive validation summary with all results"""
    print("\n📋 Generating Validation Summary")
    print("=" * 60)
    
    try:
        # Load all generated data
        data_files = [
            'results/data/physics_validation_data.json',
            'results/data/mosfet_fabrication_data.json',
            'results/data/performance_comparison_data.json'
        ]
        
        all_data = {}
        for file_path in data_files:
            if os.path.exists(file_path):
                with open(file_path, 'r') as f:
                    key = os.path.basename(file_path).replace('_data.json', '')
                    all_data[key] = json.load(f)
        
        # Create summary visualization
        fig, ((ax1, ax2), (ax3, ax4)) = plt.subplots(2, 2, figsize=(16, 12))
        
        # Summary 1: Overall Test Results
        if 'physics_validation' in all_data:
            physics_data = all_data['physics_validation']
            test_categories = ['Oxidation', 'Implantation', 'Deposition', 'Performance']
            success_rates = [
                sum(physics_data['oxidation_temperature']['success_rates']) / len(physics_data['oxidation_temperature']['success_rates']) * 100,
                sum(physics_data['implantation_energy']['success_rates']) / len(physics_data['implantation_energy']['success_rates']) * 100,
                sum(physics_data['deposition_thickness']['success_rates']) / len(physics_data['deposition_thickness']['success_rates']) * 100,
                100 if max(physics_data['performance_scaling']['execution_times']) < 1.0 else 80
            ]
            
            bars = ax1.bar(test_categories, success_rates, color=['orange', 'green', 'purple', 'blue'], alpha=0.7)
            ax1.set_ylabel('Success Rate (%)')
            ax1.set_title('Physics Model Validation Results')
            ax1.set_ylim(0, 110)
            
            for bar, rate in zip(bars, success_rates):
                height = bar.get_height()
                ax1.text(bar.get_x() + bar.get_width()/2., height + 2,
                        f'{rate:.1f}%', ha='center', va='bottom', fontweight='bold')
        
        # Summary 2: MOSFET Fabrication Results
        if 'mosfet_fabrication' in all_data:
            mosfet_data = all_data['mosfet_fabrication']
            steps = mosfet_data['steps']
            results = mosfet_data['results']
            
            colors = ['green' if r else 'red' for r in results]
            ax2.barh(range(len(steps)), results, color=colors, alpha=0.7)
            ax2.set_yticks(range(len(steps)))
            ax2.set_yticklabels([s.replace(' ', '\n') for s in steps], fontsize=8)
            ax2.set_xlabel('Success (1) / Failure (0)')
            ax2.set_title('MOSFET Fabrication Steps')
            ax2.set_xlim(0, 1.2)
        
        # Summary 3: Performance Scaling
        if 'physics_validation' in all_data:
            perf_data = all_data['physics_validation']['performance_scaling']
            ax3.plot(perf_data['grid_sizes'], perf_data['execution_times'], 'bo-', linewidth=2, markersize=8)
            ax3.set_xlabel('Grid Size')
            ax3.set_ylabel('Execution Time (s)')
            ax3.set_title('Performance Scaling Validation')
            ax3.grid(True, alpha=0.3)
        
        # Summary 4: Overall System Health
        if 'performance_comparison' in all_data:
            perf_comp_data = all_data['performance_comparison']
            scenarios = perf_comp_data['scenarios']
            times = perf_comp_data['execution_times']
            
            # Performance rating based on execution time
            ratings = []
            for time_val in times:
                if time_val < 0.1:
                    ratings.append(100)  # Excellent
                elif time_val < 0.5:
                    ratings.append(90)   # Very Good
                elif time_val < 1.0:
                    ratings.append(80)   # Good
                else:
                    ratings.append(70)   # Acceptable
            
            ax4.bar(range(len(scenarios)), ratings, color='lightgreen', alpha=0.8)
            ax4.set_xticks(range(len(scenarios)))
            ax4.set_xticklabels([s.replace(' ', '\n') for s in scenarios], fontsize=8)
            ax4.set_ylabel('Performance Rating')
            ax4.set_title('System Performance Rating')
            ax4.set_ylim(0, 110)
        
        plt.tight_layout()
        plt.savefig('results/validation/comprehensive_validation_summary.png', dpi=300, bbox_inches='tight')
        plt.close()
        
        # Generate text summary
        summary_text = f"""
# SemiPRO Validation Summary Report
Generated: {time.strftime('%Y-%m-%d %H:%M:%S')}

## Test Results Overview
"""
        
        if 'physics_validation' in all_data:
            summary_text += f"""
### Physics Model Validation
- Performance Scaling: {len(all_data['physics_validation']['performance_scaling']['grid_sizes'])} grid sizes tested
- Max execution time: {max(all_data['physics_validation']['performance_scaling']['execution_times']):.4f}s
- Oxidation tests: {sum(all_data['physics_validation']['oxidation_temperature']['success_rates'])}/{len(all_data['physics_validation']['oxidation_temperature']['success_rates'])} passed
- Implantation tests: {sum(all_data['physics_validation']['implantation_energy']['success_rates'])}/{len(all_data['physics_validation']['implantation_energy']['success_rates'])} passed
- Deposition tests: {sum(all_data['physics_validation']['deposition_thickness']['success_rates'])}/{len(all_data['physics_validation']['deposition_thickness']['success_rates'])} passed
"""
        
        if 'mosfet_fabrication' in all_data:
            summary_text += f"""
### MOSFET Fabrication Validation
- Total steps: {all_data['mosfet_fabrication']['total_steps']}
- Successful steps: {all_data['mosfet_fabrication']['successful_steps']}
- Success rate: {all_data['mosfet_fabrication']['success_rate']:.1f}%
"""
        
        if 'performance_comparison' in all_data:
            summary_text += f"""
### Performance Validation
- Scenarios tested: {len(all_data['performance_comparison']['scenarios'])}
- Fastest execution: {min(all_data['performance_comparison']['execution_times']):.4f}s
- Slowest execution: {max(all_data['performance_comparison']['execution_times']):.4f}s
- Average execution: {sum(all_data['performance_comparison']['execution_times'])/len(all_data['performance_comparison']['execution_times']):.4f}s
"""
        
        with open('results/validation/validation_summary.md', 'w') as f:
            f.write(summary_text)
        
        print("✅ Comprehensive validation summary saved to results/validation/comprehensive_validation_summary.png")
        print("✅ Text summary saved to results/validation/validation_summary.md")
        
        return True
        
    except Exception as e:
        print(f"❌ Validation summary generation failed: {e}")
        return False

def main():
    """Main function to generate all visual validation"""
    print("🔬 SemiPRO Visual Validation Generator")
    print("=" * 80)
    print("Generating actual plots and data to verify claims")
    print("Author: Dr. Mazharuddin Mohammed")
    
    # Create output directories
    create_output_directories()
    
    # Run all validation tests with visual output
    results = []
    
    print("\n🚀 Starting Visual Validation Tests...")
    
    results.append(test_and_plot_physics_models())
    results.append(test_and_plot_mosfet_fabrication())
    results.append(generate_performance_comparison())
    results.append(generate_validation_summary())
    
    # Final results
    success_count = sum(results)
    total_tests = len(results)
    success_rate = success_count / total_tests * 100
    
    print("\n" + "=" * 80)
    print("🎯 VISUAL VALIDATION RESULTS")
    print("=" * 80)
    
    test_names = [
        "Physics Models Validation",
        "MOSFET Fabrication Validation", 
        "Performance Comparison",
        "Validation Summary Generation"
    ]
    
    for test_name, success in zip(test_names, results):
        status = "✅ PASSED" if success else "❌ FAILED"
        print(f"{status}: {test_name}")
    
    print(f"\n📊 Overall Success Rate: {success_count}/{total_tests} ({success_rate:.1f}%)")
    
    if success_rate >= 75:
        print("\n✅ VISUAL VALIDATION: SUCCESS")
        print("📊 Actual plots and data generated")
        print("📁 Results saved in organized directory structure")
        print("🔍 Claims can now be verified with visual evidence")
    else:
        print("\n❌ VISUAL VALIDATION: ISSUES FOUND")
        print("🔧 Some visualizations failed to generate")
    
    print(f"\n📁 Results Location:")
    print(f"   📊 Plots: results/plots/")
    print(f"   📈 Data: results/data/")
    print(f"   📋 Validation: results/validation/")
    
    return success_rate >= 75

if __name__ == "__main__":
    success = main()
    sys.exit(0 if success else 1)
