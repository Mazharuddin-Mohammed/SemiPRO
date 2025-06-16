#ifndef ETCHING_HMODEL_HPP
#define ETCHING_HMODEL_HPP

#include "etching_interface.hpp"
#include "../../core/wafer.hpp"
#include <Eigen/Dense>

class EtchingModel : public EtchingInterface {
public:
    EtchingModel();
    void simulateEtching(std::shared_ptr<Wafer> wafer, double depth, const std::string& type) override;

private:
    void simulate_isotropic(std::shared_ptr<Wafer> wafer, double depth);
    void simulate_anisotropic(std::shared_ptr<Wafer> wafer, double depth);
};

#endif // ETCHING_HMODEL_HPP