# SemiPRO Python Frontend Tutorial

This tutorial demonstrates how to use the SemiPRO Python frontend for semiconductor process simulation, including the API and GUI components.

## Prerequisites

- Python 3.8 or higher
- NumPy, Matplotlib
- PySide6 (for GUI components)
- Built SemiPRO C++ core and Cython extensions

## Installation

```bash
# From the project root
pip install -e .

# Or for development mode
pip install -e ".[dev]"
```

## 1. Basic API Usage

The SemiPRO Python API provides a high-level interface to the C++ core functionality through Cython bindings.

```python
from semipro.api.client import SemiproClient
from semipro.api.models import WaferConfig, ProcessConfig

# Create a client
client = SemiproClient()

# Create a wafer
wafer_config = WaferConfig(
    diameter=100.0,
    grid_size_x=50,
    grid_size_y=50
)
wafer = client.create_wafer(wafer_config)

# Run an oxidation process
oxidation_config = ProcessConfig(
    process_type="oxidation",
    parameters={
        "temperature": 1000.0,
        "time": 3600.0,
        "pressure": 1.0,
        "type": "dry"
    }
)
result = client.run_process(wafer, oxidation_config)

print(f"Oxide thickness: {result.get('oxide_thickness', 'N/A')} nm")
print(f"Growth rate: {result.get('growth_rate', 'N/A')} nm/min")
```

## 2. Process Simulation Workflow

```python
from semipro.api.client import SemiproClient
from semipro.api.models import WaferConfig, ProcessConfig
import matplotlib.pyplot as plt

def run_complete_workflow():
    """Run a complete semiconductor process workflow"""
    
    client = SemiproClient()
    
    # Create wafer
    wafer_config = WaferConfig(
        diameter=100.0,
        grid_size_x=100,
        grid_size_y=100
    )
    wafer = client.create_wafer(wafer_config)
    
    # Step 1: Initial oxidation
    print("Step 1: Initial oxidation...")
    oxidation_config = ProcessConfig(
        process_type="oxidation",
        parameters={
            "temperature": 1000.0,
            "time": 7200.0,
            "pressure": 1.0,
            "type": "dry"
        }
    )
    ox_result = client.run_process(wafer, oxidation_config)
    print(f"  Oxide thickness: {ox_result.get('oxide_thickness', 'N/A')} nm")
    
    # Step 2: Photolithography
    print("Step 2: Photolithography...")
    litho_config = ProcessConfig(
        process_type="photolithography",
        parameters={
            "resist_thickness": 1000.0,
            "exposure_dose": 150.0,
            "development_time": 60.0,
            "mask_pattern": "rectangle",
            "mask_dimensions": [20.0, 20.0, 60.0, 60.0]
        }
    )
    litho_result = client.run_process(wafer, litho_config)
    print(f"  Critical dimension: {litho_result.get('critical_dimension', 'N/A')} nm")
    
    # Step 3: Etching
    print("Step 3: Etching...")
    etching_config = ProcessConfig(
        process_type="etching",
        parameters={
            "etchant": "SF6_O2",
            "temperature": 25.0,
            "pressure": 0.01,
            "time": 300.0,
            "rf_power": 100.0,
            "bias_voltage": 50.0,
            "type": "RIE"
        }
    )
    etch_result = client.run_process(wafer, etching_config)
    print(f"  Etch depth: {etch_result.get('etch_depth', 'N/A')} nm")
    print(f"  Anisotropy: {etch_result.get('anisotropy', 'N/A')}")
    
    # Step 4: Doping
    print("Step 4: Ion implantation...")
    doping_config = ProcessConfig(
        process_type="doping",
        parameters={
            "species": "phosphorus",
            "energy": 80000.0,
            "dose": 5e15,
            "angle": 7.0
        }
    )
    doping_result = client.run_process(wafer, doping_config)
    print(f"  Peak concentration: {doping_result.get('peak_concentration', 'N/A')} cm⁻³")
    print(f"  Junction depth: {doping_result.get('junction_depth', 'N/A')} nm")
    
    # Step 5: Annealing
    print("Step 5: Thermal annealing...")
    annealing_config = ProcessConfig(
        process_type="thermal",
        parameters={
            "temperature": 1000.0,
            "time": 1800.0,
            "atmosphere": "nitrogen"
        }
    )
    annealing_result = client.run_process(wafer, annealing_config)
    print(f"  Diffusion length: {annealing_result.get('diffusion_length', 'N/A')} nm")
    
    # Step 6: Deposition
    print("Step 6: Polysilicon deposition...")
    deposition_config = ProcessConfig(
        process_type="deposition",
        parameters={
            "material": "polysilicon",
            "temperature": 650.0,
            "pressure": 0.1,
            "time": 1800.0,
            "gas_flow_rate": 100.0,
            "type": "CVD"
        }
    )
    deposition_result = client.run_process(wafer, deposition_config)
    print(f"  Film thickness: {deposition_result.get('film_thickness', 'N/A')} nm")
    print(f"  Uniformity: {deposition_result.get('uniformity', 'N/A')}%")
    
    # Step 7: Metallization
    print("Step 7: Metallization...")
    metal_config = ProcessConfig(
        process_type="metallization",
        parameters={
            "metal": "aluminum",
            "thickness": 500.0,
            "temperature": 150.0,
            "deposition_rate": 10.0
        }
    )
    metal_result = client.run_process(wafer, metal_config)
    print(f"  Metal thickness: {metal_result.get('metal_thickness', 'N/A')} nm")
    print(f"  Sheet resistance: {metal_result.get('sheet_resistance', 'N/A')} Ω/sq")
    
    # Step 8: Defect inspection
    print("Step 8: Defect inspection...")
    inspection_config = ProcessConfig(
        process_type="defect_inspection",
        parameters={
            "technique": "SEM",
            "pixel_size": 0.01,
            "scan_speed": 10.0,
            "sensitivity": 0.95
        }
    )
    inspection_result = client.run_process(wafer, inspection_config)
    print(f"  Defect count: {inspection_result.get('defect_count', 'N/A')}")
    
    # Step 9: Metrology
    print("Step 9: Metrology...")
    metrology_config = ProcessConfig(
        process_type="metrology",
        parameters={
            "technique": "ellipsometry",
            "measurement_type": "thickness",
            "resolution": 0.001,
            "scan_area": 1.0
        }
    )
    metrology_result = client.run_process(wafer, metrology_config)
    print(f"  Measured thickness: {metrology_result.get('measured_value', 'N/A')} nm")
    print(f"  Uncertainty: ±{metrology_result.get('uncertainty', 'N/A')} nm")
    
    # Get final wafer state
    wafer_state = client.get_wafer_state(wafer)
    
    print("Process workflow completed successfully!")
    return wafer, wafer_state

# Run the complete workflow
wafer, wafer_state = run_complete_workflow()
```

