#include "doping_manager.hpp"

DopingManager::DopingManager() {}

void DopingManager::simulateIonImplantation(std::shared_ptr<Wafer> wafer, double energy, double dose) {
  auto profile = monte_carlo_.simulateImplantation(wafer, energy, dose);
  wafer->setDopantProfile(profile);
}

void DopingManager::simulateDiffusion(std::shared_ptr<Wafer> wafer, double temperature, double time) {
  auto initial_profile = wafer->getDopantProfile();
  auto profile = diffusion_.simulateDiffusion(initial_profile, temperature, time, dx_, dt_);
  wafer->setDopantProfile(profile);
}