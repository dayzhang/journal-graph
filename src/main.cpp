#include <iostream>
#include "../Dataset/parsing.cpp"
#include "journalGraph.cpp"
#include "../storage/btree_db.cpp"
#define exit_failure 0
#define exit_success 1

int main() {
    BTreeDB db("keys.db", "values.db");



    for (int i = 0; i < 1000; i++) {
        ValueEntry entry;
        entry.temp = i;
        db.insert(i, entry);
    }

    for (int i = 0; i < 1000; i++) {
        std::cout << db.find(i).temp << std::endl;
    }

    
    

    return 1;
}