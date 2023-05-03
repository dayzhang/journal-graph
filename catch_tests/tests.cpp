#define CATCH_CONFIG_MAIN
#include <catch2/catch_test_macros.hpp>
#include "../graph/utils.cpp"
#include "../graph/journalGraph.h"
#include "../graph/authorGraph.h"
#include "../graph/tarjansSCC.cpp"
#include "../dataset/parsing.cpp"

#include <algorithm>
#include <unordered_set>

const std::unordered_set<unsigned long> tarjans_test_set1({2142249029, 2113592602, 2103626414, 2117665592, 2023460672, 2174205032, 2022192081});

const std::unordered_set<unsigned long> tarjans_test_set2({2425818370, 2126056503, 2308774408, 2300589394});

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

bool containsAdj(std::unordered_map<unsigned long, std::unordered_map<unsigned long, int>>& graph, const unsigned long& source, const unsigned long& query) {
    auto it = graph.find(source);
    if (it == graph.end()) {
        return false;
    }
    std::unordered_map<unsigned long, int> subgraph = it -> second;
    if (subgraph.find(query) == subgraph.end()) {
        return false;
    }
    return true;
}

bool dfs(std::unordered_map<unsigned long, std::unordered_map<unsigned long, int>>& graph, const unsigned long& beginning, std::vector<unsigned long>& component, unsigned long& start) {
    if (component.empty() && containsAdj(graph, beginning, start))  {
        return true;
    }
    for (auto& adj : graph[beginning]) {
        auto it = std::find(component.begin(), component.end(), adj.first);
        if (it != component.end()) {
            component.erase(it);
            return dfs(graph, adj.first, component, start);
        }
    }
    return false;
}

bool isStronglyConnectedComponent(std::unordered_map<unsigned long, std::unordered_map<unsigned long, int>>& graph, unsigned long& beginning, std::vector<unsigned long>& component, unsigned long& start) {
    return dfs(graph, beginning, component, start);
}

bool ensureAllValidPairs(std::unordered_map<unsigned int, std::unordered_set<unsigned int>>& graph, std::vector<std::pair<unsigned int, unsigned int>>& pairs) {
    for (auto& p : pairs) {
        if (graph[p.first].find(p.second) == graph[p.first].end()) {
            return false;
        }
    }
    return true;
}

void print_dfs(std::vector<std::pair<unsigned long, unsigned long>>& ans) {
    for (auto& id : ans) {
        std::cout << id.second << " referenced by " << id.first << "\n";
    }
}

TEST_CASE("Ensure References Parser Works as Intended") {
    // Test code goes here
    std::vector<std::vector<unsigned long>> parsed_references;
    REQUIRE(parse_references(parsed_references, "../data/dblp.v12.json"));
}

TEST_CASE("Ensure DFS works as intended") {
    journalGraph g("../../build/journalgraph.bin");
    
    std::vector<std::pair<unsigned int, unsigned int>> ans = g.getIdeaHistory(2036110521);

    std::cout << ans.size();
    REQUIRE(ans.size() == 511);

    for (auto& p : ans) {
        if (p.first <= 1) {
            REQUIRE(p.second == 2036110521);
        }
    }

    //Asserts that it is an actual path and not some mumbo jumbo
    for (size_t i = 0; i < ans.size() - 1; i++) {
        REQUIRE(ans[i].second == ans[i + 1].first);
    }
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
    AuthorGraph g("../../build/author_graph.bin");
    std::vector<std::vector<unsigned long>> scc = g.tarjansSCC_with_query(2569299913);
    auto& graph = g.getGraph();
    unsigned long start = 2569299913;
    unsigned long query = 2569299913;
    for (auto& component : scc) {
        REQUIRE(isStronglyConnectedComponent(graph, query, component, start));
    }
}