#include "authorGraph.h"
#include <stack>
#include <utility>
#include <algorithm>
const int unvisited = -1;
using static_map = std::unordered_map<unsigned long, int>;
using auth = unsigned long;

std::vector<std::vector<unsigned long>> AuthorGraph::tarjansSCC() {
    int id = 0;

    static_map id_array;
    static_map low_link;
    std::unordered_map<auth, bool> on_stack;

    for (auto& ids : graph) {
        id_array[ids.first] = unvisited;
        low_link[ids.first] = 0;
        on_stack[ids.first] = false;
    }

    std::stack<unsigned long> scc_stack;
    return findSCC(id_array, low_link, on_stack, scc_stack, id);
}

std::vector<std::vector<unsigned long>> AuthorGraph::findSCC(static_map& id_array, static_map& low_link, std::unordered_map<auth, bool>& on_stack, std::stack<unsigned long>& scc_stack, int& id) {
    std::vector<std::vector<unsigned long>> all_SCCs;
    for (auto& ids : graph) {
        if (id_array[ids.first] == unvisited) {
            tarjansSearch(all_SCCs, ids.first, id_array, low_link, on_stack, scc_stack, id);
        }
    }
    return all_SCCs;
}

void AuthorGraph::tarjansSearch(std::vector<std::vector<unsigned long>>& ans, int current_id, static_map& id_array, static_map& low_link, std::unordered_map<auth, bool>& on_stack, std::stack<unsigned long>& scc_stack, int& id) {
    scc_stack.push(current_id);
    on_stack[current_id] = true;
    id_array[current_id] = low_link[current_id] = id++;
    
    for (weighted_edge& edge : graph[current_id]) {
        auto& adj = edge.destination;
        if (id_array[adj] == unvisited) {
            tarjansSearch(ans, adj, id_array, low_link, on_stack, scc_stack, id);
            low_link[current_id] = std::min(low_link[current_id], low_link[adj]);
        } else if (on_stack[adj]) {
            low_link[current_id] = std::min(low_link[current_id], id_array[adj]);
        }
    }

    if (id_array[current_id] == low_link[current_id]) {
        std::vector<unsigned long> strongly_connected;
        while (true) {
            unsigned long node = scc_stack.top();
            scc_stack.pop();
            on_stack[node] = false;
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