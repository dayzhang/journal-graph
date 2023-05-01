#include <iostream>
#include <utility>
#include "utils.cpp"
#include "../Dataset/parsing.cpp"
#include "journalGraph.h"
#include "tarjansSCC.cpp"
#define exit_failure 0
#define exit_success 1

int main() {
    std::cout << "CS225 Project by Daniel Zhang, Ian Zhang, Kevin Chen, and Jenny Hu" << "\n";
    std::vector<author_parse_wrapper> p;
    parse_authors(p, "../data/tarjanstest.json");
    AuthorGraph g(p);
    std::vector<std::vector<unsigned long>> scc = g.tarjansSCC();
    for (auto& i : scc) {
        std::cout << "Strongly Connected Component | ";
        for (auto& entry : i) {
            std::cout << entry << " - ";
        }
        std::cout << "\n";
    }
    return exit_success;
}