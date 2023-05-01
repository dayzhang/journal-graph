#include "authorGraph.h"

std::vector<std::string> AuthorGraph::dijkstrasShortestPath(const std::string& start, const std::string& dest) {
    std::unordered_map<std::string, float> distance_map;
    std::unordered_map<std::string, std::vector<weighted_edge>> graph = getGraph();
    if (graph.find(start) == graph.end() || graph.find(dest) == graph.end()) return std::vector<std::string>();
    for (auto node : graph) {
        distance_map[node.first] = std::numeric_limits<float>::max();
    }
    auto cmp = [] (weighted_edge a, weighted_edge b) {
        return a.weight > b.weight;
    };
    std::priority_queue<weighted_edge, std::vector<weighted_edge>, cmp> queue;
    distance_map[start] = 0;
    std::vector<std::string> visited
    while (true) {

    }
    return std::vector<std::string>();
}