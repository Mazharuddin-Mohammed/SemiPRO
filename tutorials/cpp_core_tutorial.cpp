/**
 * @file cpp_core_tutorial.cpp
 * @brief Tutorial demonstrating C++ core backend functionality
 * @author Dr. Mazharuddin Mohammed
 * 
 * This tutorial demonstrates the usage of the C++ core backend modules
 * including wafer management, simulation engine, and physics modules.
 */

#include <iostream>
#include <memory>
#include <vector>
#include <chrono>

// Core includes
#include "../src/cpp/core/wafer.hpp"
#include "../src/cpp/core/simulation_engine.hpp"
#include "../src/cpp/core/simulation_orchestrator.hpp"

// Physics modules
#include "../src/cpp/physics/enhanced_oxidation.hpp"
#include "../src/cpp/physics/enhanced_doping.hpp"
#include "../src/cpp/physics/enhanced_deposition.hpp"
#include "../src/cpp/physics/enhanced_etching.hpp"

// Process modules
#include "../src/cpp/modules/oxidation/oxidation_model.hpp"
#include "../src/cpp/modules/doping/doping_manager.hpp"
#include "../src/cpp/modules/deposition/deposition_model.hpp"
#include "../src/cpp/modules/etching/etching_model.hpp"

using namespace SemiPRO;

void test_wafer_creation() {
    std::cout << "\n=== Testing Wafer Creation ===" << std::endl;
    
    try {
        // Create a 300mm wafer
        auto wafer = std::make_shared<Wafer>(300.0, 775.0, "silicon");
        
        std::cout << "✅ Wafer created successfully" << std::endl;
        std::cout << "   Diameter: " << wafer->getDiameter() << " mm" << std::endl;
        std::cout << "   Thickness: " << wafer->getThickness() << " μm" << std::endl;
        std::cout << "   Material: " << wafer->getMaterialId() << std::endl;
        
        // Initialize grid
        wafer->initializeGrid(100, 100);
        std::cout << "✅ Grid initialized (100x100)" << std::endl;
        
        // Apply base layer
        wafer->applyLayer(10.0, "silicon_dioxide");
        std::cout << "✅ Base oxide layer applied (10 nm)" << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "❌ Wafer creation failed: " << e.what() << std::endl;
    }
}

