cmake_minimum_required(VERSION 3.12)

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)

add_library(gossip-sim-lib STATIC
    Graph.cpp
    Node.cpp
    Opts.cpp
    Simulator.cpp
)

target_link_libraries(gossip-sim-lib
PUBLIC
CONAN_PKG::boost
CONAN_PKG::range-v3

PRIVATE
CONAN_PKG::nlohmann_json
)

add_executable(gossip-sim
    main.cpp
)

target_link_libraries(gossip-sim
PRIVATE gossip-sim-lib
)

