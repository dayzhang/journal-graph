#include <iostream>
#include <iomanip>
#include <limits>
#include <stdexcept>
#include <string>
#include <fstream>
#include <exception>
#include <cstdlib>

#include "../storage/btree_db_v2.hpp"
#include "../storage/btree_types.cpp"
#include "../graph/journalGraph.h"

using std::cout;
using std::endl;
using std::cin;

int main(int argc, char* argv[]) {
    if (argc != 5) {
        cout << "invalid number of arguments passed" << endl;
        cout << "usage: ./paper_game [paper graph binary file (journalgraph.bin)] [paper keys db file (paper_keys.db)] [paper values db file (paper_values.db)] [start paper (try 1091 if you don't have a specific one)]" << endl;

        return 0;
    }

    cout << "Initializing the journal graph from journalgraph.bin" << endl;
    cout << "If a key/value file opening error is thrown, try running parse first or download the data directly." << endl;
    journalGraph g(argv[1]);

    cout << "Initializing the paper database using the paper_keys.db and paper_values.db files" << endl;
    BTreeDB<paper::Entry> db(argv[2], argv[3], false, true);

    // TODO: get a random start id and a random end id (using BFS to find the smallest path and to ensure a solution is possible)
    long curr = std::stol(argv[4]);
    int steps = 0;

    std::string temp;

    while (true) {
        cout << "You are currently at paper " << curr << " and have taken " << steps << " steps." << endl;

        cout << ">> ";

        std::string input;
        std::getline(cin, input);

        if (input == "get_neighbors") {
            cout << "Neighbors: ";
            for (unsigned int neighbor : g.get_neighbors(curr)) {
                if (g.in_graph(neighbor)) {
                    cout << neighbor << ' ';
                }
            }
            cout << endl;
        } else if (input == "query") {
            cout << "Which paper id do you want to query?" << endl;
            
            std::getline(cin, temp);
            unsigned int query;
            try {
                query = std::stoul(temp);
            }
            catch (const std::invalid_argument& err) {
                cout << "Non-integer id inputted; please input a valid id" << endl;
                continue;
            }


            paper::Entry entry = db.find(query);
            if (entry.id == -1) {
                cout << "Invalid paper id provided" << endl;
            } else {
                // TODO: possibly add authors
                cout << "Title: " << std::string(entry.title.data()) << endl;
                cout << "Keywords: " << std::string(entry.keywords.data()) << endl;
                cout << "Number of citations: " << entry.n_citations << endl;
                cout << "Publication year: " << entry.pub_year << endl;
            }
        } else if (input == "move") {
            cout << "Which paper would you like to move to?" << endl;

            std::getline(cin, temp);
            unsigned int target;
            try {
                target = std::stoul(temp);
            }
            catch (const std::invalid_argument& err) {
                cout << "Non-integer id inputted; please input a valid id" << endl;
                continue;
            }

            if (g.get_neighbors(curr).find(target) == g.get_neighbors(curr).end()) {
                cout << "Invalid paper specified -- please only move to ones specified in the get_neighbors function." << endl;
            } else {
                curr = target;
                steps++;
            }
        } else if (input == "quit") {
            break;
        } else if (input == "help") {
            cout << "get_neighbors - list the neighbors of the current paper (in ids)" << endl;
            cout << "query - get info about a specific paper using its id" << endl;
            cout << "move - move to another adjacent paper" << endl;
            cout << "quit - exit the CLI interface" << endl;
        } else {
            cout << "Invalid command. The available ones include get_neighbors, query, move, help, and quit." << endl;
        }
    }
}