#pragma once
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <unordered_set>

class journalGraph {

    std::unordered_map<std::string, std::vector<std::string>> graph;//vector-based adjacency list to take advantage of cache
    //A lot of performance issues with this as of now. Should consider switching to map-based implementations
    //For storage, will be important to consider a re-id to store as integers

public:

    journalGraph() = default; // subsetting author
    journalGraph(const std::vector<std::vector<std::string>>& node_data);
    ~journalGraph() = default;
    bool addEdge(std::string id1, std::string id2); //return false if fails to add

    std::vector<std::string> getIdeaHistory(const std::string& source);
    void dfs(const std::string& vertex, std::unordered_set<std::string>& seen, std::vector<std::string>& record);

};