#pragma once

#include <array>
#include <cstring>
#include <string>
#include <vector>
#include <iostream>

#define NULL_VAL -1

/**
    Template ValueEntry types for the BTrees are defined here. Each must have member variables containing all relevant data, an id field, a default constructor, a static size member detailing the size of the struct in bytes, a static deserialization function, and serialization function
*/

namespace author {
    struct Entry {
        // an author entry stores the author name, the author organization, and the author data
        std::array<char, 32> name;
        std::array<char, 56> organization;
        long id;

        /**
            Full constructor for an author entry

            @param author_name name of the author
            @param author_org the organization the author works at
            @param set_id the id of the author
        */
        Entry(const std::string& author_name, const std::string& author_org, long set_id): id(set_id) {
            // 0 initialize arrays
            name.fill(0);
            organization.fill(0);

            // truncate passed in strings to fit within the member arrays if needed (substr if equal to account for null terminator)
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

        /**
            Name constructor for author entry. Used for id lookup convenience.

            @param author_name name of the author
        */
        Entry(const std::string& author_name): id(0) {
            // truncate to fit in char array
            if (author_name.size() >= 32) {
                strcpy(name.data(), author_name.substr(0, 31).c_str());
            } else {
                strcpy(name.data(), author_name.c_str());
            }

            // 0 initialize
            organization.fill(0);
        }

        /**
            Default constructor for author entry. Never used in reality except as a temporary variable or as an invalid result (with id -1)
        */
        Entry(): id(NULL_VAL) {}

        /**
            Function to deserialize a char array to get the entry its data corresponds to

            @param source Start of char array to copy to the given entry
            @param dest Address of entry to copy data to
        */
        static void deserialize_value(char* source, Entry* dest) {
            memcpy(dest->name.data(), source, 32);
            memcpy(dest->organization.data(), source + 32, 56);
            memcpy(&(dest->id), source + 88, 8);
        }

        /**
            Function to serialize a ValueEntry into a char hex array

            @param source Address of entry to copy data from
            @param dest Start of char array to copy data to
        */
        static void serialize_value(Entry* source, char* dest) {
            memcpy(dest, source->name.data(), 32);
            memcpy(dest + 32, source->organization.data(), 56);
            memcpy(dest + 88, &(source->id), 8);
        }


        /**
            This struct occupies this many bytes
        */
        static const unsigned int size = 32 + 56 + 8;

        /**
            Equality operator. Only check the name strings are equal.

            @param other the other Entry to check equality with
        */
        bool operator==(const Entry& other) {
            return std::string(name.data()) == std::string(other.name.data());
        }
    };
}

namespace paper {
    struct Entry {
        // a paper entry contains the paper title, keywords, 8 authors at max, the number of citations it has, its publication year, and its id
        std::array<char, 96> title;
        std::array<char, 40> keywords;
        std::array<long, 8> authors;

        unsigned int n_citations;
        unsigned int pub_year;
        long id;

        /**
            Full constructor for an author entry

            @param paper_title title of the paper
            @param paper_keywords Keywords corresponding to the paper
            @param paper_citations the number of citations the paper has
            @param paper_year the year the paper was published in
            @param paper_authors array of authors of the paper (max 8)
            @param set_id id of the paper
        */
        Entry(std::string& paper_title, std::string& paper_keywords, unsigned int paper_citations, unsigned int paper_year, std::array<long, 8> paper_authors, long set_id): n_citations(paper_citations), pub_year(paper_year), id(set_id) {
            // 0 initialize to prevent undeifned behavior
            title.fill(0);
            keywords.fill(0);
            authors.fill(0);

            // truncate strings to fit inside char arrays
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

            // copy author data to this struct member
            memcpy(authors.data(), paper_authors.data(), 64);
        }

        /**
            Name constructor for author entry. Used for id lookup convenience.

            @param paper_title title of the paper
        */
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

        /**
            Default constructor for author entry. Never used in reality except as a temporary variable or as an invalid result (with id -1)
        */
        Entry(): id(NULL_VAL) {}

        /**
            Function to deserialize a char array to get the entry its data corresponds to

            @param source Start of char array to copy to the given entry
            @param dest Address of entry to copy data to
        */
        static void deserialize_value(char* source, Entry* dest) {
            memcpy(dest->title.data(), source, 96);
            memcpy(dest->keywords.data(), source + 96, 40);
            memcpy(dest->authors.data(), source + 136, 64);
            memcpy(&(dest->n_citations), source + 200, 4);
            memcpy(&(dest->pub_year), source + 204, 4);
            memcpy(&(dest->id), source + 208, 8);
        }

        /**
            Function to serialize a ValueEntry into a char hex array

            @param source Address of entry to copy data from
            @param dest Start of char array to copy data to
        */
        static void serialize_value(Entry* source, char* dest) {
            memcpy(dest, source->title.data(), 96);
            memcpy(dest + 96, source->keywords.data(), 40);
            memcpy(dest + 136, source->authors.data(), 64);
            memcpy(dest + 200, &(source->n_citations), 4);
            memcpy(dest + 204, &(source->pub_year), 4);
            memcpy(dest + 208, &(source->id), 8);
        }

        /**
            This struct occupies this many bytes
        */
        static const unsigned int size = 96 + 40 + 8 * 8 + 4 + 4 + 8;

        /**
            Equality operator. Only check if the title strings are equal.

            @param other the other Entry to check equality with
        */
        bool operator==(const Entry& other) {
            return std::string(title.data()) == std::string(other.title.data());
        }
    };
}

// dummy btree type for testing purposes
namespace test {
    struct Entry {
        int x;
        std::array<char, 12> str;
        long id;

        Entry(int set_x, const std::string& set_str, long set_id): x(set_x), id(set_id) {
            str.fill(0);
            if (set_str.size() >= 12) {
                strcpy(str.data(), set_str.substr(0, 11).c_str());
            } else {
                strcpy(str.data(), set_str.c_str());
            }
        }

        Entry(int search_x): x(search_x) {
            str.fill(0);
        }

        Entry(): id(NULL_VAL) {}

        static void deserialize_value(char* source, Entry* dest) {
            memcpy(&(dest->x), source, 4);
            memcpy(dest->str.data(), source + 4, 12);
            memcpy(&(dest->id), source + 16, 8);
        }

        static void serialize_value(Entry* source, char* dest) {
            memcpy(dest, &(source->x), 4);
            memcpy(dest + 4, source->str.data(), 12);
            memcpy(dest + 16, &(source->id), 8);
        }

        static const unsigned int size = 24;

        bool operator==(const Entry& other) {
            return x == other.x;
        }
    };
}