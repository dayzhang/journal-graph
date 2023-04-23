#pragma once
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <unordered_set>
#include <stack>

class journalGraph {

private:
    std::unordered_map<size_t, std::vector<size_t>> graph_;//vector-based adjacency list to take advantage of cache
    //A lot of performance issues with this as of now. Should consider switching to map-based implementations
    //For storage, will be important to consider a re-id to store as integers
    std::unordered_map<std::string, size_t> name_to_id_;
    std::unordered_map<size_t, std::string> id_to_name_;
    size_t nodes_;

public:

    journalGraph() = default; // subsetting author
    journalGraph(const std::vector<std::vector<std::string>>& node_data);
    ~journalGraph() = default;
    bool addEdge(std::string id1, std::string id2); //return false if fails to add

    std::vector<size_t> getIdeaHistory(const size_t& source);
    // void dfs(const std::string& vertex, std::unordered_set<std::string>& seen, std::vector<std::string>& record);
    void dfs(const size_t& start_node, std::vector<size_t>& record);
    std::vector<std::string> dijkstrasShortestPath(const std::string& start, const std::string& dest);
};