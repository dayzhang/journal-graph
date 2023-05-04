#include "journalGraph.h"

#include <fstream>
#include <cstring>
#include <stdexcept>
#include <unordered_set>
#include <vector>
#include <iomanip>

bool journalGraph::addEdge(unsigned int id1, unsigned int id2) {
    if (id1 == 0 || id2 == 0) {
        return false;
    }
    if (graph.find(id2) == graph.end()) {
        graph[id2] = std::unordered_set<unsigned int>();
    }
    graph[id1].insert(id2);
    return true;
} 

journalGraph::journalGraph(const std::vector<std::vector<unsigned int>>& node_data) {
    //node_data is organized as 1st index = source, subsequent index are its references
    unsigned int source_index = 0;
    for (const auto& entry : node_data) {
        unsigned int sourced_id = entry[source_index];
        for (unsigned int reference_index = 1; reference_index < entry.size(); reference_index++) {
            if (!addEdge(sourced_id, entry[reference_index])) {
                std::cout << "bad id found at source " << sourced_id << " with ID: " << entry[reference_index];
            }
        }
    }
    
}

struct traversal_element {
    unsigned int parent;
    unsigned int child;
    traversal_element(unsigned int _parent, unsigned int _child) : parent(_parent), child(_child) {};
};

void journalGraph::dfs(const unsigned int& vertex, std::unordered_map<unsigned int, bool>& seen, std::vector<std::pair<unsigned int, unsigned int>>& record) { 
    
    if (graph.find(vertex) == graph.end()) {
        graph[vertex] = std::unordered_set<unsigned int>();
    }

    std::stack<traversal_element> node_stack;
    node_stack.push({0, vertex});
    while (!node_stack.empty()) {

        traversal_element current_node = node_stack.top();
        node_stack.pop();

        if (seen.find(current_node.child) == seen.end()) {
            seen[current_node.child] = false;
        }

        if (!seen.at(current_node.child)) {
            seen.at(current_node.child) = true;
            record.push_back(std::make_pair(current_node.parent, current_node.child));
        }
        
        for (unsigned int other : graph.at(current_node.child)) {

            if (seen.find(other) == seen.end()) {
                seen[other] = false;
            }

            if (!seen[other]) {
                node_stack.push({current_node.child, other});
                break;
            }
        }
    }
}

std::vector<std::pair<unsigned int, unsigned int>> journalGraph::getIdeaHistory(const unsigned int& source) {
    if (graph.find(source) == graph.end()) {
        std::cout << "source: " << source << " not found in database.\n";
        return std::vector<std::pair<unsigned int, unsigned int>>();
    }

    std::unordered_map<unsigned int, bool> seen;

    std::vector<std::pair<unsigned int, unsigned int>> record;
    dfs(source, seen, record);

    return record;
    
}

void journalGraph::export_to_file(const std::string& filename) {
    std::ofstream ofs(filename, std::ios::trunc | std::ios::binary);
    unsigned int total_size = graph.size();
    ofs.write((char*) &total_size, 4);

    for (const auto& elem : graph) {
        ofs.write((char*) &(elem.first), 4);
        unsigned int size = elem.second.size();
        ofs.write((char*)(&size), 4);
        for (const auto& edge : elem.second) {
            ofs.write((char*)(&edge), 4);
        }
    }

    ofs.close();
}

journalGraph::journalGraph(const std::string& filename) {
    std::ifstream ifs(filename, std::ios::binary);

    if (!ifs.is_open()) {
        throw std::runtime_error("error opening journal graph file");
    }

    unsigned int size = 0;
    char buff[4];
    ifs.read(buff, 4);
    memcpy(&size, buff, 4);

    for (unsigned int i = 0; i < size; ++i) {
        if (i % 100000 == 0) {
            std::cout << i << std::endl;
        }
        unsigned int id = 0;
        ifs.read(buff, 4);
        memcpy(&id, buff, 4);

        unsigned int edge_num = 0;
        ifs.read(buff, 4);
        memcpy(&edge_num, buff, 4);

        char* edges = new char[edge_num * 4];
        ifs.read(edges, edge_num * 4);

        std::vector<unsigned int> arr(edge_num);
        memcpy(arr.data(), edges, edge_num * 4);
        for (unsigned int i = 0; i < arr.size(); ++i) {
            graph[id].insert(arr.at(i));
        }

        delete[] edges;
    }

    ifs.close();
}

const std::unordered_set<unsigned int>& journalGraph::get_neighbors(unsigned int node) const {
    if (graph.find(node) == graph.end()) {
        throw std::invalid_argument("node not in graph");
    }
    return graph.at(node);
}