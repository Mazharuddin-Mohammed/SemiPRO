// Author: Dr. Mazharuddin Mohammed
// Working doping example with realistic values and proper visualization

#include "../../src/cpp/core/wafer.hpp"
#include "../../src/cpp/modules/geometry/geometry_manager.hpp"
#include "../../src/cpp/modules/doping/doping_manager.hpp"
#include <memory>
#include <iostream>
#include <fstream>
#include <iomanip>

int main() {
    std::cout << "🔬 SemiPRO Doping Simulation - Working Example\n";
    std::cout << "=" << std::string(50, '=') << "\n\n";
    
    // Create wafer with realistic dimensions
    auto wafer = std::make_shared<Wafer>(300.0, 775.0, "silicon");
    std::cout << "✅ Created silicon wafer: " << wafer->getDiameter() << "mm diameter, " 
              << wafer->getThickness() << "μm thickness\n";
    
    // Initialize geometry with high resolution
    GeometryManager geometry;
    geometry.initializeGrid(wafer, 200, 200);
    std::cout << "✅ Initialized 200x200 simulation grid\n";
    
    // Create doping manager
    DopingManager doping;
    std::cout << "✅ Created doping manager\n";
    
    // Perform ion implantation with realistic parameters
    std::cout << "\n📋 Step 1: Ion Implantation\n";
    std::cout << "Parameters: Boron, 50 keV, 1e16 cm⁻²\n";
    
    doping.simulateIonImplantation(wafer, 50.0, 1e16);  // Realistic dose
    
    auto profile_after_implant = wafer->getDopantProfile();
    double max_conc_implant = profile_after_implant.maxCoeff();
    std::cout << "✅ Ion implantation completed\n";
    std::cout << "   Max concentration after implant: " << std::scientific 
              << max_conc_implant << " cm⁻³\n";
    
    // Perform thermal diffusion
    std::cout << "\n📋 Step 2: Thermal Diffusion\n";
    std::cout << "Parameters: 1000°C, 1 hour\n";
    
    doping.simulateDiffusion(wafer, 1000.0, 3600.0);  // 1000°C for 1 hour
    
    auto profile_after_diffusion = wafer->getDopantProfile();
    double max_conc_diffusion = profile_after_diffusion.maxCoeff();
    std::cout << "✅ Thermal diffusion completed\n";
    std::cout << "   Max concentration after diffusion: " << std::scientific 
              << max_conc_diffusion << " cm⁻³\n";
    
    // Generate detailed output
    std::cout << "\n📊 Detailed Results:\n";
    std::cout << "Profile size: " << profile_after_diffusion.size() << " points\n";
    std::cout << "Depth resolution: " << wafer->getThickness() / profile_after_diffusion.size() << " μm/point\n";
    
    // Find peak location
    int peak_index = 0;
    profile_after_diffusion.maxCoeff(&peak_index);
    double peak_depth = (peak_index * wafer->getThickness()) / profile_after_diffusion.size();
    std::cout << "Peak concentration at depth: " << peak_depth << " μm\n";
    
    // Save profile to file for visualization
    std::ofstream profile_file("dopant_profile.dat");
    if (profile_file.is_open()) {
        profile_file << "# Depth (μm)\tConcentration (cm⁻³)\n";
        for (int i = 0; i < profile_after_diffusion.size(); ++i) {
            double depth = (i * wafer->getThickness()) / profile_after_diffusion.size();
            profile_file << std::fixed << std::setprecision(3) << depth << "\t" 
                        << std::scientific << std::setprecision(6) << profile_after_diffusion[i] << "\n";
        }
        profile_file.close();
        std::cout << "✅ Dopant profile saved to 'dopant_profile.dat'\n";
    }
    
    // Calculate some statistics
    double total_dose = profile_after_diffusion.sum() * (wafer->getThickness() / profile_after_diffusion.size());
    double avg_concentration = profile_after_diffusion.mean();
    
    std::cout << "\n📈 Statistics:\n";
    std::cout << "   Total integrated dose: " << std::scientific << total_dose << " cm⁻²\n";
    std::cout << "   Average concentration: " << std::scientific << avg_concentration << " cm⁻³\n";
    std::cout << "   Peak/Average ratio: " << std::fixed << std::setprecision(2) 
              << max_conc_diffusion / avg_concentration << "\n";
    
    // Verify physics
    std::cout << "\n🔬 Physics Verification:\n";
    if (max_conc_diffusion > 1e14 && max_conc_diffusion < 1e20) {
        std::cout << "✅ Concentration in realistic range for semiconductor doping\n";
    } else {
        std::cout << "⚠️  Concentration outside typical range\n";
    }
    
    if (peak_depth < 1.0) {  // Peak should be near surface for 50 keV
        std::cout << "✅ Peak depth realistic for 50 keV implantation\n";
    } else {
        std::cout << "⚠️  Peak depth seems too deep for 50 keV\n";
    }
    
    std::cout << "\n🎉 Doping simulation completed successfully!\n";
    std::cout << "📁 Results saved to 'dopant_profile.dat' for plotting\n";
    
    return 0;
}
