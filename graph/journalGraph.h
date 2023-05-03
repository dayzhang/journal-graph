#pragma once
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <stack>

class journalGraph {

    std::unordered_map<unsigned int, std::unordered_set<unsigned int>> graph;//vector-based adjacency list to take advantage of cache
    //A lot of performance issues with this as of now. Should consider switching to map-based implementations
    //For storage, will be important to consider a re-id to store as integers
    std::unordered_map<std::string, size_t> name_to_id_;
    std::unordered_map<size_t, std::string> id_to_name_;
    size_t nodes_;

public:

    journalGraph() = default; // subsetting author
    journalGraph(const std::vector<std::vector<unsigned int>>& node_data);
    journalGraph(const std::string& filename);

    ~journalGraph() = default;
    bool addEdge(unsigned int id1, unsigned int id2); //return false if fails to add

    std::vector<std::pair<unsigned long, unsigned long>> getIdeaHistory(const unsigned int& source);
    void dfs(const unsigned int& vertex, std::unordered_map<unsigned long, bool>& seen, std::vector<std::pair<unsigned long, unsigned long>>& record);

    void print();

    void export_to_file(const std::string& filename);

    const std::unordered_set<unsigned int>& get_neighbors(unsigned int node) const ;

};