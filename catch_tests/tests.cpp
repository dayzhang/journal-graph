#define CATCH_CONFIG_MAIN
#include <catch2/catch_test_macros.hpp>
#include "../src/utils.cpp"
#include "../src/journalGraph.h"


bool verify_graph_properties() {

}

bool has_whitespace(std::string& test) {
    if (test.size() < 2) {
        return false;
    }

    if (test[0] == ' ' || test[test.length() - 1] == ' ') {
        return false;
    }
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
    std::vector<std::vector<std::string>> parsed_references;
    REQUIRE(parse_references(parsed_references, "../data/sample_data_unserialized.json"));
}