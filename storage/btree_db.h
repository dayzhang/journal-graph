#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <tuple>
#include <array>
#include <climits>
#include <cstring>

#define CACHE_SIZE 10
#define ORDER 337
#define PAGE_SIZE 4096
#define KEYENTRY_SIZE 12
#define HEADER_SIZE 16
#define ENTRY_SIZE 40

enum FileType {
    Key,
    Value
};

class BTreeDB {
    private: 
        struct ValueEntry {
            char name[40];
        };

        typedef std::array<char, PAGE_SIZE> Page;

        struct CacheBlock {
            unsigned int page_num;
            bool dirty;
            bool valid;
            Page page;
            CacheBlock(): page_num(0), dirty(false), valid(false) {
                page.fill(CHAR_MAX);
            }
        };

        struct Header {
            unsigned int num_cells;
            char node_type;
            char is_root;
            unsigned int parent_ptr;
            unsigned int next_ptr;
        };

        unsigned int num_value_pages;
        unsigned int num_key_pages;
        unsigned int num_entries;

        unsigned int key_root;

        std::array<CacheBlock, CACHE_SIZE> key_cache;
        std::array<CacheBlock, CACHE_SIZE> value_cache;

        std::fstream key_handler;
        std::fstream value_handler;
        std::fstream meta_handler;

    public:
        BTreeDB(const std::string& key_filename, const std::string& values_filename);

        ~BTreeDB();

        void insert(long key, const std::string& value);
        ValueEntry find(long key);

    private:
        class KeyPageInterface {
            public:
                KeyPageInterface(unsigned int page_num, BTreeDB* tree);

                unsigned int get_num_cells() const;
                void set_num_cells(unsigned int x);

                bool is_internal() const;
                void set_internal(bool x);

                bool is_root() const;
                void set_root(bool x);

                unsigned int get_child_ptr(unsigned int entry_num) const;
                void set_child_ptr(unsigned int target, unsigned int entry_num);

                unsigned int get_parent_ptr() const;
                void set_parent_ptr(unsigned int x);

                long get_key(unsigned int entry_num) const;
                unsigned int get_page_num() const;

                unsigned int get_next_ptr() const;
                void set_next_ptr(unsigned int ptr);

                void insert(long key, unsigned int child_ptr, unsigned int loc);
                void push(long key, unsigned int child_ptr);
                void change_page(unsigned int page_num);
                void move_keys(KeyPageInterface& other);

            private:
                void handle_set();

                unsigned int page_num_;
                BTreeDB* tree_;
                char* data;
        };

        class ValuePageInterface {
            public:
                ValuePageInterface(BTreeDB* tree);

                ValueEntry get_value(unsigned int entry_num) const;
                void set_value(ValueEntry& entry, unsigned int entry_num);
                unsigned int push(ValueEntry& entry);

                unsigned int get_value_size(unsigned int page_num) const;
                void set_value_size(unsigned int page_num, unsigned int size);
                
            private:
                BTreeDB* tree_;
        };

        unsigned int split_page(KeyPageInterface iter) {
            if (iter.is_root()) {

                KeyPageInterface new_root = create_new_keypage();
                KeyPageInterface right = create_new_keypage();
                
                new_root.set_root(true);
                new_root.set_num_cells(1);
                new_root.set_internal(true);
                new_root.push(iter.get_key(ORDER / 2), right.get_page_num());
                new_root.set_child_ptr(iter.get_page_num(), 0);

                key_root = new_root.get_page_num();

                iter.set_root(false);
                iter.set_internal(false);
                iter.set_parent_ptr(new_root.get_page_num());
                iter.set_num_cells(PAGE_SIZE / 2 + 1);

                right.set_root(false);
                right.set_internal(false);
                right.set_parent_ptr(new_root.get_page_num());
                right.set_num_cells(PAGE_SIZE / 2);


                
            }

            return 0;
        }

        KeyPageInterface create_new_keypage();

        unsigned int find_pos(KeyPageInterface& iter, long key) const;

        void set_dirty(unsigned int page_num, FileType type);

        unsigned int get_page_idx(unsigned int page_num) const;

        void write_page(unsigned int page_num, FileType type);

        CacheBlock& get_cache_block(unsigned int page_num, FileType type);

        char* get_page(unsigned int page_num, FileType type);

        void serialize_value(const ValueEntry* const source, char* dest) const;
        void deserialize_value(const char* const source, ValueEntry* dest) const;

        // void serialize_header(Header* source, char* dest) {
        //     memcpy(dest, &(source->num_cells), 4);
        //     memcpy(dest + 4, &(source->node_type), 1);
        //     memcpy(dest + 5, &(source->is_root), 1);
        //     memcpy(dest + 8, &(source->parent_ptr), 4);
        // }

        // void deserialize_header(char* source, Header* dest) {
        //     memcpy(&(dest->num_cells), source, 4);
        //     memcpy(&(dest->node_type), source + 4, 1);
        //     memcpy(&(dest->is_root), source + 5, 1);
        //     memcpy(&(dest->parent_ptr), source + 8, 4);
        // }
};