## 3. Data Visualization

```python
from semipro.api.client import SemiproClient
from semipro.api.models import WaferConfig, ProcessConfig
from semipro.api.visualization import plot_wafer_cross_section, plot_doping_profile
import matplotlib.pyplot as plt
import numpy as np

# Create client and wafer
client = SemiproClient()
wafer = client.create_wafer(WaferConfig(diameter=100.0, grid_size_x=50, grid_size_y=50))

# Run doping process
doping_config = ProcessConfig(
    process_type="doping",
    parameters={
        "species": "phosphorus",
        "energy": 50000.0,
        "dose": 1e15,
        "angle": 7.0
    }
)
doping_result = client.run_process(wafer, doping_config)

# Get doping profile
profile_data = doping_result.get('concentration_profile', [])
depths = [point[0] for point in profile_data]
concentrations = [point[1] for point in profile_data]

# Plot doping profile
plt.figure(figsize=(10, 6))
plt.semilogy(depths, concentrations, 'b-', linewidth=2)
plt.xlabel('Depth (nm)')
plt.ylabel('Concentration (cm⁻³)')
plt.title('Ion Implantation Concentration Profile')
plt.grid(True, alpha=0.3)
plt.show()

# Plot wafer cross-section
wafer_state = client.get_wafer_state(wafer)
plot_wafer_cross_section(wafer_state, y_position=25.0, title="Wafer Cross-Section")
```

## 4. Batch Processing and Parameter Sweeps

