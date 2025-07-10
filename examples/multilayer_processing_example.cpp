#include "../src/cpp/advanced/multi_layer_engine.hpp"
#include "../src/cpp/core/wafer_enhanced.hpp"
#include "../src/cpp/core/advanced_logger.hpp"
#include "../src/cpp/core/memory_manager.hpp"
#include "../src/cpp/core/config_manager.hpp"
#include <iostream>
#include <memory>

using namespace SemiPRO;

int main() {
    try {
        std::cout << "🏭 SemiPRO Multi-Layer Processing Example" << std::endl;
        std::cout << "=========================================" << std::endl;
        
        // Initialize core systems
        auto& memory_manager = MemoryManager::getInstance();
        auto& config_manager = ConfigManager::getInstance();
        
        std::cout << "✅ Core systems initialized" << std::endl;
        
        // Create enhanced wafer
        auto wafer = std::make_shared<WaferEnhanced>(200.0, 0.5, "silicon");
        wafer->initializeGridParallel(100, 100);
        
        std::cout << "✅ Created 200mm silicon wafer" << std::endl;
        
        // Create multi-layer engine
        MultiLayerEngine engine;
        
        // Create a sophisticated layer stack
        std::string stack_id = "cmos_stack";
        engine.createStack(stack_id);
        
        std::cout << "✅ Created layer stack: " << stack_id << std::endl;
        
        // Define layers for a CMOS process
        
        // 1. Substrate (already exists)
        AdvancedLayer substrate;
        substrate.id = "substrate";
        substrate.type = LayerType::SUBSTRATE;
        substrate.material = "silicon";
        substrate.target_thickness = 500.0; // 500 μm
        substrate.properties["resistivity"] = 10.0; // Ω·cm
        substrate.properties["crystal_orientation"] = 100;
        engine.addLayerToStack(stack_id, substrate);
        
        // 2. Gate oxide
        AdvancedLayer gate_oxide;
        gate_oxide.id = "gate_oxide";
        gate_oxide.type = LayerType::OXIDE;
        gate_oxide.material = "SiO2";
        gate_oxide.target_thickness = 0.005; // 5 nm
        gate_oxide.properties["dielectric_constant"] = 3.9;
        engine.addLayerToStack(stack_id, gate_oxide);
        
        // 3. Polysilicon gate
        AdvancedLayer poly_gate;
        poly_gate.id = "poly_gate";
        poly_gate.type = LayerType::POLYSILICON;
        poly_gate.material = "poly_silicon";
        poly_gate.target_thickness = 0.3; // 300 nm
        poly_gate.properties["resistivity"] = 0.001; // Ω·cm (doped)
        engine.addLayerToStack(stack_id, poly_gate);
        
        // 4. Interlayer dielectric
        AdvancedLayer ild;
        ild.id = "ild";
        ild.type = LayerType::DIELECTRIC;
        ild.material = "SiO2";
        ild.target_thickness = 0.5; // 500 nm
        ild.properties["dielectric_constant"] = 3.9;
        engine.addLayerToStack(stack_id, ild);
        
        // 5. Metal contact
        AdvancedLayer metal1;
        metal1.id = "metal1";
        metal1.type = LayerType::METAL;
        metal1.material = "aluminum";
        metal1.target_thickness = 0.8; // 800 nm
        metal1.properties["resistivity"] = 2.7e-6; // Ω·cm
        engine.addLayerToStack(stack_id, metal1);
        
        std::cout << "✅ Added 5 layers to stack" << std::endl;
        
        // Create process sequence
        ProcessSequence sequence;
        sequence.sequence_id = "cmos_process";
        
        // Define process steps
        sequence.process_steps = {
            "oxidation:gate_oxide",
            "deposition:poly_gate", 
            "etching:poly_gate",
            "deposition:ild",
            "etching:ild",
            "deposition:metal1"
        };
        
        // Define parameters for each step
        sequence.step_parameters["oxidation:gate_oxide"] = {
            {"temperature", 1000.0},
            {"time", 0.5},
            {"ambient", 1.0} // dry oxygen
        };
        
        sequence.step_parameters["deposition:poly_gate"] = {
            {"temperature", 620.0},
            {"pressure", 0.1}
        };
        
        sequence.step_parameters["etching:poly_gate"] = {
            {"depth", 0.1},
            {"pressure", 10.0},
            {"power", 150.0}
        };
        
        sequence.step_parameters["deposition:ild"] = {
            {"temperature", 400.0},
            {"pressure", 1.0}
        };
        
        sequence.step_parameters["etching:ild"] = {
            {"depth", 0.2},
            {"pressure", 15.0},
            {"power", 200.0}
        };
        
        sequence.step_parameters["deposition:metal1"] = {
            {"temperature", 300.0},
            {"pressure", 5.0}
        };
        
        std::cout << "✅ Created process sequence with " << sequence.process_steps.size() << " steps" << std::endl;
        
        // Process the stack
        std::cout << "\n🔄 Processing layer stack..." << std::endl;
        bool success = engine.processStack(wafer, stack_id, sequence);
        
        if (success) {
            std::cout << "✅ Stack processing completed successfully!" << std::endl;
            
            // Analyze stack quality
            auto quality_metrics = engine.analyzeStackQuality(stack_id);
            
            std::cout << "\n📊 Stack Quality Analysis:" << std::endl;
            std::cout << "  Uniformity: " << quality_metrics["uniformity"] << "%" << std::endl;
            std::cout << "  Stress Score: " << quality_metrics["stress_score"] << std::endl;
            std::cout << "  Interface Quality: " << quality_metrics["interface_quality"] << std::endl;
            std::cout << "  Overall Score: " << quality_metrics["overall_score"] << std::endl;
            
            // Optimize the stack
            std::cout << "\n🎯 Optimizing stack for minimum stress..." << std::endl;
            
            std::unordered_map<std::string, double> constraints;
            constraints["max_total_thickness"] = 2.0; // 2 μm max
            constraints["min_uniformity"] = 95.0; // 95% min uniformity
            
            auto optimization_results = engine.optimizeStack(
                stack_id, 
                OptimizationObjective::MINIMIZE_STRESS, 
                constraints
            );
            
            std::cout << "✅ Optimization completed!" << std::endl;
            std::cout << "  Objective Value: " << optimization_results.objective_value << std::endl;
            std::cout << "  Recommendations:" << std::endl;
            for (const auto& rec : optimization_results.recommendations) {
                std::cout << "    - " << rec << std::endl;
            }
            
            // Get final stack information
            const auto& final_stack = engine.getStack(stack_id);
            std::cout << "\n📋 Final Stack Summary:" << std::endl;
            std::cout << "  Total Layers: " << final_stack.layers.size() << std::endl;
            std::cout << "  Total Thickness: " << final_stack.total_thickness << " μm" << std::endl;
            std::cout << "  Total Stress: " << final_stack.total_stress << " MPa" << std::endl;
            std::cout << "  Interfaces: " << final_stack.interfaces.size() << std::endl;
            
            std::cout << "\n🎉 Multi-layer processing example completed successfully!" << std::endl;
            
        } else {
            std::cout << "❌ Stack processing failed!" << std::endl;
            return 1;
        }
        
    } catch (const std::exception& e) {
        std::cout << "❌ Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
