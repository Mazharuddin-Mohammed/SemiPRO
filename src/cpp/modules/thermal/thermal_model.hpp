#pragma once
#include "thermal_interface.hpp"
#include "../../core/utils.hpp"
#include <Eigen/Dense>

class ThermalSimulationModel : public ThermalInterface {
public:
    ThermalSimulationModel();
    void simulateThermal(std::shared_ptr<Wafer> wafer, double ambient_temperature, double current) override;

private:
    void initializeThermalProperties(std::shared_ptr<Wafer> wafer);
    void computeHeatSources(std::shared_ptr<Wafer> wafer, double current, Eigen::ArrayXXd& heat_source);
    void solveHeatEquation(std::shared_ptr<Wafer> wafer, double ambient_temperature, const Eigen::ArrayXXd& heat_source);
};