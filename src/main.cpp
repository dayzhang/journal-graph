#include <iostream>

#include <utility>
#include "../graph/utils.cpp"
#include "../Dataset/parsing.cpp"
#include "../graph/journalGraph.h"
#include "../graph/tarjansSCC.cpp"
#include "../graph/dijkstrasSP.cpp"
#define exit_failure 0
#define exit_success 1

int main() {
    std::cout << "CS225 Project by Daniel Zhang, Ian Zhang, Kevin Chen, and Jenny Hu" << "\n";
    std::vector<author_parse_wrapper> p;
    parse_authors(p, "../data/dblp.v12.json");
    AuthorGraph g(p);
    std::vector<std::vector<unsigned long>> scc = g.tarjansSCC();
    for (auto& i : scc) {
        std::cout << "Strongly Connected Component | ";
        for (auto& entry : i) {
            std::cout << entry << " - ";
        }
        std::cout << "\n";
    }

    std::vector<unsigned long> dsp1 = g.dijkstrasShortestPath(581652684, 1204625948);
    if (!dsp1.empty()) {
        std::cout << "dsp1" << std::endl;

        for (unsigned long i : dsp1) {
            std::cout << i << " -> ";
        }
        std::cout << std::endl;
    } else std::cout << "Not connected" << std::endl;
    
    
    std::vector<unsigned long> dsp2 = g.dijkstrasShortestPath(1303555294, 2019241556);
    if (!dsp2.empty()) {
        std::cout << "dsp2" << std::endl;
        for (unsigned long i : dsp2) {
            std::cout << i << " -> ";
        }
        std::cout << std::endl;
    } else std::cout << "Not connected" << std::endl;

    std::vector<unsigned long> dsp3 = g.dijkstrasShortestPath(581652684, 256856704);
    if (!dsp3.empty()) {
        std::cout << "dsp3" << std::endl;
        for (unsigned long i : dsp3) {
            std::cout << i << " -> ";
        }
        std::cout << std::endl;
    } else std::cout << "Not connected" << std::endl;
    
    return exit_success;
}