#include "journalGraph.h"

void journalGraph::print() {
    for (auto& pair : graph) {
        std::cout << "Start Node: " << pair.first << " | " << std::endl;
        for (unsigned long other : graph.at(pair.first)) {
            std::cout << other << " ";
        }
        std::cout << std::endl;
    }
}

bool journalGraph::addEdge(unsigned long id1, unsigned long id2) {
    if (id1 == 0 || id2 == 0) {
        return false;
    }

    if (graph.find(id2) == graph.end()) {
        graph.insert({id2, std::vector<unsigned long>()});
    }

    graph[id1].push_back(id2);
    return true;
} 

journalGraph::journalGraph(const std::vector<std::vector<unsigned long>>& node_data) {
    //node_data is organized as 1st index = source, subsequent index are its references
    unsigned long source_index = 0;
    for (const auto& entry : node_data) {
        unsigned long sourced_id = entry[source_index];
        for (unsigned long reference_index = 1; reference_index < entry.size(); reference_index++) {
            if (!addEdge(sourced_id, entry[reference_index])) {
                std::cout << "bad id found at source " << sourced_id << " with ID: " << entry[reference_index];
            }
        }
    }
    
}

struct traversal_element {
    unsigned long parent;
    unsigned long child;
    traversal_element(unsigned long _parent, unsigned long _child) : parent(_parent), child(_child) {};
};
void journalGraph::dfs(const unsigned long& vertex, std::unordered_map<unsigned long, bool>& seen, std::vector<std::pair<unsigned long, unsigned long>>& record) { 
    // not entirely sure what the dfs is supposed to be for, but changed it to iterative so it doesn't exceed recursion limit
    std::stack<traversal_element> node_stack;
    node_stack.push({0, vertex});
    while (!node_stack.empty()) {
        traversal_element current_node = node_stack.top();
        node_stack.pop();
        if (!seen.at(current_node.child)) {
            seen.at(current_node.child) = true;
            record.push_back(std::make_pair(current_node.parent, current_node.child));
        }
        for (unsigned long other : graph.at(current_node.child)) {
            if (!seen.at(other)) {
                node_stack.push({current_node.child, other});
            }
        }
    }
}

/*
// not entirely sure what the dfs is supposed to be for, but changed it to iterative so it doesn't exceed recursion limit
void journalGraph::dfs(const unsigned long& start_node, std::vector<size_t>& record) {
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
*/


std::vector<std::pair<unsigned long, unsigned long>> journalGraph::getIdeaHistory(const unsigned long& source) {
    if (graph.find(source) == graph.end()) {
        std::cout << "source: " << source << " not found in database.\n";
        return std::vector<std::pair<unsigned long, unsigned long>>();
    }

    std::unordered_map<unsigned long, bool> seen;
    for (auto& key : graph) {
        seen[key.first] = false;
    }
    std::vector<std::pair<unsigned long, unsigned long>> record;
    dfs(source, seen, record);
    return record;
}
