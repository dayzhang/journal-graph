#include <iostream>
#include <utility>
#include <chrono>

#include "../graph/utils.cpp"
#include "../graph/journalGraph.h"
#include "../graph/authorGraph.h"
#include "../graph/tarjansSCC.cpp"
#include "../graph/dijkstrasSP.cpp"
#include "../dataset/parsing.cpp"

#define exit_failure 0
#define exit_success 1

using namespace std::chrono;

int main() {
    std::cout << "CS225 Project by Daniel Zhang, Ian Zhang, Kevin Chen, and Jenny Hu" << "\n";
    std::vector<author_parse_wrapper> p;
    auto start = high_resolution_clock::now();
    AuthorGraph g("../../build/author_graph.bin");
    std::vector<std::vector<unsigned long>> scc = g.tarjansSCC_with_query(2569299913);
    for (auto& i : scc) {
        std::cout << "Strongly Connected Component | ";
        for (auto& entry : i) {
            std::cout << entry << " - ";
        }
        std::cout << "\n";
    }
    auto stop = high_resolution_clock::now();
 
    // Get duration. Substart timepoints to
    // get duration. To cast it to proper unit
    // use duration cast method
    auto duration = duration_cast<microseconds>(stop - start);
    std::cout << duration.count() << " microseconds\n";
    return exit_success;
}