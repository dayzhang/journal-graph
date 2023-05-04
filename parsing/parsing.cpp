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
#include "../lib/simdjson.h" // the simdjson library was used to do the json parsing

using namespace simdjson;

void build_db(const std::string &filename) {
    // create ifstream to read from json
    std::ifstream ifs(filename);

    // create new author and paper dbs, overwriting as necessary
    BTreeDB<author::Entry> author_db("author_keys.db", "author_values.db", true);
    BTreeDB<paper::Entry> paper_db("paper_keys.db", "paper_values.db", true);

    // creating a new journal graph
    journalGraph g;

    // check for ifstream error
    if (!ifs.is_open()) {
        throw std::runtime_error("filename not valid");
    }

    // skip the first line -- is a bracket and is not parseable by json
    std::string line;
    std::getline(ifs, line);

    // coutner variable to determine which line we are at
    size_t i = 0;

    // initialize ondemand simdjson parser
    ondemand::parser parser;

    // keep track of traversed authors to prevent duplicates
    std::unordered_set<long> traversed;

    // loop through the json line by line
    while(std::getline(ifs, line)) {
        if (i % 100000 == 0) {
            std::cout << i << std::endl;
        }

        // get rid of the comma at the beginning of the line if it exists
        if (line.at(0) == ',') {
            line = line.substr(1);
        }

        // make the line a padded string and parse it to json; if not parseable, skip
        padded_string temp(line);
        ondemand::document curr;

        if (parser.iterate(temp).get(curr)) {
            std::cout << "error parsing line " + std::to_string(i) << std::endl;
            continue;
        }

        // extract the paper id from the line; if not found, skip
        long paper_id;
        auto id_err = curr.find_field("id").get_int64().get(paper_id);
        if (id_err) {
            continue;
        }

        // extract author array; if not found, skip
        std::array<long, 8> author_vec;
        author_vec.fill(0);

        ondemand::array authors;
        auto author_error = curr.find_field("authors").get(authors);
        if (author_error) {
            continue;
        }

        // iterate over author array
        int j = 0;

        for (ondemand::object author : authors) {
            // only traverse 8 authors for time/memory reasons
            if (j == 8) break;

            // get name of author and organization (if found)
            std::string name = std::string(author.find_field("name").get_string().value());

            std::string org;
            auto org_err = author.find_field("org").get_string();
            if (org_err.error() != SUCCESS) {
                org = "na";
            } else {
                org = std::string(org_err.value());
            }

            // get author id
            long id = author["id"].get_int64();

            // if id already traversed, continue
            if (traversed.find(id) != traversed.end()) {
                break;
            }

            // insert author into the database, updating things as needed
            author::Entry entry(name, org, id);
            author_db.insert(id, entry);

            traversed.insert(id);

            author_vec[j] = id;

            ++j;
        }

        // extract paper title; if not found, skip
        std::string_view title_view;
        auto title_err = curr.find_field("title").get_string().get(title_view);
        if (title_err) {
            continue;
        }
        std::string title(title_view);

        // extract paper year; if not found, skip
        long paper_year;
        auto year_handler = curr.find_field("year");
        if (year_handler.error() != SUCCESS) {
            std::cout << "missing year " + std::to_string(i) << std::endl;
            continue;
        } else {
            paper_year = year_handler.value();
        }
        
        // extract number of citations; if not found, skip
        long n_citations;
        auto cit_handler = curr.find_field("n_citation");
        if (cit_handler.error() != SUCCESS) {
            std::cout << "missing # citations " + std::to_string(i) << std::endl;
            continue;
        } else {
            n_citations = cit_handler.value();
        }

        // extract references if they exist
        ondemand::array refs;
        auto refs_error = curr["references"].get_array().get(refs);
        if (refs_error) {

        } else {
            for (long id : refs) {
                // add a connection between this paper and the reference paper
                g.addEdge(paper_id, id);
            }
        }

        // extract fields of study if they exist; used as keywords
        std::string fos_list;
        ondemand::array foses;
        auto fos_error = curr["fos"].get(foses);
        if (fos_error) {

        } else {
            int j = 0;
            for (ondemand::object fos : foses) {
                // at maximum, iterate over 3 fields of study
                if (j == 3) break;

                // combine fields of study with spaces between them
                std::string curr = std::string(fos.find_field("name").get_string().value());
                fos_list += curr + ' ';

                ++j;
            }
        }

        // get rid of trailing space
        fos_list = fos_list.substr(0, fos_list.size() - 1);

        // insert the paper into the database
        paper::Entry to_insert(title, fos_list, n_citations, paper_year, author_vec, paper_id);
        paper_db.insert(paper_id, to_insert);

        ++i;
    }

    // save journal graph to disk (db files implicitly do this when out of scope)
    g.export_to_file("journalgraph.bin");
}

void build_author_graph(const std::string& filename) {
    // open the dblp json
    std::ifstream ifs(filename);

    // instantiate the paper db in read only format to prevent mutating it
    // only loads into memory as needed; speeds up when used for longer
    BTreeDB<paper::Entry> paper_db("paper_keys.db", "paper_values.db", false, true);

    // create author graph
    AuthorGraph g;

    // check for ifstream error
    if (!ifs.is_open()) {
        throw std::runtime_error("filename not valid");
    }

    // skip first line -- is bracket
    std::string line;
    std::getline(ifs, line);

    // keep track of number of things traversed
    size_t count = 0;
    // instantiate json parser
    ondemand::parser parser;

    // iterate over all lines of json
    while(std::getline(ifs, line)) {
        if (count % 10000 == 0) {
            std::cout << count << std::endl;
        }

        // get rid of leading comma if it exists
        if (line.at(0) == ',') {
            line = line.substr(1);
        }

        // convert the current line to a workable json, if possible (if not, skip)
        padded_string temp(line);
        ondemand::document curr;

        if (parser.iterate(temp).get(curr)) {
            std::cout << "error parsing line " + std::to_string(count) << std::endl;
            continue;
        }

        // if id, title, year, or citations are not found, skip; extract number of citations
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

        // traverse of authors of paper and store them (if authors not found, skip)
        std::vector<unsigned long> author_vec;

        ondemand::array authors;
        auto author_error = curr["authors"].get_array().get(authors);
        if (author_error) {
            continue;
        }

        int num_authors = 0;

        for (ondemand::object author : authors) {
            // only allow AUTHOR_EDGE_LIMIT authors to be worked with at a time
            if (num_authors == AUTHOR_EDGE_LIMIT) break;

            unsigned long id = author["id"];
            author_vec.push_back(id);

            ++num_authors;
        }

        // build connections for these coauthors of the paper
        g.add_same_paper_authors(author_vec, n_citations);

        // traverse through the references of this paper
        ondemand::array refs;
        auto refs_error = curr["references"].get_array().get(refs);
        if (refs_error) {
            
        } else {
            for (unsigned long id : refs) {
                // get the data for this paper id
                paper::Entry cur_paper = paper_db.find(id);

                // if this paper doesn't have authors or has id -1 (invalid), skip
                if (cur_paper.authors[0] == 0) continue;
                if (cur_paper.id == -1) continue;

                // add connections between the authors of this paper and those in the referenced paper (which is max 8)
                g.add_referenced_authors(author_vec, cur_paper.authors, n_citations, cur_paper.n_citations);
            }
        }

        ++count;
    }

    // save author graph to disk
    g.export_to_file("author_graph.bin");
}
