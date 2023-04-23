#include <iostream>
#include "../Dataset/parsing.cpp"
#include "journalGraph.cpp"
#define exit_failure 0
#define exit_success 1

int main() {
    std::cout << "CS225 Project by Daniel Zhang, Ian Zhang, Kevin Chen, and Jenny Hu" << "\n";
    std::vector<std::vector<std::string>> parsed_info;
    int success = parse_references(parsed_info, "../data/dfs_test.json");

    if (!success) {
        return exit_failure;
    }
    std::cout << __LINE__ << std::endl;

    journalGraph g(parsed_info);
    g.print();
    std::vector<std::string> answer = g.getIdeaHistory("53e99804b7602d97020196b8");
    
    for (std::string& id : answer) {
        std::cout << id << "\n";
    }

    return exit_success;
}