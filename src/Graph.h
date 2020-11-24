#pragma once

#include <map>
#include <vector>

#include <range/v3/view/any_view.hpp>

namespace gossip {
namespace simulator {

class Graph final {
public:
    Graph(int numVertices,
          int numAdjacents,
          bool makeConnected = true);

    ranges::any_view<int> vertices() const;
    ranges::any_view<int> adjacents(int vertex) const;

private:
    explicit Graph(std::map<int, std::vector<int>> graph);

    void makeConnected_();
    Graph transpose_() const;

    std::map<int, std::vector<int>> graph_;
};

} // namespace simulator
} // namespace gossip

