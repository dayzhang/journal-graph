#pragma once
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#define author_edge_limit 3
#define same_paper_weight float(1) / float(5)
#define ref_author_weight 5

class AuthorGraph {

    struct weighted_edge {
        float weight;
        //temporary: currently, ID's are represeted as strings;
        std::string destination;
        weighted_edge(float _weight, std::string _destination) : weight(_weight), destination(_destination) {}
    };

    std::unordered_map<std::string, std::vector<weighted_edge>> graph;

public:

    AuthorGraph(); // subsetting author graphs: Only consider most significant (first) author for each paper
    AuthorGraph(const std::vector<std::pair<std::vector<std::string>, std::vector<std::string>>>& node_data);
    AuthorGraph(AuthorGraph& other_graph);
    ~AuthorGraph() = default;
    bool addEdge(float weight, const std::string& id1, const std::string& id2); //return false if fails to add
    void add_same_paper_authors(const std::vector<std::string>& authors_in_paper);
    void add_referenced_authors(const std::vector<std::string>& authors_in_paper, const std::vector<std::string>& authors_referenced);

    void print_graph();
    std::unordered_map<std::string, std::vector<weighted_edge>> getGraph();

    std::vector<std::string> dijkstrasShortestPath(const std::string& start, const std::string& dest);
};