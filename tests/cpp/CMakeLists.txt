cmake_minimum_required(VERSION 3.10)
project(Tests)

find_package(Catch2 REQUIRED)
add_executable(tests
    main.cpp
    test_wafer.cpp
    test_geometry.cpp
    test_oxidation.cpp
    test_doping.cpp
    test_photolithography.cpp
    test_deposition.cpp
    test_etching.cpp
    test_metallization.cpp
    test_packaging.cpp
    test_thermal.cpp
    test_reliability.cpp
    test_renderer.cpp
    ../src/cpp/core/wafer.cpp
    ../src/cpp/core/utils.cpp
    ../src/cpp/modules/geometry/geometry_manager.cpp
    ../src/cpp/modules/oxidation/oxidation_model.cpp
    ../src/cpp/modules/doping/monte_carlo_solver.cpp
    ../src/cpp/modules/doping/diffusion_solver.cpp
    ../src/cpp/modules/doping/doping_manager.cpp
    ../src/cpp/modules/photolithography/lithography_model.cpp
    ../src/cpp/modules/deposition/deposition_model.cpp
    ../src/cpp/modules/etching/etching_model.cpp
    ../src/cpp/modules/metallization/metallization_model.cpp
    ../src/cpp/modules/packaging/packaging_model.cpp
    ../src/cpp/modules/thermal/thermal_model.cpp
    ../src/cpp/modules/reliability/reliability_model.cpp
    ../src/cpp/renderer/vulkan_renderer.cpp
)
target_link_libraries(tests Catch2::Catch2WithMain Eigen3::Eigen Vulkan::Vulkan glfw)