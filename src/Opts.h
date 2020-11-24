#pragma once

#include <chrono>
#include <optional>
#include <string>

namespace gossip {
namespace simulator {

struct Opts {
    static std::optional<Opts> parse(int argc, char *argv[], int maxNodes);

    int numNodes;
    int numNeighbors;
    std::chrono::seconds period;
    int fanout;
    std::optional<std::string> outfile;
};

} // namespace simulator
} // namespace gossip

