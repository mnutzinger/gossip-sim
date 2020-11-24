cmake_minimum_required(VERSION 3.12)

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)

find_package(Boost 1.74.0 REQUIRED program_options)

add_library(gossip-sim-lib STATIC
    Graph.cpp
    Node.cpp
    Opts.cpp
    Simulator.cpp
)

target_link_libraries(gossip-sim-lib
PUBLIC
Boost::boost
CONAN_PKG::range-v3

PRIVATE
Boost::program_options
CONAN_PKG::nlohmann_json
)

add_executable(gossip-sim
    main.cpp
)

target_link_libraries(gossip-sim
PRIVATE gossip-sim-lib
)

