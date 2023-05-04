#define CATCH_CONFIG_MAIN
#include <catch2/catch_test_macros.hpp>
#include "../graph/utils.cpp"
#include "../graph/journalGraph.h"
#include "../graph/authorGraph.h"
#include "../graph/tarjansSCC.cpp"
#include "../dataset/parsing.cpp"
#include "../storage/btree_db_v2.hpp"
#include "../storage/btree_types.cpp"
#include "../graph/dijkstrasSP.cpp"

#include <algorithm>
#include <unordered_set>
#include <unordered_map>
#include <random>
#include <climits>

std::string gen_random(const int len) {
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    std::string tmp_s;
    tmp_s.reserve(len);

    for (int i = 0; i < len; ++i) {
        tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];
    }
    
    return tmp_s;
}

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
    journalGraph g("../data/journalgraph.bin");
    
    std::vector<std::pair<unsigned int, unsigned int>> ans = g.getIdeaHistory(2036110521);

    for (auto& pair : ans) {
        std::cout << pair.first << " -> " << pair.second << " -> ";
    }
    std::cout << "\n";
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
    std::vector<author_parse_wrapper> values;
    parse_authors(values, "../data/tarjanstest.json");
    AuthorGraph g(values);
    std::vector<unsigned long> path1 = g.dijkstrasShortestPath(2022192081, 2113592602);
    std::vector<unsigned long> path2 = g.dijkstrasShortestPath(2022192081, 2117665592);
    REQUIRE(path1.size() == 4);
    REQUIRE(path2.size() == 2);
    
}

TEST_CASE("Dijkstra's Test 2") {
    std::vector<author_parse_wrapper> values;
    parse_authors(values, "../data/tarjanstest.json");
    AuthorGraph g(values);
    std::vector<unsigned long> path1 = g.dijkstrasShortestPath(2022192081, 2425818370);
    REQUIRE(path1.empty());
}

TEST_CASE("BTree - simple") {
    std::string str = "simple test";
    test::Entry entry(56, str, 199);

    BTreeDB<test::Entry> db("test_db_keys.db", "test_db_values.db", true);
    db.insert(entry.id, entry);

    test::Entry find = db.find(entry.id);

    REQUIRE(find.x == entry.x);
    REQUIRE(find.id == entry.id);
    REQUIRE(std::string(find.str.data()) == std::string(find.str.data()));

    test::Entry find_false = db.find(-1);
    REQUIRE(find_false.id == -1);

    test::Entry search(56);
    REQUIRE(db.get_id_from_name(search) == entry.id);
    
    entry.x = 77;
    db.insert(entry.id, entry);
    REQUIRE(db.find(entry.id).x == 77);
}

TEST_CASE("BTree - multiple inserts, no splitting") {
    BTreeDB<test::Entry> db("test_db_keys.db", "test_db_values.db", true);

    std::unordered_map<long, test::Entry> record;
    for (unsigned int i = 0; i < 50; ++i) {
        int curr = rand() % INT_MAX;
        record[curr] = test::Entry(rand() % INT_MAX, gen_random(10), curr);
        db.insert(record[curr].id, record[curr]);
    }

    for (const auto& pair : record) {
        REQUIRE(db.find(pair.first).x == record[pair.first].x);
        REQUIRE(db.find(pair.first).id == record[pair.first].id);
        REQUIRE(std::string(db.find(pair.first).str.data()) == std::string(record[pair.first].str.data()));
    }
}

TEST_CASE("BTree - persistence, no splitting") {
    std::unordered_map<long, test::Entry> record;

    {
        BTreeDB<test::Entry> db("test_db_keys.db", "test_db_values.db", true);
        for (unsigned int i = 0; i < 50; ++i) {
            int curr = rand() % INT_MAX;
            record[curr] = test::Entry(rand() % INT_MAX, gen_random(10), curr);
            db.insert(record[curr].id, record[curr]);
        }
    }

    BTreeDB<test::Entry> new_db("test_db_keys.db", "test_db_values.db", false, true);

    for (const auto& pair : record) {
        REQUIRE(new_db.find(pair.first).x == record[pair.first].x);
        REQUIRE(new_db.find(pair.first).id == record[pair.first].id);
        REQUIRE(std::string(new_db.find(pair.first).str.data()) == std::string(record[pair.first].str.data()));
    }
}

