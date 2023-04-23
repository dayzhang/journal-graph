#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>
#include <fstream>
#include <exception>
#include <tuple>
#include <array>
#include <iostream>

#define CACHE_SIZE 10

const unsigned int PAGE_SIZE = 4096;

const unsigned int NAME_SIZE = 56;
const unsigned int ENTRY_SIZE = NAME_SIZE + 8;
const unsigned int ENTRIES_PER_PAGE = PAGE_SIZE / ENTRY_SIZE;

class VectorDatabase {
    public:
        VectorDatabase(const std::string& filename) {
            std::fstream fs(filename, std::ios::binary | std::ios::in | std::ios::out);
            
            if (!fs.is_open()) {
                fs.open(filename, std::ios::binary | std::ios::trunc | std::fstream::out);
                fs.close();
                fs.open(filename, std::ios::binary | std::ios::in | std::ios::out);
            }

            if (!fs.is_open()) {
                throw std::runtime_error("error reading/creating file");
            }

            file_handler = std::move(fs);

            file_handler.seekg(0, std::ios::end);
            file_length = file_handler.tellg();
            if (file_length != 0) {
                file_handler.seekg(-4, std::ios::end);
                char temp[4];
                file_handler.read(temp, 4);
                memcpy(&num_entries, temp, 4);
            } else {
                num_entries = 0;
            }
            
        }

        ~VectorDatabase() {
            for (unsigned int i = 0; i < cache.size(); ++i) {
                if (std::get<1>(cache.at(i))) {
                    write_page(std::get<0>(cache.at(i)));
                }
            }
            char temp[4];
            memcpy(temp, &num_entries, 4);
            file_handler.seekg(-4, std::ios::end);
            file_handler.write(temp, 4);
            file_handler.close();
        }

        void insert(long key, const std::string& value) {
            Entry entry;
            entry.id = key;
            if (value.size() >= NAME_SIZE) {
                strcpy(entry.name, value.substr(0, NAME_SIZE - 1).c_str());
            } else {
                strcpy(entry.name, value.c_str());
            }
            
            unsigned int i = 0;
            for ( ; i < num_entries; ++i) {
                char* curr_ptr = row_slot(i);
                Entry curr_data;
                convert_bin_to_row(curr_ptr, &curr_data);
                if (curr_data.id == key) {
                    int page_num = i / ENTRIES_PER_PAGE;
                    convert_row_to_bin(&entry, curr_ptr);
                    std::get<1>(cache.at(page_num % CACHE_SIZE)) = true;
                    return;
                }
            }

            // key is not present in the db

            // no more room
            if (file_length - num_entries * ENTRY_SIZE <= ENTRY_SIZE) {
                Page new_page;
                new_page.fill(0);
                file_handler.seekg(0, std::ios::end);
                file_handler.write(new_page.data(), PAGE_SIZE);
                file_length += PAGE_SIZE;
            } 
            int page_num = i / ENTRIES_PER_PAGE;
            char* curr_ptr = row_slot(i);
            convert_row_to_bin(&entry, curr_ptr);
            std::get<1>(cache.at(page_num % CACHE_SIZE)) = true;
            ++num_entries;
            
        }

        std::string select(long key) {
            for (unsigned int i = 0 ; i < num_entries; ++i) {
                char* curr_ptr = row_slot(i);
                Entry curr_data;
                convert_bin_to_row(curr_ptr, &curr_data);
                if (curr_data.id == key) {
                    return curr_data.name;
                }
            }
            return "null";
        }

    private:
        struct Entry {
            long id;
            char name[NAME_SIZE];
        };

        typedef std::array<char, PAGE_SIZE> Page;

        unsigned int num_entries;
        std::array<std::tuple<unsigned int, bool, Page, bool>, CACHE_SIZE> cache;
        unsigned int file_length;
        std::fstream file_handler;

        unsigned int get_page_idx(unsigned int page_num) {
            return page_num % cache.size();
        }

        void write_page(unsigned int page_num) {
            file_handler.seekg(page_num * PAGE_SIZE, std::ios::beg);

            char* curr_page = get_page(page_num);
            file_handler.write(curr_page, PAGE_SIZE);
        }

        char* get_page(unsigned int page_num) {
            unsigned int num_pages = file_length / PAGE_SIZE;

            if (page_num > num_pages) {
                throw std::runtime_error("requested page out of bounds");
            }

            auto& cache_get = cache[get_page_idx((page_num))];

            // cache miss (block doesn't have right num or block is empty)
            if (std::get<0>(cache_get) != page_num || !std::get<3>(cache_get)) {
                Page& page = std::get<Page>(cache_get);
                // block no longer empty
                std::get<3>(cache_get) = true;

                // if block is occupied by something else and it is dirty, writeback
                if (std::get<0>(cache_get)) {
                    write_page(std::get<unsigned int>(cache_get));
                }

                char buffer[PAGE_SIZE];
                file_handler.seekg(page_num * PAGE_SIZE, std::ios::beg);

                file_handler.read(buffer, PAGE_SIZE);
                memcpy(page.data(), buffer, PAGE_SIZE);

                std::get<0>(cache_get) = page_num;
                std::get<2>(cache_get) = page;
                std::get<1>(cache_get) = false;
            }

            return std::get<2>(cache_get).data();
        }

        char* row_slot(unsigned int row_num) {
            unsigned int page_num = row_num / ENTRIES_PER_PAGE;

            char* page_ptr = get_page(page_num);
            return page_ptr + (row_num % ENTRIES_PER_PAGE) * ENTRY_SIZE;
        }

        void convert_row_to_bin(Entry* source, char* dest) {
            memcpy(dest, &(source->id), 8);
            memcpy(dest + 8, &(source->name), NAME_SIZE);
        }

        void convert_bin_to_row(char* source, Entry* dest) {
            memcpy(&(dest->id), source, 8);
            memcpy(&(dest->name), source + 8, NAME_SIZE);
        }


};