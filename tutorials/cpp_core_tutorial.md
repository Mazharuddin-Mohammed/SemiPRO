# SemiPRO C++ Core Tutorial

This tutorial demonstrates how to use the SemiPRO C++ core modules for semiconductor process simulation.

## Prerequisites

- C++17 compatible compiler
- CMake 3.15 or higher
- Eigen3 library
- OpenMP (optional, for parallel processing)

## Building the Core Modules

```bash
# From the project root
mkdir build
cd build
cmake ..
make -j$(nproc)
```

## 1. Basic Wafer Creation and Geometry

```cpp
#include "core/wafer.hpp"
#include "modules/geometry/geometry_model.hpp"

int main() {
    // Create a 100mm wafer with 50x50 grid
    auto wafer = std::make_shared<Wafer>(100.0, 50, 50);
    
    // Initialize geometry model
    GeometryModel geometry;
    
    // Create a simple rectangular structure
    geometry.addRectangle(wafer, 10.0, 10.0, 80.0, 80.0, 1.0);
    
    // Add a circular via
    geometry.addCircle(wafer, 50.0, 50.0, 5.0, 2.0);
    
    std::cout << "Wafer created with " << wafer->getGridSizeX() 
              << "x" << wafer->getGridSizeY() << " grid" << std::endl;
    
    return 0;
}
```

## 2. Oxidation Process Simulation

```cpp
#include "modules/oxidation/oxidation_model.hpp"
#include "physics/enhanced_oxidation.hpp"

void oxidation_example() {
    auto wafer = std::make_shared<Wafer>(100.0, 50, 50);
    
    // Create oxidation model
    OxidationModel oxidation;
    
    // Set up oxidation conditions
    OxidationConditions conditions;
    conditions.temperature = 1100.0;  // °C
    conditions.time = 3600.0;         // seconds
    conditions.pressure = 1.0;        // atm
    conditions.oxygen_partial_pressure = 0.21;
    conditions.water_vapor_pressure = 0.0;
    conditions.oxidation_type = OxidationType::DRY_OXIDATION;
    
    // Run simulation
    OxidationResult result = oxidation.simulateOxidation(wafer, conditions);
    
    std::cout << "Oxide thickness: " << result.oxide_thickness << " nm" << std::endl;
    std::cout << "Growth rate: " << result.growth_rate << " nm/min" << std::endl;
    std::cout << "Stress: " << result.stress << " MPa" << std::endl;
}
```

## 3. Ion Implantation and Doping

```cpp
#include "modules/doping/monte_carlo_solver.hpp"
#include "physics/enhanced_doping.hpp"

void doping_example() {
    auto wafer = std::make_shared<Wafer>(100.0, 50, 50);
    
    // Create Monte Carlo solver
    MonteCarloSolver mc_solver;
    
    // Set up implantation conditions
    ImplantationConditions conditions;
    conditions.species = IonSpecies::PHOSPHORUS_31;
    conditions.energy = 50000.0;     // eV
    conditions.dose = 1e15;          // ions/cm²
    conditions.angle = 7.0;          // degrees
    conditions.temperature = 25.0;   // °C
    
    // Run Monte Carlo simulation
    DopingResult result = mc_solver.simulateImplantation(wafer, conditions);
    
    std::cout << "Peak concentration: " << result.peak_concentration << " cm⁻³" << std::endl;
    std::cout << "Junction depth: " << result.junction_depth << " nm" << std::endl;
    std::cout << "Sheet resistance: " << result.sheet_resistance << " Ω/sq" << std::endl;
}
```

## 4. Deposition Process

```cpp
#include "modules/deposition/deposition_model.hpp"
#include "physics/enhanced_deposition.hpp"

void deposition_example() {
    auto wafer = std::make_shared<Wafer>(100.0, 50, 50);
    
    // Create deposition model
    DepositionModel deposition;
    
    // Set up deposition conditions
    DepositionConditions conditions;
    conditions.material = MaterialType::POLYSILICON;
    conditions.temperature = 650.0;   // °C
    conditions.pressure = 0.1;        // Torr
    conditions.time = 1800.0;         // seconds
    conditions.gas_flow_rate = 100.0; // sccm
    conditions.deposition_type = DepositionType::CVD;
    
    // Run simulation
    DepositionResult result = deposition.simulateDeposition(wafer, conditions);
    
    std::cout << "Film thickness: " << result.film_thickness << " nm" << std::endl;
    std::cout << "Uniformity: " << result.uniformity << "%" << std::endl;
    std::cout << "Deposition rate: " << result.deposition_rate << " nm/min" << std::endl;
}
```

## 5. Etching Process

