#ifndef LITHOGRAPHY_MODEL_HPP
#define LITHOGRAPHY_MODEL_HPP

#include "lithography_interface.hpp"
#include "../../core/wafer.hpp"
#include <Eigen/Dense>
#include <vector>

class LithographyModel : public LithographyInterface {
public:
  LithographyModel();
  void simulateExposure(std::shared_ptr<Wafer> wafer, double wavelength, double na, const std::vector<std::vector<int>>& mask) override;
  void simulateMultiPatterning(std::shared_ptr<Wafer> wafer, double wavelength, double na, const std::vector<std::vector<std::vector<int>>>& masks) override;

private:
  Eigen::ArrayXXd computeAerialImage(const std::vector<std::vector<int>>& mask, double wavelength, double na, int x_dim, int y_dim) const;
};

#endif // LITHOGRAPHY_MODEL_HPP