# SemiPRO Cython Integration Tutorial

This tutorial demonstrates how to use the SemiPRO Cython wrappers to access C++ functionality from Python.

## Prerequisites

- Python 3.8 or higher
- Cython 0.29 or higher
- NumPy
- C++ compiler with C++17 support
- Built SemiPRO C++ core modules

## Building the Cython Extensions

```bash
# From the project root
cd src/cython
python setup.py build_ext --inplace

# Or using pip (recommended)
pip install -e .
```

## 1. Basic Wafer Operations

```python
import numpy as np
from semipro.cython import geometry

# Create a wafer
wafer = geometry.PyWafer(diameter=100.0, grid_x=50, grid_y=50)

print(f"Wafer diameter: {wafer.diameter} mm")
print(f"Grid size: {wafer.grid_size_x} x {wafer.grid_size_y}")

# Create geometry model
geom_model = geometry.PyGeometryModel()

# Add structures
geom_model.add_rectangle(wafer, 10.0, 10.0, 80.0, 80.0, 1.0)
geom_model.add_circle(wafer, 50.0, 50.0, 5.0, 2.0)

print("Geometry structures added successfully")
```

## 2. Oxidation Process

```python
from semipro.cython import oxidation

# Create wafer and oxidation model
wafer = geometry.PyWafer(100.0, 50, 50)
ox_model = oxidation.PyOxidationModel()

# Set up oxidation conditions
conditions = oxidation.PyOxidationConditions()
conditions.temperature = 1100.0  # °C
conditions.time = 3600.0         # seconds
conditions.pressure = 1.0        # atm
conditions.oxygen_partial_pressure = 0.21
conditions.water_vapor_pressure = 0.0
conditions.oxidation_type = oxidation.OXIDATION_DRY

# Run simulation
result = ox_model.simulate_oxidation(wafer, conditions)

print(f"Oxide thickness: {result.oxide_thickness:.2f} nm")
print(f"Growth rate: {result.growth_rate:.4f} nm/min")
print(f"Stress: {result.stress:.2f} MPa")
print(f"Interface roughness: {result.interface_roughness:.3f} nm")
```

## 3. Ion Implantation and Doping

```python
from semipro.cython import doping

# Create wafer and doping model
wafer = geometry.PyWafer(100.0, 50, 50)
doping_model = doping.PyMonteCarloSolver()

# Set up implantation conditions
conditions = doping.PyImplantationConditions()
conditions.species = doping.ION_PHOSPHORUS_31
conditions.energy = 50000.0     # eV
conditions.dose = 1e15          # ions/cm²
conditions.angle = 7.0          # degrees
conditions.temperature = 25.0   # °C

# Configure Monte Carlo parameters
doping_model.set_num_particles(10000)
doping_model.enable_channeling(True)
doping_model.set_random_seed(12345)

# Run simulation
result = doping_model.simulate_implantation(wafer, conditions)

print(f"Peak concentration: {result.peak_concentration:.2e} cm⁻³")
print(f"Junction depth: {result.junction_depth:.2f} nm")
print(f"Sheet resistance: {result.sheet_resistance:.2f} Ω/sq")
print(f"Straggle: {result.straggle:.2f} nm")

# Get concentration profile
profile = result.get_concentration_profile()
depths = [point[0] for point in profile]
concentrations = [point[1] for point in profile]

print(f"Profile has {len(profile)} points")
```

## 4. Deposition Process

```python
from semipro.cython import deposition

# Create wafer and deposition model
wafer = geometry.PyWafer(100.0, 50, 50)
dep_model = deposition.PyDepositionModel()

# Set up deposition conditions
conditions = deposition.PyDepositionConditions()
conditions.material = deposition.MATERIAL_POLYSILICON
conditions.temperature = 650.0   # °C
conditions.pressure = 0.1        # Torr
conditions.time = 1800.0         # seconds
conditions.gas_flow_rate = 100.0 # sccm
conditions.deposition_type = deposition.DEPOSITION_CVD

# Configure advanced parameters
dep_model.enable_stress_calculation(True)
dep_model.set_nucleation_density(1e12)
dep_model.enable_grain_growth(True)

# Run simulation
result = dep_model.simulate_deposition(wafer, conditions)

print(f"Film thickness: {result.film_thickness:.2f} nm")
print(f"Uniformity: {result.uniformity:.1f}%")
print(f"Deposition rate: {result.deposition_rate:.2f} nm/min")
print(f"Stress: {result.stress:.2f} MPa")
print(f"Grain size: {result.grain_size:.2f} nm")
```

