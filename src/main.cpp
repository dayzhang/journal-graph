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

void print_tarjans(std::vector<std::vector<unsigned long>>& ans, BTreeDB<author::Entry>& db) {
    if (ans.size() > 0) {
        for (auto& i : ans) {
            std::cout << "Strongly Connected Component | ";
            for (auto& entry : i) {
                author::Entry name = db.find(entry);
                std::cout << std::string(name.name.data()) << " - ";
            }
            std::cout << "\n";
        }
    } else {
        std::cout << "No SCCs here" << "\n";
    }
}

void run_tarjans(BTreeDB<author::Entry>& db, AuthorGraph& g) {

    std::string query;
    while (true) {
        std::cout << "Please enter an author id. Recommended ones are:\n";
        std::cout << "include \"G. Carl Evans\" (id 2109906170), \"Brad Solomon\" (id 2189947603), \"Geoffrey Challen\" (id 2231335109), \"Michael Nowak\" (id 2688443206), \"Geoffrey L. Herman\" (id 2148163125), and \"Lawrence Angrave\" (id 2645015366) in the author database.\n";
        std::cout << "\n";

        std::cout << ">>";
        std::cin >> query;

        try {
            if (query == "q") {
                return;
            }
            unsigned long q = std::stoul(query);

            author::Entry entry = db.find(q);

            std::cout << "You entered the id for: " << std::string(entry.name.data()) << "\n";

            std::vector<std::vector<unsigned long>> ans = g.tarjansSCC_with_query(q);

            print_tarjans(ans, db);

            return;

        } catch (exception& e) {
            std::cout << "Failed to convert to unsigned long" << "\n";
            std::cout << e.what() << "\n";
        }
    }
    std::cout << "Returning to start" << "\n";
}

void run_dijkstras(BTreeDB<author::Entry>& db, AuthorGraph& g) {

    while (true) {
        std::cout << "Please enter an author id: ";
        unsigned long source;
        std::cin >> source;
        std::cout << "\n" << "Please enter a destination: ";
        unsigned long destination;
        std::cin >> destination;
        try {
            author::Entry entry = db.find(source);

            std::cout << "You entered the source id for: " << std::string(entry.name.data()) << "\n";

            std::vector<unsigned long> ans = g.dijkstrasShortestPath(source, destination);

            if (ans.size() > 0) {
                std::cout << "Shortest Path | ";
                for (auto& i : ans) {
                    author::Entry src = db.find(i);  
                    std::cout << std::string(src.name.data()) << " - ";
                }
                break;
            } else {
                break;
            }
        } catch (exception& e) {
            std::cout << e.what() << "\n\n";
        }
    }
    std::cout << "Returning to start" << "\n";
}

void run_authors_graph(AuthorGraph& g) {
    std::cout << "Initializing author database" << "\n";
    BTreeDB<author::Entry> db("author_keys.db", "author_values.db", false, true);

    std::string algorithm;
    while (true) {
        std::cout << "Enter and algorithm (Tarjans or Dijkstras).\n";
        
        std::cin >> algorithm;

        std::cout << "Running " << algorithm << " algorithm on Authors graph" << std::endl;
        if (algorithm == "Tarjans") {
            run_tarjans(db, g);
            break;
        } else if (algorithm == "Dijkstras") {
            run_dijkstras(db, g);
            break;
        } else {
            std::cout << "Invalid algorithm. Please enter \"Tarjans\" or \"Dijkstras\"" << "\n";
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
        paper::Entry source = db.find(pair.first);
        paper::Entry dest = db.find(pair.second);
        std::cout << std::string(source.title.data()) << " -> " << std::string(dest.title.data()) << "\n";
    }

    return;
}

void run_journals_graph(journalGraph& graph) {
    BTreeDB<paper::Entry> db("paper_keys.db", "paper_values.db", false, true);
    
    std::string paper_id;
    std::cout << "\nRecommended ID: 162256\n";
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
    std::cout << "--------------------------------------------------------------------\n";
    std::cout << "Journal Graph by Daniel Zhang, Ian Zhang, Jenny Hu, and Kevin Chen" << "\n";
    std::cout << "\nWelcome! To run the program, start by entering either \"Journals\" or \"Authors\" on the CLI when executing\n\n";

    std::cout << "*Note, not all papers in the database are available in the graph due to parsing\n";    
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " [\"Journals\" or \"Authors\"]" << std::endl;
        return exit_failure;
    }

    std::string graph = argv[1];

    if (graph == "Authors") {

        std::cout << "Initializing an Author Graph... \n";

        AuthorGraph g("author_graph.bin");

        while (true) {
            std::cout << "Type r to run, q to quit" << std::endl;
            std::cout << std::endl;
            std::string go;
            cin >> go;

            if (go == "r") {

                run_authors_graph(g);

            } else if (go == "q") {
                break;
            } else {
                std::cout << "Unknown Input \n";
            }

            std::cout << std::endl;
        }

    } else if (graph == "Journals") {

        std::cout << "Initializing an Journal Graph... \n";

        journalGraph g("journalgraph.bin");

        while (true) {
            std::cout << "Type r to run, q to quit" << "\n";
            std::string go;
            cin >> go;

            if (go == "r") {

                run_journals_graph(g);

            } else if (go == "q") {
                break;
            } else {
                std::cout << "\nUnknown Input\n";
            }
            std::cout << std::endl;
        }
        
    } else {
        std::cerr << "Invalid graph name. Available options: Authors, Journals" << std::endl;
        return exit_failure;
    }

    std::cout << "\nThank you for using journal graph!\n" << "\n";

    return exit_success;
}