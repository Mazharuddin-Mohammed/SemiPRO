// Author: Dr. Mazharuddin Mohammed
#ifndef MULTI_DIE_INTERFACE_HPP
#define MULTI_DIE_INTERFACE_HPP

#include "../../core/wafer.hpp"
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>

class MultiDieInterface {
public:
    virtual ~MultiDieInterface() = default;
    
    // Die types and configurations
    enum DieType {
        LOGIC_DIE,
        MEMORY_DIE,
        ANALOG_DIE,
        RF_DIE,
        SENSOR_DIE,
        POWER_DIE,
        PROCESSOR_DIE,
        CUSTOM_DIE
    };
    
    enum IntegrationMethod {
        WIRE_BONDING,
        FLIP_CHIP,
        THROUGH_SILICON_VIA,
        WAFER_LEVEL_PACKAGING,
        CHIPLET_INTEGRATION,
        HETEROGENEOUS_INTEGRATION
    };
    
    // Die structure definition
    struct Die {
        DieType type;
        std::string name;
        std::string id;  // Unique identifier
        double width, height, thickness; // μm
        std::pair<double, double> position; // x, y coordinates
        std::vector<std::pair<double, double>> io_positions;
        std::unordered_map<std::string, double> electrical_properties;
        std::unordered_map<std::string, double> thermal_properties;
        std::string technology_node;
        double power_consumption; // Watts

        Die(DieType t, const std::string& n, double w, double h, double thick)
            : type(t), name(n), id(n), width(w), height(h), thickness(thick),
              position({0.0, 0.0}), power_consumption(0.0) {}
    };
    
    // Multi-die system structure
    struct MultiDieSystem {
        std::vector<Die> dies;
        std::vector<std::pair<int, int>> connections; // Die index pairs
        IntegrationMethod integration_method;
        std::string substrate_material;
        double substrate_thickness;
        std::unordered_map<std::string, double> system_properties;
        
        MultiDieSystem(IntegrationMethod method) : integration_method(method) {}
    };
    
    // Core multi-die functions
    virtual void createMultiDieSystem(
        std::shared_ptr<Wafer> wafer,
        const MultiDieSystem& system
    ) = 0;
    
    virtual void placeDie(
        std::shared_ptr<Wafer> wafer,
        const Die& die,
        double x, double y,
        double rotation = 0.0
    ) = 0;
    
    virtual void createInterDieConnections(
        std::shared_ptr<Wafer> wafer,
        const std::vector<std::pair<int, int>>& connections,
        IntegrationMethod method
    ) = 0;
    
    virtual void simulateAssemblyProcess(
        std::shared_ptr<Wafer> wafer,
        const MultiDieSystem& system
    ) = 0;
    
    // Electrical analysis
    virtual std::unordered_map<std::string, double> analyzeElectricalPerformance(
        const MultiDieSystem& system
    ) = 0;
    
    virtual double calculateInterDieDelay(
        const Die& die1,
        const Die& die2,
        IntegrationMethod method
    ) = 0;
    
    virtual double calculatePowerConsumption(
        const MultiDieSystem& system
    ) = 0;
    
    // Thermal analysis
    virtual std::vector<std::vector<double>> simulateThermalDistribution(
        const MultiDieSystem& system,
        const std::unordered_map<int, double>& power_map
    ) = 0;
    
    virtual double calculateThermalResistance(
        const Die& die1,
        const Die& die2,
        IntegrationMethod method
    ) = 0;
    
    // Mechanical analysis
    virtual std::vector<double> analyzeStress(
        const MultiDieSystem& system,
        double temperature_change
    ) = 0;
    
    virtual double calculateReliability(
        const MultiDieSystem& system,
        double operating_temperature,
        double operating_time
    ) = 0;
    
    // Yield and testing
    virtual double calculateSystemYield(
        const MultiDieSystem& system,
        const std::vector<double>& die_yields
    ) = 0;
    
    virtual std::vector<std::string> generateTestPlan(
        const MultiDieSystem& system
    ) = 0;
    
    // Design optimization
    virtual MultiDieSystem optimizeLayout(
        const std::vector<Die>& dies,
        const std::unordered_map<std::string, double>& constraints
    ) = 0;
    
    virtual std::vector<std::pair<double, double>> optimizeDiePlacement(
        const std::vector<Die>& dies,
        double substrate_area
    ) = 0;
};

#endif // MULTI_DIE_INTERFACE_HPP
