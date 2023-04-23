#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
class authorGraph {
    
private:
    struct weighted_edge {
        float weight;
        //temporary: currently, ID's are represeted as strings;
        int num_times_cited;
        std::string destination;
        weighted_edge(float _weight, std::string _destination) : weight(_weight), destination(_destination) {}
    };
    size_t nodes_;
    std::unordered_map<std::size_t, std::vector<weighted_edge>> graph_;//vector-based adjacency list to take advantage of cache
    //A lot of performance issues with this as of now. Should consider switching to map-based implementations
    //For storage, will be important to consider a re-id to store as integers

public:

    authorGraph() = default; // subsetting author graphs: Only consider most significant (first) author for each paper
    authorGraph(const std::vector<std::vector<std::string>>& node_data);
    authorGraph(authorGraph& other_graph);
    ~authorGraph();
    bool addEdge(std::string id1, std::string id2); //return false if fails to add
    bool adjustWeight(const std::string& id1, const std::string& id2); // adjust the weighted edge from id1 to id2. Returns false if fails to adjust

    std::vector<std::string> dijkstrasShortestPath(const std::string& start, const std::string& dest);
};