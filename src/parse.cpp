#include <iostream>
#include <iomanip>
#include <string>
#include <fstream>

#include "../parsing/parsing.h"
#include "../storage/btree_db_v2.hpp"
#include "../storage/btree_types.cpp"
#include "../graph/journalGraph.h"
#include "../graph/authorGraph.h"

using std::cout;
using std::endl;
using std::cin;

int main(int argc, char* argv[]) {
    if (argc != 1) {
        cout << "Invalid number of arguments passed." << endl;
        cout << "Usage: ./parse [name of json file to parse]" << endl;
        return 0;
    }

    cout << "Are you sure you want to parse the data? It will take around half an hour with the full dataset and will wipe any any existing db and graph files." << endl;
    cout << "Type y if you want to proceed." << endl;
    
    std::string input;
    cin >> input;

    if (input != "y") {
        cout << "Terminating program..." << endl;
        return 0;
    }

    cout << "Building the databases and the paper graph" << endl;
    build_db(argv[0]);

    cout << "Building the author graph" << std::endl;
    build_author_graph(argv[0]);

    cout << "Success parsing the dblp data. Everything should now be in the build directory with the names author_keys.db, author_values.db, paper_keys.db, paper_values.db, author_graph.bin, and journalgraph.bin." << endl;

    return 0;
}