#include <iostream>
#include <iomanip>
#include <limits>
#include <stdexcept>
#include <string>
#include <fstream>
#include <exception>

#include "../storage/btree_db_v2.hpp"
#include "../storage/btree_types.cpp"
#include "../graph/journalGraph.h"

using std::cout;
using std::endl;
using std::cin;

int main() {
    cout << "Initializing the journal graph from journalgraph.bin" << endl;
    journalGraph g("journalgraph.bin");

    cout << "Initializing the paper database using the paper_keys.db and paper_values.db files" << endl;
    BTreeDB<paper::Entry> db("paper_keys.db", "paper_values.db", false, true);
    

    // TODO: get a random start id and a random end id (using BFS to find the smallest path and to ensure a solution is possible)
    long curr = 1091;
    int steps = 0;

    while (true) {
        cout << "You are currently at paper " << curr << " and have taken " << steps << " steps." << endl;

        cout << ">> ";

        std::string input;
        cin >> input;

        if (input == "get_neighbors") {
            cout << "Neighbors: ";
            for (unsigned int neighbor : g.get_neighbors(curr)) {
                cout << neighbor << ' ';
            }
            cout << endl;
        } else if (input == "query") {
            cout << "Which paper id do you want to query?" << endl;
            int query;
            cin >> query;

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
            int target;
            cin >> target;

            if (g.get_neighbors(curr).find(target) == g.get_neighbors(curr).end()) {
                cout << "Invalid paper specified -- please only move to ones specified in the get_neighbors function." << endl;
            } else {
                curr = target;
                steps++;
            }
        } else if (input == "quit") {
            break;
        } else {
            cout << "Invalid command. The available ones include get_neighbors, query, move, and quit." << endl;
        }
        cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
}