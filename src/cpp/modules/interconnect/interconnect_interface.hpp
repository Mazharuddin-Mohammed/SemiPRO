#ifndef INTERCONNECT_INTERFACE_HPP
#define INTERCONNECT_INTERFACE_HPP

#include "../../core/wafer.hpp"
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>

class InterconnectInterface {
public:
    virtual ~InterconnectInterface() = default;
    
    // Interconnect structure types
    enum InterconnectType {
        SINGLE_DAMASCENE,
        DUAL_DAMASCENE,
        THROUGH_SILICON_VIA,
        REDISTRIBUTION_LAYER,
        BUMP,
        PILLAR
    };
    
    // Metal levels and hierarchy
    struct MetalLevel {
        int level;
        std::string metal_type;
        double line_width;
        double line_spacing;
        double thickness;
        double via_size;
        std::string dielectric_material;
        
        MetalLevel(int lvl, const std::string& metal, double width, double spacing, double thick)
            : level(lvl), metal_type(metal), line_width(width), line_spacing(spacing), 
              thickness(thick), via_size(width * 0.8), dielectric_material("SiO2") {}
    };
    
    // Interconnect structure definition
    struct InterconnectStructure {
        InterconnectType type;
        std::vector<MetalLevel> metal_levels;
        std::vector<std::pair<double, double>> via_positions;
        std::unordered_map<std::string, double> electrical_properties;
        
        InterconnectStructure(InterconnectType t) : type(t) {}
    };
    
    // Core interconnect functions
    virtual void createInterconnectStack(
        std::shared_ptr<Wafer> wafer,
        const std::vector<MetalLevel>& metal_levels
    ) = 0;
    
    virtual void simulateDamasceneProcess(
        std::shared_ptr<Wafer> wafer,
        const MetalLevel& level,
        InterconnectType type
    ) = 0;
    
    virtual void createVias(
        std::shared_ptr<Wafer> wafer,
        const std::vector<std::pair<double, double>>& via_positions,
        double via_diameter,
        int from_level,
        int to_level
    ) = 0;
    
    virtual void simulateCMP(
        std::shared_ptr<Wafer> wafer,
        const std::string& target_material,
        double target_thickness
    ) = 0;
    
    // Electrical characterization
    virtual std::unordered_map<std::string, double> calculateElectricalProperties(
        std::shared_ptr<Wafer> wafer,
        const InterconnectStructure& structure
    ) = 0;
    
    virtual double calculateResistance(
        const MetalLevel& level,
        double length
    ) = 0;
    
    virtual double calculateCapacitance(
        const MetalLevel& level1,
        const MetalLevel& level2,
        double overlap_area
    ) = 0;
    
    // Reliability analysis
    virtual double calculateElectromigrationMTTF(
        const MetalLevel& level,
        double current_density,
        double temperature
    ) = 0;
    
    virtual std::vector<double> analyzeStressMigration(
        std::shared_ptr<Wafer> wafer,
        const InterconnectStructure& structure
    ) = 0;
};

#endif // INTERCONNECT_INTERFACE_HPP
