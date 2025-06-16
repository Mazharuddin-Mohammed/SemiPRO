// Author: Dr. Mazharuddin Mohammed
#include "multi_die_model.hpp"
#include "../../core/utils.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>

MultiDieModel::MultiDieModel() {
    system_metrics_["total_power"] = 0.0;
    system_metrics_["max_temperature"] = 300.0;
    system_metrics_["system_mttf"] = 0.0;
    system_metrics_["total_area"] = 0.0;
    system_metrics_["interconnect_delay"] = 0.0;
}

void MultiDieModel::addDie(const Die& die) {
    // Check for duplicate IDs
    auto it = std::find_if(dies_.begin(), dies_.end(),
                          [&die](const Die& d) { return d.id == die.id; });
    if (it != dies_.end()) {
        throw std::invalid_argument("Die with ID " + die.id + " already exists");
    }
    
    dies_.push_back(die);
    system_metrics_["total_power"] += die.power_consumption;
    system_metrics_["total_area"] += die.width * die.height;
    
    Logger::getInstance().log("Added die: " + die.id + " (type: " + 
                             std::to_string(static_cast<int>(die.type)) + ")");
}

void MultiDieModel::removeDie(const std::string& die_id) {
    auto it = std::find_if(dies_.begin(), dies_.end(),
                          [&die_id](const Die& d) { return d.id == die_id; });
    if (it == dies_.end()) {
        throw std::invalid_argument("Die with ID " + die_id + " not found");
    }
    
    system_metrics_["total_power"] -= it->power_consumption;
    system_metrics_["total_area"] -= it->width * it->height;
    dies_.erase(it);
    
    // Remove interconnects involving this die
    interconnects_.erase(
        std::remove_if(interconnects_.begin(), interconnects_.end(),
                      [&die_id](const Interconnect& ic) {
                          return ic.from_die == die_id || ic.to_die == die_id;
                      }),
        interconnects_.end());
    
    Logger::getInstance().log("Removed die: " + die_id);
}

void MultiDieModel::positionDie(const std::string& die_id, double x, double y) {
    auto it = std::find_if(dies_.begin(), dies_.end(),
                          [&die_id](Die& d) { return d.id == die_id; });
    if (it == dies_.end()) {
        throw std::invalid_argument("Die with ID " + die_id + " not found");
    }
    
    it->position = {x, y};
    Logger::getInstance().log("Positioned die " + die_id + " at (" + 
                             std::to_string(x) + ", " + std::to_string(y) + ")");
}

void MultiDieModel::performWireBonding(std::shared_ptr<Wafer> wafer,
                                      const std::string& die1, const std::string& die2,
                                      const std::vector<std::pair<std::pair<int, int>, std::pair<int, int>>>& bonds) {
    if (!wafer) {
        throw std::invalid_argument("Wafer pointer is null");
    }
    
    // Find the dies
    auto it1 = std::find_if(dies_.begin(), dies_.end(),
                           [&die1](const Die& d) { return d.id == die1; });
    auto it2 = std::find_if(dies_.begin(), dies_.end(),
                           [&die2](const Die& d) { return d.id == die2; });
    
    if (it1 == dies_.end() || it2 == dies_.end()) {
        throw std::invalid_argument("One or both dies not found");
    }
    
    // Add wire bonds to wafer
    wafer->addPackaging(100.0, "ceramic", bonds);
    
    // Calculate wire bond resistance and delay
    double total_resistance = 0.0;
    double total_delay = 0.0;
    
    for (const auto& bond : bonds) {
        double dx = bond.second.first - bond.first.first;
        double dy = bond.second.second - bond.first.second;
        double length = std::sqrt(dx*dx + dy*dy) * 1e-6; // Convert to meters
        
        // Wire bond resistance (Au wire, 25um diameter)
        double wire_resistance = 2.44e-8 * length / (M_PI * (12.5e-6) * (12.5e-6));
        total_resistance += wire_resistance;
        
        // Wire bond delay (RC delay)
        double wire_capacitance = 1e-12 * length; // Approximate capacitance
        total_delay += wire_resistance * wire_capacitance;
    }
    
    // Create interconnect
    Interconnect interconnect(die1, die2, "wire_bond");
    interconnect.resistance = total_resistance;
    interconnect.delay = total_delay;
    addInterconnect(interconnect);
    
    Logger::getInstance().log("Wire bonding completed between " + die1 + " and " + die2 +
                             " with " + std::to_string(bonds.size()) + " bonds");
}

void MultiDieModel::performFlipChipBonding(std::shared_ptr<Wafer> wafer,
                                          const std::string& die1, const std::string& die2,
                                          double bump_pitch, double bump_diameter) {
    if (!wafer) {
        throw std::invalid_argument("Wafer pointer is null");
    }
    
    // Find the dies
    auto it1 = std::find_if(dies_.begin(), dies_.end(),
                           [&die1](const Die& d) { return d.id == die1; });
    auto it2 = std::find_if(dies_.begin(), dies_.end(),
                           [&die2](const Die& d) { return d.id == die2; });
    
    if (it1 == dies_.end() || it2 == dies_.end()) {
        throw std::invalid_argument("One or both dies not found");
    }
    
    // Calculate number of bumps
    int bumps_x = static_cast<int>(std::min(it1->width, it2->width) / bump_pitch);
    int bumps_y = static_cast<int>(std::min(it1->height, it2->height) / bump_pitch);
    int total_bumps = bumps_x * bumps_y;
    
    // Calculate flip-chip interconnect parameters
    double bump_resistance = 1e-3; // Ohms per bump
    double bump_capacitance = 1e-13; // Farads per bump
    double total_resistance = bump_resistance / total_bumps; // Parallel resistance
    double total_capacitance = bump_capacitance * total_bumps;
    double delay = total_resistance * total_capacitance;
    
    // Create interconnect
    Interconnect interconnect(die1, die2, "flip_chip");
    interconnect.resistance = total_resistance;
    interconnect.capacitance = total_capacitance;
    interconnect.delay = delay;
    addInterconnect(interconnect);
    
    Logger::getInstance().log("Flip-chip bonding completed between " + die1 + " and " + die2 +
                             " with " + std::to_string(total_bumps) + " bumps");
}

