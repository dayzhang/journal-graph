#pragma once
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <limits>
#include <queue>

#include "../lib/graph_defines.h"

#define author_edge_limit 3
#define same_paper_weight float(1) / float(5)
#define ref_author_weight 5

class AuthorGraph {

    struct weighted_edge {
        float weight;
        //temporary: currently, ID's are represeted as strings;
        unsigned long destination;
        weighted_edge(float _weight, unsigned long _destination) : weight(_weight), destination(_destination) {}
    };

    std::unordered_map<unsigned long, std::vector<weighted_edge>> graph;
    int num_nodes;

public:

    AuthorGraph(); // subsetting author graphs: Only consider most significant (first) author for each paper
    AuthorGraph(const std::vector<author_parse_wrapper>& node_data);
    AuthorGraph(AuthorGraph& other_graph);
    ~AuthorGraph() = default;
    bool addEdge(float weight, const unsigned long& id1, const unsigned long& id2); //return false if fails to add
    void add_same_paper_authors(const std::vector<unsigned long>& authors_in_paper);
    void add_referenced_authors(const std::vector<unsigned long>& authors_in_paper, const std::vector<unsigned long>& authors_referenced);

    void print_graph();
    std::unordered_map<unsigned long, std::vector<weighted_edge>> getGraph();

    std::vector<unsigned long> dijkstrasShortestPath(const unsigned long& start, const unsigned long& dest);
    std::vector<std::vector<unsigned long>> tarjansSCC();
    std::vector<std::vector<unsigned long>> findSCC(std::unordered_map<unsigned long, int>& id_array, std::unordered_map<unsigned long, int>& low_link, std::unordered_map<unsigned long, bool>& on_stack, std::stack<unsigned long>& scc_stack, int& id);
    void tarjansSearch(std::vector<std::vector<unsigned long>>& ans, int current_id, std::unordered_map<unsigned long, int>& id_array, std::unordered_map<unsigned long, int>& low_link, std::unordered_map<unsigned long, bool>& on_stack, std::stack<unsigned long>& scc_stack, int& id);
    //assign lowlink w/ dfs

};