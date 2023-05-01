#include <iostream>

#include "../parsing/parsing.h"
// #include "../storage/btree_db.hpp"
#include "../storage/btree_types.cpp"

int main() {
    // BTreeDB<test::Entry> db("keys.db", "values.db");

    // // for (unsigned int i = 0; i < 10000; ++i) {
    // //     test::Entry temp(i);
    // //     db.insert(i, temp);
    // // }

    // for (unsigned int i = 0; i < 10000; ++i) {
    //     std::cout << db.find(i).x << std::endl;;
    // }

    traverse_data("../data/dblp.v12.json");


    
    

    return 1;
}