#ifndef OXIDATION_MODEL_HPP
#define OXIDATION_MODEL_HPP

#include "oxidation_interface.hpp"
#include "../../core/wafer.hpp"
#include <memory>

class OxidationModel : public OxidationInterface {
public:
  OxidationModel();
  void simulateOxidation(std::shared_ptr<Wafer> wafer, double temperature, double time) override;

private:
  double calculateOxideThickness(double temperature, double time) const;
};

#endif // OXIDATION_MODEL_HPP