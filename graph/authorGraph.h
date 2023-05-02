#pragma once
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <limits>
#include <queue>
#include <algorithm>
#include <stack>
#include "../lib/graph_defines.h"

#define AUTHOR_EDGE_LIMIT 8
#define same_paper_weight 10
#define ref_author_weight_ref 1
#define ref_author_weight_orig 2
#define DIJKSTRA_INIT 100000000;

class AuthorGraph {

    // struct weighted_edge {
    //     int weight;
    //     //temporary: currently, ID's are represeted as strings;
    //     unsigned long source;
    //     unsigned long destination;
    //     weighted_edge(float _weight, unsigned long _source, unsigned long _destination) : weight(_weight), source(_source), destination(_destination) {}
    // };

    std::unordered_map<unsigned long, std::unordered_map<unsigned long, int>> adj_list;

    unsigned long num_nodes;

private:

    //Tarjan's Algorithm 
    struct tarjans_t {
        int disc;
        int low_link;
        bool on_stack;
        tarjans_t() {
            disc = -1;
            low_link = -1;
            on_stack = false;
        }
    };
    std::vector<std::vector<unsigned long>> findSCC(std::unordered_map<unsigned long, tarjans_t>& tarjans_data, std::stack<unsigned long>& scc_stack, int& id);
    void tarjansSearch(std::vector<std::vector<unsigned long>>& ans, const unsigned long& current_id, std::unordered_map<unsigned long, tarjans_t>& tarjans_data, std::stack<unsigned long>& scc_stack, int& id);

    //Graph Construction
    // bool addEdge(float weight, const unsigned long& id1, const unsigned long& id2); //return false if fails to add
    void add_same_paper_authors(const std::vector<unsigned long>& authors_in_paper);
    void add_referenced_authors(const std::vector<unsigned long>& authors_in_paper, const std::vector<unsigned long>& authors_referenced);

public:

    AuthorGraph() = default; // subsetting author graphs: Only consider most significant (first) author for each paper
    // AuthorGraph(const std::vector<author_parse_wrapper>& node_data);
    AuthorGraph(AuthorGraph& other_graph);
    AuthorGraph(const std::string& filename);

    ~AuthorGraph() = default;

    void addEdge(int weight, long source, long dest);
    void add_same_paper_authors(const std::vector<unsigned long>& authors_in_paper, unsigned int n_citation);
    void add_referenced_authors(const std::vector<unsigned long>& authors_in_paper, const std::array<long, 8>& authors_referenced, unsigned int n_citation_paper, unsigned int n_citation_ref) ;

    void print_graph();
    std::unordered_map<unsigned long, std::unordered_map<unsigned long, int>>& getGraph() { return adj_list; };

    std::vector<unsigned long> dijkstrasShortestPath(const unsigned long& start, const unsigned long& dest);
    std::vector<std::vector<unsigned long>> tarjansSCC();
    std::vector<std::vector<unsigned long>> tarjansSCC_with_query(const unsigned long& query);
    //assign lowlink w/ dfs

    void export_to_file(const std::string& filename);
};