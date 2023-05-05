#pragma once
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <limits>
#include <queue>
#include <algorithm>
#include <stack>
#include "../graph/graph_defines.h"
/**
 * Limiting the amount of referenced authors added to 8
*/
#define AUTHOR_EDGE_LIMIT 8

/**
 * Base weight given to an in-paper connection is 10
*/
#define same_paper_weight 10

/**
 * Base weight given to a reference connection from referenced to original is 1
*/
#define ref_author_weight_ref 1

/**
 * Base weight given to a reference connection from original to referenced is 2. Directed towards referenced.
*/
#define ref_author_weight_orig 2

/**
 * Constant maximum for Dijkstra's algorithm distance initialization
*/
#define DIJKSTRA_INIT 100000000;

/**
 * Alias for unvisited in Tarjans
*/
#define unvisited -1

class AuthorGraph {
/**
 * Graph structure. Defined to allow for constant-time updates of weights. Maps an ID to an ID which maps to a weight.
 * Weighing is defined by the constants above, as well as the amount of citations a paper has. See implementation for details
 * The graph is subsetted such that each author can only add 8 other authors at a time. Otherwise, connections would be upwards of 45 million citations * >5 milion authors
*/
    std::unordered_map<unsigned long, std::unordered_map<unsigned long, int>> adj_list;

/**
 * Number of nodes in the graph
*/
    unsigned long num_nodes;

private:

/**
 * Datatype used to conserve space in Tarjan's algorithm. Typically, it is stored using three different containers. Combining it to one reduced the amount of unsigned long keys to store by 3x to conserve stack space.
*/
    struct tarjans_t {
        int disc;
        int low_link;
        bool on_stack;
        tarjans_t() {
            //disc, low_link, set to unvisited.
            disc = unvisited;
            low_link = unvisited;
            on_stack = false;
        }
    };

/**
 * Helper that returns a vector of Strongly Connected Components
 * @param tarjans_data utility map that keeps track of low link values, disc values and on stack values
 * @param scc_stack stack that keeps track of the current strongly connected component
 * @param id the algorithm keeps track of an id, which is assigned dynamically and starts at 0
 * @return Strongly Connected Components
*/
    std::vector<std::vector<unsigned long>> findSCC(std::unordered_map<unsigned long, tarjans_t>& tarjans_data, std::stack<unsigned long>& scc_stack, int& id);

/**
 * Recursive helper than performs the majority of the algorithm
 * @param ans Stores all found SCCs
 * @param current_id Stores the node for recursion
 * @param tarjans_data utility map that keeps track of low link values, disc values and on stack values
 * @param scc_stack stack that keeps track of the current strongly connected component
 * @param id the algorithm keeps track of an id, which is assigned dynamically and starts at 0
 * @return Strongly Connected Components
*/
    void tarjansSearch(std::vector<std::vector<unsigned long>>& ans, const unsigned long& current_id, std::unordered_map<unsigned long, tarjans_t>& tarjans_data, std::stack<unsigned long>& scc_stack, int& id);

public:

/**
 * Constructs default using the author_graph.bin file in build
*/
    AuthorGraph() { AuthorGraph("author_graph.bin"); } 

/**
 * Constructs authorGraph using a filename
 * @param filename Path to construction file
*/
    AuthorGraph(const std::string& filename);

/**
 * Tells the user that the graph is closing. Used because destruction can take awhile
*/
    ~AuthorGraph();

/**
 * Helper to add an edge
 * @param weight adds weight from source to destination
 * @param source source node
 * @param dest destination node
*/
    void addEdge(int weight, long source, long dest);

/**
 * Adds authors that are in the same paper
 * @param authors_in_paper a vector of all the authors within a paper.
 * @param n_citation used to determine the important of the paper for weighing
*/
    void add_same_paper_authors(const std::vector<unsigned long>& authors_in_paper, unsigned int n_citation);

/**
 * Adds authors that are in the same paper
 * @param authors_in_paper a vector of all the authors within a paper.
 * @param authors_referenced a max size 8 array of authors that were referenced from a paper
 * @param n_citation_paper credibility of a paper, used for weighing
 * @param n_citation_ref credibility of the referenced paper, used for weighing
*/
    void add_referenced_authors(const std::vector<unsigned long>& authors_in_paper, const std::array<long, 8>& authors_referenced, unsigned int n_citation_paper, unsigned int n_citation_ref) ;

/**
 * Performs Dijkstras algorithm from a given node to a destination node
 * @param start The starting node to traverse from
 * @param dest Te destination node to find
 * @return shortest path between the two node
*/
    std::vector<unsigned long> dijkstrasShortestPath(const unsigned long& start, const unsigned long& dest);

/**
 * Gets all strongly connected components in the graph. Limited to a recursion limit of 1024
 * Ensures that the largest SCC for each node is found using a DFS-based traversal
 * @return vector of strongly connected components
*/
    std::vector<std::vector<unsigned long>> tarjansSCC();

/**
 * Gets strongly connected components in the graph connected to the query. Limited to a recursion limit of 1024
 * Ensures that the largest SCC for each node is found using a DFS-based traversal
 * @param query node to run Tarjans from
 * @return vector of strongly connected components
*/
    std::vector<std::vector<unsigned long>> tarjansSCC_with_query(const unsigned long& query);

/**
 * Writes the graph to a file to reuse
 * @param filename destination to write to
*/
    void export_to_file(const std::string& filename);

//For Testing Only

/**
 * Prints the graph for debugging
*/
    void print_graph();

/**
 * Helper to get graph for testing
 * @return the adjacency map graph
*/
    std::unordered_map<unsigned long, std::unordered_map<unsigned long, int>>& getGraph() { return adj_list; };

/**
 * Constructor to test on smaller sets of data
 * @param node_data small (<10MB) parsed data as defined in dataset/parsing.cpp
*/
    AuthorGraph(const std::vector<author_parse_wrapper>& node_data);
};