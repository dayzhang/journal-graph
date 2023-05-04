#pragma once
#include <vector>

struct author_parse_wrapper {
    unsigned long source;
    std::vector<unsigned long> cited;
    std::vector<unsigned long> authors;
    author_parse_wrapper(unsigned long src) : source(src) {};
};