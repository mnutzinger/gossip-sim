#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <boost/asio/io_context.hpp>

#include "Opts.h"

namespace gossip {
namespace simulator {

class Node;

class Simulator {
public:
    static constexpr uint16_t firstPort{ 49152 };
    static constexpr uint16_t lastPort{ (2 << 15) - 1 };
    static constexpr int maxNodes{ lastPort - firstPort };

    explicit Simulator(Opts opts);

    std::vector<std::shared_ptr<Node>> run();

private:
    Opts opts_;
    boost::asio::io_context io_;
};

void printStats(const std::vector<std::shared_ptr<Node>>& nodes,
                const std::optional<std::string>& outfile);

} // namespace simulator
} // namespace gossip

