#include <chrono>
#include <fstream>
#include <iostream>

#include <range/v3/algorithm/all_of.hpp>
#include <range/v3/algorithm/max_element.hpp>
#include <range/v3/algorithm/minmax_element.hpp>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/transform.hpp>

#include <nlohmann/json.hpp>

#include "Graph.h"
#include "Node.h"
#include "Simulator.h"

using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::ofstream;
using std::ostream;
using std::optional;
using std::shared_ptr;
using std::string;
using std::vector;

using ranges::all_of;
using ranges::max_element;
using ranges::minmax_element;
using ranges::to;

namespace views = ranges::views;

using nlohmann::json;

namespace gossip {
namespace simulator {

namespace {

uint16_t vertexToPort(int vertex, uint16_t firstPort)
{
    return (vertex & 0xffff) + firstPort;
}

int portToVertex(uint16_t port, uint16_t firstPort)
{
    return port - firstPort + 1;
}

class JsonWriter {
public:
    JsonWriter(const vector<shared_ptr<Node>>& nodes,
               int maxRounds,
               uint16_t firstPort);

    void write(ostream& out);

private:
    const vector<shared_ptr<Node>>& nodes_;
    int maxRounds_;
    uint16_t firstPort_;
};

JsonWriter::JsonWriter(const vector<shared_ptr<Node>>& nodes,
                       int maxRounds,
                       uint16_t firstPort) :
    nodes_(nodes),
    maxRounds_(maxRounds),
    firstPort_(firstPort)
{}

void JsonWriter::write(ostream& out)
{
    vector<json> nodes;
    vector<json> links;

    auto nodeId = [this](uint16_t port) {
        return "N" + std::to_string(portToVertex(port, firstPort_));
    };

    for (const auto& node : nodes_) {
        string id = nodeId(node->port());
        for (uint16_t neighbor : node->neighbors()) {
            links.push_back({
                { "source", id },
                { "target", nodeId(neighbor) }
            });
        }

        int it = maxRounds_ - node->stats().numSent;
        nodes.push_back({
            { "id", std::move(id) },
            { "iterations", it }
        });
    }

    out << json{
        { "nodes", std::move(nodes) },
        { "links", std::move(links) }
    }.dump();
}

} // namespace

Simulator::Simulator(Opts opts) :
    opts_(std::move(opts))
{
    std::cout << "Initializing simulator - #nodes=" << opts_.numNodes <<
        ", #neighbors=" << opts_.numNeighbors <<
        ", period=" << duration_cast<milliseconds>(opts_.period).count() << "ms" <<
        ", fanout=" << opts_.fanout << std::endl;
}

vector<shared_ptr<Node>> Simulator::run()
{
    Graph g(opts_.numNodes, opts_.numNeighbors);

    vector<shared_ptr<Node>> nodes =
        g.vertices() | views::transform([this, &g](int vertex) {
            auto adjacents =
                g.adjacents(vertex) | views::transform([](int adjacent) {
                    return vertexToPort(adjacent, firstPort);
                });

            return Node::create(io_,
                                vertexToPort(vertex, firstPort),
                                adjacents | to<vector>,
                                opts_.period,
                                opts_.fanout);
        }) | to<vector>;

    auto allDone = [&nodes]{
        return all_of(nodes, [](const auto& node) {
            return node->stats().numReceived > 0;
        });
    };

    while (!allDone()) {
        io_.run_one();
    }

    return nodes;
}

void printStats(const vector<shared_ptr<Node>>& nodes,
                const optional<string>& outfile)
{
    auto stats = nodes | views::transform([](const auto& node) {
        return node->stats();
    });

    auto receiveTimes = stats | views::transform([](const auto& stat) {
        return stat.firstReceived;
    });

    auto numSent = stats | views::transform([](const auto& stat) {
        return stat.numSent;
    });

    const auto [minTime, maxTime] = minmax_element(receiveTimes);
    auto diff = duration_cast<milliseconds>(*maxTime - *minTime);
    milliseconds avg{ 0 };

    const auto maxRounds = max_element(numSent);

    for (const auto& node : nodes) {
        const Node::Stats& stat = node->stats();

        auto diff = duration_cast<milliseconds>(stat.firstReceived - *minTime);
        avg += diff;

        std::cout << node->port() << ": latency=" <<
            duration_cast<milliseconds>(diff).count() << "ms" <<
            ", received=" << stat.numReceived << ", sent=" <<
            stat.numSent << std::endl;
    }

    avg /= nodes.size();

    std::cout << "---" << std::endl;
    std::cout << "Avg. latency: " << avg.count() << "ms" << std::endl;
    std::cout << "Max. latency: " << diff.count() << "ms" << std::endl;
    std::cout << "Rounds of gossip: " << *maxRounds << std::endl;

    if (outfile) {
        ofstream out(*outfile);

        JsonWriter writer(nodes, *maxRounds, Simulator::firstPort);
        writer.write(out);

        out.close();
        std::cout << "Wrote results to " << *outfile << std::endl;
    }
}

} // namespace simulator
} // namespace gossip

