#pragma once
#include <iostream>
#include <utility>
#include <vector>
#include "../dataset/parsing.cpp"
#include "journalGraph.h"
#include "authorGraph.h"
#define exit_failure 0
#define exit_success 1

bool print_parsed_references() {
    std::vector<std::vector<unsigned long>> parsed_info;

    if (!parse_references(parsed_info, "../data/dfs_test.json")) {
        return exit_failure;
    }

    for (auto& entry : parsed_info) {
        std::cout << "Paper ID: " << entry[0] << " with references:\n";
        for (unsigned long paper_num = 1; paper_num < entry.size(); paper_num++) {
            std::cout << entry[paper_num] << " ";
        }

        std::cout << "\n\n\n\n";
    }

    return exit_success;
}

bool print_parsed_authors() {
    std::vector<author_parse_wrapper> parsed_info;

    if (!parse_authors(parsed_info, "../data/sample_data_unserialized.json")) {
        return exit_failure;
    }

    for (auto& entry : parsed_info) {
        std::cout << "Paper ID: " << entry.source << "with references and authors:\n";
        for (unsigned long i = 0; i < entry.cited.size(); i++) {
            std::cout << entry.cited[i] << " | ";
        }
        std::cout << "authors --------- ";
        for (unsigned long i = 0; i < entry.authors.size(); i++) {
            std::cout << entry.authors[i] << " ";
        }

        std::cout << "\n\n\n\n";
    }

    return exit_success;
}

bool run_dfs() {
    std::cout << "CS225 Project by Daniel Zhang, Ian Zhang, Kevin Chen, and Jenny Hu" << "\n";
    std::vector<std::vector<unsigned long>> parsed_info;
    int success = parse_references(parsed_info, "../data/dblp.v12.json");

    if (!success) {
        return exit_failure;
    }

    journalGraph g(std::string("../data/dblp.v12.json"));

    std::vector<std::pair<unsigned long, unsigned long>> answer = g.getIdeaHistory(86197);

    for (auto& id : answer) {
        std::cout << id.second << " referenced by " << id.first << "\n";
    }

    return exit_success;
}
