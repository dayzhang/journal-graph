#pragma once
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <stack>

/**
    This class defines the journalGraph. It is built in a std::unordered_map, which maps an id to a bucket of referenced ids

    Constant time searching of nodes allows for the maximum speed DFS at O(V+E). It is notable, however, that the tradeoff is space efficiency as hash tables often take up more space than the amount of elements it holds (based on the load factor)

    IDs were determined to all be less than an the max unsigned int, so we took the liberty in reducing each ID's storage by half from a long.
*/


class journalGraph {
/**
    The structure of the graph for fast lookups. It maps IDs to a bucket of referenced IDs
*/

    std::unordered_map<unsigned int, std::unordered_set<unsigned int>> graph;

/**
    A count of the number of nodes
*/

    size_t nodes_;

public:
/**
 * Default constructor is initialized with build/journalgraph.bin
*/
    journalGraph();

/**
 * Used for testing pre-modeled data using a custom parser.
 * @param node_data Parsed based on the parsers in dataset/parsing.cpp
*/
    journalGraph(const std::vector<std::vector<unsigned int>>& node_data);

/**
 * Used for testing pre-modeled data using a custom parser.
 * @param filename Parsed based on the parsers in dataset/parsing.cpp
*/
    journalGraph(const std::string& filename);

/**
 * Tells the user that the journal graph is closing. This is done because it can take some time for it to destruct.
*/
    ~journalGraph();

/**
 * Helper to add an edge between two ids from id1 to id2
 * @param id1 Source id
 * @param id2 Destination id
 * @return bool determining if it was successful
*/
    bool addEdge(unsigned int id1, unsigned int id2); //return false if fails to add

/**
 * Functions to find the idea history given an article using DFS
 * @param source Source id
 * @return pairs of nodes connected from first to second
*/
    std::vector<std::pair<unsigned int, unsigned int>> getIdeaHistory(const unsigned int& source);

/**
 * Helper function for getIdeaHistory
 * @param vertex current id
 * @param seen seen set
 * @param record a record of papers traversed so far
*/
    void dfs(const unsigned int& vertex, std::unordered_map<unsigned int, bool>& seen, std::vector<std::pair<unsigned int, unsigned int>>& record);

/**
 * Prints graph. See implementation for details
*/
    void print();

/**
 * Gets graphed. Used for debugging and tests cases.
*/
    std::unordered_map<unsigned int, std::unordered_set<unsigned int>> getGraph();

/**
 * Write the graph to a file such that it can be reused
 * @param filename file destination
*/
    void export_to_file(const std::string& filename);

/**
 * Returns the neighbors of a node
 * @param source node
 * @return a set of adjacent nodes
*/
    const std::unordered_set<unsigned int>& get_neighbors(unsigned int node) const ;

/**
 * Returns if the node is in the graph
 * @param source node
 * @return Whether or not it was found
*/
    bool in_graph(unsigned int node) const;

};