#pragma once
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <stack>

class journalGraph {

    std::unordered_map<unsigned long, std::vector<unsigned long>> graph;//vector-based adjacency list to take advantage of cache
    //A lot of performance issues with this as of now. Should consider switching to map-based implementations
    //For storage, will be important to consider a re-id to store as integers
    std::unordered_map<std::string, size_t> name_to_id_;
    std::unordered_map<size_t, std::string> id_to_name_;
    size_t nodes_;

public:

    journalGraph() = default; // subsetting author
    journalGraph(const std::vector<std::vector<unsigned long>>& node_data);
    ~journalGraph() = default;
    bool addEdge(unsigned long id1, unsigned long id2); //return false if fails to add

    std::vector<std::pair<unsigned long, unsigned long>> getIdeaHistory(const unsigned long& source);
    void dfs(const unsigned long& vertex, std::unordered_map<unsigned long, bool>& seen, std::vector<std::pair<unsigned long, unsigned long>>& record);

    void print();

};