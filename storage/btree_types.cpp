#include <array>
#include <cstring>
#include <string>
#include <vector>

namespace author {
    struct Entry {
        std::array<char, 40> name;
        std::array<char, 100> organization;
        long id;

        Entry(const std::string& author_name, const std::string& author_org, long set_id): id(set_id) {
            if (author_name.size() >= 40) {
                strcpy(name.data(), author_name.substr(0, 39).c_str());
            } else {
                strcpy(name.data(), author_name.c_str());
            }

            if (author_org.size() >= 100) {
                strcpy(organization.data(), author_org.substr(0, 99).c_str());
            } else {
                strcpy(organization.data(), author_org.c_str());
            }
        }

        Entry() {}

        static void deserialize_value(char* source, Entry* dest) {
            memcpy(dest->name.data(), source, 40);
            memcpy(dest->organization.data(), source + 40, 100);
            memcpy(&(dest->id), source + 140, 8);
        }

        static void serialize_value(Entry* source, char* dest) {
            memcpy(dest, source->name.data(), 40);
            memcpy(dest + 40, source->organization.data(), 100);
            memcpy(dest + 140, &(source->id), 8);
        }

        static const unsigned int size = 148;

        bool operator=(const Entry& other) {
            return std::string(name.data()) == std::string(other.name.data());
        }
    };
}

namespace paper {
    struct Entry {
        std::array<char, 60> title;
        std::array<char, 60> keywords;
        std::array<char, 40> venue;
        std::array<long, 10> authors;

        unsigned int n_citations;
        unsigned int pub_year;
        long id;

        Entry(std::string& paper_title, std::string& paper_keywords, std::string& paper_venue, unsigned int paper_citations, unsigned int paper_year, std::vector<long> paper_authors, long set_id): n_citations(paper_citations), pub_year(paper_year), id(set_id) {
            if (title.size() >= 60) {
                strcpy(title.data(), paper_title.substr(0, 59).c_str());
            } else {
                strcpy(title.data(), paper_title.c_str());
            }

            if (paper_keywords.size() >= 60) {
                strcpy(title.data(), paper_keywords.substr(0, 59).c_str());
            } else {
                strcpy(title.data(), paper_keywords.c_str());
            }

            if (paper_venue.size() >= 40) {
                strcpy(title.data(), paper_venue.substr(0, 39).c_str());
            } else {
                strcpy(title.data(), paper_venue.c_str());
            }

            for (unsigned int i = 0; i < 10; ++i) {
                authors[i] = paper_authors[i];
            }
        }

        Entry() {}

        static void deserialize_value(char* source, Entry* dest) {
            memcpy(dest->title.data(), source, 60);
            memcpy(dest->keywords.data(), source + 60, 60);
            memcpy(dest->venue.data(), source + 120, 40);
            memcpy(dest->authors.data(), source + 160, 80);
            memcpy(&(dest->n_citations), source + 240, 4);
            memcpy(&(dest->pub_year), source + 244, 4);
            memcpy(&(dest->id), source + 258, 8);
        }

        static void serialize_value(Entry* source, char* dest) {
            memcpy(dest, source->title.data(), 60);
            memcpy(dest + 60, source->keywords.data(), 60);
            memcpy(dest + 120, source->venue.data(), 40);
            memcpy(dest + 160, source->authors.data(), 80);
            memcpy(dest + 240, &(source->n_citations), 4);
            memcpy(dest + 244, &(source->pub_year), 4);
            memcpy(dest + 248, &(source->id), 8);
        }

        static const unsigned int size = 256;

        bool operator=(const Entry& other) {
            return std::string(title.data()) == std::string(other.title.data());
        }
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