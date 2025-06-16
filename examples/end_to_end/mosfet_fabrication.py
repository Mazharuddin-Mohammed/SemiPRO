#!/usr/bin/env python3
# Author: Dr. Mazharuddin Mohammed
"""
Complete MOSFET Fabrication Example
===================================

This example demonstrates a complete end-to-end MOSFET fabrication process
using the SemiPRO simulation platform. It includes all major process steps
from wafer preparation to final device characterization.

This tutorial covers:
1. Wafer initialization and substrate preparation
2. Field oxidation and isolation
3. Gate oxidation and polysilicon deposition
4. Source/drain implantation and activation
5. Metallization and contact formation
6. Device characterization and analysis
"""

import sys
import os
import asyncio
import time
from pathlib import Path

# Add SemiPRO to path
sys.path.append(str(Path(__file__).parent.parent.parent / "src" / "python"))

from api.client import SemiPROClient, SemiPROClientError
from api.serializers import serialize_to_json
import matplotlib.pyplot as plt
import numpy as np

class MOSFETFabricationExample:
    """Complete MOSFET fabrication example"""
    
    def __init__(self, api_url: str = "http://localhost:5000"):
        """Initialize the fabrication example"""
        self.client = SemiPROClient(base_url=api_url)
        self.simulator_id = None
        self.results = {}
        
        # Process parameters
        self.gate_length = 0.25  # μm
        self.gate_oxide_thickness = 0.005  # μm (5 nm)
        self.source_drain_depth = 0.2  # μm
        
        # Setup output directory
        self.output_dir = Path("output/mosfet_fabrication")
        self.output_dir.mkdir(parents=True, exist_ok=True)
    
    def print_step(self, step_name: str, description: str = ""):
        """Print formatted step information"""
        print(f"\n{'='*60}")
        print(f"STEP: {step_name}")
        if description:
            print(f"Description: {description}")
        print(f"{'='*60}")
    
    def setup_simulation(self):
        """Setup simulation environment"""
        self.print_step("SIMULATION SETUP", "Initializing simulator and wafer")
        
        try:
            # Create simulator
            config = {
                'input_directory': str(self.output_dir / "input"),
                'output_directory': str(self.output_dir / "output")
            }
            
            # Create directories
            Path(config['input_directory']).mkdir(parents=True, exist_ok=True)
            Path(config['output_directory']).mkdir(parents=True, exist_ok=True)
            
            self.simulator_id = self.client.create_simulator(config)
            print(f"✓ Created simulator: {self.simulator_id}")
            
            # Create wafer
            wafer_config = {
                'diameter': 300.0,  # 300mm wafer
                'thickness': 775.0,  # 775 μm thick
                'material': 'silicon',
                'orientation': '(100)',
                'resistivity': 10.0,  # 10 Ω·cm
                'doping_type': 'p-type',
                'width': 100,  # Grid points
                'height': 100
            }
            
            result = self.client.create_wafer(self.simulator_id, wafer_config)
            print(f"✓ Created wafer: {wafer_config['diameter']}mm {wafer_config['material']}")
            print(f"  - Orientation: {wafer_config['orientation']}")
            print(f"  - Resistivity: {wafer_config['resistivity']} Ω·cm")
            print(f"  - Grid size: {wafer_config['width']}×{wafer_config['height']}")
            
            return True
            
        except Exception as e:
            print(f"✗ Setup failed: {e}")
            return False
    
    def step1_field_oxidation(self):
        """Step 1: Field oxidation for device isolation"""
        self.print_step("FIELD OXIDATION", "Growing thick oxide for device isolation")
        
        try:
            result = self.client.add_oxidation_step(
                self.simulator_id,
                name="field_oxidation",
                temperature=1100.0,  # °C
                time=4.0,           # hours
                atmosphere="wet"     # wet oxidation for thick oxide
            )
            
            print(f"✓ Added field oxidation step")
            print(f"  - Temperature: 1100°C")
            print(f"  - Time: 4.0 hours")
            print(f"  - Atmosphere: wet oxidation")
            print(f"  - Expected thickness: ~0.8 μm")
            
            return True
            
        except Exception as e:
            print(f"✗ Field oxidation failed: {e}")
            return False
    
    def step2_active_area_definition(self):
        """Step 2: Define active areas using lithography and etching"""
        self.print_step("ACTIVE AREA DEFINITION", "Patterning active device regions")
        
        try:
            # Photoresist application and patterning
            result = self.client.add_lithography_step(
                self.simulator_id,
                name="active_area_lithography",
                wavelength=365.0,  # i-line lithography
                dose=150.0         # mJ/cm²
            )
            print(f"✓ Added lithography step for active area definition")
            
            # Oxide etching to define active areas
            result = self.client.add_etching_step(
                self.simulator_id,
                name="field_oxide_etch",
                depth=0.8,         # μm - remove field oxide
                etch_type="anisotropic"
            )
            print(f"✓ Added etching step to remove field oxide")
            print(f"  - Etch depth: 0.8 μm")
            print(f"  - Etch type: anisotropic")
            
            return True
            
        except Exception as e:
            print(f"✗ Active area definition failed: {e}")
            return False
    
    def step3_gate_oxidation(self):
        """Step 3: Grow thin gate oxide"""
        self.print_step("GATE OXIDATION", "Growing thin, high-quality gate oxide")
        
        try:
            result = self.client.add_oxidation_step(
                self.simulator_id,
                name="gate_oxidation",
                temperature=950.0,   # °C - lower temp for thin oxide
                time=0.2,           # hours
                atmosphere="dry"     # dry oxidation for high quality
            )
            
            print(f"✓ Added gate oxidation step")
            print(f"  - Temperature: 950°C")
            print(f"  - Time: 0.2 hours (12 minutes)")
            print(f"  - Atmosphere: dry oxidation")
            print(f"  - Target thickness: {self.gate_oxide_thickness} μm")
            
            return True
            
        except Exception as e:
            print(f"✗ Gate oxidation failed: {e}")
            return False
    
    def step4_polysilicon_gate(self):
        """Step 4: Deposit and pattern polysilicon gate"""
        self.print_step("POLYSILICON GATE", "Depositing and patterning gate electrode")
        
        try:
            # Polysilicon deposition
            result = self.client.add_deposition_step(
                self.simulator_id,
                name="polysilicon_deposition",
                material="polysilicon",
                thickness=0.35,     # μm
                temperature=620.0   # °C
            )
            print(f"✓ Added polysilicon deposition")
            print(f"  - Thickness: 0.35 μm")
            print(f"  - Temperature: 620°C")
            
            # Polysilicon doping (for low resistance)
            result = self.client.add_doping_step(
                self.simulator_id,
                name="polysilicon_doping",
                dopant="phosphorus",
                concentration=1e20,  # cm⁻³
                energy=50.0,        # keV
                temperature=950.0   # °C
            )
            print(f"✓ Added polysilicon doping")
            print(f"  - Dopant: phosphorus")
            print(f"  - Concentration: 1×10²⁰ cm⁻³")
            
            # Gate lithography
            result = self.client.add_lithography_step(
                self.simulator_id,
                name="gate_lithography",
                wavelength=248.0,   # DUV lithography for fine features
                dose=25.0          # mJ/cm²
            )
            print(f"✓ Added gate lithography")
            print(f"  - Wavelength: 248 nm (DUV)")
            print(f"  - Target gate length: {self.gate_length} μm")
            
            # Gate etching
            result = self.client.add_etching_step(
                self.simulator_id,
                name="gate_etch",
                depth=0.35,        # μm - etch through polysilicon
                etch_type="anisotropic"
            )
            print(f"✓ Added gate etching")
            
            return True
            
        except Exception as e:
            print(f"✗ Polysilicon gate formation failed: {e}")
            return False
    
    def step5_source_drain_formation(self):
        """Step 5: Form source and drain regions"""
        self.print_step("SOURCE/DRAIN FORMATION", "Implanting and activating source/drain")
        
        try:
            # Source/drain implantation
            result = self.client.add_doping_step(
                self.simulator_id,
                name="source_drain_implant",
                dopant="arsenic",    # n-type for NMOS
                concentration=1e20,  # cm⁻³ - heavy doping
                energy=80.0,        # keV
                temperature=1000.0  # °C
            )
            print(f"✓ Added source/drain implantation")
            print(f"  - Dopant: arsenic (n-type)")
            print(f"  - Concentration: 1×10²⁰ cm⁻³")
            print(f"  - Energy: 80 keV")
            print(f"  - Target depth: {self.source_drain_depth} μm")
            
            # Activation annealing
            result = self.client.add_process_step(
                self.simulator_id,
                {
                    'type': 'annealing',
                    'name': 'activation_anneal',
                    'parameters': {
                        'temperature': 1000.0,  # °C
                        'time': 0.5,           # hours
                        'atmosphere': 'N2'      # inert atmosphere
                    }
                }
            )
            print(f"✓ Added activation annealing")
            print(f"  - Temperature: 1000°C")
            print(f"  - Time: 0.5 hours")
            print(f"  - Atmosphere: N₂")
            
            return True
            
        except Exception as e:
            print(f"✗ Source/drain formation failed: {e}")
            return False
    
    def step6_metallization(self):
        """Step 6: Form metal contacts and interconnects"""
        self.print_step("METALLIZATION", "Forming metal contacts and interconnects")
        
        try:
            # Interlayer dielectric deposition
            result = self.client.add_deposition_step(
                self.simulator_id,
                name="interlayer_dielectric",
                material="sio2",
                thickness=0.8,      # μm
                temperature=400.0   # °C - low temp to preserve junctions
            )
            print(f"✓ Added interlayer dielectric")
            print(f"  - Material: SiO₂")
            print(f"  - Thickness: 0.8 μm")
            
            # Contact hole lithography
            result = self.client.add_lithography_step(
                self.simulator_id,
                name="contact_lithography",
                wavelength=365.0,   # i-line
                dose=120.0         # mJ/cm²
            )
            print(f"✓ Added contact lithography")
            
            # Contact hole etching
            result = self.client.add_etching_step(
                self.simulator_id,
                name="contact_etch",
                depth=0.8,         # μm - etch through ILD
                etch_type="anisotropic"
            )
            print(f"✓ Added contact etching")
            
            # Metal deposition
            result = self.client.add_deposition_step(
                self.simulator_id,
                name="metal_deposition",
                material="aluminum",
                thickness=1.0,      # μm
                temperature=350.0   # °C
            )
            print(f"✓ Added metal deposition")
            print(f"  - Material: aluminum")
            print(f"  - Thickness: 1.0 μm")
            
            # Metal patterning
            result = self.client.add_lithography_step(
                self.simulator_id,
                name="metal_lithography",
                wavelength=365.0,
                dose=100.0
            )
            print(f"✓ Added metal lithography")
            
            result = self.client.add_etching_step(
                self.simulator_id,
                name="metal_etch",
                depth=1.0,
                etch_type="anisotropic"
            )
            print(f"✓ Added metal etching")
            
            return True
            
        except Exception as e:
            print(f"✗ Metallization failed: {e}")
            return False
    
    async def run_simulation(self):
        """Execute the complete simulation"""
        self.print_step("SIMULATION EXECUTION", "Running complete MOSFET fabrication")
        
        try:
            # Start simulation
            task_id = self.client.start_simulation(self.simulator_id, "mosfet_wafer")
            print(f"✓ Started simulation with task ID: {task_id}")
            
            # Monitor progress
            start_time = time.time()
            last_progress = -1
            
            while True:
                status = self.client.get_simulation_status(task_id)
                current_progress = status.get('progress', {}).get('progress_percentage', 0)
                
                if current_progress != last_progress:
                    print(f"Progress: {current_progress:.1f}% - {status.get('progress', {}).get('current_operation', 'Processing...')}")
                    last_progress = current_progress
                
                if status['status'] in ['completed', 'failed', 'cancelled']:
                    break
                
                await asyncio.sleep(2)
            
            elapsed_time = time.time() - start_time
            print(f"\nSimulation completed in {elapsed_time:.1f} seconds")
            print(f"Final status: {status['status']}")
            
            if status['status'] == 'completed':
                # Get results
                self.results = self.client.get_results(self.simulator_id)
                print(f"✓ Retrieved simulation results")
                return True
            else:
                print(f"✗ Simulation failed: {status}")
                return False
                
        except Exception as e:
            print(f"✗ Simulation execution failed: {e}")
            return False
    
    def analyze_results(self):
        """Analyze and visualize simulation results"""
        self.print_step("RESULTS ANALYSIS", "Analyzing MOSFET characteristics")
        
        try:
            if not self.results:
                print("✗ No results available for analysis")
                return False
            
            print(f"✓ Analyzing results...")
            
            # Save results to file
            results_file = self.output_dir / "mosfet_results.json"
            with open(results_file, 'w') as f:
                f.write(serialize_to_json(self.results))
            print(f"✓ Saved results to {results_file}")
            
            # Extract key metrics (simulated data for demonstration)
            metrics = {
                'gate_length': self.gate_length,
                'gate_oxide_thickness': self.gate_oxide_thickness,
                'threshold_voltage': 0.7,  # V
                'transconductance': 150,   # μS/μm
                'drain_current': 250,      # μA/μm at Vgs=Vds=3V
                'subthreshold_slope': 85,  # mV/decade
                'gate_leakage': 1e-12     # A/cm²
            }
            
            print(f"\n📊 MOSFET CHARACTERISTICS:")
            print(f"  Gate Length: {metrics['gate_length']} μm")
            print(f"  Gate Oxide Thickness: {metrics['gate_oxide_thickness']} μm")
            print(f"  Threshold Voltage: {metrics['threshold_voltage']} V")
            print(f"  Transconductance: {metrics['transconductance']} μS/μm")
            print(f"  Drain Current: {metrics['drain_current']} μA/μm")
            print(f"  Subthreshold Slope: {metrics['subthreshold_slope']} mV/decade")
            print(f"  Gate Leakage: {metrics['gate_leakage']:.2e} A/cm²")
            
            # Create visualization
            self.create_visualizations(metrics)
            
            return True
            
        except Exception as e:
            print(f"✗ Results analysis failed: {e}")
            return False
    
    def create_visualizations(self, metrics):
        """Create visualization plots"""
        try:
            # Create I-V characteristics plot
            fig, ((ax1, ax2), (ax3, ax4)) = plt.subplots(2, 2, figsize=(12, 10))
            
            # Transfer characteristics (Id vs Vgs)
            vgs = np.linspace(0, 3, 100)
            vth = metrics['threshold_voltage']
            gm = metrics['transconductance'] * 1e-6  # Convert to S/μm
            
            # Simplified MOSFET model
            ids = np.where(vgs > vth, 
                          gm * (vgs - vth)**2,  # Saturation region
                          0)  # Cutoff region
            
            ax1.semilogy(vgs, np.maximum(ids, 1e-12))
            ax1.set_xlabel('Gate Voltage (V)')
            ax1.set_ylabel('Drain Current (A/μm)')
            ax1.set_title('Transfer Characteristics')
            ax1.grid(True)
            
            # Output characteristics (Id vs Vds)
            vds = np.linspace(0, 3, 100)
            vgs_values = [1.0, 1.5, 2.0, 2.5, 3.0]
            
            for vgs_val in vgs_values:
                if vgs_val > vth:
                    ids_output = np.where(vds < (vgs_val - vth),
                                        gm * (2*(vgs_val - vth)*vds - vds**2),  # Linear
                                        gm * (vgs_val - vth)**2)  # Saturation
                    ax2.plot(vds, ids_output, label=f'Vgs = {vgs_val}V')
            
            ax2.set_xlabel('Drain Voltage (V)')
            ax2.set_ylabel('Drain Current (A/μm)')
            ax2.set_title('Output Characteristics')
            ax2.legend()
            ax2.grid(True)
            
            # Device cross-section (schematic)
            ax3.barh(['Metal', 'ILD', 'Poly Gate', 'Gate Oxide', 'Silicon'], 
                    [1.0, 0.8, 0.35, 0.005, 10], 
                    color=['silver', 'lightblue', 'orange', 'red', 'gray'])
            ax3.set_xlabel('Thickness (μm)')
            ax3.set_title('Device Layer Stack')
            
            # Performance metrics
            metrics_names = ['Vth (V)', 'gm (μS/μm)', 'SS (mV/dec)', 'Ig (pA/cm²)']
            metrics_values = [metrics['threshold_voltage'], 
                            metrics['transconductance'],
                            metrics['subthreshold_slope'],
                            metrics['gate_leakage'] * 1e12]
            
            bars = ax4.bar(metrics_names, metrics_values, 
                          color=['blue', 'green', 'orange', 'red'])
            ax4.set_title('Key Performance Metrics')
            ax4.set_ylabel('Value')
            
            # Add value labels on bars
            for bar, value in zip(bars, metrics_values):
                height = bar.get_height()
                ax4.text(bar.get_x() + bar.get_width()/2., height,
                        f'{value:.1f}' if value >= 1 else f'{value:.2f}',
                        ha='center', va='bottom')
            
            plt.tight_layout()
            
            # Save plot
            plot_file = self.output_dir / "mosfet_characteristics.png"
            plt.savefig(plot_file, dpi=300, bbox_inches='tight')
            print(f"✓ Saved characteristics plot to {plot_file}")
            
            plt.show()
            
        except Exception as e:
            print(f"✗ Visualization creation failed: {e}")
    
    def cleanup(self):
        """Cleanup simulation resources"""
        if self.simulator_id:
            try:
                self.client.delete_simulator(self.simulator_id)
                print(f"✓ Cleaned up simulator: {self.simulator_id}")
            except Exception as e:
                print(f"✗ Cleanup failed: {e}")
    
    async def run_complete_fabrication(self):
        """Run the complete MOSFET fabrication process"""
        print("🔬 SemiPRO MOSFET Fabrication Example")
        print("=====================================")
        print(f"Target specifications:")
        print(f"  - Gate length: {self.gate_length} μm")
        print(f"  - Gate oxide: {self.gate_oxide_thickness} μm")
        print(f"  - S/D depth: {self.source_drain_depth} μm")
        
        try:
            # Execute all fabrication steps
            if not self.setup_simulation():
                return False
            
            if not self.step1_field_oxidation():
                return False
            
            if not self.step2_active_area_definition():
                return False
            
            if not self.step3_gate_oxidation():
                return False
            
            if not self.step4_polysilicon_gate():
                return False
            
            if not self.step5_source_drain_formation():
                return False
            
            if not self.step6_metallization():
                return False
            
            # Run simulation
            if not await self.run_simulation():
                return False
            
            # Analyze results
            if not self.analyze_results():
                return False
            
            print(f"\n🎉 MOSFET fabrication completed successfully!")
            print(f"📁 Results saved to: {self.output_dir}")
            
            return True
            
        except Exception as e:
            print(f"\n❌ Fabrication failed: {e}")
            return False
        
        finally:
            self.cleanup()

async def main():
    """Main function to run the MOSFET fabrication example"""
    example = MOSFETFabricationExample()
    
    try:
        success = await example.run_complete_fabrication()
        if success:
            print("\n✅ Example completed successfully!")
        else:
            print("\n❌ Example failed!")
            sys.exit(1)
    
    except KeyboardInterrupt:
        print("\n⚠️  Example interrupted by user")
        example.cleanup()
    
    except Exception as e:
        print(f"\n💥 Unexpected error: {e}")
        example.cleanup()
        sys.exit(1)

if __name__ == "__main__":
    # Check if matplotlib is available for visualization
    try:
        import matplotlib.pyplot as plt
    except ImportError:
        print("Warning: matplotlib not available. Visualizations will be skipped.")

    asyncio.run(main())
