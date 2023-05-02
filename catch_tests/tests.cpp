#define CATCH_CONFIG_MAIN
#include <catch2/catch_test_macros.hpp>
#include "../graph/utils.cpp"
#include "../graph/journalGraph.h"
#include "../graph/authorGraph.h"
#include "../graph/tarjansSCC.cpp"
#include "../dataset/parsing.cpp"

#include <unordered_set>

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

const std::unordered_set<unsigned long> tarjans_test_set1({2142249029, 2113592602, 2103626414, 2117665592, 2023460672, 2174205032, 2022192081});

TEST_CASE("TarjansTest 1") {
    std::vector<author_parse_wrapper> values;
    parse_authors(values, "../data/tarjanstest.json");
    AuthorGraph g(values);
    auto ans = g.tarjansSCC_with_query(2142249029);

    REQUIRE(ans.size() == 1);
    std::sort(ans.begin(), ans.end());
    for (auto& scc : ans) {
        for (auto& id : scc) {
            REQUIRE(tarjans_test_set1.find(id) != tarjans_test_set1.end());
        }
    }
}

const std::unordered_set<unsigned long> tarjans_test_set2({2425818370, 2126056503, 2308774408, 2300589394});

TEST_CASE("TarjansTest 2") {
    std::vector<author_parse_wrapper> values;
    parse_authors(values, "../data/tarjanstest.json");
    AuthorGraph g(values);
    auto ans = g.tarjansSCC();
    g.print_graph();
    for (auto& i : ans) {
        std::cout << "Strongly Connected Component | ";
        for (auto& entry : i) {
            std::cout << entry << " - ";
        }
        std::cout << "\n";
    }
    REQUIRE(ans.size() == 2); // 2 SCCS
    std::sort(ans.begin(), ans.end());
    for (auto& scc : ans) {
        if (scc.size() >= 7) {
            for (auto& id : scc) {
                REQUIRE(tarjans_test_set1.find(id) != tarjans_test_set1.end());
            }
        } else if (scc.size() >= 4) {
            for (auto& id : scc) {
                REQUIRE(tarjans_test_set2.find(id) != tarjans_test_set1.end());
            }
        }
    }
}

TEST_CASE("TarjansTestFull") {

}