#include <iostream>
#include <utility>
#include "utils.cpp"
#include "../Dataset/parsing.cpp"
#include "journalGraph.h"
#define exit_failure 0
#define exit_success 1

int main() {
    std::cout << "CS225 Project by Daniel Zhang, Ian Zhang, Kevin Chen, and Jenny Hu" << "\n";
    run_dfs();
    return exit_success;
}