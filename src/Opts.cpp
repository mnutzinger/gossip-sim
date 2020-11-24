#include <exception>
#include <iostream>
#include <utility>

#include <boost/program_options.hpp>

#include "Opts.h"

using std::chrono::seconds;
using std::exception;
using std::nullopt;
using std::optional;
using std::string;

namespace po = boost::program_options;

namespace gossip {
namespace simulator {

optional<Opts> Opts::parse(int argc, char *argv[], int maxNodes)
{
    int numNodes;
    int numNeighbors;
    int periodSec;
    int fanout;
    string outfile;

    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "produce help message")
        ("num-nodes", po::value<int>(&numNodes)->required(), "total number of nodes")
        ("num-neighbors",
         po::value<int>(&numNeighbors)->required(),
         "number of neighbors per node")
        ("period-sec", po::value<int>(&periodSec)->default_value(5), "gossip interval")
        ("fanout", po::value<int>(&fanout)->default_value(1), "fanout per round of gossip")
        ("json-out",
         po::value<string>(&outfile),
         "path to write results as Json");

    try {
        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);

        if (vm.count("help")) {
            std::cerr << desc << std::endl;
            return nullopt;
        }

        po::notify(vm);
    } catch (const exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        std::cerr << desc << std::endl;
        return nullopt;
    }

    if (numNodes <= 0 || numNodes > maxNodes) {
        std::cerr << "Number of nodes must be between 1 and " << maxNodes << std::endl;
        return nullopt;
    }

    if (numNeighbors <= 0 || numNeighbors > numNodes) {
        std::cerr << "Number of neighbors must be between 1 and number of nodes" <<
            std::endl;
        return nullopt;
    }

    if (fanout <= 0 || fanout > numNeighbors) {
        std::cerr << "Fanout must be between 1 and number of neighbors" << std::endl;
        return nullopt;
    }

    Opts opts;
    opts.numNodes = numNodes;
    opts.numNeighbors = numNeighbors;
    opts.period = seconds(periodSec);
    opts.fanout = fanout;

    if (!outfile.empty()) {
        opts.outfile = std::move(outfile);
    }

    return opts;
}

} // namespace simulator
} // namespace gossip

