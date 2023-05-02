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
#include "../graph/authorGraph.h"

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
        if (i % 100000 == 0) {
            std::cout << i << std::endl;
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

        long paper_id;
        auto id_err = curr.find_field("id").get_int64().get(paper_id);
        if (id_err) {
            continue;
        }

        std::array<long, 8> author_vec;
        author_vec.fill(0);

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

            author_vec[j] = id;

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
                g.addEdge(paper_id, id);
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

void build_author_graph(const std::string& filename) {
    std::ifstream ifs(filename);

    BTreeDB<paper::Entry> paper_db("paper_keys.db", "paper_values.db", false, true);

    AuthorGraph g;

    if (!ifs.is_open()) {
        throw std::runtime_error("filename not valid");
    }

    std::string line;
    std::getline(ifs, line);

    size_t count = 0;
    ondemand::parser parser;

    while(std::getline(ifs, line)) {
        if (count % 10000 == 0) {
            std::cout << count << std::endl;
        }

        if (line.at(0) == ',') {
            line = line.substr(1);
        }

        padded_string temp(line);
        ondemand::document curr;

        if (parser.iterate(temp).get(curr)) {
            std::cout << "error parsing line " + std::to_string(count) << std::endl;
            continue;
        }

        if (curr.find_field("id").error() != SUCCESS) {
            continue;
        }
        if (curr.find_field("title").error() != SUCCESS) {
            continue;
        }
        if (curr.find_field("year").error() != SUCCESS) { 
            continue;
        }
        long n_citations = 0;
        auto cit_handler = curr.find_field("n_citation").get(n_citations);
        if (cit_handler) {
            std::cout << "missing # citations " + std::to_string(count) << std::endl;
            continue;
        }
        ++n_citations;

        std::vector<unsigned long> author_vec;

        ondemand::array authors;
        auto author_error = curr["authors"].get_array().get(authors);
        if (author_error) {
            continue;
        }

        int num_authors = 0;

        for (ondemand::object author : authors) {
            if (num_authors == AUTHOR_EDGE_LIMIT) break;

            unsigned long id = author["id"];

            author_vec.push_back(id);

            ++num_authors;
        }

        g.add_same_paper_authors(author_vec, n_citations);

        ondemand::array refs;
        auto refs_error = curr["references"].get_array().get(refs);
        if (refs_error) {
            
        } else {
            for (unsigned long id : refs) {
                paper::Entry cur_paper = paper_db.find(id);
                if (cur_paper.authors[0] == 0) continue;
                if (cur_paper.pub_year == 0) continue;

                g.add_referenced_authors(author_vec, cur_paper.authors, n_citations, cur_paper.n_citations);
            }
        }

        ++count;
    }

    g.export_to_file("author_graph.bin");
}
