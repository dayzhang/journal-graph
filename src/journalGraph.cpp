#include "journalGraph.h"

bool journalGraph::addEdge(std::string id1, std::string id2) {
    if (id1.empty() || id2.empty()) {
        return false;
    }
    /*
    if (name_to_id_.find(id1) == name_to_id_.end()) {
        name_to_id_[id1] = nodes_;
        id_to_name_[nodes_] = id1;
        graph_[nodes]
        nodes_++;
    }
    */
    if (name_to_id_.find(id2) == name_to_id_.end()) {
        name_to_id_[id2] = nodes_;
        id_to_name_[nodes_] = id2;
        graph_[nodes_] = std::vector<size_t>();
        nodes_++;
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
        i++;
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


void journalGraph::dfs(const std::string& vertex, const std::string& pred, std::unordered_map<std::string, int>& seen, std::vector<std::string>& record) { // note that due to the prescence of directed graph + tree structure, cycle should not be possible
    seen[vertex] = seen[pred] + 1;

    record.push_back(vertex + " Cited by: " + pred);

    for (std::string& adjacent : graph[vertex]) {
        if (seen.find(adjacent) == seen.end()) {
            dfs(adjacent, vertex, seen, record);
        }
    }
}

void journalGraph::dfs_iterative(const std::string& vertex, const std::string& pred, std::unordered_map<std::string, int>& seen, std::vector<std::string>& record) { // note that due to the prescence of directed graph + tree structure, cycle should not be possible
    seen[vertex] = seen[pred] + 1;

    record.push_back(vertex + " Cited by: " + pred);

    for (std::string& adjacent : graph[vertex]) {
        if (seen.find(adjacent) == seen.end()) {
            dfs(adjacent, vertex, seen, record);
        }
    }
}
*/
// not entirely sure what the dfs is supposed to be for, but changed it to iterative so it doesn't exceed recursion limit
void journalGraph::dfs(const size_t& start_node, std::vector<size_t>& record) {
    // std::cout << __LINE__ << std::endl;

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
void journalGraph::print() {
    for (size_t start = 0; start < nodes_; start++) {
        std::cout << "Start Node: " << start << std::endl;
        for (size_t other : graph_.at(start)) {
            std::cout << other << " ";
        }
        std::cout << std::endl;
    }
}

std::vector<std::string> journalGraph::getIdeaHistory(const std::string& source) {
    if (graph_.find(name_to_id_.at(source)) == graph_.end()) {
        std::cout << "source: " << source << " not found in database.\n";
        return std::vector<std::string>();
    }

<<<<<<< HEAD
    std::vector<size_t> record;
    // std::cout << __LINE__ << std::endl;

    dfs(name_to_id_.at(source), record);
    // std::cout << __LINE__ << std::endl;

    std::vector<std::string> out;
    for (size_t id : record) out.push_back(id_to_name_.at(id));
    return out;
=======
    std::unordered_map<std::string, int> seen;
    std::string root("root");
    seen[root] = -1;
    std::vector<std::string> record;
    dfs(source, root, seen, record);
    return record;
>>>>>>> kevin-develop
}