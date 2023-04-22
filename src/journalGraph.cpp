#include "journalGraph.h"

bool journalGraph::addEdge(std::string id1, std::string id2) {
    if (id1.empty() || id2.empty()) {
        return false;
    }
    graph_[name_to_id_[id1]].push_back(name_to_id_[id2]);
    return true;
} 
// using the assumption that every journal pointed to by the source is also within node_data
journalGraph::journalGraph(const std::vector<std::vector<std::string>>& node_data) {
    //node_data is organized as 1st index = source, subsequent index are its references
    size_t source_index = 0;
    nodes_ = node_data.size();
    size_t i = 0;
    // Adding all the source nodes to start with
    for (const auto& entry : node_data) {
        std::string source = entry.at(source_index);
        if (source.empty()) continue;
        name_to_id_[source] = i;
        id_to_name_[i] = source;
    }

    for (const auto& entry : node_data) {
        std::string sourced_id = entry.at(source_index);
        if (sourced_id.empty()) continue;
        for (size_t reference_index = 1; reference_index < entry.size(); reference_index++) {
            if (!addEdge(sourced_id, entry.at(reference_index))) {
                std::cout << "bad id found at source " << sourced_id << " with ID: " << entry[reference_index];
            }
        }
    }
}

/*
void journalGraph::dfs(const std::string& vertex, std::unordered_set<std::string>& seen, std::vector<std::string>& record) { // note that due to the prescence of directed graph + tree structure, cycle should not be possible
    seen.insert(vertex);

    record.push_back(vertex);
    
    for (auto& adjacent : graph_[name_to_id_[vertex]]) {
        if (seen.find(adjacent) == seen.end()) {
            dfs(adjacent, seen, record);
        }
    }
}
*/
// not entirely sure what the dfs is supposed to be for, but changed it to iterative so it doesn't exceed recursion limit
void journalGraph::dfs(const size_t& start_node, std::vector<size_t>& record) {
    std::vector<bool> seen(nodes_, false);
    std::stack<size_t> node_stack;
    node_stack.push(start_node);
    while (!node_stack.empty()) {
        size_t current_node = node_stack.top();
        node_stack.pop();
        if (!seen.at(current_node)) {
            seen.at(current_node) = true;
            record.push_back(current_node);
        }


        for (size_t other : graph_.at(current_node)) {
            if (!seen.at(other)) {
                node_stack.push(other);
            }

        }
    }
}

std::vector<size_t> journalGraph::getIdeaHistory(const size_t& source) {
    if (graph_.find(source) == graph_.end()) {
        std::cout << "source: " << source << " not found in database.\n";
        return std::vector<size_t>();
    }

    std::vector<size_t> record;
    dfs(source, record);
    return record;
}