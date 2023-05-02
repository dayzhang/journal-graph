#include "authorGraph.h"
#include <stack>
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

    for (auto& ids : adj_list) {
        tarjans_data[ids.first] = tarjans_t();
    }

    std::stack<unsigned long> scc_stack;
    return findSCC(tarjans_data, scc_stack, id);
}

std::vector<std::vector<unsigned long>> AuthorGraph::findSCC(std::unordered_map<unsigned long, tarjans_t>& tarjans_data, std::stack<unsigned long>& scc_stack, int& id) {

    std::vector<std::vector<unsigned long>> all_SCCs;

    for (auto& ids : adj_list) {
        if (tarjans_data[ids.first].disc == unvisited) {
            tarjansSearch(all_SCCs, ids.first, tarjans_data, scc_stack, id);
        }
    }

    return all_SCCs;
}

void AuthorGraph::tarjansSearch(std::vector<std::vector<unsigned long>>& ans, const unsigned long& current_id, std::unordered_map<unsigned long, tarjans_t>& tarjans_data, std::stack<unsigned long>& scc_stack, int& id) {

    bool& _on_stack = tarjans_data[current_id].on_stack;
    int& _low_link = tarjans_data[current_id].low_link;
    int& _disc = tarjans_data[current_id].disc;

    scc_stack.push(current_id);
    _on_stack = true;
    _disc = _low_link = id++;
    
    for (std::pair<const unsigned long, int>& edge : adj_list[current_id]) {
        auto& adj = edge.first;
        if (tarjans_data[adj].disc == unvisited) {
            tarjansSearch(ans, adj, tarjans_data, scc_stack, id);
            _low_link = std::min(_low_link, tarjans_data[adj].low_link);
        } else if (tarjans_data[adj].on_stack) {
            _low_link = std::min(_low_link, tarjans_data[adj].disc);
        }
    }

    unsigned long node = 0;
    if (_disc == _low_link) {
        std::vector<unsigned long> strongly_connected;

        while (scc_stack.top() != current_id) { //while the stack is not at ID
            node = scc_stack.top();
            strongly_connected.push_back(node);
            tarjans_data[node].on_stack = false;
            scc_stack.pop();
        }
        strongly_connected.push_back(current_id);
        _on_stack = false;
        scc_stack.pop();
        if (strongly_connected.size() > 1) {
            ans.push_back(strongly_connected);
        }
    }
}

std::vector<std::vector<unsigned long>> AuthorGraph::tarjansSCC_with_query(const unsigned long& query) {

    if (adj_list.find(query) == adj_list.end()) {
        std::cout << "did not find " << query << " in database\n";
    }
    int id = 0;

    std::unordered_map<unsigned long, tarjans_t> tarjans_data;

    for (auto& ids : adj_list) {
        tarjans_data[ids.first] = tarjans_t();
    }

    std::stack<unsigned long> scc_stack;

    std::vector<std::vector<unsigned long>> all_SCCs;

    tarjansSearch(all_SCCs, query, tarjans_data, scc_stack, id);

    return all_SCCs;
}