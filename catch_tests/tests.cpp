#define CATCH_CONFIG_MAIN
#include <catch2/catch_test_macros.hpp>
#include "../graph/utils.cpp"
#include "../graph/journalGraph.cpp"
#include "../graph/authorGraph.cpp"
#include "../dataset/parsing.cpp"


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

TEST_CASE("Ensure References Parser Works as Intended") {
    // Test code goes here
    std::vector<std::vector<unsigned long>> parsed_references;
    REQUIRE(parse_references(parsed_references, "../data/dblp.v12.json"));
}

TEST_CASE("Ensure Author Parser works as intended") {

}

TEST_CASE("Ensure DFS works as intended") {

}

TEST_CASE("Ensure BFS works as intended") {

}

TEST_CASE("Ensure valid AuthorGraph") {
}

TEST_CASE("Ensure valid JournalGraph") {

}

TEST_CASE("Dijkstra's Test 1") {

}

TEST_CASE("Dijkstra's Test 2") {

}

TEST_CASE("Database Stores All Queries") {

}

TEST_CASE("Database successfully access all queries") {

}

TEST_CASE("TarjansTest 1") {

}

TEST_CASE("TarjansTest 2") {

}