## 5. Etching Process

```python
from semipro.cython import etching

# Create wafer and etching model
wafer = geometry.PyWafer(100.0, 50, 50)
etch_model = etching.PyEtchingModel()

# Set up etching conditions
conditions = etching.PyEtchingConditions()
conditions.etchant_type = etching.ETCHANT_SF6_O2
conditions.temperature = 25.0    # °C
conditions.pressure = 0.01       # Torr
conditions.time = 300.0          # seconds
conditions.rf_power = 100.0      # W
conditions.bias_voltage = 50.0   # V
conditions.etching_type = etching.ETCHING_RIE

# Configure plasma parameters
etch_model.set_plasma_density(1e11)
etch_model.enable_ion_bombardment(True)
etch_model.set_mask_selectivity(10.0)

# Run simulation
result = etch_model.simulate_etching(wafer, conditions)

print(f"Etch depth: {result.etch_depth:.2f} nm")
print(f"Etch rate: {result.etch_rate:.2f} nm/min")
print(f"Selectivity: {result.selectivity:.1f}")
print(f"Anisotropy: {result.anisotropy:.3f}")
print(f"Surface roughness: {result.surface_roughness:.3f} nm")
```

## 6. Advanced Modules

### Defect Inspection

```python
from semipro.cython import defect_inspection

# Create wafer and inspection model
wafer = geometry.PyWafer(100.0, 50, 50)
inspector = defect_inspection.PyDefectInspectionModel()

# Configure inspection parameters
inspector.set_inspection_parameters(
    defect_inspection.INSPECTION_SEM,
    pixel_size=0.01,      # μm
    scan_speed=10.0,      # mm/s
    sensitivity=0.95
)
inspector.enable_defect_classification(True)
inspector.set_defect_size_threshold(0.1)  # μm

# Perform inspection
result = inspector.perform_inspection(wafer, defect_inspection.INSPECTION_SEM)

print(f"Found {result.defect_count} defects")
print(f"Coverage area: {result.coverage_area:.2f} cm²")
print(f"Inspection time: {result.inspection_time:.1f} s")
print(f"False positive rate: {result.false_positive_rate:.3f}")

# Get individual defects
defects = result.get_defects()
for i, defect in enumerate(defects[:5]):  # Show first 5 defects
    print(f"Defect {i+1}: Type={defect.type}, Size={defect.size:.3f}μm, "
          f"Position=({defect.x:.1f}, {defect.y:.1f})")
```

### Interconnect (Damascene) Process

```python
from semipro.cython import interconnect

# Create wafer and damascene model
wafer = geometry.PyWafer(100.0, 50, 50)
damascene = interconnect.PyDamasceneModel()

# Set up damascene parameters
params = interconnect.PyDamasceneParameters(
    damascene_type=interconnect.DAMASCENE_DUAL,
    line_width=100.0,        # nm
    line_spacing=100.0,      # nm
    metal_thickness=200.0,   # nm
    barrier_thickness=5.0,   # nm
    dielectric_thickness=300.0,  # nm
    barrier_material=interconnect.BARRIER_TANTALUM_NITRIDE,
    metal_material=interconnect.METAL_COPPER,
    cmp_enabled=True,
    cmp_pressure=5.0,        # psi
    cmp_time=60.0           # s
)

# Configure advanced settings
damascene.set_process_temperature(350.0)  # °C
damascene.enable_advanced_models(True)
damascene.set_barrier_conformality(0.9)

# Run damascene simulation
result = damascene.simulate_damascene(wafer, params)

print(f"Final thickness: {result.final_thickness:.2f} nm")
print(f"Resistance: {result.resistance:.6f} Ω")
print(f"Capacitance: {result.capacitance:.3e} F")
print(f"RC delay: {result.rc_delay:.3e} s")
print(f"Barrier coverage: {result.barrier_coverage:.1f}%")
print(f"Void probability: {result.void_probability:.3f}")
print(f"Electromigration lifetime: {result.electromigration_lifetime:.2e} hours")
```

### Metrology Measurements