```cpp
#include "modules/etching/etching_model.hpp"
#include "physics/enhanced_etching.hpp"

void etching_example() {
    auto wafer = std::make_shared<Wafer>(100.0, 50, 50);
    
    // First deposit a layer to etch
    // ... (deposition code here)
    
    // Create etching model
    EtchingModel etching;
    
    // Set up etching conditions
    EtchingConditions conditions;
    conditions.etchant_type = EtchantType::SF6_O2;
    conditions.temperature = 25.0;    // °C
    conditions.pressure = 0.01;       // Torr
    conditions.time = 300.0;          // seconds
    conditions.rf_power = 100.0;      // W
    conditions.bias_voltage = 50.0;   // V
    conditions.etching_type = EtchingType::RIE;
    
    // Run simulation
    EtchingResult result = etching.simulateEtching(wafer, conditions);
    
    std::cout << "Etch depth: " << result.etch_depth << " nm" << std::endl;
    std::cout << "Etch rate: " << result.etch_rate << " nm/min" << std::endl;
    std::cout << "Selectivity: " << result.selectivity << std::endl;
    std::cout << "Anisotropy: " << result.anisotropy << std::endl;
}
```

## 6. Advanced Process Integration

```cpp
#include "core/simulation_engine.hpp"

void complete_process_flow() {
    auto wafer = std::make_shared<Wafer>(100.0, 100, 100);
    SimulationEngine engine;
    
    // Step 1: Initial oxidation
    ProcessParameters oxidation_params;
    oxidation_params.parameters["temperature"] = 1000.0;
    oxidation_params.parameters["time"] = 7200.0;
    oxidation_params.parameters["pressure"] = 1.0;
    oxidation_params.string_parameters["type"] = "dry";
    
    engine.runProcess(wafer, "oxidation", oxidation_params);
    
    // Step 2: Ion implantation
    ProcessParameters doping_params;
    doping_params.parameters["energy"] = 80000.0;
    doping_params.parameters["dose"] = 5e15;
    doping_params.parameters["angle"] = 7.0;
    doping_params.string_parameters["species"] = "phosphorus";
    
    engine.runProcess(wafer, "doping", doping_params);
    
    // Step 3: Annealing
    ProcessParameters annealing_params;
    annealing_params.parameters["temperature"] = 1000.0;
    annealing_params.parameters["time"] = 1800.0;
    annealing_params.string_parameters["atmosphere"] = "nitrogen";
    
    engine.runProcess(wafer, "thermal", annealing_params);
    
    // Step 4: Polysilicon deposition
    ProcessParameters poly_params;
    poly_params.parameters["temperature"] = 650.0;
    poly_params.parameters["time"] = 1800.0;
    poly_params.parameters["pressure"] = 0.1;
    poly_params.string_parameters["material"] = "polysilicon";
    
    engine.runProcess(wafer, "deposition", poly_params);
    
    std::cout << "Complete process flow executed successfully!" << std::endl;
}
```

## 7. Advanced Features

### Defect Inspection

```cpp
#include "modules/defect_inspection/defect_inspection_model.hpp"

void defect_inspection_example() {
    auto wafer = std::make_shared<Wafer>(100.0, 50, 50);
    
    DefectInspectionModel inspector;
    
    // Configure inspection parameters
    inspector.setInspectionParameters(InspectionMethod::SEM_INSPECTION, 
                                    0.01, 10.0, 0.95);
    inspector.enableDefectClassification(true);
    inspector.setDefectSizeThreshold(0.1);
    
    // Perform inspection
    InspectionResult result = inspector.performInspection(wafer, 
                                                        InspectionMethod::SEM_INSPECTION);
    
    std::cout << "Found " << result.defects.size() << " defects" << std::endl;
    std::cout << "Coverage area: " << result.coverage_area << " cm²" << std::endl;
    std::cout << "False positive rate: " << result.false_positive_rate << std::endl;
}
```

### Interconnect Simulation

```cpp
#include "modules/interconnect/damascene_model.hpp"

void damascene_example() {
    auto wafer = std::make_shared<Wafer>(100.0, 50, 50);
    
    DamasceneModel damascene;
    
    // Set up damascene parameters
    DamasceneParameters params;
    params.type = DamasceneType::DUAL_DAMASCENE;
    params.line_width = 100.0;        // nm
    params.line_spacing = 100.0;      // nm
    params.metal_thickness = 200.0;   // nm
    params.barrier_thickness = 5.0;   // nm
    params.dielectric_thickness = 300.0; // nm
    params.barrier_material = BarrierMaterial::TANTALUM_NITRIDE;
    params.metal_material = MetalMaterial::COPPER;
    params.cmp_enabled = true;
    params.cmp_pressure = 5.0;        // psi
    params.cmp_time = 60.0;           // s
    
    // Run damascene simulation
    DamasceneResult result = damascene.simulateDamascene(wafer, params);
    
    std::cout << "Final thickness: " << result.final_thickness << " nm" << std::endl;
    std::cout << "Resistance: " << result.resistance << " Ω" << std::endl;
    std::cout << "RC delay: " << result.rc_delay << " s" << std::endl;
    std::cout << "Barrier coverage: " << result.barrier_coverage << "%" << std::endl;
}
```

