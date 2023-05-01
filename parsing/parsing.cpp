#include "../dependencies/simdjson.h"
#include "parsing.h"

#include <fstream>
#include <stdexcept>
#include <string>
#include <sstream>
#include <exception>
#include <iostream>
#include <unordered_set>

#include "../storage/btree_db_v2.hpp"
#include "../storage/btree_types.cpp"

using namespace simdjson;

void traverse_data(const std::string &filename) {
    std::ifstream ifs(filename);

    BTreeDB<author::Entry> db("author_keys.db", "author_values.db", true);

    if (!ifs.is_open()) {
        throw std::runtime_error("filename not valid");
    }

    std::string line;
    std::getline(ifs, line);

    size_t i = 0;
    ondemand::parser parser;

    std::unordered_set<long> traversed;

    while(std::getline(ifs, line)) {
        // std::cout << i << std::endl;
        if (i % 10000 == 0) {
            std::cout << i << std::endl;
        }
        if (i == 100000) {
            break;
        }
        if (line.at(0) == ',') {
            line = line.substr(1);
        }

        padded_string temp(line);
        ondemand::document curr;

        if (parser.iterate(temp).get(curr)) {
            std::cout << "error parsing line " + std::to_string(i) << std::endl;
            continue;
        }

        ondemand::array authors;
        auto author_error = curr.find_field("authors").get(authors);
        if (author_error) {
            continue;
        }

        for (ondemand::object author : authors) {
            std::string name = std::string(author.find_field("name").get_string().value());

            std::string org;
            auto org_err = author.find_field("org").get_string();
            if (org_err.error() != SUCCESS) {
                org = "na";
            } else {
                org = std::string(org_err.value());
            }

            long id = author["id"].get_int64();

            if (traversed.find(id) != traversed.end()) {
                break;
            }


            author::Entry entry(name, org, id);
            db.insert(id, entry);

            traversed.insert(id);
        }

        ++i;
    }
}