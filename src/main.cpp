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

using namespace std::chrono;

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

void run_authors_graph(std::string& algorithm) {
    // Your code for running the authors graph goes here...
    std::cout << "Running " << algorithm << " algorithm on Authors graph" << std::endl;
}

void print_dfs_ids_to_names_proxy(BTreeDB<paper::Entry>& db, const std::vector<std::pair<unsigned int, unsigned int>>& ids) {
    if (ids.size() == 0) {
        return;
    }

    paper::Entry first = db.find(ids[0].second);

    std::cout << "starting from " << std::string(first.title.data()) << ", it references: ";

    for (auto& pair : ids) {
        paper::Entry source = db.find(pair.first);
        paper::Entry destination = db.find(pair.second);

        std::cout << std::string(source.title.data()) << " -> " << std::string(destination.title.data()) << "\n";
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

    std::cout << "Getting the hisstorical trace of the paper's origins using DFS... \n";

    const std::vector<std::pair<unsigned int, unsigned int>>& answer = graph.getIdeaHistory(std::stoul(paper_id));

    print_dfs_ids_to_names_proxy(db, answer);

    return;
}


int main(int argc, char* argv[]) {
    BTreeDB<paper::Entry> db("paper_keys.db", "paper_values.db", false, true);
    std::cout << "Journal Graph by Daniel Zhang, Ian Zhang, Jenny Hu, and Kevin Chen" << "\n";
    std::cout << "Welcome! To run the program, start by entering either \"Journals\" or \"Authors\" on the CLI when executing\n";

    paper::Entry entry = db.find(107151);

    std::cout << "You entered the id for " << std::string(entry.title.data()) << "\n";
    /*
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " [\"Journals\" or \"Authors\"]" << std::endl;
        return 1;
    }

    std::string graph = argv[1];

    if (graph == "Authors") {
        std::string algorithm;
        std::cout << "Please choose an algorithm (Tarjans or Dijkstras): ";
        std::cin >> algorithm;

        run_authors_graph(algorithm);

    } else if (graph == "Journals") {

        std::cout << "Initializing a Journal Graph... \n";

        journalGraph g("../data/journalgraph.bin");

        run_journals_graph(g);
    } else {
        std::cerr << "Invalid graph name. Available options: Authors, Journals" << std::endl;
        return 1;
    }

    std::cout << "Thank you for using journal graph!" << "\n";
    */

    return 0;
}