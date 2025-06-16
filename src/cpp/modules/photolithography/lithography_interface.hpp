#ifndef LITHOGRAPHY_INTERFACE_HPP
#define LITHOGRAPHY_INTERFACE_HPP

#include "../../core/wafer.hpp"
#include <memory>
#include <vector>

class LithographyInterface {
public:
  virtual ~LithographyInterface() = default;
  virtual void simulateExposure(std::shared_ptr<Wafer> wafer, double wavelength, double na, const std::vector<std::vector<int>>& mask) = 0;
  virtual void simulateMultiPatterning(std::shared_ptr<Wafer> wafer, double wavelength, double na, const std::vector<std::vector<std::vector<int>>>& masks) = 0;
};

#endif // LITHOGRAPHY_INTERFACE_HPP