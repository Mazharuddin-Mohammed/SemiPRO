#include "lithography_model.hpp"
#include "../../core/utils.hpp"
#include <cmath>

LithographyModel::LithographyModel() {}

void LithographyModel::simulateExposure(std::shared_ptr<Wafer> wafer, double wavelength, double na,
                                       const std::vector<std::vector<int>>& mask) {
  int x_dim = wafer->getGrid().rows();
  int y_dim = wafer->getGrid().cols();
  auto aerial_image = computeAerialImage(mask, wavelength, na, x_dim, y_dim);

  // Threshold aerial image with sigmoid for smoother transitions
  Eigen::ArrayXXd pattern = 1.0 / (1.0 + (-10.0 * (aerial_image - 0.5)).exp());
  pattern = (pattern > 0.5).cast<double>();
  wafer->setPhotoresistPattern(pattern);

  Logger::getInstance().log("Exposure simulated: wavelength=" + std::to_string(wavelength) +
                            "nm, NA=" + std::to_string(na));
}

void LithographyModel::simulateMultiPatterning(std::shared_ptr<Wafer> wafer, double wavelength, double na,
                                              const std::vector<std::vector<std::vector<int>>>& masks) {
  int x_dim = wafer->getGrid().rows();
  int y_dim = wafer->getGrid().cols();
  Eigen::ArrayXXd combined_pattern = Eigen::ArrayXXd::Zero(x_dim, y_dim);

  for (const auto& mask : masks) {
    auto aerial_image = computeAerialImage(mask, wavelength, na, x_dim, y_dim);
    Eigen::ArrayXXd pattern = 1.0 / (1.0 + (-10.0 * (aerial_image - 0.5)).exp());
    pattern = (pattern > 0.5).cast<double>();
    combined_pattern = combined_pattern.max(pattern);
  }

  wafer->setPhotoresistPattern(combined_pattern);
  Logger::getInstance().log("Multi-patterning simulated: " + std::to_string(masks.size()) +
                            " masks, wavelength=" + std::to_string(wavelength) + "nm, NA=" + std::to_string(na));
}

Eigen::ArrayXXd LithographyModel::computeAerialImage(const std::vector<std::vector<int>>& mask, double wavelength,
                                                    double na, int x_dim, int y_dim) const {
  Eigen::ArrayXXd aerial_image = Eigen::ArrayXXd::Zero(x_dim, y_dim);
  double k1 = 0.25; // Process factor
  double resolution = k1 * wavelength / na; // um
  double sigma = 0.5; // Partial coherence factor
  double sigma_psf = resolution * sigma; // Gaussian PSF width

  // Partially coherent imaging with Gaussian PSF
  for (int i = 0; i < x_dim; ++i) {
    for (int j = 0; j < y_dim; ++j) {
      double intensity = 0.0;
      for (size_t m = 0; m < mask.size(); ++m) {
        for (size_t n = 0; n < mask[0].size(); ++n) {
          if (mask[m][n] == 1) {
            double dx = (i - m) * resolution / x_dim;
            double dy = (j - n) * resolution / y_dim;
            double r2 = dx * dx + dy * dy;
            intensity += std::exp(-r2 / (2 * sigma_psf * sigma_psf)); // Gaussian PSF
          }
        }
      }
      aerial_image(i, j) = std::clamp(intensity / (2 * M_PI * sigma_psf * sigma_psf), 0.0, 1.0);
    }
  }

  return aerial_image;
}