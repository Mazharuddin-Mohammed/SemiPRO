// Author: Dr. Mazharuddin Mohammed
#ifndef MULTI_DIE_MODEL_HPP
#define MULTI_DIE_MODEL_HPP

#include "multi_die_interface.hpp"
#include "../../core/wafer.hpp"
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>

enum class DieType {
    LOGIC,
    MEMORY,
    ANALOG,
    RF,
    SENSOR,
    POWER
};

enum class IntegrationMethod {
    WIRE_BONDING,
    FLIP_CHIP,
    TSV,
    WLP,
    CHIPLET
};

struct Die {
    std::string id;
    DieType type;
    double width;
    double height;
    double thickness;
    std::pair<double, double> position;
    std::string material;
    std::vector<std::pair<std::string, double>> electrical_properties;
    double thermal_conductivity;
    double power_consumption;
    
    Die(const std::string& die_id, DieType die_type, double w, double h, double t)
        : id(die_id), type(die_type), width(w), height(h), thickness(t),
          position({0.0, 0.0}), material("silicon"), thermal_conductivity(150.0),
          power_consumption(0.0) {}
};

struct Interconnect {
    std::string from_die;
    std::string to_die;
    std::string signal_type;
    double resistance;
    double capacitance;
    double inductance;
    double delay;
    
    Interconnect(const std::string& from, const std::string& to, const std::string& type)
        : from_die(from), to_die(to), signal_type(type),
          resistance(0.0), capacitance(0.0), inductance(0.0), delay(0.0) {}
};

class MultiDieModel : public MultiDieInterface {
public:
    MultiDieModel();
    
    // Die management
    void addDie(const Die& die);
    void removeDie(const std::string& die_id);
    void positionDie(const std::string& die_id, double x, double y);
    
    // Integration methods
    void performWireBonding(std::shared_ptr<Wafer> wafer, 
                           const std::string& die1, const std::string& die2,
                           const std::vector<std::pair<std::pair<int, int>, std::pair<int, int>>>& bonds);
    
    void performFlipChipBonding(std::shared_ptr<Wafer> wafer,
                               const std::string& die1, const std::string& die2,
                               double bump_pitch, double bump_diameter);
    
    void performTSVIntegration(std::shared_ptr<Wafer> wafer,
                              const std::string& die_id,
                              const std::vector<std::pair<double, double>>& tsv_positions,
                              double tsv_diameter, double tsv_depth);
    
    void performWLPIntegration(std::shared_ptr<Wafer> wafer,
                              const std::vector<std::string>& die_ids,
                              double redistribution_thickness);
    
    void performChipletIntegration(std::shared_ptr<Wafer> wafer,
                                  const std::vector<std::string>& chiplet_ids,
                                  const std::string& interposer_material);
    
    // Analysis methods
    void analyzeElectricalPerformance(std::shared_ptr<Wafer> wafer);
    void analyzeThermalPerformance(std::shared_ptr<Wafer> wafer);
    void analyzeMechanicalStress(std::shared_ptr<Wafer> wafer);
    void analyzeSystemReliability(std::shared_ptr<Wafer> wafer);
    
    // Interconnect modeling
    void addInterconnect(const Interconnect& interconnect);
    void calculateInterconnectParameters();
    void optimizeInterconnectRouting();
    
    // System-level simulation
    void simulateSystemOperation(std::shared_ptr<Wafer> wafer, double simulation_time);
    void calculateSystemMTTF(std::shared_ptr<Wafer> wafer);
    
    // Getters
    const std::vector<Die>& getDies() const { return dies_; }
    const std::vector<Interconnect>& getInterconnects() const { return interconnects_; }
    const std::unordered_map<std::string, double>& getSystemMetrics() const { return system_metrics_; }

private:
    std::vector<Die> dies_;
    std::vector<Interconnect> interconnects_;
    std::unordered_map<std::string, double> system_metrics_;
    
    // Helper methods
    void calculateThermalCoupling();
    void calculateElectricalCoupling();
    void calculateMechanicalCoupling();
    void updateSystemMetrics(std::shared_ptr<Wafer> wafer);
    
    // Validation methods
    bool validateDiePositions() const;
    bool validateInterconnectRouting() const;
    bool validateThermalConstraints() const;
    bool validateElectricalConstraints() const;
};

#endif // MULTI_DIE_MODEL_HPP
