#pragma once
#include "json.hpp"
#include "../lib/graph_defines.h"
#include <fstream>
#include <iostream>
using namespace std;
using json = nlohmann::json;
// https://medium.com/ml2b/a-guide-to-json-using-c-a48039124f3a

int dataset_parse()
{
    std::ifstream f("../data/sample_data_unserialized.json");
    json data = json::parse(f);
    // Access the values existing in JSON data
    auto& array_of_json = data; // extracts array of json strings
    // Print the values
    for (auto json_line : array_of_json) {
        std::cout << "Paper id is: " << json_line.value("id", "not found")  << " with authors:      ";
        for (auto& elem : json_line["authors"]) {
            std::cout << elem.value("id", "not_found") << " | ";
        } 
        std::cout << "\n";
        // extracts the id from a json line to a string
    }
    return 1;
}

void big_clean(std::string input_file) {
    int count = 0;
    json::parser_callback_t cb = [&count](int depth, json::parse_event_t event, json& parsed) {
        (void) depth;
        if (count % 100 == 0) {
            std::cout << "at " << count << " elements so far\n";
        }
        if (event == json::parse_event_t::key and !(parsed == json("id") || parsed == json("references") || parsed == json("authors")))
        {
            return false;
        } else if (event == json::parse_event_t::object_end) {
            count++;
        }
        return true;
    };
    std::ifstream f(input_file);
    auto these = json::parse(f, cb);

    std::ofstream file("cleaned_dblp_v14.json");
    file << these;
}

//2DV to take advantage of cache locality
//organized as source paper (first will have 1 element in first, none in second)
int parse_authors(std::vector<author_parse_wrapper>& parsed_data, std::string input_file) {
    std::ifstream f(input_file);
    json data = json::parse(f);
    // Access the values existing in JSON data
    auto& array_of_json = data; // extracts array of json strings
    // Print the values

    //All papers with no references are cleaned out 

    for (auto json_line : array_of_json) {

        unsigned long id;
        try {
            id = json_line["id"];
        } catch (exception& e) {
            std::cout << e.what() << "\n";
            continue;
        }
        author_parse_wrapper new_line_of_data(id);

        std::vector<unsigned long>& references = new_line_of_data.cited;
        std::vector<unsigned long>& authors = new_line_of_data.authors;

        for (auto& elem : json_line["references"]) {
            references.push_back(elem);
        } 

        for (auto& author : json_line["authors"]) {
            long long val;
            try {
                val = author["id"];
            } catch (exception& e) {
                std::cout << e.what() << "\n";
                continue;
            }
            if (val > 0) {
                authors.push_back(val);
            } else {
                std::cout << "Missing Author ID " << "\n";
            }
        }

        if (authors.size() > 0) {
            parsed_data.push_back(new_line_of_data);
        } else {
            std::cout << "No authors or references found for this paper, ommiting line.\n";
        }
    }
    return 1;
}

//2DV to take advantage of cache locality
int parse_references(std::vector<std::vector<unsigned long>>& parsed_data, std::string input_file) {
    std::ifstream f(input_file);
    json data = json::parse(f);
    // Access the values existing in JSON data
    auto& array_of_json = data; // extracts array of json strings
    // Print the values

    //All papers with no references are cleaned out 

    for (auto json_line : array_of_json) {
        std::vector<unsigned long> new_line_of_data;
        unsigned long id;
        try {
            id = json_line["id"];
        } catch (exception& e) {
            std::cout << e.what() << "\n";
            continue;
        }
        new_line_of_data.push_back(id);

        for (auto& elem : json_line["references"]) {
            new_line_of_data.push_back(elem);
        } 

        if (new_line_of_data.size() > 1) {
            parsed_data.push_back(new_line_of_data);
        } else {
            std::cout << "No references found for this paper, ommiting line.\n";
        }
        // extracts the id from a json line to a string
    }

    //clean data

    return 1;
}
/*
//2DV to take advantage of cache locality
int parse_references_v12(std::vector<std::vector<long>>& parsed_data, std::string input_file) {
    std::ifstream f(input_file);
    json data = json::parse(f);
    // Access the values existing in JSON data
    auto& array_of_json = data; // extracts array of json strings
    // Print the values

    //All papers with no references are cleaned out 

    for (auto json_line : array_of_json) {
        std::vector<long> new_line_of_data;
        try {
            long long id = json_line["v12_id"];
        } catch (exception& e) {
            std::cout << e.what() << "\n";
            continue;
        }
        long long id = json_line["v12_id"];
        new_line_of_data.push_back(id);

        for (auto& elem : json_line["references"]) {
            new_line_of_data.push_back(-1);
        } 
        if (new_line_of_data.size() > 1) {
            parsed_data.push_back(new_line_of_data);
        } else {
            std::cout << "No references found for this paper, ommiting line.\n";
        }
        // extracts the id from a json line to a string
    }

    //clean data

    return 1;
}
*/