TEST_CASE("BTree - insertion, no splitting") {
    BTreeDB<test::Entry> db("test_db_keys.db", "test_db_values.db", true);

    std::unordered_map<long, test::Entry> record;
    for (unsigned int i = 0; i < 50; ++i) {
        int curr = rand() % INT_MAX;
        record[curr] = test::Entry(rand() % INT_MAX, gen_random(10), curr);
        db.insert(record[curr].id, record[curr]);
    }

    for (unsigned int i = 0; i < 50; ++i) {
        int curr = rand() % INT_MAX;
        record[curr] = test::Entry(rand() % INT_MAX, gen_random(10), curr);
        db.insert(record[curr].id, record[curr]);
    }

    for (const auto& pair : record) {
        REQUIRE(db.find(pair.first).x == record[pair.first].x);
        REQUIRE(db.find(pair.first).id == record[pair.first].id);
        REQUIRE(std::string(db.find(pair.first).str.data()) == std::string(record[pair.first].str.data()));
    }
}

TEST_CASE("BTree - find_id, no splitting") {
    BTreeDB<test::Entry> db("test_db_keys.db", "test_db_values.db", true);

    std::unordered_map<long, test::Entry> record;
    for (unsigned int i = 0; i < 50; ++i) {
        int curr = rand() % INT_MAX;
        record[curr] = test::Entry(rand() % INT_MAX, gen_random(10), curr);
        db.insert(record[curr].id, record[curr]);
    }

    for (const auto& pair : record) {
        REQUIRE(db.get_id_from_name(pair.second.x) == pair.first);
    }
}

TEST_CASE("BTree - multiple inserts, splitting") {
    BTreeDB<test::Entry> db("test_db_keys.db", "test_db_values.db", true);

    std::unordered_map<long, test::Entry> record;
    for (unsigned int i = 0; i < 10000; ++i) {
        int curr = rand() % INT_MAX;
        record[curr] = test::Entry(rand() % INT_MAX, gen_random(10), curr);
        db.insert(record[curr].id, record[curr]);
    }

    for (const auto& pair : record) {
        REQUIRE(db.find(pair.first).x == record[pair.first].x);
        REQUIRE(db.find(pair.first).id == record[pair.first].id);
        REQUIRE(std::string(db.find(pair.first).str.data()) == std::string(record[pair.first].str.data()));
    }
}

TEST_CASE("BTree - persistence, splitting") {
    std::unordered_map<long, test::Entry> record;

    {
        BTreeDB<test::Entry> db("test_db_keys.db", "test_db_values.db", true);
        for (unsigned int i = 0; i < 10000; ++i) {
            int curr = rand() % INT_MAX;
            record[curr] = test::Entry(rand() % INT_MAX, gen_random(10), curr);
            db.insert(record[curr].id, record[curr]);
        }
    }

    BTreeDB<test::Entry> new_db("test_db_keys.db", "test_db_values.db", false, true);

    for (const auto& pair : record) {
        REQUIRE(new_db.find(pair.first).x == record[pair.first].x);
        REQUIRE(new_db.find(pair.first).id == record[pair.first].id);
        REQUIRE(std::string(new_db.find(pair.first).str.data()) == std::string(record[pair.first].str.data()));
    }
}

TEST_CASE("BTree - insertion, splitting") {
    BTreeDB<test::Entry> db("test_db_keys.db", "test_db_values.db", true);

    std::unordered_map<long, test::Entry> record;
    for (unsigned int i = 0; i < 10000; ++i) {
        int curr = rand() % INT_MAX;
        record[curr] = test::Entry(rand() % INT_MAX, gen_random(10), curr);
        db.insert(record[curr].id, record[curr]);
    }

    for (unsigned int i = 0; i < 10000; ++i) {
        int curr = rand() % INT_MAX;
        record[curr] = test::Entry(rand() % INT_MAX, gen_random(10), curr);
        db.insert(record[curr].id, record[curr]);
    }

    for (const auto& pair : record) {
        REQUIRE(db.find(pair.first).x == record[pair.first].x);
        REQUIRE(db.find(pair.first).id == record[pair.first].id);
        REQUIRE(std::string(db.find(pair.first).str.data()) == std::string(record[pair.first].str.data()));
    }
}

TEST_CASE("BTree - find_id, splitting") {
    BTreeDB<test::Entry> db("test_db_keys.db", "test_db_values.db", true);

    std::unordered_map<long, test::Entry> record;
    for (unsigned int i = 0; i < 10000; ++i) {
        int curr = rand() % INT_MAX;
        record[curr] = test::Entry(rand() % INT_MAX, gen_random(10), curr);
        db.insert(record[curr].id, record[curr]);
    }

    for (const auto& pair : record) {
        REQUIRE(db.get_id_from_name(pair.second.x) == pair.first);
    }
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
    AuthorGraph g("../data/author_graph.bin");
    std::vector<std::vector<unsigned long>> scc = g.tarjansSCC_with_query(2569299913);
    auto& graph = g.getGraph();
    unsigned long start = 2569299913;
    unsigned long query = 2569299913;
    for (auto& component : scc) {
        REQUIRE(isStronglyConnectedComponent(graph, query, component, start));
    }
}