void MultiDieModel::performTSVIntegration(std::shared_ptr<Wafer> wafer,
                                         const std::string& die_id,
                                         const std::vector<std::pair<double, double>>& tsv_positions,
                                         double tsv_diameter, double tsv_depth) {
    if (!wafer) {
        throw std::invalid_argument("Wafer pointer is null");
    }
    
    auto it = std::find_if(dies_.begin(), dies_.end(),
                          [&die_id](const Die& d) { return d.id == die_id; });
    if (it == dies_.end()) {
        throw std::invalid_argument("Die with ID " + die_id + " not found");
    }
    
    // Calculate TSV parameters
    double tsv_area = M_PI * (tsv_diameter/2) * (tsv_diameter/2);
    double tsv_resistance = 1.7e-8 * tsv_depth / tsv_area; // Copper resistivity
    double tsv_capacitance = 8.854e-12 * tsv_area / tsv_depth; // Approximate
    
    // Add TSV metal layer to wafer
    wafer->addMetalLayer(tsv_depth, "copper");
    
    Logger::getInstance().log("TSV integration completed for die " + die_id +
                             " with " + std::to_string(tsv_positions.size()) + " TSVs");
}

void MultiDieModel::analyzeElectricalPerformance(std::shared_ptr<Wafer> wafer) {
    if (!wafer) {
        throw std::invalid_argument("Wafer pointer is null");
    }
    
    calculateInterconnectParameters();
    
    double total_delay = 0.0;
    double max_resistance = 0.0;
    
    for (const auto& interconnect : interconnects_) {
        total_delay += interconnect.delay;
        max_resistance = std::max(max_resistance, interconnect.resistance);
    }
    
    system_metrics_["interconnect_delay"] = total_delay;
    system_metrics_["max_resistance"] = max_resistance;
    
    Logger::getInstance().log("Electrical performance analysis completed. Total delay: " +
                             std::to_string(total_delay) + " s");
}

void MultiDieModel::analyzeThermalPerformance(std::shared_ptr<Wafer> wafer) {
    if (!wafer) {
        throw std::invalid_argument("Wafer pointer is null");
    }
    
    calculateThermalCoupling();
    
    // Simple thermal analysis - more sophisticated models would use FEM
    double total_power = system_metrics_["total_power"];
    double total_area = system_metrics_["total_area"];
    double power_density = total_power / total_area;
    
    // Estimate temperature rise
    double thermal_resistance = 10.0; // K/W (simplified)
    double temperature_rise = total_power * thermal_resistance;
    double max_temperature = 300.0 + temperature_rise; // Base temperature + rise
    
    system_metrics_["max_temperature"] = max_temperature;
    system_metrics_["power_density"] = power_density;
    
    Logger::getInstance().log("Thermal performance analysis completed. Max temperature: " +
                             std::to_string(max_temperature) + " K");
}

void MultiDieModel::addInterconnect(const Interconnect& interconnect) {
    interconnects_.push_back(interconnect);
}

void MultiDieModel::calculateInterconnectParameters() {
    for (auto& interconnect : interconnects_) {
        // Update delay calculation
        interconnect.delay = interconnect.resistance * interconnect.capacitance;
    }
}

void MultiDieModel::calculateThermalCoupling() {
    // Simplified thermal coupling calculation
    for (size_t i = 0; i < dies_.size(); ++i) {
        for (size_t j = i + 1; j < dies_.size(); ++j) {
            double dx = dies_[i].position.first - dies_[j].position.first;
            double dy = dies_[i].position.second - dies_[j].position.second;
            double distance = std::sqrt(dx*dx + dy*dy);
            
            // Thermal coupling decreases with distance
            double thermal_coupling = 1.0 / (1.0 + distance);
            // Store coupling information if needed
        }
    }
}

void MultiDieModel::updateSystemMetrics(std::shared_ptr<Wafer> wafer) {
    // Update all system-level metrics
    system_metrics_["total_power"] = 0.0;
    system_metrics_["total_area"] = 0.0;
    
    for (const auto& die : dies_) {
        system_metrics_["total_power"] += die.power_consumption;
        system_metrics_["total_area"] += die.width * die.height;
    }
}

bool MultiDieModel::validateDiePositions() const {
    // Check for overlapping dies
    for (size_t i = 0; i < dies_.size(); ++i) {
        for (size_t j = i + 1; j < dies_.size(); ++j) {
            const auto& die1 = dies_[i];
            const auto& die2 = dies_[j];
            
            double dx = std::abs(die1.position.first - die2.position.first);
            double dy = std::abs(die1.position.second - die2.position.second);
            
            if (dx < (die1.width + die2.width) / 2 && dy < (die1.height + die2.height) / 2) {
                return false; // Overlap detected
            }
        }
    }
    return true;
}
