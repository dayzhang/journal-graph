#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
class AuthorGraph {

    struct weighted_edge {
        float weight;
        //temporary: currently, ID's are represeted as strings;
        int num_times_cited;
        std::string destination;
        weighted_edge(float _weight, std::string _destination) : weight(_weight), destination(_destination) {}
    };

    std::unordered_map<std::string, std::vector<weighted_edge>> graph;//vector-based adjacency list to take advantage of cache
    //A lot of performance issues with this as of now. Should consider switching to map-based implementations
    //For storage, will be important to consider a re-id to store as integers

public:

    AuthorGraph(); // subsetting author graphs: Only consider most significant (first) author for each paper
    AuthorGraph(const std::vector<std::vector<std::string>>& node_data);
    AuthorGraph(AuthorGraph& other_graph);
    ~AuthorGraph();
    bool addEdge(std::string id1, std::string id2); //return false if fails to add
    bool adjustWeight(const std::string& id1, const std::string& id2); // adjust the weighted edge from id1 to id2. Returns false if fails to adjust

    std::vector<std::string> dijkstrasShortestPath(const std::string& start, const std::string& dest);
};