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
                if (cache.at(i).dirty) {
                    write_page(cache.at(i).page_num);
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
                    cache.at(page_num % CACHE_SIZE).dirty = true;
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
            cache.at(page_num % CACHE_SIZE).dirty = true;
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

        struct CacheBlock {
            unsigned int page_num;
            bool dirty;
            Page page;
            bool valid;
            CacheBlock(): page_num(0), dirty(false), valid(false) {
                page.fill(0);
            }
        };

        unsigned int num_entries;
        std::array<CacheBlock, CACHE_SIZE> cache;
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
            if (cache_get.page_num != page_num || !cache_get.valid) {
                Page& page = cache_get.page;
                // block no longer empty
                cache_get.valid = true;

                // if block is occupied by something else and it is dirty, writeback
                if (cache_get.dirty) {
                    write_page(cache_get.page_num);
                }

                char buffer[PAGE_SIZE];
                file_handler.seekg(page_num * PAGE_SIZE, std::ios::beg);

                file_handler.read(buffer, PAGE_SIZE);
                memcpy(page.data(), buffer, PAGE_SIZE);

                cache_get.page_num = page_num;
                cache_get.page = page;
                cache_get.dirty = false;
            }

            return cache_get.page.data();
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