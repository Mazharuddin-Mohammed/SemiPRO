"""
Author: Dr. Mazharuddin Mohammed
Enhanced Python bindings for SemiPRO Semiconductor Simulator
Provides high-level Python interface with additional features
"""

import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
from typing import Dict, List, Tuple, Optional, Union
import json
import yaml
from pathlib import Path

class EnhancedWafer:
    """Enhanced Python wrapper for Wafer class with additional functionality"""
    
    def __init__(self, diameter: float, thickness: float, material: str = "silicon"):
        self.diameter = diameter
        self.thickness = thickness
        self.material = material
        self.layers = []
        self.process_history = []
        self.properties = {}
        
    def add_layer(self, material: str, thickness: float, properties: Dict = None):
        """Add a layer to the wafer stack"""
        layer = {
            'material': material,
            'thickness': thickness,
            'properties': properties or {}
        }
        self.layers.append(layer)
        self.process_history.append(f"Added {material} layer ({thickness} um)")
        
    def get_total_thickness(self) -> float:
        """Calculate total thickness including all layers"""
        return self.thickness + sum(layer['thickness'] for layer in self.layers)
        
    def get_layer_stack(self) -> pd.DataFrame:
        """Return layer stack as pandas DataFrame"""
        data = []
        cumulative_thickness = 0
        
        # Base wafer
        data.append({
            'layer': 0,
            'material': self.material,
            'thickness': self.thickness,
            'start_depth': 0,
            'end_depth': self.thickness
        })
        cumulative_thickness = self.thickness
        
        # Additional layers
        for i, layer in enumerate(self.layers, 1):
            start_depth = cumulative_thickness
            end_depth = start_depth + layer['thickness']
            data.append({
                'layer': i,
                'material': layer['material'],
                'thickness': layer['thickness'],
                'start_depth': start_depth,
                'end_depth': end_depth
            })
            cumulative_thickness = end_depth
            
        return pd.DataFrame(data)
        
    def visualize_cross_section(self, figsize: Tuple[int, int] = (10, 6)):
        """Create a cross-section visualization of the wafer"""
        fig, ax = plt.subplots(figsize=figsize)
        
        layer_stack = self.get_layer_stack()
        colors = plt.cm.Set3(np.linspace(0, 1, len(layer_stack)))
        
        for i, (_, layer) in enumerate(layer_stack.iterrows()):
            ax.barh(0, layer['thickness'], left=layer['start_depth'], 
                   height=1, color=colors[i], alpha=0.7, 
                   label=f"{layer['material']} ({layer['thickness']} μm)")
        
        ax.set_xlabel('Depth (μm)')
        ax.set_ylabel('Wafer Cross-section')
        ax.set_title(f'Wafer Layer Stack (Ø{self.diameter} mm)')
        ax.legend(bbox_to_anchor=(1.05, 1), loc='upper left')
        ax.set_ylim(-0.5, 1.5)
        
        plt.tight_layout()
        return fig, ax

