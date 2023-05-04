#include <iostream>
#include <utility>
#include <chrono>

#include "../graph/utils.cpp"
#include "../graph/journalGraph.h"
#include "../graph/authorGraph.h"
#include "../graph/tarjansSCC.cpp"
#include "../graph/dijkstrasSP.cpp"
#include "../dataset/parsing.cpp"
#include "../storage/btree_db_v2.hpp"
#include "../storage/btree_types.cpp"

#define exit_failure 0
#define exit_success 1

bool is_valid_algorithm(const std::string& algorithm) {
    return algorithm == "Tarjans" || algorithm == "Dijkstras";
}

bool is_valid_paper_id(std::string paper_id, BTreeDB<paper::Entry>& db) {
    try {

        paper::Entry entry = db.find(std::stol(paper_id));
        return entry.pub_year != 0;

    } catch (exception& e) {

        std::cout << "Entered id could not be converted to an unsigned int. Please enter an id that can be a long\n";

        std::cout << e.what() << "\n";

    }
    return false;
}

void run_tarjans(BTreeDB<author::Entry>& db, AuthorGraph& g) {

    std::cout << "Please enter an author id: ";

    std::string query;
    while (true) {
        std::cin >> query;
        try {
            unsigned long q = std::stoul(query);

            author::Entry entry = db.find(q);

            std::cout << "You entered the id for: " << std::string(entry.name.data()) << "\n";

            std::vector<std::vector<unsigned long>> ans = g.tarjansSCC_with_query(q);

            if (ans.size() > 0) {
                for (auto& i : ans) {
                    std::cout << "Strongly Connected Component | ";
                    for (auto& entry : i) {
                        std::cout << entry << " - ";
                    }
                    std::cout << "\n";
                }
                break;
            } else {
                std::cout << "No SCCs here" << "\n";
            }
        } catch (exception& e) {
            std::cout << "Failed to convert to unsigned long" << "\n";
            std::cout << e.what() << "\n";
        }
    }
    std::cout << "Returning to start" << "\n";

}

void run_authors_graph(std::string& algorithm, AuthorGraph& g) {
    std::cout << "Initializing author database" << "\n";
    BTreeDB<author::Entry> db("author_keys.db", "author_values.db", false, true);

    std::cout << "Running " << algorithm << " algorithm on Authors graph" << std::endl;

    std::string algo;
    while (true) {
        if (algorithm == "Tarjans") {
            run_tarjans(db, g);
        } else if (algorithm == "Dijkstras") {

        } else {
            std::cout << "Invalid algorithm. Please enter \"Tarjans\" or \"Dijkstras\"" << "\n";
            std::cin >> algorithm;
        }
    }
}

void print_dfs_ids_to_names_proxy(BTreeDB<paper::Entry>& db, const std::vector<std::pair<unsigned int, unsigned int>>& ids) {
    if (ids.size() == 0) {
        return;
    }

    paper::Entry first = db.find(ids[0].second);

    std::cout << "starting from " << std::string(first.title.data()) << ", it references: ";

    for (auto& pair : ids) {
        std::cout << pair.first << " -> " << pair.second << "\n";
    }

    return;
}

void run_journals_graph(journalGraph& graph) {
    BTreeDB<paper::Entry> db("paper_keys.db", "paper_values.db", false, true);
    
    std::string paper_id;

    std::cout << "Please enter a paper ID: ";
    std::cin >> paper_id;

    while (!is_valid_paper_id(paper_id, db)) {
        std::cout << "Please enter a paper ID: ";
        std::cin >> paper_id;
    }

    paper::Entry entry = db.find(std::stol(paper_id));

    std::cout << "You entered the id for: " << std::string(entry.title.data()) << "\n";

    std::cout << "Getting the historical trace of the paper's origins using DFS... \n";

    const std::vector<std::pair<unsigned int, unsigned int>>& answer = graph.getIdeaHistory(std::stoul(paper_id));

    print_dfs_ids_to_names_proxy(db, answer);

    return;
}


int main(int argc, char* argv[]) {
    
    std::cout << "Journal Graph by Daniel Zhang, Ian Zhang, Jenny Hu, and Kevin Chen" << "\n";
    std::cout << "Welcome! To run the program, start by entering either \"Journals\" or \"Authors\" on the CLI when executing\n";
    
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " [\"Journals\" or \"Authors\"]" << std::endl;
        return 1;
    }

    std::string graph = argv[1];

    if (graph == "Authors") {

        std::cout << "Initializing an Author Graph... \n";

        AuthorGraph g("author_graph.bin");

        std::string algorithm;
        std::cout << "Please choose an algorithm (Tarjans or Dijkstras): ";
        std::cin >> algorithm;

        run_authors_graph(algorithm, g);

    } else if (graph == "Journals") {

        std::cout << "Initializing a Journal Graph... \n";

        journalGraph g("journalgraph.bin");
        
        while (true) {
            run_journals_graph(g);
        }
    } else {
        std::cerr << "Invalid graph name. Available options: Authors, Journals" << std::endl;
        return 1;
    }

    std::cout << "Thank you for using journal graph!" << "\n";

    return exit_success;
}