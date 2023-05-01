#define CATCH_CONFIG_MAIN
#include <catch2/catch_test_macros.hpp>
#include "../src/utils.cpp"
#include "../src/journalGraph.h"
#include "../src/authorGraph.h"
#include "../Dataset/parsing.cpp"


bool verify_graph_properties() {
    return false;
}

bool has_whitespace(std::string& test) {
    if (test.size() < 2) {
        return false;
    }

    if (test[0] == ' ' || test[test.length() - 1] == ' ') {
        return false;
    }
    return true;
}

bool verify_valid_parsed_data(std::vector<std::vector<std::string>>& parsed_references) {
    //ensure every line has references
    for (auto& references : parsed_references) {
        if (references.size() < 2) {
            return false;
        }
        for (auto& reference : references) {
            if (has_whitespace(reference)) {
                return false;
            }
        }
    }
    return true;
    //ensure no whitespace
}

TEST_CASE("Ensure Parser Works as Intended") {
    // Test code goes here
    std::vector<std::vector<unsigned long>> parsed_references;
    REQUIRE(parse_references(parsed_references, "../data/dblp.v12.json"));
}

TEST_CASE("Ensure valid AuthorGraph") {
    std::vector<author_parse_wrapper> node_data; 
    parse_authors(node_data, "../data/dblp.v12.json");
    AuthorGraph g(node_data);
    g.print_graph();
}