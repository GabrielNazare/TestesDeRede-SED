cmake_minimum_required(VERSION 3.16)
project(HandoverSimulator VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

option(ENABLE_COVERAGE "Enable code coverage instrumentation" OFF)
if(ENABLE_COVERAGE)
    message(STATUS "Code coverage ENABLED")
    add_compile_options(--coverage -O0 -g)
    add_link_options(--coverage)
endif()

include(FetchContent)

FetchContent_Declare(googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0
)

FetchContent_Declare(json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.3
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest json)

include_directories(include)

add_library(handover_core
    src/SignalFilter.cpp
    src/SimulationEngine.cpp
    src/MetricsCollector.cpp
)
target_link_libraries(handover_core PUBLIC nlohmann_json::nlohmann_json)

add_executable(handover_sim src/main.cpp)
target_link_libraries(handover_sim PRIVATE handover_core)

enable_testing()

add_executable(handover_tests
    tests/test_signal_filter.cpp
    tests/test_a3_event.cpp
    tests/test_ttt_logic.cpp
    tests/test_path_loss.cpp
    tests/test_simulation_engine.cpp
    tests/test_ping_pong.cpp
)
target_link_libraries(handover_tests PRIVATE handover_core GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(handover_tests)
