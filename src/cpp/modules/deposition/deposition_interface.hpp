#ifndef DEPOSITION_HINTERFACE_HPP
#define DEPOSITION_HINTERFACE_HPP

#include "../../core/wafer.hpp"
#include <memory>

class DepositionInterface {
public:
    virtual ~DepositionInterface() = default;
    virtual void simulateDeposition(std::shared_ptr<Wafer> wafer, double thickness, const std::string& material, const std::string& type) = 0;
};

#endif // DEPOSITION_HINTERFACE_HPP