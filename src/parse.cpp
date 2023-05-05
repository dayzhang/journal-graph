#include <iostream>
#include <iomanip>
#include <string>
#include <fstream>

#include "../parsing/parsing.h"
#include "../storage/btree_db_v2.hpp"
#include "../storage/btree_types.cpp"

using std::cout;
using std::endl;
using std::cin;

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cout << "Invalid number of arguments passed." << endl;
        cout << "Usage: ./parse [path to dblp json file relative to the build folder]" << endl;
        return 0;
    }

    cout << "Are you sure you want to parse the data? It will take around half an hour with the full dataset and will wipe any any existing db and graph files. It is also relatively intensive, requiring around 8GB of memory. An alternative is to simply download the built things from the provide google drivel ink" << endl;
    cout << "Type y if you want to proceed." << endl;
    
    std::string input;
    cin >> input;

    if (input != "y") {
        cout << "Terminating program..." << endl;
        return 0;
    }

    cout << "Building the databases and the paper graph" << endl;
    cout << "You may experience a significant pause when 4.8 million is reached; that is the code writing everything to the build folder" << endl;
    build_db(argv[1]);

    cout << "Building the author graph" << std::endl;
    cout << "This will gradually speed up as more of the database is loaded directly into memory as the code goes on. A small pause will also occur due to writebacks near the end of execution." << endl;
    build_author_graph(argv[1]);

    cout << "Successfully parsed the dblp data. Everything should now be in the build directory with the names author_keys.db, author_values.db, paper_keys.db, paper_values.db, author_graph.bin, and journalgraph.bin, along with the associated metadata for the database files." << endl;

    return 0;
}