```python
from semipro.api.client import SemiproClient
from semipro.api.models import WaferConfig, ProcessConfig
import matplotlib.pyplot as plt
import numpy as np
from concurrent.futures import ProcessPoolExecutor
import time

def run_oxidation(temperature):
    """Run oxidation process at specified temperature"""
    client = SemiproClient()
    wafer = client.create_wafer(WaferConfig(diameter=100.0, grid_size_x=50, grid_size_y=50))
    
    oxidation_config = ProcessConfig(
        process_type="oxidation",
        parameters={
            "temperature": temperature,
            "time": 3600.0,
            "pressure": 1.0,
            "type": "dry"
        }
    )
    result = client.run_process(wafer, oxidation_config)
    return temperature, result.get('oxide_thickness', 0.0)

def parameter_sweep():
    """Perform parameter sweep for oxidation temperature"""
    
    # Define temperature range
    temperatures = np.linspace(800, 1200, 9)
    
    # Sequential execution
    start_time = time.time()
    results_sequential = []
    for temp in temperatures:
        results_sequential.append(run_oxidation(temp))
    sequential_time = time.time() - start_time
    
    # Parallel execution
    start_time = time.time()
    results_parallel = []
    with ProcessPoolExecutor(max_workers=4) as executor:
        results_parallel = list(executor.map(run_oxidation, temperatures))
    parallel_time = time.time() - start_time
    
    # Extract results
    temps, thicknesses = zip(*results_parallel)
    
    # Plot results
    plt.figure(figsize=(10, 6))
    plt.plot(temps, thicknesses, 'bo-', linewidth=2)
    plt.xlabel('Temperature (°C)')
    plt.ylabel('Oxide Thickness (nm)')
    plt.title('Oxidation Temperature Parameter Sweep')
    plt.grid(True, alpha=0.3)
    
    print(f"Sequential execution time: {sequential_time:.2f} seconds")
    print(f"Parallel execution time: {parallel_time:.2f} seconds")
    print(f"Speedup: {sequential_time/parallel_time:.2f}x")
    
    plt.show()

# Run parameter sweep
parameter_sweep()
```

## 5. GUI Application

### 5.1 Running the GUI

```python
from semipro.gui.main import run_gui

# Launch the GUI application
run_gui()
```

### 5.2 Creating Custom GUI Panels

```python
from PySide6.QtWidgets import (
    QWidget, QVBoxLayout, QHBoxLayout, QLabel, QPushButton,
    QComboBox, QDoubleSpinBox, QGroupBox
)
from PySide6.QtCore import Signal

class CustomProcessPanel(QWidget):
    """Custom process panel for the SemiPRO GUI"""
    
    # Signal to notify when process is complete
    process_updated = Signal(object)
    
    def __init__(self, parent=None):
        """Initialize the custom process panel"""
        super().__init__(parent)
        
        # Initialize UI
        self.init_ui()
        
        # Connect signals
        self.connect_signals()
        
    def init_ui(self):
        """Initialize the user interface"""
        main_layout = QVBoxLayout()
        
        # Process parameters
        params_group = QGroupBox("Process Parameters")
        params_layout = QVBoxLayout()
        
        # Parameter 1
        param1_layout = QHBoxLayout()
        param1_layout.addWidget(QLabel("Parameter 1:"))
        self.param1_spin = QDoubleSpinBox()
        self.param1_spin.setRange(0.0, 100.0)
        self.param1_spin.setValue(50.0)
        param1_layout.addWidget(self.param1_spin)
        params_layout.addLayout(param1_layout)
        
        # Parameter 2
        param2_layout = QHBoxLayout()
        param2_layout.addWidget(QLabel("Parameter 2:"))
        self.param2_combo = QComboBox()
        self.param2_combo.addItems(["Option 1", "Option 2", "Option 3"])
        param2_layout.addWidget(self.param2_combo)
        params_layout.addLayout(param2_layout)
        
        params_group.setLayout(params_layout)
        main_layout.addWidget(params_group)
        
        # Run button
        self.run_button = QPushButton("Run Process")
        main_layout.addWidget(self.run_button)
        
        self.setLayout(main_layout)
        
    def connect_signals(self):
        """Connect UI signals to slots"""
        self.run_button.clicked.connect(self.run_process)
        
    def run_process(self):
        """Run the custom process"""
        # Get parameters
        param1 = self.param1_spin.value()
        param2 = self.param2_combo.currentText()
        
        print(f"Running custom process with param1={param1}, param2={param2}")
        
        # TODO: Implement actual process logic
        
        # Emit signal to notify process completion
        self.process_updated.emit({"param1": param1, "param2": param2})
```

