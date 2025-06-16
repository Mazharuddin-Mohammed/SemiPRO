#ifndef ETCHING_INTERFACE_HPP
#define ETCHING_INTERFACE_HPP

#include "../../core/wafer.hpp"
#include <memory>

class EtchingInterface {
public:
    virtual ~EtchingInterface() = default;
    virtual void simulateEtching(std::shared_ptr<Wafer> wafer, double depth, const std::string& type) = 0;
};

#endif // ETCHING_INTERFACE_HPP