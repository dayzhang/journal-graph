#include <iostream>

#include "../parsing/parsing.h"
#include "../storage/btree_db_v2.hpp"
#include "../storage/btree_types.cpp"

void create_db() {
    build_db("../data/dblp.v12.json");
}

int main() {
    // std::cout << sizeof(long) << std::endl;
    // create_db();
    // BTreeDB<author::Entry> db("author_keys.db", "author_values.db");
    // // BTreeDB<test::Entry> db("keys.db", "values.db");

    // // // for (unsigned int i = 0; i < 10000; ++i) {
    // // //     test::Entry temp(i);
    // // //     db.insert(i, temp);
    // // // }

    // // for (unsigned int i = 0; i < 10000; ++i) {
    // //     std::cout << db.find(i).x << std::endl;;
    // // }

    // std::cout << std::string(db.find(2103626414).organization.data()) << std::endl;

    // create_db();


    create_db();

    
    BTreeDB<paper::Entry> db("paper_keys.db", "paper_values.db");

    std::cout << std::string(db.find(1674).title.data()) << std::endl;




    
    

    return 1;
}