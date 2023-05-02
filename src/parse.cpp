#include <iostream>
#include <iomanip>
#include <string>
#include <fstream>

#include "../parsing/parsing.h"
#include "../storage/btree_db_v2.hpp"
#include "../storage/btree_types.cpp"
#include "../graph/journalGraph.h"
#include "../graph/authorGraph.h"

int main() {
    // std::cout << sizeof(long) << std::endl;
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

    
    // BTreeDB<paper::Entry> db("paper_keys.db", "paper_values.db", false, true);
    // std::string title = "Formal agent-oriented ubiquitous computing: a computational intelligence support for information and services integration";

    // paper::Entry entry(title);

    // std::cout << db.get_id_from_name(entry) << std::endl;

    // std::cout << std::string(db.find(1388).title.data()) << std::endl;
    // std::cout << db.find(1388).authors[0] << std::endl;


    // journalGraph g("journalgraph.bin");

    // build_db("../data/dblp.v12.json");

    // build_author_graph("../data/dblp.v12.json");

    AuthorGraph g("author_graph.bin");
    
    return 1;
}