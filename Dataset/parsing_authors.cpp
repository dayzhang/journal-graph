#include "simdjson.h"
#include "parsing_authors.h"

#include <fstream>
#include <string>
#include <sstream>
#include <exception>
#include <iostream>

void traverse_data(const std::string &filename) {
    // std::ifstream ifs("../data/" + filename);

    // if (!ifs.is_open()) {
    //     throw std::runtime_error("invalid file name");
    // }

    // int i = 0;
    // std::string line;
    // while (std::getline(ifs, line)) {
    //     if (i % 100000 == 0) { 
    //         std::cout << i << std::endl;
    //     }
    //     if (line.at(0) == ',') {
    //         line = line.substr(1);
    //     }
    //     try {
    //         // simdjson::ondemand::parser parser;
    //         // simdjson::padded_string temp(line);
    //         // simdjson::ondemand::document curr = parser.iterate(line);

    //         // if (!curr.contains("id") || !curr.contains("title") || !curr.contains("authors") || !curr.contains("year") || !curr.contains("references") ||
    //         //     !curr.contains("n_citation")) { 
    //         //     std::cout << "line " + std::to_string(i) + " is malformed" << std::endl;
    //         // }
    //     }
    //     catch (const simdjson::simdjson_error& e) {
    //         std::cout << "error parsing: line " + std::to_string(i) << std::endl;
    //         std::cout << std::endl;
    //     }
    //     ++i;
    // }
}