#pragma once
#include "json.hpp"
#include <fstream>
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
        std::cout << json_line.value("id", "not found") << " " << json_line.value("title", "not found") << " " << json_line.value("abstract", "not found") << "\n"; 
        // extracts the id from a json line to a string
    }
    return 1;
}
