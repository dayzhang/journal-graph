#include <iostream>
#include "../Dataset/parsing.cpp"
#include "journalGraph.cpp"
#include "../storage/btree_db.cpp"
#define exit_failure 0
#define exit_success 1

int main() {
    BTreeDB db("keys.db", "values.db");

    // ValueEntry entry;
    // entry.name.fill(1);
    // entry.temp = 500;
    // db.insert(5, entry);

    std::cout << db.find(5).temp << std::endl;

    
    

    return 1;
}