```python
from semipro.cython import metrology

# Create wafer and metrology model
wafer = geometry.PyWafer(100.0, 50, 50)
metro = metrology.PyMetrologyModel()

# Configure measurement parameters
params = metrology.PyMetrologyParameters(
    technique=metrology.TECHNIQUE_ELLIPSOMETRY,
    measurement_type=metrology.MEASUREMENT_THICKNESS,
    resolution=0.001,
    accuracy=0.99,
    precision=0.001,
    scan_area=1.0,          # mm²
    scan_time=60.0,         # s
    destructive=False
)

# Configure model settings
metro.enable_noise_simulation(True)
metro.set_calibration_factor(1.0)

# Perform measurement
result = metro.perform_measurement(wafer, params)

print(f"Measured value: {result.measured_value:.6f} {result.units}")
print(f"Uncertainty: ±{result.uncertainty:.6f} {result.units}")
print(f"Range: {result.min_value:.6f} - {result.max_value:.6f} {result.units}")
print(f"Signal-to-noise ratio: {result.signal_to_noise:.2f}")
print(f"Summary: {result.summary}")

# Get profile data
profile_data = result.get_profile_data()
if profile_data:
    print(f"Profile contains {len(profile_data)} data points")
```

## 7. Process Integration and Workflows

```python
import numpy as np
import matplotlib.pyplot as plt

def complete_process_flow():
    """Demonstrate a complete semiconductor process flow"""
    
    # Create wafer
    wafer = geometry.PyWafer(100.0, 100, 100)
    
    # Step 1: Initial oxidation
    print("Step 1: Initial oxidation...")
    ox_model = oxidation.PyOxidationModel()
    ox_conditions = oxidation.PyOxidationConditions()
    ox_conditions.temperature = 1000.0
    ox_conditions.time = 7200.0
    ox_conditions.oxidation_type = oxidation.OXIDATION_DRY
    
    ox_result = ox_model.simulate_oxidation(wafer, ox_conditions)
    print(f"  Oxide thickness: {ox_result.oxide_thickness:.2f} nm")
    
    # Step 2: Ion implantation
    print("Step 2: Ion implantation...")
    doping_model = doping.PyMonteCarloSolver()
    doping_conditions = doping.PyImplantationConditions()
    doping_conditions.species = doping.ION_PHOSPHORUS_31
    doping_conditions.energy = 80000.0
    doping_conditions.dose = 5e15
    
    doping_result = doping_model.simulate_implantation(wafer, doping_conditions)
    print(f"  Peak concentration: {doping_result.peak_concentration:.2e} cm⁻³")
    
    # Step 3: Polysilicon deposition
    print("Step 3: Polysilicon deposition...")
    dep_model = deposition.PyDepositionModel()
    dep_conditions = deposition.PyDepositionConditions()
    dep_conditions.material = deposition.MATERIAL_POLYSILICON
    dep_conditions.temperature = 650.0
    dep_conditions.time = 1800.0
    
    dep_result = dep_model.simulate_deposition(wafer, dep_conditions)
    print(f"  Film thickness: {dep_result.film_thickness:.2f} nm")
    
    # Step 4: Metrology verification
    print("Step 4: Metrology verification...")
    metro = metrology.PyMetrologyModel()
    metro_params = metrology.PyMetrologyParameters(
        metrology.TECHNIQUE_ELLIPSOMETRY,
        metrology.MEASUREMENT_THICKNESS
    )
    
    metro_result = metro.perform_measurement(wafer, metro_params)
    print(f"  Measured thickness: {metro_result.measured_value:.2f} ± {metro_result.uncertainty:.2f} nm")
    
    print("Process flow completed successfully!")
    return wafer

# Run the complete process flow
final_wafer = complete_process_flow()
```

## 8. Data Visualization and Analysis

```python
import matplotlib.pyplot as plt
import numpy as np

def plot_doping_profile():
    """Plot ion implantation concentration profile"""
    
    wafer = geometry.PyWafer(100.0, 50, 50)
    doping_model = doping.PyMonteCarloSolver()
    
    conditions = doping.PyImplantationConditions()
    conditions.species = doping.ION_PHOSPHORUS_31
    conditions.energy = 50000.0
    conditions.dose = 1e15
    
    result = doping_model.simulate_implantation(wafer, conditions)
    profile = result.get_concentration_profile()
    
    depths = [point[0] for point in profile]
    concentrations = [point[1] for point in profile]
    
    plt.figure(figsize=(10, 6))
    plt.semilogy(depths, concentrations, 'b-', linewidth=2)
    plt.xlabel('Depth (nm)')
    plt.ylabel('Concentration (cm⁻³)')
    plt.title('Ion Implantation Concentration Profile')
    plt.grid(True, alpha=0.3)
    plt.show()

def plot_deposition_uniformity():
    """Plot deposition thickness uniformity across wafer"""
    
    wafer = geometry.PyWafer(100.0, 50, 50)
    dep_model = deposition.PyDepositionModel()
    
    conditions = deposition.PyDepositionConditions()
    conditions.material = deposition.MATERIAL_POLYSILICON
    conditions.temperature = 650.0
    conditions.time = 1800.0
    
    result = dep_model.simulate_deposition(wafer, conditions)
    thickness_map = result.get_thickness_map()
    
    # Convert to numpy array for plotting
    thickness_array = np.array(thickness_map).reshape(50, 50)
    
    plt.figure(figsize=(8, 8))
    plt.imshow(thickness_array, cmap='viridis', origin='lower')
    plt.colorbar(label='Thickness (nm)')
    plt.title('Deposition Thickness Uniformity')
    plt.xlabel('X Position')
    plt.ylabel('Y Position')
    plt.show()

# Generate plots
plot_doping_profile()
plot_deposition_uniformity()
```

