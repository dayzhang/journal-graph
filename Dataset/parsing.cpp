#pragma once
#include "json.hpp"
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
        if (count % 10000 == 0) {
            std::cout << "at " << count << " elements so far\n";
        }
        if (event == json::parse_event_t::key and !(parsed == json("id") || parsed == json("references") || parsed == json("authors")))
        {
            return false;
        } else if (event == json::parse_event_t::object_end) {
            count++;
            return true;
        }
    };
    std::ifstream f(input_file);
    auto these = json::parse(f, cb);

    std::ofstream file("cleaned_dblp_v14.json");
    file << these;
}

//2DV to take advantage of cache locality
using author_parse_wrapper = std::pair<std::vector<std::string>, std::vector<std::string>>;
//organized as source paper (first will have 1 element in first, none in second)
int parse_authors(std::vector<author_parse_wrapper>& parsed_data, std::string input_file) {
    std::ifstream f(input_file);
    json data = json::parse(f);
    // Access the values existing in JSON data
    auto& array_of_json = data; // extracts array of json strings
    // Print the values

    //All papers with no references are cleaned out 

    for (auto json_line : array_of_json) {

        author_parse_wrapper new_line_of_data;

        if (json_line.value("id", "not found") == "not_found") {
            std::cout << "failed to find paper id, omitting line \n";
            continue;
        }

        std::vector<std::string>& references = new_line_of_data.first;
        std::vector<std::string>& authors = new_line_of_data.second;
        references.push_back(json_line.value("id", "not found"));

        for (auto& elem : json_line["references"]) {
            references.push_back(elem);
        } 

        for (auto& author : json_line["authors"]) {
            authors.push_back(author.value("id", "not found"));
        }

        if (references.size() > 1 && authors.size() > 0) {
            parsed_data.push_back(new_line_of_data);
        } else {
            std::cout << "No authors or references found for this paper, ommiting line.\n";
        }
    }
    return 1;
}

//2DV to take advantage of cache locality
int parse_references(std::vector<std::vector<std::string>>& parsed_data, std::string input_file) {
    std::ifstream f(input_file);
    json data = json::parse(f);
    // Access the values existing in JSON data
    auto& array_of_json = data; // extracts array of json strings
    // Print the values

    //All papers with no references are cleaned out 

    for (auto json_line : array_of_json) {
        std::vector<std::string> new_line_of_data;

        if (json_line.value("id", "not found") == "not_found") {
            std::cout << "failed to find paper id, omitting line \n";
            continue;
        }
        new_line_of_data.push_back(json_line.value("id", "not found"));

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