void test_simulation_engine() {
    std::cout << "\n=== Testing Simulation Engine ===" << std::endl;
    
    try {
        auto& engine = SimulationEngine::getInstance();
        std::cout << "✅ Simulation engine instance obtained" << std::endl;
        
        // Create wafer
        auto wafer = std::make_shared<Wafer>(300.0, 775.0, "silicon");
        wafer->initializeGrid(50, 50);
        
        // Register wafer
        engine.registerWafer("test_wafer", wafer);
        std::cout << "✅ Wafer registered with engine" << std::endl;
        
        // Test oxidation process
        ProcessParameters oxidation_params;
        oxidation_params.operation = "oxidation";
        oxidation_params.duration = 3600.0; // 1 hour
        oxidation_params.priority = ProcessPriority::NORMAL;
        oxidation_params.parameters["temperature"] = 1000.0;
        oxidation_params.parameters["time"] = 1.0;
        oxidation_params.string_parameters["atmosphere"] = "dry";
        
        auto future = engine.simulateProcessAsync("test_wafer", oxidation_params);
        bool result = future.get();
        
        if (result) {
            std::cout << "✅ Oxidation simulation completed successfully" << std::endl;
        } else {
            std::cout << "❌ Oxidation simulation failed" << std::endl;
        }
        
        // Test doping process
        ProcessParameters doping_params;
        doping_params.operation = "doping";
        doping_params.duration = 1800.0; // 30 minutes
        doping_params.priority = ProcessPriority::HIGH;
        doping_params.parameters["energy"] = 50000.0; // 50 keV
        doping_params.parameters["dose"] = 1e15;      // cm^-2
        doping_params.string_parameters["species"] = "boron";
        
        auto doping_future = engine.simulateProcessAsync("test_wafer", doping_params);
        bool doping_result = doping_future.get();
        
        if (doping_result) {
            std::cout << "✅ Doping simulation completed successfully" << std::endl;
        } else {
            std::cout << "❌ Doping simulation failed" << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cout << "❌ Simulation engine test failed: " << e.what() << std::endl;
    }
}

void test_physics_modules() {
    std::cout << "\n=== Testing Physics Modules ===" << std::endl;
    
    try {
        // Test Enhanced Oxidation
        EnhancedOxidation oxidation_engine;
        std::cout << "✅ Enhanced oxidation engine created" << std::endl;
        
        // Test Enhanced Doping
        EnhancedDoping doping_engine;
        std::cout << "✅ Enhanced doping engine created" << std::endl;
        
        // Test Enhanced Deposition
        EnhancedDeposition deposition_engine;
        std::cout << "✅ Enhanced deposition engine created" << std::endl;
        
        // Test Enhanced Etching
        EnhancedEtching etching_engine;
        std::cout << "✅ Enhanced etching engine created" << std::endl;
        
        std::cout << "✅ All physics modules initialized successfully" << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "❌ Physics modules test failed: " << e.what() << std::endl;
    }
}

void test_process_modules() {
    std::cout << "\n=== Testing Process Modules ===" << std::endl;
    
    try {
        auto wafer = std::make_shared<Wafer>(300.0, 775.0, "silicon");
        wafer->initializeGrid(50, 50);
        
        // Test Oxidation Model
        OxidationModel oxidation_model;
        oxidation_model.simulateOxidation(wafer, 1000.0, 1.0);
        std::cout << "✅ Oxidation model simulation completed" << std::endl;
        
        // Test Doping Manager
        DopingManager doping_manager;
        doping_manager.simulateIonImplantation(wafer, 50000.0, 1e15);
        doping_manager.simulateDiffusion(wafer, 1000.0, 0.5);
        std::cout << "✅ Doping manager simulations completed" << std::endl;
        
        // Test Deposition Model
        DepositionModel deposition_model;
        deposition_model.simulateDeposition(wafer, 100.0, "silicon_nitride", "LPCVD");
        std::cout << "✅ Deposition model simulation completed" << std::endl;
        
        // Test Etching Model
        EtchingModel etching_model;
        etching_model.simulateEtching(wafer, 50.0, "anisotropic");
        std::cout << "✅ Etching model simulation completed" << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "❌ Process modules test failed: " << e.what() << std::endl;
    }
}

void test_simulation_orchestrator() {
    std::cout << "\n=== Testing Simulation Orchestrator ===" << std::endl;
    
    try {
        SimulationOrchestrator orchestrator;
        std::cout << "✅ Simulation orchestrator created" << std::endl;
        
        // Create a simple process flow
        ProcessFlow flow;
        flow.name = "Simple MOSFET Flow";
        flow.description = "Basic MOSFET fabrication steps";
        
        // Add oxidation step
        ProcessStepDefinition oxidation_step;
        oxidation_step.name = "Gate Oxidation";
        oxidation_step.type = StepType::OXIDATION;
        oxidation_step.estimated_duration = 3600.0;
        oxidation_step.priority = ProcessPriority::HIGH;
        oxidation_step.parameters["temperature"] = 1000.0;
        oxidation_step.parameters["time"] = 1.0;
        oxidation_step.parallel_compatible = false;
        
        // Add doping step
        ProcessStepDefinition doping_step;
        doping_step.name = "Source/Drain Implant";
        doping_step.type = StepType::DOPING;
        doping_step.estimated_duration = 1800.0;
        doping_step.priority = ProcessPriority::NORMAL;
        doping_step.parameters["energy"] = 80000.0;
        doping_step.parameters["dose"] = 1e16;
        doping_step.dependencies.push_back("Gate Oxidation");
        doping_step.parallel_compatible = false;
        
        flow.steps.push_back(oxidation_step);
        flow.steps.push_back(doping_step);
        
        // Load and execute flow
        orchestrator.loadProcessFlow(flow);
        std::cout << "✅ Process flow loaded" << std::endl;
        
        bool execution_result = orchestrator.executeFlow("test_wafer", ExecutionMode::SEQUENTIAL);
        
        if (execution_result) {
            std::cout << "✅ Process flow executed successfully" << std::endl;
        } else {
            std::cout << "❌ Process flow execution failed" << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cout << "❌ Simulation orchestrator test failed: " << e.what() << std::endl;
    }
}

void benchmark_performance() {
    std::cout << "\n=== Performance Benchmark ===" << std::endl;
    
    try {
        auto start_time = std::chrono::high_resolution_clock::now();
        
        // Create multiple wafers and run simulations
        const int num_wafers = 5;
        std::vector<std::shared_ptr<Wafer>> wafers;
        
        for (int i = 0; i < num_wafers; ++i) {
            auto wafer = std::make_shared<Wafer>(300.0, 775.0, "silicon");
            wafer->initializeGrid(100, 100);
            wafers.push_back(wafer);
        }
        
        auto creation_time = std::chrono::high_resolution_clock::now();
        auto creation_duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            creation_time - start_time).count();
        
        std::cout << "✅ Created " << num_wafers << " wafers in " << creation_duration << " ms" << std::endl;
        
        // Run simulations on all wafers
        auto& engine = SimulationEngine::getInstance();
        
        for (int i = 0; i < num_wafers; ++i) {
            std::string wafer_name = "benchmark_wafer_" + std::to_string(i);
            engine.registerWafer(wafer_name, wafers[i]);
            
            ProcessParameters params;
            params.operation = "oxidation";
            params.duration = 1800.0;
            params.priority = ProcessPriority::NORMAL;
            params.parameters["temperature"] = 1000.0;
            params.parameters["time"] = 0.5;
            
            auto future = engine.simulateProcessAsync(wafer_name, params);
            future.get(); // Wait for completion
        }
        
        auto end_time = std::chrono::high_resolution_clock::now();
        auto total_duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            end_time - start_time).count();
        
        std::cout << "✅ Completed " << num_wafers << " simulations in " << total_duration << " ms" << std::endl;
        std::cout << "   Average time per simulation: " << (total_duration / num_wafers) << " ms" << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "❌ Performance benchmark failed: " << e.what() << std::endl;
    }
}

int main() {
    std::cout << "SemiPRO C++ Core Backend Tutorial" << std::endl;
    std::cout << "==================================" << std::endl;
    
    // Run all tests
    test_wafer_creation();
    test_simulation_engine();
    test_physics_modules();
    test_process_modules();
    test_simulation_orchestrator();
    benchmark_performance();
    
    std::cout << "\n=== Tutorial Complete ===" << std::endl;
    std::cout << "All C++ core backend modules have been tested." << std::endl;
    
    return 0;
}
