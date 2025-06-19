# Author: Dr. Mazharuddin Mohammed
cmake_minimum_required(VERSION 3.10)
project(SemiconductorSimulator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Eigen3 REQUIRED)
find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)
find_package(OpenMP REQUIRED)
find_package(Catch2 QUIET)

include_directories(
    ${EIGEN3_INCLUDE_DIR}
    ${Vulkan_INCLUDE_DIRS}
    src/cpp/core
    src/cpp/modules/geometry
    src/cpp/modules/oxidation
    src/cpp/modules/doping
    src/cpp/modules/photolithography
    src/cpp/modules/deposition
    src/cpp/modules/etching
    src/cpp/modules/metallization
    src/cpp/modules/packaging
    src/cpp/modules/thermal
    src/cpp/modules/reliability
    src/cpp/modules/metrology
    src/cpp/modules/interconnect
    src/cpp/modules/defect_inspection
    src/cpp/modules/multi_die
    src/cpp/modules/design_rule_check
    src/cpp/modules/advanced_visualization
    src/cpp/renderer
    src/cpp/api
    src/cpp/integration
)

set(SOURCES
    src/cpp/core/wafer.cpp
    src/cpp/core/wafer_enhanced.cpp
    src/cpp/core/simulation_engine.cpp
    src/cpp/core/utils.cpp
    src/cpp/core/performance_utils.cpp
    src/cpp/core/plugin_manager.cpp
    src/cpp/modules/geometry/geometry_manager.cpp
    src/cpp/modules/oxidation/oxidation_model.cpp
    src/cpp/modules/doping/doping_manager.cpp
    src/cpp/modules/doping/monte_carlo_solver.cpp
    src/cpp/modules/doping/diffusion_solver.cpp
    src/cpp/modules/photolithography/lithography_model.cpp
    src/cpp/modules/deposition/deposition_model.cpp
    src/cpp/modules/etching/etching_model.cpp
    src/cpp/modules/metallization/metallization_model.cpp
    src/cpp/modules/packaging/packaging_model.cpp
    src/cpp/modules/thermal/thermal_model.cpp
    src/cpp/modules/reliability/reliability_model.cpp
    src/cpp/modules/metrology/metrology_model.cpp
    src/cpp/modules/interconnect/damascene_model.cpp
    src/cpp/modules/defect_inspection/defect_inspection_model.cpp
    src/cpp/modules/multi_die/multi_die_model.cpp
    src/cpp/modules/design_rule_check/drc_model.cpp
    src/cpp/modules/advanced_visualization/advanced_visualization_model.cpp
    src/cpp/renderer/vulkan_renderer.cpp
    src/cpp/integration/eda_integration.cpp
)

add_library(simulator_lib ${SOURCES})

add_executable(simulator src/cpp/main.cpp)
target_link_libraries(simulator simulator_lib ${Vulkan_LIBRARIES} glfw yaml-cpp OpenMP::OpenMP_CXX dl)

# Tests
enable_testing()
add_executable(tests
    tests/cpp/main.cpp
    tests/cpp/test_wafer.cpp
    tests/cpp/test_geometry.cpp
    tests/cpp/test_oxidation.cpp
    tests/cpp/test_doping.cpp
    tests/cpp/test_photolithography.cpp
    tests/cpp/test_deposition.cpp
    tests/cpp/test_etching.cpp
    tests/cpp/test_metallization.cpp
    tests/cpp/test_packaging.cpp
    tests/cpp/test_thermal.cpp
    tests/cpp/test_reliability.cpp
    tests/cpp/test_renderer.cpp
)
target_link_libraries(tests simulator_lib ${Vulkan_LIBRARIES} glfw yaml-cpp Catch2::Catch2)

# Examples
add_executable(example_geometry examples/cpp/example_geometry.cpp)
add_executable(example_oxidation examples/cpp/example_oxidation.cpp)
add_executable(example_doping examples/cpp/example_doping.cpp)
add_executable(example_photolithography examples/cpp/example_photolithography.cpp)
add_executable(example_deposition examples/cpp/example_deposition.cpp)
add_executable(example_etching examples/cpp/example_etching.cpp)
add_executable(example_metallization examples/cpp/example_metallization.cpp)
add_executable(example_packaging examples/cpp/example_packaging.cpp)
add_executable(example_thermal examples/cpp/example_thermal.cpp)
add_executable(example_reliability examples/cpp/example_reliability.cpp)
add_executable(working_doping_example examples/cpp/working_doping_example.cpp)
target_link_libraries(example_geometry simulator_lib yaml-cpp)
target_link_libraries(example_oxidation simulator_lib yaml-cpp)
target_link_libraries(example_doping simulator_lib yaml-cpp)
target_link_libraries(example_photolithography simulator_lib yaml-cpp)
target_link_libraries(example_deposition simulator_lib yaml-cpp)
target_link_libraries(example_etching simulator_lib yaml-cpp)
target_link_libraries(example_metallization simulator_lib yaml-cpp)
target_link_libraries(example_packaging simulator_lib yaml-cpp)
target_link_libraries(example_thermal simulator_lib ${Vulkan_LIBRARIES} glfw yaml-cpp)
target_link_libraries(example_reliability simulator_lib ${Vulkan_LIBRARIES} glfw yaml-cpp)
target_link_libraries(working_doping_example simulator_lib yaml-cpp)

# Tutorials
add_executable(tutorial_geometry tutorials/cpp/tutorial_geometry.cpp)
add_executable(tutorial_oxidation tutorials/cpp/tutorial_oxidation.cpp)
add_executable(tutorial_doping tutorials/cpp/tutorial_doping.cpp)
add_executable(tutorial_photolithography tutorials/cpp/tutorial_photolithography.cpp)
add_executable(tutorial_deposition tutorials/cpp/tutorial_deposition.cpp)
add_executable(tutorial_etching tutorials/cpp/tutorial_etching.cpp)
add_executable(tutorial_metallization tutorials/cpp/tutorial_metallization.cpp)
add_executable(tutorial_packaging tutorials/cpp/tutorial_packaging.cpp)
add_executable(tutorial_thermal tutorials/cpp/tutorial_thermal.cpp)
add_executable(tutorial_reliability tutorials/cpp/tutorial_reliability.cpp)
target_link_libraries(tutorial_geometry simulator_lib yaml-cpp)
target_link_libraries(tutorial_oxidation simulator_lib yaml-cpp)
target_link_libraries(tutorial_doping simulator_lib yaml-cpp)
target_link_libraries(tutorial_photolithography simulator_lib yaml-cpp)
target_link_libraries(tutorial_deposition simulator_lib yaml-cpp)
target_link_libraries(tutorial_etching simulator_lib yaml-cpp)
target_link_libraries(tutorial_metallization simulator_lib yaml-cpp)
target_link_libraries(tutorial_packaging simulator_lib yaml-cpp)
target_link_libraries(tutorial_thermal simulator_lib ${Vulkan_LIBRARIES} glfw yaml-cpp)
target_link_libraries(tutorial_reliability simulator_lib ${Vulkan_LIBRARIES} glfw yaml-cpp)