## 9. Error Handling and Debugging

```python
import traceback

def robust_simulation():
    """Demonstrate proper error handling in Cython simulations"""
    
    try:
        # Create wafer with validation
        wafer = geometry.PyWafer(100.0, 50, 50)
        if not wafer:
            raise RuntimeError("Failed to create wafer")
        
        # Oxidation with parameter validation
        ox_model = oxidation.PyOxidationModel()
        conditions = oxidation.PyOxidationConditions()
        
        # Validate temperature range
        temperature = 1100.0
        if temperature < 500.0 or temperature > 1200.0:
            raise ValueError(f"Temperature {temperature}°C out of valid range (500-1200°C)")
        
        conditions.temperature = temperature
        conditions.time = 3600.0
        conditions.oxidation_type = oxidation.OXIDATION_DRY
        
        result = ox_model.simulate_oxidation(wafer, conditions)
        
        # Validate results
        if result.oxide_thickness <= 0:
            raise RuntimeError("Invalid oxidation result: negative thickness")
        
        print(f"Simulation successful: {result.oxide_thickness:.2f} nm oxide")
        
    except Exception as e:
        print(f"Simulation error: {e}")
        traceback.print_exc()

# Run robust simulation
robust_simulation()
```

## 10. Performance Optimization

```python
import time
import multiprocessing

def performance_comparison():
    """Compare performance of different simulation approaches"""
    
    # Small wafer for quick testing
    small_wafer = geometry.PyWafer(50.0, 25, 25)
    
    # Large wafer for performance testing
    large_wafer = geometry.PyWafer(200.0, 200, 200)
    
    ox_model = oxidation.PyOxidationModel()
    conditions = oxidation.PyOxidationConditions()
    conditions.temperature = 1000.0
    conditions.time = 3600.0
    conditions.oxidation_type = oxidation.OXIDATION_DRY
    
    # Test small wafer
    start_time = time.time()
    result_small = ox_model.simulate_oxidation(small_wafer, conditions)
    small_time = time.time() - start_time
    
    # Test large wafer
    start_time = time.time()
    result_large = ox_model.simulate_oxidation(large_wafer, conditions)
    large_time = time.time() - start_time
    
    print(f"Small wafer (25x25): {small_time:.3f} seconds")
    print(f"Large wafer (200x200): {large_time:.3f} seconds")
    print(f"Performance ratio: {large_time/small_time:.1f}x")
    print(f"Grid size ratio: {(200*200)/(25*25):.1f}x")

# Run performance comparison
performance_comparison()
```

## Building Custom Extensions

To create your own Cython extensions:

1. Create a `.pyx` file with your wrapper code
2. Add it to `setup.py`
3. Build with `python setup.py build_ext --inplace`

Example custom extension:

```python
# custom_module.pyx
from libcpp.memory cimport shared_ptr
from geometry cimport PyWafer, Wafer

cdef extern from "custom_model.hpp":
    cdef cppclass CustomModel:
        CustomModel() except +
        double customFunction(shared_ptr[Wafer] wafer, double parameter) except +

cdef class PyCustomModel:
    cdef CustomModel* thisptr
    
    def __cinit__(self):
        self.thisptr = new CustomModel()
    
    def __dealloc__(self):
        del self.thisptr
    
    def custom_function(self, PyWafer wafer, double parameter):
        return self.thisptr.customFunction(wafer.thisptr, parameter)
```

## Next Steps

1. Explore the Python frontend tutorial for GUI development
2. Learn about advanced visualization techniques
3. Study the complete application architecture
4. Implement custom process modules

For more information, refer to the Cython documentation and SemiPRO API reference.