### 5.3 Integrating Custom Panels

To integrate your custom panel into the main GUI:

1. Add your panel to `src/python/gui/main_window.py`:

```python
from .custom_process_panel import CustomProcessPanel

# In MainWindow.__init__:
self.custom_panel = CustomProcessPanel()
tab_widget.addTab(self.custom_panel, "Custom Process")

# Connect signals
self.custom_panel.process_updated.connect(self.visualization_panel.update_visualization)
```

2. Rebuild and run the application

## 6. Advanced Visualization

```python
from semipro.api.client import SemiproClient
from semipro.api.models import WaferConfig, ProcessConfig
from semipro.api.visualization import (
    plot_wafer_3d, plot_temperature_distribution, 
    plot_stress_distribution, plot_defect_map
)
import matplotlib.pyplot as plt

# Create client and wafer
client = SemiproClient()
wafer = client.create_wafer(WaferConfig(diameter=100.0, grid_size_x=100, grid_size_y=100))

# Run thermal process
thermal_config = ProcessConfig(
    process_type="thermal",
    parameters={
        "temperature": 1000.0,
        "time": 1800.0,
        "ramp_rate": 10.0,
        "atmosphere": "nitrogen"
    }
)
thermal_result = client.run_process(wafer, thermal_config)

# Get wafer state
wafer_state = client.get_wafer_state(wafer)

# Plot 3D wafer visualization
plot_wafer_3d(wafer_state, title="3D Wafer Visualization")

# Plot temperature distribution
plot_temperature_distribution(
    thermal_result.get('temperature_map', []),
    wafer_state,
    title="Temperature Distribution"
)

# Run stress analysis
reliability_config = ProcessConfig(
    process_type="reliability",
    parameters={
        "analysis_type": "stress",
        "temperature": 25.0
    }
)
reliability_result = client.run_process(wafer, reliability_config)

# Plot stress distribution
plot_stress_distribution(
    reliability_result.get('stress_map', []),
    wafer_state,
    title="Stress Distribution"
)

# Run defect inspection
inspection_config = ProcessConfig(
    process_type="defect_inspection",
    parameters={
        "technique": "SEM",
        "sensitivity": 0.95
    }
)
inspection_result = client.run_process(wafer, inspection_config)

# Plot defect map
plot_defect_map(
    inspection_result.get('defects', []),
    wafer_state,
    title="Defect Map"
)
```

## 7. Configuration and Settings

```python
from semipro.api.client import SemiproClient
from semipro.api.config import SemiproConfig

# Load default configuration
config = SemiproConfig.load_default()
print(f"Default configuration loaded from: {config.config_path}")

# Modify configuration
config.simulation.num_threads = 4
config.simulation.enable_gpu = False
config.visualization.default_colormap = "viridis"
config.logging.level = "INFO"

# Save configuration
config.save("custom_config.json")

# Create client with custom configuration
client = SemiproClient(config_path="custom_config.json")
```

## 8. Logging and Debugging

```python
import logging
from semipro.api.client import SemiproClient
from semipro.api.models import WaferConfig, ProcessConfig

# Configure logging
logging.basicConfig(
    level=logging.DEBUG,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s',
    filename='semipro_debug.log'
)

# Create client with debug mode
client = SemiproClient(debug=True)

try:
    # Create wafer
    wafer = client.create_wafer(WaferConfig(diameter=100.0, grid_size_x=50, grid_size_y=50))
    
    # Run process with invalid parameters to trigger error
    invalid_config = ProcessConfig(
        process_type="oxidation",
        parameters={
            "temperature": 2000.0,  # Invalid temperature
            "time": -100.0,         # Invalid time
            "type": "invalid_type"  # Invalid type
        }
    )
    
    result = client.run_process(wafer, invalid_config)
    
except Exception as e:
    logging.error(f"Process simulation failed: {e}")
    print(f"Error: {e}")
    
    # Get detailed error information
    error_info = client.get_last_error()
    print(f"Error code: {error_info.get('code', 'unknown')}")
    print(f"Error message: {error_info.get('message', 'unknown')}")
    print(f"Error location: {error_info.get('location', 'unknown')}")
```

