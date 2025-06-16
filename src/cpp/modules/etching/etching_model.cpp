#include "etching_model.hpp"
#include "../../core/utils.hpp"

EtchingModel::EtchingModel() {}

void EtchingModel::simulateEtching(std::shared_ptr<Wafer> wafer, double depth, const std::string& type) {
    if (type == "isotropic") {
        simulate_isotropic(wafer, depth);
    } else if (type == "anisotropic") {
        simulate_anisotropic(wafer, depth);
    } else {
        Logger::getInstance().log("ERROR: Unknown etching type: " + type);
        return;
    }
    Logger::getInstance().log("Etching simulated: type=" + type + ", depth=" + std::to_string(depth) + "um");
}

void EtchingModel::simulate_isotropic(std::shared_ptr<Wafer> wafer, double depth) {
    Eigen::ArrayXXd grid = wafer->getGrid();
    const Eigen::ArrayXXd& pattern = wafer->getPhotoresistPattern();
    Eigen::ArrayXXd new_grid = grid;
    for (int i = 1; i < grid.rows() - 1; ++i) {
        for (int j = 1; j < grid.cols() - 1; ++j) {
            if (i < pattern.rows() && j < pattern.cols() && pattern(i, j) < 0.5) {
                new_grid(i, j) -= depth;
                // Lateral undercutting
                new_grid(i + 1, j) -= 0.5 * depth;
                new_grid(i - 1, j) -= 0.5 * depth;
                new_grid(i, j + 1) -= 0.5 * depth;
                new_grid(i, j - 1) -= 0.5 * depth;
            }
        }
    }
    wafer->updateGrid(new_grid);
}

void EtchingModel::simulate_anisotropic(std::shared_ptr<Wafer> wafer, double depth) {
    Eigen::ArrayXXd grid = wafer->getGrid();
    const Eigen::ArrayXXd& pattern = wafer->getPhotoresistPattern();
    for (int i = 0; i < grid.rows(); ++i) {
        for (int j = 0; j < grid.cols(); ++j) {
            if (i < pattern.rows() && j < pattern.cols() && pattern(i, j) < 0.5) {
                grid(i, j) -= depth;
            }
        }
    }
    wafer->updateGrid(grid);
}