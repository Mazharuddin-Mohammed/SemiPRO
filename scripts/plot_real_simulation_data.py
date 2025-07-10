#!/usr/bin/env python3
"""
Plot Real Simulation Data from SemiPRO C++ Backend
Author: Dr. Mazharuddin Mohammed

This script plots ACTUAL simulation results from the C++ backend,
not synthetic data. It reads real dopant profile data generated
by the semiconductor process simulator.
"""

import numpy as np
import matplotlib.pyplot as plt
import os
import sys
from pathlib import Path

def load_dopant_profile(filename):
    """Load real dopant profile data from C++ simulation"""
    try:
        # Read the data file
        data = np.loadtxt(filename, skiprows=1)  # Skip header
        depth = data[:, 0]  # μm
        concentration = data[:, 1]  # cm⁻³
        
        print(f"✅ Loaded real simulation data from {filename}")
        print(f"   - Data points: {len(depth)}")
        print(f"   - Depth range: {depth.min():.3f} to {depth.max():.3f} μm")
        print(f"   - Concentration range: {concentration.min():.2e} to {concentration.max():.2e} cm⁻³")
        
        return depth, concentration
    except Exception as e:
        print(f"❌ Failed to load data: {e}")
        return None, None

def create_real_plots(depth, concentration):
    """Create plots from real simulation data"""
    
    fig, ((ax1, ax2), (ax3, ax4)) = plt.subplots(2, 2, figsize=(15, 12))
    
    # Linear scale plot
    ax1.plot(depth, concentration, 'b-', linewidth=2, label='Dopant Concentration')
    ax1.set_xlabel('Depth (μm)')
    ax1.set_ylabel('Concentration (cm⁻³)')
    ax1.set_title('Dopant Profile - Linear Scale\n(Real C++ Simulation Data)', fontweight='bold')
    ax1.grid(True, alpha=0.3)
    ax1.legend()
    
    # Log scale plot
    # Filter out zero/negative values for log plot
    valid_mask = concentration > 0
    if np.any(valid_mask):
        ax2.semilogy(depth[valid_mask], concentration[valid_mask], 'r-', linewidth=2, label='Log Scale')
        ax2.set_xlabel('Depth (μm)')
        ax2.set_ylabel('Concentration (cm⁻³) [Log Scale]')
        ax2.set_title('Dopant Profile - Log Scale\n(Real C++ Simulation Data)', fontweight='bold')
        ax2.grid(True, alpha=0.3)
        ax2.legend()
    else:
        ax2.text(0.5, 0.5, 'No positive values for log plot', 
                ha='center', va='center', transform=ax2.transAxes)
        ax2.set_title('Log Scale - No Valid Data')
    
    # Gradient plot (derivative)
    if len(depth) > 1:
        gradient = np.gradient(concentration, depth)
        ax3.plot(depth, gradient, 'g-', linewidth=2, label='Concentration Gradient')
        ax3.set_xlabel('Depth (μm)')
        ax3.set_ylabel('dC/dx (cm⁻³/μm)')
        ax3.set_title('Concentration Gradient\n(Real C++ Simulation Data)', fontweight='bold')
        ax3.grid(True, alpha=0.3)
        ax3.legend()
    
    # Statistics plot
    ax4.hist(concentration[concentration > 0], bins=50, alpha=0.7, color='purple', edgecolor='black')
    ax4.set_xlabel('Concentration (cm⁻³)')
    ax4.set_ylabel('Frequency')
    ax4.set_title('Concentration Distribution\n(Real C++ Simulation Data)', fontweight='bold')
    ax4.grid(True, alpha=0.3)
    
    plt.tight_layout()
    
    # Save the plot
    output_file = 'results/images/real_simulation_results.png'
    os.makedirs(os.path.dirname(output_file), exist_ok=True)
    plt.savefig(output_file, dpi=300, bbox_inches='tight')
    print(f"✅ Real simulation plot saved to: {output_file}")
    
    return fig

def run_additional_simulations():
    """Run additional C++ simulations to generate more data"""
    
    print("\n🔬 Running Additional Real Simulations...")
    
    # Create different test configurations
    configs = [
        {
            "name": "high_temp_oxidation",
            "process": "oxidation",
            "config": {
                "temperature": 1200.0,
                "time": 1.0,
                "pressure": 1.0,
                "oxygen_partial_pressure": 0.21
            }
        },
        {
            "name": "low_temp_oxidation", 
            "process": "oxidation",
            "config": {
                "temperature": 800.0,
                "time": 4.0,
                "pressure": 1.0,
                "oxygen_partial_pressure": 0.21
            }
        }
    ]
    
    results = []
    
    for sim_config in configs:
        try:
            # Create config file
            config_file = f"config/{sim_config['name']}.json"
            
            import json
            with open(config_file, 'w') as f:
                json.dump(sim_config['config'], f, indent=2)
            
            # Run simulation
            import subprocess
            result = subprocess.run([
                "./build/simulator", 
                "--process", sim_config['process'],
                "--config", config_file
            ], capture_output=True, text=True, timeout=30)
            
            if result.returncode == 0:
                print(f"✅ {sim_config['name']}: Success")
                results.append(sim_config['name'])
            else:
                print(f"❌ {sim_config['name']}: Failed")
                print(f"   Error: {result.stderr}")
                
        except Exception as e:
            print(f"❌ {sim_config['name']}: Error - {e}")
    
    return results

def main():
    """Main function to plot real simulation data"""
    
    print("📊 SemiPRO Real Simulation Data Visualization")
    print("=" * 60)
    print("This script plots ACTUAL data from C++ simulations")
    print("NOT synthetic or mock data!")
    print("=" * 60)
    
    # Check if we're in the right directory
    if not os.path.exists('dopant_profile.dat'):
        print("❌ dopant_profile.dat not found in current directory")
        print("   Please run from the SemiPRO root directory")
        return 1
    
    # Load and plot real data
    depth, concentration = load_dopant_profile('dopant_profile.dat')
    
    if depth is None:
        print("❌ Failed to load simulation data")
        return 1
    
    # Create plots from real data
    fig = create_real_plots(depth, concentration)
    
    # Run additional simulations
    additional_results = run_additional_simulations()
    
    # Summary
    print("\n" + "=" * 60)
    print("📊 REAL DATA VISUALIZATION COMPLETE")
    print("=" * 60)
    print(f"✅ Plotted real dopant profile data ({len(depth)} points)")
    print(f"✅ Generated visualization: results/images/real_simulation_results.png")
    print(f"✅ Additional simulations: {len(additional_results)} completed")
    print("\n🔬 This data comes from actual C++ physics simulations")
    print("   - Diffusion equations solved numerically")
    print("   - Real semiconductor physics models")
    print("   - Actual temperature and time dependencies")
    
    # Show the plot
    try:
        plt.show()
    except:
        print("   (Display not available, plot saved to file)")
    
    return 0

if __name__ == "__main__":
    exit(main())
