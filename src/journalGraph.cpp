#include "journalGraph.h"

bool journalGraph::addEdge(std::string id1, std::string id2) {
    if (id1.empty() || id2.empty()) {
        return false;
    }
    graph[id1].push_back(id2);
    return true;
} 

journalGraph::journalGraph(const std::vector<std::vector<std::string>>& node_data) {
    //node_data is organized as 1st index = source, subsequent index are its references
    size_t source_index = 0;
    for (const auto& entry : node_data) {
        std::string sourced_id = entry[source_index];
        for (size_t reference_index = 1; reference_index < entry.size(); reference_index++) {
            if (!addEdge(sourced_id, entry[reference_index])) {
                std::cout << "bad id found at source " << sourced_id << " with ID: " << entry[reference_index];
            }
        }
    }
}


void journalGraph::dfs(const std::string& vertex, std::unordered_set<std::string>& seen, std::vector<std::string>& record) { // note that due to the prescence of directed graph + tree structure, cycle should not be possible
    seen.insert(vertex);

    record.push_back(vertex);
    
    for (std::string& adjacent : graph[vertex]) {
        if (seen.find(adjacent) == seen.end()) {
            dfs(adjacent, seen, record);
        }
    }
}


std::vector<std::string> journalGraph::getIdeaHistory(const std::string& source) {
    if (graph.find(source) == graph.end()) {
        std::cout << "source: " << source << " not found in database.\n";
    }

    std::unordered_set<std::string> seen;
    std::vector<std::string> record;
    dfs(source, seen, record);
    return record;
}