## 9. API Extensions

To extend the SemiPRO API with custom functionality:

1. Create a new module in `src/python/api/extensions/`:

```python
# src/python/api/extensions/custom_extension.py
from ..client import SemiproClient
from ..models import ProcessConfig

class CustomExtension:
    """Custom extension for SemiPRO API"""
    
    def __init__(self, client):
        """Initialize with a SemiproClient instance"""
        self.client = client
    
    def run_custom_process(self, wafer, param1, param2):
        """Run a custom process with the given parameters"""
        config = ProcessConfig(
            process_type="custom",
            parameters={
                "param1": param1,
                "param2": param2,
                "custom_flag": True
            }
        )
        return self.client.run_process(wafer, config)
    
    def analyze_results(self, result):
        """Perform custom analysis on process results"""
        # Custom analysis logic here
        return {
            "custom_metric_1": result.get("value_1", 0) * 2,
            "custom_metric_2": result.get("value_2", 0) / 10
        }

# Extend SemiproClient
def extend_client(client):
    """Extend a SemiproClient instance with custom functionality"""
    client.custom = CustomExtension(client)
    return client
```

2. Use the extension:

```python
from semipro.api.client import SemiproClient
from semipro.api.models import WaferConfig
from semipro.api.extensions.custom_extension import extend_client

# Create and extend client
client = SemiproClient()
client = extend_client(client)

# Create wafer
wafer = client.create_wafer(WaferConfig(diameter=100.0, grid_size_x=50, grid_size_y=50))

# Use custom extension
result = client.custom.run_custom_process(wafer, 10.0, 20.0)
analysis = client.custom.analyze_results(result)

print(f"Custom analysis results: {analysis}")
```

## 10. Integration with External Tools

```python
from semipro.api.client import SemiproClient
from semipro.api.models import WaferConfig, ProcessConfig
from semipro.api.exporters import GDSExporter, SEMExporter
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

# Create client and wafer
client = SemiproClient()
wafer = client.create_wafer(WaferConfig(diameter=100.0, grid_size_x=50, grid_size_y=50))

# Run process
litho_config = ProcessConfig(
    process_type="photolithography",
    parameters={
        "resist_thickness": 1000.0,
        "exposure_dose": 150.0,
        "development_time": 60.0,
        "mask_pattern": "rectangle",
        "mask_dimensions": [20.0, 20.0, 60.0, 60.0]
    }
)
litho_result = client.run_process(wafer, litho_config)

# Export to GDS format for use with layout tools
gds_exporter = GDSExporter()
gds_file = gds_exporter.export(wafer, "layout.gds")
print(f"GDS file exported to: {gds_file}")

# Export SEM image for analysis
sem_exporter = SEMExporter()
sem_image = sem_exporter.export(wafer, "sem_image.png", resolution=1024)
print(f"SEM image exported to: {sem_image}")

# Export data for analysis in pandas
wafer_state = client.get_wafer_state(wafer)
thickness_data = np.array(wafer_state.get('thickness_map', []))
df = pd.DataFrame(thickness_data)
df.to_csv("thickness_data.csv")
print("Thickness data exported to CSV for analysis")

# Plot histogram of thickness distribution
plt.figure(figsize=(8, 6))
plt.hist(thickness_data.flatten(), bins=50)
plt.xlabel('Thickness (nm)')
plt.ylabel('Frequency')
plt.title('Thickness Distribution')
plt.grid(True, alpha=0.3)
plt.savefig("thickness_histogram.png")
print("Thickness histogram saved")
```

## Next Steps

1. Explore the C++ core tutorial for low-level implementation details
2. Learn about the Cython integration for performance-critical applications
3. Study the complete application architecture
4. Implement custom process modules and GUI panels

For more information, refer to the API documentation and source code comments.