class ProcessSimulator:
    """High-level process simulation interface"""
    
    def __init__(self, config_file: Optional[str] = None):
        self.config = self._load_config(config_file)
        self.wafers = {}
        self.simulation_log = []
        
    def _load_config(self, config_file: Optional[str]) -> Dict:
        """Load simulation configuration"""
        if config_file and Path(config_file).exists():
            with open(config_file, 'r') as f:
                if config_file.endswith('.yaml') or config_file.endswith('.yml'):
                    return yaml.safe_load(f)
                else:
                    return json.load(f)
        else:
            return self._default_config()
            
    def _default_config(self) -> Dict:
        """Default simulation configuration"""
        return {
            'grid_resolution': 100,
            'temperature_ambient': 300.0,
            'pressure_ambient': 1.0,
            'default_materials': {
                'silicon': {'density': 2.33, 'thermal_conductivity': 150},
                'sio2': {'density': 2.2, 'thermal_conductivity': 1.4},
                'si3n4': {'density': 3.2, 'thermal_conductivity': 20}
            }
        }
        
    def create_wafer(self, name: str, diameter: float, thickness: float, 
                    material: str = "silicon") -> EnhancedWafer:
        """Create a new wafer"""
        wafer = EnhancedWafer(diameter, thickness, material)
        self.wafers[name] = wafer
        self.simulation_log.append(f"Created wafer '{name}': Ø{diameter}mm, {thickness}μm {material}")
        return wafer
        
    def simulate_oxidation(self, wafer_name: str, temperature: float, 
                          time: float, atmosphere: str = "dry") -> Dict:
        """Simulate thermal oxidation"""
        if wafer_name not in self.wafers:
            raise ValueError(f"Wafer '{wafer_name}' not found")
            
        wafer = self.wafers[wafer_name]
        
        # Deal-Grove model for oxide growth
        if atmosphere == "dry":
            A, B = 0.165, 0.0117  # μm, μm²/h at 1000°C
        else:  # wet
            A, B = 0.05, 0.720   # μm, μm²/h at 1000°C
            
        # Temperature scaling (simplified Arrhenius)
        temp_factor = np.exp(-1.23 * (1000 - temperature) / temperature)
        A *= temp_factor
        B *= temp_factor
        
        # Calculate oxide thickness
        oxide_thickness = A * time + np.sqrt((A * time)**2 + B * time)
        
        # Add oxide layer
        wafer.add_layer("SiO2", oxide_thickness, {
            'growth_temperature': temperature,
            'growth_time': time,
            'atmosphere': atmosphere
        })
        
        result = {
            'oxide_thickness': oxide_thickness,
            'temperature': temperature,
            'time': time,
            'atmosphere': atmosphere
        }
        
        self.simulation_log.append(
            f"Oxidation on '{wafer_name}': {oxide_thickness:.3f}μm SiO2 "
            f"({temperature}°C, {time}h, {atmosphere})"
        )
        
        return result
        
    def simulate_deposition(self, wafer_name: str, material: str, 
                           thickness: float, method: str = "CVD") -> Dict:
        """Simulate thin film deposition"""
        if wafer_name not in self.wafers:
            raise ValueError(f"Wafer '{wafer_name}' not found")
            
        wafer = self.wafers[wafer_name]
        
        # Add deposited layer
        wafer.add_layer(material, thickness, {
            'deposition_method': method,
            'target_thickness': thickness
        })
        
        result = {
            'deposited_thickness': thickness,
            'material': material,
            'method': method
        }
        
        self.simulation_log.append(
            f"Deposition on '{wafer_name}': {thickness}μm {material} by {method}"
        )
        
        return result
        
    def simulate_etch(self, wafer_name: str, depth: float, 
                     etch_type: str = "isotropic") -> Dict:
        """Simulate etching process"""
        if wafer_name not in self.wafers:
            raise ValueError(f"Wafer '{wafer_name}' not found")
            
        wafer = self.wafers[wafer_name]
        
        if not wafer.layers:
            raise ValueError("No layers to etch")
            
        # Remove material from top layer
        top_layer = wafer.layers[-1]
        if top_layer['thickness'] <= depth:
            # Remove entire layer
            removed_thickness = top_layer['thickness']
            wafer.layers.pop()
        else:
            # Partially etch layer
            top_layer['thickness'] -= depth
            removed_thickness = depth
            
        result = {
            'etched_depth': removed_thickness,
            'etch_type': etch_type,
            'remaining_layers': len(wafer.layers)
        }
        
        self.simulation_log.append(
            f"Etching on '{wafer_name}': {removed_thickness}μm removed ({etch_type})"
        )
        
        return result
        
    def generate_process_flow(self, wafer_name: str) -> pd.DataFrame:
        """Generate process flow summary"""
        if wafer_name not in self.wafers:
            raise ValueError(f"Wafer '{wafer_name}' not found")
            
        wafer = self.wafers[wafer_name]
        
        data = []
        for i, step in enumerate(wafer.process_history):
            data.append({
                'step': i + 1,
                'process': step,
                'timestamp': f"Step {i + 1}"
            })
            
        return pd.DataFrame(data)
        
    def export_results(self, filename: str, format: str = "json"):
        """Export simulation results"""
        results = {
            'wafers': {},
            'simulation_log': self.simulation_log,
            'config': self.config
        }
        
        for name, wafer in self.wafers.items():
            results['wafers'][name] = {
                'diameter': wafer.diameter,
                'thickness': wafer.thickness,
                'material': wafer.material,
                'layers': wafer.layers,
                'process_history': wafer.process_history,
                'total_thickness': wafer.get_total_thickness()
            }
            
        if format.lower() == "json":
            with open(filename, 'w') as f:
                json.dump(results, f, indent=2)
        elif format.lower() == "yaml":
            with open(filename, 'w') as f:
                yaml.dump(results, f, default_flow_style=False)
        else:
            raise ValueError(f"Unsupported format: {format}")
            
    def create_summary_report(self) -> str:
        """Create a text summary report"""
        report = ["SemiPRO Simulation Summary", "=" * 30, ""]
        
        report.append(f"Total wafers: {len(self.wafers)}")
        report.append(f"Total process steps: {len(self.simulation_log)}")
        report.append("")
        
        for name, wafer in self.wafers.items():
            report.append(f"Wafer: {name}")
            report.append(f"  Base: Ø{wafer.diameter}mm, {wafer.thickness}μm {wafer.material}")
            report.append(f"  Layers: {len(wafer.layers)}")
            report.append(f"  Total thickness: {wafer.get_total_thickness():.3f}μm")
            report.append("")
            
        report.append("Process Log:")
        for step in self.simulation_log:
            report.append(f"  - {step}")
            
        return "\n".join(report)

# Example usage and demonstration
def demo_simulation():
    """Demonstrate the enhanced simulation capabilities"""
    print("SemiPRO Enhanced Simulation Demo")
    print("=" * 40)
    
    # Create simulator
    sim = ProcessSimulator()
    
    # Create wafer
    wafer = sim.create_wafer("demo_wafer", diameter=200, thickness=525)
    
    # Simulate process flow
    sim.simulate_oxidation("demo_wafer", temperature=1000, time=2.0, atmosphere="dry")
    sim.simulate_deposition("demo_wafer", material="Si3N4", thickness=0.1, method="LPCVD")
    sim.simulate_deposition("demo_wafer", material="Al", thickness=0.5, method="Sputtering")
    
    # Generate reports
    print("\nProcess Flow:")
    print(sim.generate_process_flow("demo_wafer"))
    
    print("\nLayer Stack:")
    print(wafer.get_layer_stack())
    
    print("\nSummary Report:")
    print(sim.create_summary_report())
    
    # Visualize (if matplotlib available)
    try:
        fig, ax = wafer.visualize_cross_section()
        plt.show()
    except ImportError:
        print("Matplotlib not available for visualization")
    
    return sim, wafer

if __name__ == "__main__":
    demo_simulation()
