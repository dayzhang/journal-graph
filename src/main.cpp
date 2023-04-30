#include <iostream>
#include <utility>
#include "utils.cpp"
#include "../Dataset/parsing.cpp"
#include "journalGraph.h"
#define exit_failure 0
#define exit_success 1

int main() {
    std::cout << "CS225 Project by Daniel Zhang, Ian Zhang, Kevin Chen, and Jenny Hu" << "\n";
    std::vector<std::vector<long>> parsed_data;
    parse_references_v12(parsed_data, "../data/sample_data_unserialized.json");
    for (auto a : parsed_data) {
        if (!a.empty()) {
            std::cout << a[0] << " | ";
        }
        for (int i = 1; i < a.size(); i++) {
            std::cout << a[i] << " ";
        }

        std::cout << "\n";
    }
    return exit_success;
}