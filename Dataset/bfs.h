#include <vector>
#include <iostream> 
#include <list> 

using namespace std; 
class Graph {
    int v; // number of vertices
    vector<list<int>> adj; //adjancey list

    public: 
    Graph(int v); 
    void BFS(int id); 
    void addEdge(int id1, int id2); 

}
