#include "bfs.h" 

Graph::Graph(int v) {
    this->v = v;
    adj.resize(v);
}
 
void Graph::addEdge(int id1, int id2) {
    adj[id1].push_back(id2); // Add w to v’s list.
}
 
void Graph::BFS(int id) {
    // Mark all the vertices as not visited
    vector<bool> visited;
    visited.resize(V, false);
 
    // Create a queue for BFS
    list<int> l;
 
    // Mark the current node as visited and enqueue it
    visited[id] = true;
    l.push_back(id);
 
    while (!l.empty()) {
        // Dequeue a vertex from queue and print it
        id = l.front();
       // cout << id << " ";
        l.pop_front();
 
        // Get all adjacent vertices of the dequeued
        // vertex s. If a adjacent has not been visited,
        // then mark it visited and enqueue it
        for (auto adjacent : adj[id]) {
            if (!visited[adjacent]) {
                visited[adjacent] = true;
                l.push_back(adjacent);
            }
        }
    }
}
 