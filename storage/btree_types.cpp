#pragma once

#include <array>
#include <cstring>
#include <string>
#include <vector>
#include <iostream>

namespace author {
    struct Entry {
        std::array<char, 32> name;
        std::array<char, 56> organization;
        long id;

        Entry(const std::string& author_name, const std::string& author_org, long set_id): id(set_id) {
            // std::cout << author_name << std::endl;
            name.fill(0);
            organization.fill(0);
            // organization.fill(0);
            if (author_name.size() >= 32) {
                strcpy(name.data(), author_name.substr(0, 31).c_str());
            } else {
                strcpy(name.data(), author_name.c_str());
            }

            if (author_org.size() >= 56) {
                strcpy(organization.data(), author_org.substr(author_org.size() - 55).c_str());
            } else {
                strcpy(organization.data(), author_org.c_str());
            }
        }

        Entry() {}

        static void deserialize_value(char* source, Entry* dest) {
            memcpy(dest->name.data(), source, 32);
            memcpy(dest->organization.data(), source + 32, 56);
            memcpy(&(dest->id), source + 88, 8);
        }

        static void serialize_value(Entry* source, char* dest) {
            memcpy(dest, source->name.data(), 32);
            memcpy(dest + 32, source->organization.data(), 56);
            memcpy(dest + 88, &(source->id), 8);
        }

        static const unsigned int size = 32 + 56 + 8;

        bool operator=(const Entry& other) {
            return std::string(name.data()) == std::string(other.name.data());
        }
    };
}

namespace paper {
    struct Entry {
        std::array<char, 96> title;
        std::array<char, 40> keywords;
        std::array<long, 8> authors;

        unsigned int n_citations;
        unsigned int pub_year;
        long id;

        Entry(std::string& paper_title, std::string& paper_keywords, unsigned int paper_citations, unsigned int paper_year, std::array<long, 8> paper_authors, long set_id): n_citations(paper_citations), pub_year(paper_year), id(set_id) {
            title.fill(0);
            keywords.fill(0);
            authors.fill(0);

            if (title.size() >= 96) {
                strcpy(title.data(), paper_title.substr(0, 95).c_str());
            } else {
                strcpy(title.data(), paper_title.c_str());
            }

            if (paper_keywords.size() >= 40) {
                strcpy(keywords.data(), paper_keywords.substr(0, 39).c_str());
            } else {
                strcpy(keywords.data(), paper_keywords.c_str());
            }

            memcpy(authors.data(), paper_authors.data(), 64);
        }

        Entry(std::string& paper_title): n_citations(0), pub_year(0), id(0) {
            title.fill(0);
            keywords.fill(0);
            authors.fill(0);
            if (title.size() >= 96) {
                strcpy(title.data(), paper_title.substr(0, 95).c_str());
            } else {
                strcpy(title.data(), paper_title.c_str());
            }
        }

        Entry(): pub_year(0) {}

        static void deserialize_value(char* source, Entry* dest) {
            memcpy(dest->title.data(), source, 96);
            memcpy(dest->keywords.data(), source + 96, 40);
            memcpy(dest->authors.data(), source + 136, 64);
            memcpy(&(dest->n_citations), source + 200, 4);
            memcpy(&(dest->pub_year), source + 204, 4);
            memcpy(&(dest->id), source + 208, 8);
        }

        static void serialize_value(Entry* source, char* dest) {
            memcpy(dest, source->title.data(), 96);
            memcpy(dest + 96, source->keywords.data(), 40);
            memcpy(dest + 136, source->authors.data(), 64);
            memcpy(dest + 200, &(source->n_citations), 4);
            memcpy(dest + 204, &(source->pub_year), 4);
            memcpy(dest + 208, &(source->id), 8);
        }

        static const unsigned int size = 96 + 40 + 8 * 8 + 4 + 4 + 8;

        bool operator==(const Entry& other) {
            return std::string(title.data()) == std::string(other.title.data());
        }

        Entry(const paper::Entry& other) = default;
    };
}

namespace test {
    struct Entry {
        int x;
        long id;
        Entry(int set_x, long set_id): x(set_x), id(set_id) {}
        Entry() {}

        static void deserialize_value(char* source, Entry* dest) {
            memcpy(&(dest->x), source, 4);
            memcpy(&dest->id, source + 4, 8);
        }

        static void serialize_value(Entry* source, char* dest) {
            memcpy(dest, &(source->x), 4);
            memcpy(dest + 4, &(source->id), 8);
        }

        static const unsigned int size = 12;

        bool operator=(const Entry& other) {
            return x == other.x;
        }
    };
}