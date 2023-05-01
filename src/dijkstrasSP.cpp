
#include "authorGraph.h"
std::vector<unsigned long> AuthorGraph::dijkstrasShortestPath(const unsigned long& start, const unsigned long& dest) {
    // std::unordered_map<unsigned long, float> distance_map;
    
    std::unordered_map<unsigned long, std::vector<weighted_edge>> graph = getGraph();
    if (graph.find(start) == graph.end() || graph.find(dest) == graph.end()) return std::vector<unsigned long>();
    
    struct CompareWeight {
        bool operator()(weighted_edge const& p1, weighted_edge const& p2)
        {
            // return "true" if "p1" is ordered
            // before "p2", for example:
            return p1.weight > p2.weight;
        }
    };
    std::priority_queue<weighted_edge, std::vector<weighted_edge>, CompareWeight> queue(graph.at(start).begin(), graph.at(start).end());
    // std::set<unsigned long> visited;
    std::unordered_map<unsigned long, unsigned long> previous;
    // distance_map[start] = 0;
    
    previous[start] = start;
    
    // visited.insert(start);
    /*
    for (weighted_edge edge : graph.at(start)) {
        queue.push(edge);
    }
    */
    bool flag = true;
    while (!queue.empty()) {
        weighted_edge cur = queue.top();
        queue.pop();
        if (previous.find(cur.destination) != previous.end()) continue;
        previous[cur.destination] = cur.source;
        if (cur.destination == dest) {
            flag = false;
            break;
        }
        for (weighted_edge edge : graph.at(cur.destination)) {
            if (previous.find(edge.destination) == previous.end()) {
                queue.push(edge);
            }
        }
    }
    if (flag) {
        return std::vector<unsigned long>();
    }
    std::vector<unsigned long> ans;
    for (unsigned long cur = dest; previous.at(cur) != cur; cur = previous.at(cur)) {
        ans.push_back(cur);
    }
    ans.push_back(start);
    std::reverse(ans.begin(), ans.end());
    return ans;
}