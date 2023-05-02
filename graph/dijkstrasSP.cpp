
#include "authorGraph.h"
std::vector<unsigned long> AuthorGraph::dijkstrasShortestPath(const unsigned long& start, const unsigned long& dest) {
    // std::unordered_map<unsigned long, float> distance_map;
    
    if (adj_list.find(start) == adj_list.end() || adj_list.find(dest) == adj_list.end()) return std::vector<unsigned long>();
    
    auto CompareWeight = [this](std::pair<unsigned long, unsigned long> const& p1, std::pair<unsigned long, unsigned long> const& p2) -> bool {
        return static_cast<double>(1 / this -> adj_list[p1.first][p1.second]) > static_cast<double>(1 / this -> adj_list[p2.first][p2.second]);
    };
    
    std::priority_queue<std::pair<unsigned long, unsigned long>, 
                        std::vector<std::pair<unsigned long, unsigned long>>, 
                        decltype(CompareWeight)> queue(CompareWeight);
                        
    // std::set<unsigned long> visited;
    std::unordered_map<unsigned long, unsigned long> previous;
    // distance_map[start] = 0;
    std::unordered_map<unsigned long, double> distance;
    for (auto i : adj_list) {
        distance[i.first] = DIJKSTRA_INIT;
    }
    previous[start] = start;
    distance[start] = 0;
    
    // visited.insert(start);
    /*
    for (weighted_edge edge : graph.at(start)) {
        queue.push(edge);
    }
    */
    bool flag = true;
    while (!queue.empty() && previous.size() != num_nodes) {
        std::pair<unsigned long, unsigned long> cur = queue.top();
        queue.pop();
        if (cur.second == dest) {
            flag = false;
        }
        if (previous.find(cur.second) != previous.end()) {
            for (auto other : adj_list.at(cur.second)) {
                queue.push(std::pair<unsigned long, unsigned long>(cur.second, other.first));
            }
        }
        if (distance.at(cur.first) + static_cast<double>(1 / adj_list[cur.first][cur.second]) < distance.at(cur.second)) {
            distance.at(cur.second) = distance.at(cur.first) + static_cast<double>(1 / adj_list[cur.first][cur.second]);
            previous[cur.second] = cur.first;
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