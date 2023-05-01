#include "../lib/simdjson.h"
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
#include "../graph/journalGraph.h"

using namespace simdjson;

void build_db(const std::string &filename) {
    std::ifstream ifs(filename);

    BTreeDB<author::Entry> author_db("author_keys.db", "author_values.db", true);
    BTreeDB<paper::Entry> paper_db("paper_keys.db", "paper_values.db", true);

    journalGraph g;

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
        // if (i == 200000) {
        //     break;
        // }
        if (line.at(0) == ',') {
            line = line.substr(1);
        }

        padded_string temp(line);
        ondemand::document curr;

        if (parser.iterate(temp).get(curr)) {
            std::cout << "error parsing line " + std::to_string(i) << std::endl;
            continue;
        }

        long paper_id;
        auto id_err = curr.find_field("id").get_int64().get(paper_id);
        if (id_err) {
            continue;
        }

        std::vector<long> author_vec(8);

        ondemand::array authors;
        auto author_error = curr.find_field("authors").get(authors);
        if (author_error) {
            continue;
        }

        int j = 0;

        for (ondemand::object author : authors) {
            if (j == 8) break;

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
            author_db.insert(id, entry);

            traversed.insert(id);

            author_vec.push_back(id);

            ++j;
        }


        
        std::string_view title_view;
        auto title_err = curr.find_field("title").get_string().get(title_view);
        if (title_err) {
            continue;
        }
        std::string title(title_view);

        long paper_year;
        auto year_handler = curr.find_field("year");
        if (year_handler.error() != SUCCESS) {
            std::cout << "missing year " + std::to_string(i) << std::endl;
            continue;
        } else {
            paper_year = year_handler.value();
        }
        
        long n_citations;
        auto cit_handler = curr.find_field("n_citation");
        if (cit_handler.error() != SUCCESS) {
            std::cout << "missing # citations " + std::to_string(i) << std::endl;
            continue;
        } else {
            n_citations = cit_handler.value();
        }


        ondemand::array refs;
        auto refs_error = curr["references"].get_array().get(refs);
        if (refs_error) {

        } else {
            for (long id : refs) {
                g.addEdge(id, paper_id);
            }
        }

        std::string fos_list;
        ondemand::array foses;
        auto fos_error = curr["fos"].get(foses);
        if (fos_error) {

        } else {
            int j = 0;
            for (ondemand::object fos : foses) {
                if (j == 3) break;

                std::string curr = std::string(fos.find_field("name").get_string().value());

                fos_list += curr + ' ';

                ++j;
            }
        }

        fos_list = fos_list.substr(0, fos_list.size() - 1);

        paper::Entry to_insert(title, fos_list, n_citations, paper_year, author_vec, paper_id);
        paper_db.insert(paper_id, to_insert);

        ++i;
    }

    g.export_to_file("journalgraph.bin");
}