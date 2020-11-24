#include <chrono>
#include <iostream>
#include <random>
#include <set>
#include <stack>
#include <utility>

#include <range/v3/action/sort.hpp>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/all.hpp>
#include <range/v3/view/empty.hpp>
#include <range/v3/view/map.hpp>
#include <range/v3/view/transform.hpp>

#include "Graph.h"

using std::chrono::system_clock;
using std::default_random_engine;
using std::map;
using std::set;
using std::stack;
using std::vector;

using ranges::any_view;
using ranges::to;

namespace actions = ranges::actions;
namespace views = ranges::views;

namespace gossip {
namespace simulator {

namespace {

vector<int> generateAdjacents(int start,
                              int end,
                              int num,
                              int exclude,
                              default_random_engine& rand)
{
    set<int> adjacents;

    while (adjacents.size() < num) {
        int vertex = (rand() % (end-start+1)) + start;
        if (vertex != exclude) {
            adjacents.insert(vertex);
        }
    }

    return { adjacents.begin(), adjacents.end() };
}

class Kosaraju {
public:
    Kosaraju(const Graph& g, const Graph& t);

    vector<vector<int>> compute();

private:
    void visit_(int vertex, map<int, bool>& visited, stack<int>& vertices);
    void assign_(int vertex,
                 int root,
                 map<int, bool>& visited,
                 map<int, vector<int>>& components);

    const Graph& g_;
    const Graph& t_;
};

Kosaraju::Kosaraju(const Graph& g, const Graph& t) :
    g_(g),
    t_(t)
{}

vector<vector<int>> Kosaraju::compute()
{
    map<int, bool> visited;
    stack<int> vertices;

    for (int vertex : g_.vertices()) {
        visit_(vertex, visited, vertices);
    }

    map<int, vector<int>> components;
    visited.clear();

    while (!vertices.empty()) {
        int vertex = vertices.top();
        vertices.pop();

        assign_(vertex, vertex, visited, components);
    }

    return components | views::transform([](const auto& entry) {
        return entry.second;
    }) | to<vector>;
}

void Kosaraju::visit_(int vertex, map<int, bool>& visited, stack<int>& vertices)
{
    if (visited[vertex]) {
        return;
    }

    visited[vertex] = true;

    for (int adjacent : g_.adjacents(vertex)) {
        visit_(adjacent, visited, vertices);
    }

    vertices.push(vertex);
}

void Kosaraju::assign_(int vertex,
                       int root,
                       map<int, bool>& visited,
                       map<int, vector<int>>& components)
{
    if (visited[vertex]) {
        return;
    }

    visited[vertex] = true;
    components[root].push_back(vertex);

    for (int adjacent : t_.adjacents(vertex)) {
        assign_(adjacent, root, visited, components);
    }
}

} // namespace

Graph::Graph(int numVertices, int numAdjacents, bool makeConnected)
{
    default_random_engine rand(system_clock::now().time_since_epoch().count());

    for (int cur = 1; cur <= numVertices; ++cur) {
        int vertex = cur;
        vector<int> adjacents = generateAdjacents(1,
                                                  numVertices,
                                                  numAdjacents,
                                                  vertex,
                                                  rand);

        graph_.emplace(vertex, std::move(adjacents));
    }

    if (makeConnected) {
        makeConnected_();
    }
}

Graph::Graph(map<int, vector<int>> graph) :
    graph_(std::move(graph))
{}

any_view<int> Graph::vertices() const
{
    return graph_ | views::keys;
}

any_view<int> Graph::adjacents(int vertex) const
{
    if (auto pos = graph_.find(vertex); pos != graph_.end()) {
        return pos->second | views::all;
    }

    return views::empty<int>;
}

void Graph::makeConnected_()
{
    Graph t = transpose_();

    Kosaraju scc(*this, t);
    vector<vector<int>> components = scc.compute();

    if (components.size() <= 1) {
        return;
    }

    components |= actions::sort([](const auto& lhs, const auto& rhs) {
        return lhs.size() > rhs.size();
    });

    for (int i = 1; i < components.size(); ++i) {
        int parent = components[i-1].back();
        int child = components[i].front();

        graph_[parent].push_back(child);
    }
}

Graph Graph::transpose_() const
{
    map<int, vector<int>> transposed;

    for (int vertex : vertices()) {
        for (int adjacent : adjacents(vertex)) {
            transposed[adjacent].push_back(vertex);
        }
    }

    return Graph{ std::move(transposed) };
}

} // namespace simulator
} // namespace gossip

