#include <memory>
#include <optional>
#include <vector>

#include "Node.h"
#include "Opts.h"
#include "Simulator.h"

using std::optional;
using std::shared_ptr;
using std::vector;

using gossip::simulator::Node;
using gossip::simulator::Opts;
using gossip::simulator::Simulator;

int main(int argc, char* argv[])
{
    optional<Opts> opts = Opts::parse(argc, argv, Simulator::maxNodes);
    if (!opts) {
        return 1;
    }

    Simulator simulator(*opts);
    vector<shared_ptr<Node>> nodes = simulator.run();

    gossip::simulator::printStats(nodes, opts->outfile);
    return 0;
}

