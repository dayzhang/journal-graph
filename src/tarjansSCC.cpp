#include "authorGraph.h"
#include <stack>
#include <utility>
#include <algorithm>
/* data type defined as
struct tarjans_t {
    int disc;
    int low_link;
    bool on_stack;
    tarjans_t() {
        disc = -1;
        low_link = -1;
        on_stack = false;
    }
};
    in authorGraph.h
*/
const int unvisited = -1;

std::vector<std::vector<unsigned long>> AuthorGraph::tarjansSCC() {
    int id = 0;

    std::unordered_map<unsigned long, tarjans_t> tarjans_data;

    for (auto& ids : graph) {
        tarjans_data[ids.first] = tarjans_t();
    }

    std::stack<unsigned long> scc_stack;
    return findSCC(tarjans_data, scc_stack, id);
}

std::vector<std::vector<unsigned long>> AuthorGraph::findSCC(std::unordered_map<unsigned long, tarjans_t>& tarjans_data, std::stack<unsigned long>& scc_stack, int& id) {
    std::vector<std::vector<unsigned long>> all_SCCs;
    for (auto& ids : graph) {
        if (tarjans_data[ids.first].disc == unvisited) {
            tarjansSearch(all_SCCs, ids.first, tarjans_data, scc_stack, id);
        }
    }
    return all_SCCs;
}

void AuthorGraph::tarjansSearch(std::vector<std::vector<unsigned long>>& ans, int current_id, std::unordered_map<unsigned long, tarjans_t>& tarjans_data, std::stack<unsigned long>& scc_stack, int& id) {
    scc_stack.push(current_id);
    bool& _on_stack = tarjans_data[current_id].on_stack;
    int& _low_link = tarjans_data[current_id].low_link;
    int& _disc = tarjans_data[current_id].disc;
    _on_stack = true;
    _disc = _low_link = id++;
    
    for (weighted_edge& edge : graph[current_id]) {
        auto& adj = edge.destination;
        if (tarjans_data[adj].disc == unvisited) {
            tarjansSearch(ans, adj, tarjans_data, scc_stack, id);
            _low_link = std::min(_low_link, tarjans_data[adj].low_link);
        } else if (tarjans_data[adj].on_stack) {
            _low_link = std::min(_low_link, tarjans_data[adj].disc);
        }
    }

    if (_disc == _low_link) {
        std::vector<unsigned long> strongly_connected;
        while (true) {
            unsigned long node = scc_stack.top();
            scc_stack.pop();
            tarjans_data[node].on_stack = false;
            strongly_connected.push_back(node);
            if (node == current_id) {
                break;
            }
        }
        if (strongly_connected.size() > 1) {
            ans.push_back(strongly_connected);
        }
    }
}