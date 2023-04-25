#pragma once
#include <iostream>
#include <utility>
#include <vector>
#include "../Dataset/parsing.cpp"
#include "journalGraph.cpp"
#define exit_failure 0
#define exit_success 1

bool print_parsed_references() {
    std::vector<std::vector<std::string>> parsed_info;

    if (!parse_references(parsed_info, "../data/dfs_test.json")) {
        return exit_failure;
    }

    for (auto& entry : parsed_info) {
        std::cout << "Paper ID: " << entry[0] << " with references:\n";
        for (int paper_num = 1; paper_num < entry.size(); paper_num++) {
            std::cout << entry[paper_num] << " ";
        }

        std::cout << "\n\n\n\n";
    }

    return exit_success;
}

bool print_parsed_authors() {
    std::vector<std::pair<std::vector<std::string>, std::vector<std::string>>> parsed_info;

    if (!parse_authors(parsed_info, "../data/sample_data_unserialized.json")) {
        return exit_failure;
    }

    for (auto& entry : parsed_info) {
        std::cout << "Paper ID: " << entry.first[0] << "with references and authors:\n";
        for (int i = 1; i < entry.first.size(); i++) {
            std::cout << entry.first[i] << " | ";
        }
        std::cout << "authors --------- ";
        for (int i = 0; i < entry.second.size(); i++) {
            std::cout << entry.second[i] << " ";
        }

        std::cout << "\n\n\n\n";
    }

    return exit_success;
}

bool run_dfs() {
    std::cout << "CS225 Project by Daniel Zhang, Ian Zhang, Kevin Chen, and Jenny Hu" << "\n";
    std::vector<std::vector<std::string>> parsed_info;
    int success = parse_references(parsed_info, "../data/dfs_test.json");

    if (!success) {
        return exit_failure;
    }

    journalGraph g(parsed_info);

    std::vector<std::string> answer = g.getIdeaHistory("53e99804b7602d97020196b8");

    for (std::string& id : answer) {
        std::cout << id << "\n";
    }

    return exit_success;
}