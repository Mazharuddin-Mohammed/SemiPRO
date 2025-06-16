#ifndef DEPOSITION_HMODEL_HPP
#define DEPOSITION_HMODEL_HPP

#include "deposition_interface.hpp"
#include "../../core/wafer.hpp"
#include <Eigen/Dense>

class DepositionModel : public DepositionInterface {
public:
    DepositionModel();
    void simulateDeposition(std::shared_ptr<Wafer> wafer, double thickness, const std::string& material, const std::string& type) override;

private:
    void simulate_uniform(std::shared_ptr<Wafer> wafer, double thickness, const std::string& material);
    void simulate_conformal(std::shared_ptr<Wafer> wafer, double thickness, const std::string& material);
};

#endif // DEPOSITION_HMODEL_HPP