### Metrology Measurements

```cpp
#include "modules/metrology/metrology_model.hpp"

void metrology_example() {
    auto wafer = std::make_shared<Wafer>(100.0, 50, 50);
    
    MetrologyModel metrology;
    
    // Configure measurement parameters
    metrology.setMeasurementParameters(MetrologyTechnique::ELLIPSOMETRY, 
                                     0.001, 0.99);
    metrology.enableNoiseSimulation(true);
    metrology.setCalibrationFactor(1.0);
    
    // Perform thickness measurement
    MetrologyResult thickness_result = metrology.measureThickness(wafer, 
                                                                MetrologyTechnique::ELLIPSOMETRY);
    
    std::cout << "Measured thickness: " << thickness_result.measured_value 
              << " ± " << thickness_result.uncertainty << " " 
              << thickness_result.units << std::endl;
    std::cout << "Signal-to-noise ratio: " << thickness_result.signal_to_noise << std::endl;
    
    // Perform composition analysis
    MetrologyResult composition_result = metrology.measureComposition(wafer, 
                                                                    MetrologyTechnique::XRF);
    
    std::cout << "Composition measurement: " << composition_result.measured_value 
              << " " << composition_result.units << std::endl;
}
```

## 8. Error Handling and Best Practices

```cpp
#include <stdexcept>
#include <iostream>

void robust_simulation_example() {
    try {
        auto wafer = std::make_shared<Wafer>(100.0, 50, 50);
        
        // Validate wafer before processing
        if (!wafer || wafer->getGridSizeX() == 0 || wafer->getGridSizeY() == 0) {
            throw std::runtime_error("Invalid wafer configuration");
        }
        
        OxidationModel oxidation;
        OxidationConditions conditions;
        
        // Validate process conditions
        if (conditions.temperature < 500.0 || conditions.temperature > 1200.0) {
            throw std::runtime_error("Temperature out of valid range");
        }
        
        conditions.temperature = 1000.0;
        conditions.time = 3600.0;
        conditions.pressure = 1.0;
        conditions.oxidation_type = OxidationType::DRY_OXIDATION;
        
        // Run simulation with error checking
        OxidationResult result = oxidation.simulateOxidation(wafer, conditions);
        
        // Validate results
        if (result.oxide_thickness <= 0.0) {
            throw std::runtime_error("Invalid oxidation result");
        }
        
        std::cout << "Simulation completed successfully" << std::endl;
        std::cout << "Oxide thickness: " << result.oxide_thickness << " nm" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Simulation error: " << e.what() << std::endl;
        return;
    }
}
```

## 9. Performance Optimization

```cpp
#include <omp.h>
#include <chrono>

void performance_example() {
    auto start = std::chrono::high_resolution_clock::now();
    
    // Enable OpenMP for parallel processing
    omp_set_num_threads(std::thread::hardware_concurrency());
    
    auto wafer = std::make_shared<Wafer>(200.0, 200, 200);  // Large wafer
    
    OxidationModel oxidation;
    OxidationConditions conditions;
    conditions.temperature = 1000.0;
    conditions.time = 3600.0;
    conditions.pressure = 1.0;
    conditions.oxidation_type = OxidationType::DRY_OXIDATION;
    
    // Run simulation
    OxidationResult result = oxidation.simulateOxidation(wafer, conditions);
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Simulation completed in " << duration.count() << " ms" << std::endl;
    std::cout << "Grid size: " << wafer->getGridSizeX() << "x" << wafer->getGridSizeY() << std::endl;
}
```

## Compilation Instructions

To compile these examples:

```bash
# Single file compilation
g++ -std=c++17 -O3 -fopenmp -I../src/cpp example.cpp -L../build -lsemipro_core -leigen3

# Using CMake (recommended)
# Create CMakeLists.txt:
cmake_minimum_required(VERSION 3.15)
project(SemiPROTutorial)

set(CMAKE_CXX_STANDARD 17)
find_package(Eigen3 REQUIRED)
find_package(OpenMP)

add_executable(tutorial tutorial.cpp)
target_link_libraries(tutorial semipro_core Eigen3::Eigen)

if(OpenMP_CXX_FOUND)
    target_link_libraries(tutorial OpenMP::OpenMP_CXX)
endif()
```

## Next Steps

1. Explore the Cython integration tutorial for Python bindings
2. Learn about the GUI components in the Python frontend tutorial
3. Study advanced visualization techniques
4. Implement custom process modules

For more detailed information, refer to the API documentation and source code comments.
