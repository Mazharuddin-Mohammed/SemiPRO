# Author: Dr. Mazharuddin Mohammed
"""
C++ Bridge for SemiPRO - Direct interface to C++ backend
This module provides a bridge to the C++ backend without requiring Cython
"""

import subprocess
import json
import os
import tempfile
import numpy as np
from pathlib import Path

class CPPBridge:
    """Bridge to C++ backend using subprocess calls"""
    
    def __init__(self, build_dir="build"):
        self.build_dir = Path(build_dir)
        self.temp_dir = Path(tempfile.mkdtemp(prefix="semipro_"))
        self.temp_dir.mkdir(exist_ok=True)
        
        # Check if C++ executables exist
        self.simulator_path = self.build_dir / "simulator"
        if not self.simulator_path.exists():
            raise RuntimeError(f"C++ simulator not found at {self.simulator_path}")
    
    def __del__(self):
        """Cleanup temporary files"""
        try:
            import shutil
            if hasattr(self, 'temp_dir') and self.temp_dir and self.temp_dir.exists():
                shutil.rmtree(self.temp_dir, ignore_errors=True)
        except (ImportError, AttributeError):
            # Ignore cleanup errors during shutdown
            pass
    
    def create_config_file(self, config_data):
        """Create a configuration file for C++ backend"""
        config_file = self.temp_dir / "config.json"
        with open(config_file, 'w') as f:
            json.dump(config_data, f, indent=2)
        return config_file
    
    def run_simulation(self, process_type, parameters):
        """Run a simulation process"""
        # Create config with just the parameters (not nested)
        config_file = self.create_config_file(parameters)
        
        try:
            # Run the C++ simulator
            result = subprocess.run([
                str(self.simulator_path),
                "--config", str(config_file),
                "--process", process_type
            ], capture_output=True, text=True, timeout=60)
            
            if result.returncode != 0:
                raise RuntimeError(f"C++ simulation failed: {result.stderr}")
            
            return self.parse_results()
            
        except subprocess.TimeoutExpired:
            raise RuntimeError("Simulation timed out")
    
    def parse_results(self):
        """Parse simulation results from output files"""
        results = {}
        
        # Look for common output files
        output_files = [
            "concentration.dat",
            "temperature.dat", 
            "stress.dat",
            "results.json"
        ]
        
        for filename in output_files:
            filepath = self.temp_dir / filename
            if filepath.exists():
                if filename.endswith('.json'):
                    with open(filepath, 'r') as f:
                        results[filename[:-5]] = json.load(f)
                elif filename.endswith('.dat'):
                    # Assume binary data files contain numpy arrays
                    try:
                        data = np.fromfile(filepath, dtype=np.float64)
                        results[filename[:-4]] = data
                    except:
                        # Fallback to text format
                        data = np.loadtxt(filepath)
                        results[filename[:-4]] = data
        
        return results

class GeometryBridge(CPPBridge):
    """Bridge for geometry operations"""
    
    def initialize_wafer(self, diameter, thickness, material="silicon"):
        """Initialize wafer geometry"""
        return self.run_simulation("geometry_init", {
            "diameter": diameter,
            "thickness": thickness,
            "material": material
        })
    
    def initialize_grid(self, x_dim, y_dim):
        """Initialize simulation grid"""
        return self.run_simulation("grid_init", {
            "x_dimension": x_dim,
            "y_dimension": y_dim
        })

    def apply_layer(self, thickness, material_id):
        """Apply a layer to the wafer using deposition process"""
        return self.run_simulation("deposition", {
            "thickness": thickness,
            "material": material_id,
            "temperature": 400.0  # Default deposition temperature
        })

class OxidationBridge(CPPBridge):
    """Bridge for oxidation simulations"""
    
    def simulate_oxidation(self, temperature, time, atmosphere="dry"):
        """Simulate thermal oxidation"""
        return self.run_simulation("oxidation", {
            "temperature": temperature,
            "time": time,
            "atmosphere": atmosphere
        })

class DopingBridge(CPPBridge):
    """Bridge for doping simulations"""
    
    def simulate_doping(self, dopant_type, concentration, energy, temperature):
        """Simulate ion implantation"""
        return self.run_simulation("doping", {
            "dopant_type": dopant_type,
            "concentration": concentration,
            "energy": energy,
            "temperature": temperature
        })

class DepositionBridge(CPPBridge):
    """Bridge for deposition simulations"""
    
    def simulate_deposition(self, material, thickness, temperature):
        """Simulate material deposition"""
        return self.run_simulation("deposition", {
            "material": material,
            "thickness": thickness,
            "temperature": temperature
        })

class EtchingBridge(CPPBridge):
    """Bridge for etching simulations"""
    
    def simulate_etching(self, etch_type, rate, time, selectivity=1.0):
        """Simulate etching process"""
        return self.run_simulation("etching", {
            "etch_type": etch_type,
            "rate": rate,
            "time": time,
            "selectivity": selectivity
        })

class ThermalBridge(CPPBridge):
    """Bridge for thermal simulations"""
    
    def simulate_thermal(self, temperature_profile, time_step, total_time):
        """Simulate thermal effects"""
        # Save temperature profile to file
        temp_file = self.temp_dir / "temperature_profile.dat"
        np.savetxt(temp_file, temperature_profile)
        
        return self.run_simulation("thermal", {
            "temperature_file": str(temp_file),
            "time_step": time_step,
            "total_time": total_time
        })

class ReliabilityBridge(CPPBridge):
    """Bridge for reliability simulations"""
    
    def simulate_reliability(self, stress_conditions, time_duration):
        """Simulate reliability effects"""
        return self.run_simulation("reliability", {
            "stress_conditions": stress_conditions,
            "time_duration": time_duration
        })

class RendererBridge(CPPBridge):
    """Bridge for Vulkan rendering"""
    
    def __init__(self, build_dir="build"):
        super().__init__(build_dir)
        self.renderer_path = self.build_dir / "example_geometry"  # Use existing example
    
    def render_wafer(self, data, visualization_type="concentration"):
        """Render wafer data using Vulkan"""
        if not self.renderer_path.exists():
            raise RuntimeError(f"Renderer not found at {self.renderer_path}")
        
        # Save data to file for rendering
        data_file = self.temp_dir / "render_data.dat"
        if isinstance(data, np.ndarray):
            np.savetxt(data_file, data)
        
        try:
            # Run the renderer (will open a window)
            result = subprocess.run([
                str(self.renderer_path),
                "--data", str(data_file),
                "--type", visualization_type
            ], capture_output=True, text=True, timeout=10)
            
            return result.returncode == 0
            
        except subprocess.TimeoutExpired:
            # Timeout is expected for GUI applications
            return True
        except Exception as e:
            print(f"Rendering failed: {e}")
            return False

# Factory function to create appropriate bridge
def create_bridge(module_name, build_dir="build"):
    """Create appropriate bridge for the given module"""
    bridges = {
        "geometry": GeometryBridge,
        "oxidation": OxidationBridge,
        "doping": DopingBridge,
        "deposition": DepositionBridge,
        "etching": EtchingBridge,
        "thermal": ThermalBridge,
        "reliability": ReliabilityBridge,
        "renderer": RendererBridge,
    }
    
    bridge_class = bridges.get(module_name, CPPBridge)
    return bridge_class(build_dir)
