#pragma once

#include <string>
#include <fstream>
#include <array>
#include <climits>
#include <iostream>
#include <cassert>
#include <stack>
#include <cstdint>
#include <stdexcept>
#include <exception>
#include <cstring>
#include <vector>
#include <sys/mman.h>

#define CACHE_SIZE 64
#define ORDER 337
#define PAGE_SIZE 4096

#define KEYENTRY_SIZE 12
#define HEADER_SIZE 16
// #define ENTRY_SIZE 44

#define DEFAULT_VAL 0

// const unsigned int ENTRIES_PER_PAGE = PAGE_SIZE / ENTRY_SIZE - 1;

enum FileType {
    Key,
    Value
};

/**
    This struct details how values are stored in the values database.
*/
// struct ValueEntry {
//     std::array<char, 40> name;
//     int temp;
// };

template <typename T>
class BTreeDB {
    private: 
        /**
            A page is a 4096 byte (4096 chars) region of data.
        */
        typedef std::array<char, PAGE_SIZE> Page;
        
        /**
            This struct is how cache blocks are stored. page_num is what page the block is referring to. dirty is whether the block should be written back in case of a cache miss. valid is whether the block is actually filled with valid data. page is the actual data contained within the block.
        */
        struct CacheBlock {
            unsigned int page_num;
            bool dirty;
            bool valid;
            Page page;
            CacheBlock(): page_num(0), dirty(false), valid(false) {
                page.fill(DEFAULT_VAL);
            }
        };

        const unsigned int values_per_page = PAGE_SIZE / T::size - 1;

        std::hash<unsigned int> h;

        /**
            A cache set is a pair of cache block with an lru bit.
        */
        typedef std::pair<std::array<CacheBlock, 2>, bool> CacheSet;

        /**
            This struct is how the header of key pages are structured, although it is unused. num_cells is the number of key-pointer pairs are in the page, node_type is whether the page is internal (1) or a leaf (not 1), is_root is whether the page is the root (1) or not (not 1), parent_ptr is the page number of the parent pointer or CHAR_MAX if it doesn't have one, and next_ptr is the next node on the same level the page points to (if this page is a leaf).
        */
        struct Header {
            unsigned int num_cells;
            char node_type;
            char is_root;
            // unsigned int parent_ptr;
            // unsigned int next_ptr;
        };

        /**
            Number of pages in the values vector database.
        */
        unsigned int num_value_pages;
        /**
            Number of pages in the key-pointer database.
        */
        unsigned int num_key_pages;
        /**
            Number of total key-value entries in the database. Should apply ot both the value and key pages. 
        */
        unsigned int num_entries;

        /**
            The number of the page that is the root.
        */
        unsigned int key_root;

        /**
            Cache for the key pages. Has 10 cache sets and is 2-way associative.
        */
        std::array<CacheSet, CACHE_SIZE> key_cache;
        /**
            Cache for the value pages. Has 10 cache sets and is 2-way associative.
        */
        std::array<CacheSet, CACHE_SIZE> value_cache;

        /**
            File handler for the key database (read and write access).
        */
        std::fstream key_handler;
        /**
            File handler for the value database (read and write access).
        */
        std::fstream value_handler;
        /**
            File handler for the metadata for the database (only write access).
        */
        std::fstream meta_handler;

        /**
            Dummy array filled with 0's for copying.
        */
        char empty_array[PAGE_SIZE];

    public:
        /**
            Constructor for the BTree database. Either creates a new database if the filename doesn't refer to anything or instantitates a previously created database if the filenames do refer to something. The key and value databases should be compatible with each other.

            @param key_filename The filename for the key database
            @param values_filename The filename for the value database
        */
        BTreeDB(const std::string& key_filename, const std::string& values_filename, bool create_new=false);

        /**
            Destructor for the BTree database. Iterates through the cache block and writebacks any still-dirty blocks of data to the databases and closes the file handlers.
        */
        ~BTreeDB();

        /**
            Inserts a key-value pair into the database according to the BTree structure.

            @param key The key
            @param value The value the key is associated with
        */
        void insert(long key, T& value);
        /**
            Retrieves a value from the database according to a key.

            @param key The key to lookup
            @return The ValueEntry the key is associated with, or the default ValueEntry if it was not found.
        */
        T find(long key);

        long get_id_from_name(const T& search_val);

    private:
    
        /**
            This class is a wrapper for working with individual key pages. Supports both reading data and writing data to the key page.

            NOTE: For a given page number, only one interface should ever exist in memory at once, or the interfaces will be out of sync. (TODO: possibly sync them, if needed)
        */
        class KeyPageInterface {
            public:
                /**
                    Constructur for the interface.

                    @param page_num The page number this interface should wrap.
                    @param tree The BTreeDB this page is within. Used for accessing member functions/data.
                */
                KeyPageInterface(unsigned int page_num, BTreeDB* tree);

                /**
                    Return the number of filled cells in the page.

                    @return number of filled cells in this page; UINT_MAX if null
                */
                unsigned int get_size() const;
                /**
                    Sets the number of cells stored in the page to something else. This doesn't actually add any cells, but only changes the internal member.

                    @param x The number to change this page's size to
                */
                void set_size(unsigned int x);

                /**
                    Returns whether this page is internal.

                    @return true (internal) or false (leaf)
                */
                bool is_internal() const;
                /**
                    Sets this page's metadata for whether it is internal. Doesn't actually change its representation.

                    @param x true (internal) or false (leaf) value to set the internal flag of this page to
                */
                void set_internal(bool x);

                /**
                    Returns whether this page is a root.

                    @return true (is root) or false (not root), UINT_MAX if null
                */
                bool is_root() const;
                /**
                    Sets this page's flag for whether it is a root. Doesn't actually change its representation.

                    @param x true (is root) or false (not root) to set the root flag of this page to
                */
                void set_root(bool x);

                /**
                    Gets the page number (if it is internal) or entry number (if it is a root) of the nth entry within the key page. 

                    @param entry_num 0 to size if internal and 1 to size if root 
                    @return child bucket number (if internal) or value entry num (if leaf), UINT_MAX if null
                */
                unsigned int get_child_ptr(unsigned int entry_num) const;
                /**
                    Sets the nth child pointer in the page to the target value.

                    @param target the value to set the child pointer value to
                    @param entry_num the number child pointer to set; 0 to size if internal and 1 to size if root
                */
                void set_child_ptr(unsigned int target, unsigned int entry_num);

                // /**
                //     Returns the parent pointer of the current page. 

                //     @return parent pointer (bucket number) of parent; UINT_MAX if null
                // */
                // unsigned int get_parent_ptr() const;
                // /**
                //     Sets the parent pointer of the current page to a given value.

                //     @param x The parent pointer (page number) to set
                // */
                // void set_parent_ptr(unsigned int x);

                /**
                    Returns the nth key of the current page.

                    @param entry_num the entry number to get a key for, 0 to size - 1
                    @return the key of the entry specified by the entry number
                */
                long get_key(unsigned int entry_num) const;
                /**
                    Returns the page number this interface refers to

                    @return the page number this interface refers to; UINT_MAX if null
                */
                unsigned int get_page_num() const;

                // /**
                //     Returns the next pointer of this page

                //     @return next pointer (page number), UINT_MAX if null
                // */
                // unsigned int get_next_ptr() const;
                // /**
                //     Set the next pointer of this page

                //     @param ptr pointer (page number) to set this page's next pointer to
                // */
                // void set_next_ptr(unsigned int ptr);

                /**
                    Pushes a new key/child_ptr key pair in sorted order.

                    @param key Key to insert
                    @param child_ptr child pointer to insert following the key
                */
                void push(long key, unsigned int child_ptr);
                /**
                    Moves the latter half of the keys in this interface to another interface. Used for splitting nodes.

                    @param other A reference to the interface to copy keys to (should be empty)
                */
                void move_keys(KeyPageInterface& other);

                /**
                    Helper function to do binary search on a given keypage interface.

                    @param iter A reference to the interface for the target key page
                    @param key The key to do binary search on

                    @return The key index that is the first key index that is greater than the passed in key
                */
                unsigned int find_pos(long key) const;

                 /**
                    Splits a key page according to the BTree logic. Should only be called on a full key page. 

                    Splits the latter half of the keys to a new key page and pushes the middle key to the parent. If there is no parent, creates a new parent key page.

                    @param iter An interface to the key page to split
                */
                void split_page();

                void split_page(KeyPageInterface& parent);

            private:
                /**
                    Helper function to mark the current page number as dirty for writeback later. Used in all non-const functions in this class.
                */
                void handle_set();

                /**
                    Helper function to get the data for the given page. Called instead of stored to ensure that data remains in sync.
                */
                char* get_data() const;

                /**
                    Inserts a key entry (key + child_ptr) at the given location, moving everything else to the right accordingly.

                    @param key Key of the entry to insert
                    @param child_ptr child pointer that follows this key
                    @param loc location to insert this entry into; 0 to size
                */
                void insert(long key, unsigned int child_ptr, unsigned int loc);

                /**
                    The page number this interface refers to.
                */
                unsigned int page_num_;
                /**
                    A pointer to the tree this interface is serving. Used to access functions.
                */
                BTreeDB* tree_;
        };

        /**
            This class is a wrapper for accessing the value database. It wraps the entire database and not any particular page.
        */
        class ValuePageInterface {
            public:
                /**
                    Constructor for value page interface.

                    @param tree A pointer to the tree this interface is accessing. Used to allow function access.
                */
                ValuePageInterface(BTreeDB* tree);

                /**
                    Get the nth value in the value database.

                    @param entry_num N (number of entry to get value for); 0 to num_entries - 1
                    @return A ValueEntry object that can be unpacked
                */
                T get_value(unsigned int entry_num) const;
                /**
                    Sets the nth value in the value database to the passed value entry.

                    @param entry The ValueEntry to copy into the value database
                    @param entry_num The number value entry to change
                */
                void set_value(T& entry, unsigned int entry_num);
                /**
                    Push a new value entry record into the value database. Inserts into the last page, if possible, or creates a new page if the last page is full.

                    @param entry The entry to insert into the value database
                    @return The entry number that represents the newly inserted record
                */
                unsigned int push(T& entry);

                /**
                    Returns the number of entries in the passed in page number.
                    
                    @param page_num The page number of the value database to query
                    @return The size (number of entries) of the page 
                */
                unsigned int get_size(unsigned int page_num) const;
                /**
                    Sets the size of the passed in page_num. Does not change its internal representation otherwise.

                    @param size the value to set the size to
                    @param page_num The page number to change the size for
                */
                void set_size(unsigned int size, unsigned int page_num);
                
            private:
                BTreeDB* tree_;
        };

        /**
            Creates a new keypage.

            @return A keypage interface for the newly created keypage
        */
        KeyPageInterface create_new_keypage(unsigned int num_cells, bool internal, bool is_root, unsigned int parent_ptr = UINT_MAX, unsigned int next_ptr = UINT_MAX);

        void write_all();

        /**
            Helper function to set the page_num page dirty for writeback.

            @param page_num the page number to set dirty
            @param type which cache to set dirty
        */
        void set_dirty(unsigned int page_num, FileType type);

        /**
            Helper function for mapping page numbers to cache entries. Is currently a naive modulus

            @param page_num The page to get a mapping to cache for
        */
        unsigned int get_page_idx(unsigned int page_num) const;

        /**
            Helper function to write a page_num to the file database.

            @param page_num the page number to write
            @param type Specifies which database to write to
        */
        void write_page(unsigned int page_num, FileType type);

        /**
            Helper function to get the cache block associated with page_num. There is no guarantee the returned block is in-sync with the page_num, just gets the cache block that the page would map to.

            @param page_num the page number to get the associated cache block for
            @param type whether to read from the key or value caches
            @return a reference to the cacheblock the page num is associated with
        */
        std::pair<std::array<BTreeDB::CacheBlock, 2>, bool>& get_cache_set(unsigned int page_num, FileType type);

        /**
            Gets the actual data of a given page number. Handles cache misses and cache hits implicitly.

            @param page_num The page being requested
            @param type Whwether a key or value page is being requested
            @return the character array representing the page's binary data
        */
        char* get_page(unsigned int page_num, FileType type);

        /**
            Helper function to convert a valueentry to its binary character array representation.

            @param source Valueentry to convert to binary
            @param dest The character pointer to write the binary data to
        // */
        // void serialize_value(const T* const source, char* dest) const;
        // /**
        //     Helper function to convert binary data to a human readable ValueEntry

        //     @param source The source pointer to read binary data from
        //     @param dest The ValueEntry pointer to write to
        // */
        // void deserialize_value(const char* const source, ValueEntry* dest) const;

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

template <typename T>
BTreeDB<T>::BTreeDB(const std::string& key_filename, const std::string& values_filename, bool create_new) {
    std::fstream fs_keys;
    std::fstream fs_values;
    std::fstream fs_meta;

    std::string metadata_file = key_filename.substr(0, key_filename.size() - 3) + values_filename.substr(0, values_filename.size() - 3) + ".txt";

    if (create_new) {
        fs_keys.open(key_filename, std::ios::binary | std::ios::in | std::ios::out | std::ios::trunc);
        fs_values.open(values_filename, std::ios::binary | std::ios::in | std::ios::out | std::ios::trunc);
        fs_meta.open(metadata_file, std::ios::binary | std::ios::in | std::ios::out | std::ios::trunc);

        // uint64_t size = 2 * 1000 *1000 * 1000;
        // std::vector<char> data;
        // data.reserve(size);

        // for (int i = 0; i < size; ++i)
        // {
        //     data.push_back('a');
        // }

        // fs_values.write(data.data(), size);
        // num_value_pages = 262144;

        num_entries = num_key_pages = key_root = num_value_pages = 0;
    } else {
        fs_keys.open(key_filename, std::ios::binary | std::ios::in | std::ios::out);
        fs_values.open(values_filename, std::ios::binary | std::ios::in | std::ios::out);
        fs_meta.open(metadata_file, std::ios::binary | std::ios::in | std::ios::out);
    }
    
    if (!fs_keys.is_open()) {
        throw std::runtime_error("error reading/creating keys file");
    }

    if (!fs_values.is_open()) {
        throw std::runtime_error("error reading/creating values file");
    }

    if (!fs_meta.is_open()) {
        throw std::runtime_error("error reading/creating metadata file");
    }

    value_handler = std::move(fs_values);
    key_handler = std::move(fs_keys);

    value_handler.seekg(0, std::ios::end);
    key_handler.seekg(0, std::ios::end);
    unsigned int values_len = value_handler.tellg();
    unsigned int keys_len = key_handler.tellg();


    if (!create_new) {
        unsigned int curr = 0;

        fs_meta >> curr;
        num_entries = curr;
        fs_meta >> curr;
        num_value_pages = curr;
        fs_meta >> curr;
        num_key_pages = curr;
        fs_meta >> curr;
        key_root = curr;
    }

    fs_meta.close();
    fs_meta.open(metadata_file, std::ios::out | std::ios::trunc);

    std::cout << num_entries << ' ' << num_value_pages << ' ' << num_key_pages << ' ' << key_root << std::endl;

    meta_handler = std::move(fs_meta);
    for (unsigned int i = 0; i < PAGE_SIZE; ++i) {
        empty_array[i] = 0;
    }

    memset(&key_cache, 0, sizeof(key_cache));
    memset(&value_cache, 0, sizeof(value_cache));
}
template <typename T>
void BTreeDB<T>::write_all() {
    for (unsigned int i = 0; i < CACHE_SIZE; ++i) {
        for (CacheBlock& block : key_cache.at(i).first) {
            if (block.dirty) {
                write_page(block.page_num, Key);
            }
        }

        for (CacheBlock& block2 : value_cache.at(i).first) {
            if (block2.dirty) {
                write_page(block2.page_num, Value);
            }
        }
    }

    key_handler.close();
    value_handler.close();

    meta_handler << num_entries << std::endl;
    meta_handler << num_value_pages << std::endl;
    meta_handler << num_key_pages << std::endl;
    meta_handler << key_root << std::endl;

    meta_handler.close();
}

template <typename T>
BTreeDB<T>::~BTreeDB() {
    write_all();
}

template <typename T>
void BTreeDB<T>::insert(long key, T& value) {
    if (num_key_pages == 0) {
        KeyPageInterface key_iter = create_new_keypage(0, false, true);
        ValuePageInterface value_iter(this);

        key_iter.push(key, value_iter.push(value));

        return;
    }

    unsigned int curr_page = key_root;
    KeyPageInterface key_iter(curr_page, this);

    std::stack<unsigned int> traversal;

    while (key_iter.is_internal()) {
        traversal.push(key_iter.get_page_num());
        unsigned int target = key_iter.find_pos(key);
        unsigned int next_page = key_iter.get_child_ptr(target);
        key_iter = KeyPageInterface(next_page, this);
    }

    unsigned int target = key_iter.find_pos(key);
    ValuePageInterface value_iter(this);

    if (target < key_iter.get_size() && key_iter.get_key(target) == key) {
        value_iter.set_value(value, key_iter.get_child_ptr(target + 1));

        return;
    }

    key_iter.push(key, value_iter.push(value));
    if (key_iter.get_size() == ORDER) {
        while (!traversal.empty()) {
            if (key_iter.get_size() != ORDER) {
                break;
            }
            KeyPageInterface curr(traversal.top(), this);
            traversal.pop();
            key_iter.split_page(curr);
            key_iter = curr;
        }
        if (key_iter.get_size() == ORDER) {
                key_iter.split_page();
        }
    }
}

template <typename T>
T BTreeDB<T>::find(long key) {
    if (num_entries == 0) {
        write_all();
        throw std::runtime_error("database is empty");
    }

    unsigned int curr_page = key_root;
    KeyPageInterface iter(curr_page, this);
    while (iter.is_internal()) {
        unsigned int target = iter.find_pos(key);
        unsigned int next_page = iter.get_child_ptr(target);

        iter = KeyPageInterface(next_page, this);
    }

    unsigned int target = iter.find_pos(key);

    ValuePageInterface value_iter(this);

    if (target < iter.get_size() && iter.get_key(target) == key) {
        return value_iter.get_value(iter.get_child_ptr(target + 1));
    }

    return T();
}

template <typename T>
BTreeDB<T>::KeyPageInterface::KeyPageInterface(unsigned int page_num, BTreeDB* tree): page_num_(page_num), tree_(tree) {}

template <typename T>
unsigned int BTreeDB<T>::KeyPageInterface::get_size() const {
    unsigned int res;
    memcpy(&res, get_data(), 4);
    return res;
}

template <typename T>
void BTreeDB<T>::KeyPageInterface::set_size(unsigned int x) {
    memcpy(get_data(), &x, 4);
    handle_set();
}

template <typename T>
bool BTreeDB<T>::KeyPageInterface::is_internal() const {
    char res;
    memcpy(&res, get_data() + 4, 1);
    return res == 1;
}

template <typename T>
void BTreeDB<T>::KeyPageInterface::set_internal(bool x) {
    char temp = x;
    memcpy(get_data() + 4, &temp, 1);
    handle_set();
}

template <typename T>
bool BTreeDB<T>::KeyPageInterface::is_root() const {
    char res;
    memcpy(&res, get_data() + 5, 1);
    return res == 1;
}

template <typename T>
void BTreeDB<T>::KeyPageInterface::set_root(bool x) {
    char temp = x;
    memcpy(get_data() + 5, &temp, 1);
    handle_set();
}

// unsigned int BTreeDB::KeyPageInterface::get_parent_ptr() const {
//     unsigned int res;
//     memcpy(&res, get_data() + 8, 4);
//     return res;
// }

// void BTreeDB::KeyPageInterface::set_parent_ptr(unsigned int x) {
//     memcpy(get_data() + 8, &x, 4);
//     handle_set();
// }

template <typename T>
long BTreeDB<T>::KeyPageInterface::get_key(unsigned int entry_num) const {
    if (entry_num >= get_size()) {
        tree_->write_all();
        std::cout << entry_num << ' ' << get_size() << std::endl;
        throw std::runtime_error("entry num out of bounds key");
    }
    char* curr = get_data();
    curr += HEADER_SIZE;
    curr += 4;
    curr += entry_num * KEYENTRY_SIZE;

    long res;
    memcpy(&res, curr, 8);
    return res;
}

template <typename T>
unsigned int BTreeDB<T>::KeyPageInterface::get_page_num() const  {
    return page_num_;
}

template <typename T>
void BTreeDB<T>::KeyPageInterface::insert(long key, unsigned int child_ptr, unsigned int loc) {
    unsigned int num_cells = get_size();
    char* curr = get_data();
    curr += HEADER_SIZE;
    curr += 4;

    curr += loc * KEYENTRY_SIZE;

    memmove(curr + KEYENTRY_SIZE, curr, (num_cells - loc) * KEYENTRY_SIZE);

    memcpy(curr, &key, 8);
    memcpy(curr + 8, &child_ptr, 4);
    
    set_size(num_cells + 1);
}

template <typename T>
void BTreeDB<T>::KeyPageInterface::push(long key, unsigned int child_ptr) {
    unsigned int num_cells = get_size();

    if (num_cells == 0) {
        char* curr = get_data();
        curr += HEADER_SIZE;
        curr += 4;

        memcpy(curr, &key, 8);
        memcpy(curr + 8, &child_ptr, 4);
    } else {
        unsigned int pos = find_pos(key);
        insert(key, child_ptr, pos);
    }

    set_size(num_cells + 1);
}

template <typename T>
unsigned int BTreeDB<T>::KeyPageInterface::get_child_ptr(unsigned int entry_num) const {
    if (entry_num > get_size()) {
        tree_->write_all();
        throw std::runtime_error("entry num out of bounds childptr get");
    }
    char* curr = get_data();
    curr += HEADER_SIZE; // get past headers

    if (entry_num != 0) {
        curr += 4;
        curr += (entry_num - 1) * KEYENTRY_SIZE;
        curr += 8;
    }

    unsigned int res;
    memcpy(&res, curr, 4);
    return res;
}

template <typename T>
void BTreeDB<T>::KeyPageInterface::set_child_ptr(unsigned int target, unsigned int entry_num) {
    if (entry_num > get_size()) {
        tree_->write_all();
        throw std::runtime_error("entry num out of bounds childptr set");
    }
    char* curr = get_data();
    curr += HEADER_SIZE; // get past headers

    if (entry_num != 0) {
        curr += 4;
        curr += (entry_num - 1) * KEYENTRY_SIZE;
        curr += 8;
    }

    unsigned int res;
    memcpy(curr, &target, 4);
    handle_set();
}

// unsigned int BTreeDB::KeyPageInterface::get_next_ptr() const {
//     unsigned int res;
//     memcpy(&res, get_data() + 12, 4);
//     return res;
// }

// void BTreeDB::KeyPageInterface::set_next_ptr(unsigned int ptr) {
//     memcpy(get_data() + 12, &ptr, 4);
//     handle_set();
// }

template <typename T>
void BTreeDB<T>::KeyPageInterface::move_keys(KeyPageInterface& other) {
    static const unsigned int offset = HEADER_SIZE + 4 + (ORDER / 2) * KEYENTRY_SIZE;
    
    // get pointers to the two pages
    char* source = get_data();
    char* target = other.get_data();

    if (is_internal()) {
        // copy the latter half of the key/children to the other key page (skipping the first key for alignment)
        memcpy(target + HEADER_SIZE, source + offset + 8, PAGE_SIZE - (offset + 8));
        // replace the moved key files in the original key page with nothing, erase the middle key
        memcpy(source + offset, tree_->empty_array, PAGE_SIZE - (offset + 8));

        set_size(ORDER / 2);
        other.set_size(ORDER / 2);
    } else {
        // copy the latter half of the key/children to the other key page (skipping the first pointer)
        memcpy(target + HEADER_SIZE + 4, source + offset + KEYENTRY_SIZE, PAGE_SIZE - offset - KEYENTRY_SIZE);
        // replace the moved key files in the original key page with nothing; keep the middle key
        memcpy(source + offset + KEYENTRY_SIZE, tree_->empty_array, PAGE_SIZE - (offset + KEYENTRY_SIZE));

        set_size(ORDER / 2 + 1);
        other.set_size(ORDER / 2);
    }

    // set everything to dirty
    handle_set();
    other.handle_set();
}

template <typename T>
char* BTreeDB<T>::KeyPageInterface::get_data() const {
    return tree_->get_page(page_num_, Key);
}

template <typename T>
void BTreeDB<T>::KeyPageInterface::handle_set()  {
    tree_->set_dirty(page_num_, Key);
}

template <typename T>
BTreeDB<T>::ValuePageInterface::ValuePageInterface(BTreeDB* tree): tree_(tree) {}

template <typename T>
T BTreeDB<T>::ValuePageInterface::get_value(unsigned int entry_num) const {
    unsigned int page_num = entry_num / tree_->values_per_page;
    if (page_num >= tree_->num_value_pages) {
        tree_->write_all();
        throw std::runtime_error("entry num out of bounds for pages value get");
    }
    unsigned int target = entry_num % tree_->values_per_page;
    if (target >= get_size(page_num)) {
        tree_->write_all();
        std::cout << "data: " << page_num << ' ' << entry_num << ' ' << target << ' ' << get_size(page_num) << std::endl;
        throw std::runtime_error("entry_num out of bounds for specific page value get");
    }
    char* data = tree_->get_page(page_num, Value);
    data += 4; // get past size header

    data += target * T::size;

    T res;
    int temp;
    memcpy(&temp, data + 40, 4);
    T::deserialize_value(data, &res);

    return res;
}

template <typename T>
void BTreeDB<T>::ValuePageInterface::set_value(T& entry, unsigned int entry_num) {
    unsigned int page_num = entry_num / tree_->values_per_page;
    if (page_num >= tree_->num_value_pages) {
        tree_->write_all();
        throw std::runtime_error("entry num out of bounds value set");
    }
    unsigned int target = entry_num % tree_->values_per_page;
    char* data = tree_->get_page(page_num, Value);
    data += 4; // get past size header

    data += target * T::size;

    T::serialize_value(&entry, data);
    tree_->set_dirty(page_num, Value);
}

template <typename T>
unsigned int BTreeDB<T>::ValuePageInterface::push(T& entry) {
    tree_->num_entries += 1;

    if (tree_->num_value_pages == 0 || get_size(tree_->num_value_pages - 1) == tree_->values_per_page) {
        tree_->num_value_pages += 1;
        std::array<char, PAGE_SIZE> new_value_page;
        new_value_page.fill(DEFAULT_VAL);
        tree_->value_handler.write(new_value_page.data(), PAGE_SIZE);
    } 
    
    set_size(get_size(tree_->num_value_pages - 1) + 1, tree_->num_value_pages - 1);
    set_value(entry, tree_->num_entries - 1);
    tree_->set_dirty(tree_->num_value_pages - 1, Value);

    return tree_->num_entries - 1;
}

template <typename T>
unsigned int BTreeDB<T>::ValuePageInterface::get_size(unsigned int page_num) const {
    if (page_num >= tree_->num_value_pages) {
        tree_->write_all();
        std::cout << page_num << ' ' << tree_->num_value_pages << std::endl;
        throw std::runtime_error("entry num out of bounds size get");
    }
    char* data = tree_->get_page(page_num, Value);
    unsigned int res;
    memcpy(&res, data, 4);
    return res;
}

template <typename T>
void BTreeDB<T>::ValuePageInterface::set_size(unsigned int size, unsigned int page_num) {
    if (page_num >= tree_->num_value_pages) {
        tree_->write_all();
        throw std::runtime_error("entry num out of bounds size set");
    }
    char* data = tree_->get_page(page_num, Value);
    memcpy(data, &size, 4);
    tree_->set_dirty(page_num, Value);
}

template <typename T>
void BTreeDB<T>::KeyPageInterface::split_page() {
    // splitting a node without a parent (the new root)
    KeyPageInterface new_root = tree_->create_new_keypage(0, true, true);
    KeyPageInterface right = tree_->create_new_keypage(0, is_internal(), false);

    new_root.push(get_key(ORDER / 2), right.get_page_num());
    new_root.set_child_ptr(get_page_num(), 0);

    set_root(false);
    // iter.set_size(PAGE_SIZE / 2 + 1);

    move_keys(right);

    tree_->key_root = new_root.get_page_num();
    
}

template <typename T>
void BTreeDB<T>::KeyPageInterface::split_page(KeyPageInterface& parent) {
    KeyPageInterface right = tree_->create_new_keypage(0, is_internal(), false);

    long middle_key = get_key(ORDER / 2);

    move_keys(right);
    parent.push(middle_key, right.get_page_num());
}

template <typename T>
typename BTreeDB<T>::KeyPageInterface BTreeDB<T>::create_new_keypage(unsigned int num_cells, bool internal, bool is_root, unsigned int parent_ptr, unsigned int next_ptr) {
    std::array<char, PAGE_SIZE> new_page;
    new_page.fill(DEFAULT_VAL);

    char temp[8] = {'n', 'e', 'w', 'b', 'l', 'o', 'c', 'k'};
    memcpy(new_page.data() + HEADER_SIZE - 8, temp, 8);
    new_page[6] = -1;
    new_page[7] = -1;

    key_handler.seekg(0, std::ios::end);
    key_handler.write(new_page.data(), PAGE_SIZE);
    ++num_key_pages;
    get_page(num_key_pages - 1, Key);

    KeyPageInterface curr(num_key_pages - 1, this);
    curr.set_size(num_cells);
    curr.set_internal(internal);
    curr.set_root(is_root);
    // curr.set_next_ptr(next_ptr);
    // curr.set_parent_ptr(parent_ptr);

    return curr;
}

template <typename T>
unsigned int BTreeDB<T>::KeyPageInterface::find_pos(long key) const {
    unsigned int size = get_size();
    if (size == 0) {
        return 0;
    }

    int left = 0;
    int right = size - 1;

    while (left <= right) {
        int middle = (left + right) / 2;
        if (get_key(middle) == key) {
            return middle;
        }

        if (get_key(middle) < key) {
            left = middle + 1;
        } else {
            right = middle - 1;
        }
    }
    return left;
}

template <typename T>
void BTreeDB<T>::set_dirty(unsigned int page_num, FileType type) {
    CacheSet& set = get_cache_set(page_num, type);
    for (CacheBlock& block : set.first) {
        if (block.page_num == page_num && block.valid) {
            block.dirty = true;
            return;
        }
    }
    write_all();
    throw std::runtime_error("page_num not in cache");
}

template <typename T>
unsigned int BTreeDB<T>::get_page_idx(unsigned int page_num) const {
    return h(page_num) % CACHE_SIZE;
}

template <typename T>
void BTreeDB<T>::write_page(unsigned int page_num, FileType type) {
    switch (type) {
        case Key: {
            key_handler.seekg(page_num * PAGE_SIZE, std::ios::beg);
            char* curr_page = get_page(page_num, type);
            key_handler.write(curr_page, PAGE_SIZE);
            break;
        }
        case Value: {
            value_handler.seekg(page_num * PAGE_SIZE, std::ios::beg);
            char* curr_page = get_page(page_num, type);
            value_handler.write(curr_page, PAGE_SIZE);
            break;
        }
    }
}

template <typename T>
typename BTreeDB<T>::CacheSet& BTreeDB<T>::get_cache_set(unsigned int page_num, FileType type) {
    switch(type) {
        case Key: {
            if (page_num >= num_key_pages) {
                write_all();
                throw std::runtime_error("requested key page out of bounds");
            }
            return key_cache[get_page_idx(page_num)];
        }
        case Value: {
            if (page_num >= num_value_pages) {
                write_all();
                throw std::runtime_error("requested value page out of bounds");
            }
            return value_cache[get_page_idx(page_num)];
        }
        default: {
            write_all();
            throw std::runtime_error("invalid filetype");
        }
    }
}

template <typename T>
char* BTreeDB<T>::get_page(unsigned int page_num, FileType type) {
    // get the cache set this page maps to
    CacheSet& cache_set = get_cache_set(page_num, type);

    // loop over the two cache blocks to check for a hit
    for (unsigned int i = 0; i < 2; i++) {
        // cache hit
        if (cache_set.first[i].page_num == page_num && cache_set.first[i].valid) {
            // set lru to the other one
            cache_set.second = !i;
            return cache_set.first[i].page.data();
        }
    }

    //cache miss, replace lru
    CacheBlock& block = cache_set.first[cache_set.second];
    // set valid
    block.valid = true;

    // write if dirty
    if (block.dirty) {
        write_page(block.page_num, type);
    }

    // read from disk
    char buffer[PAGE_SIZE];
    
    switch (type) {
        case Key: {
            key_handler.seekg(page_num * PAGE_SIZE, std::ios::beg);
            key_handler.read(buffer, PAGE_SIZE);
            break;
        }
        case Value: {
            value_handler.seekg(page_num * PAGE_SIZE, std::ios::beg);
            value_handler.read(buffer, PAGE_SIZE);
            break;
        }
    }

    memcpy(block.page.data(), buffer, PAGE_SIZE);

    // no longer dirty and replace page number
    block.page_num = page_num;
    block.dirty = false;

    // swap lru
    cache_set.second = !cache_set.second;

    return block.page.data();
}

// void BTreeDB::serialize_value(const ValueEntry* const source, char* dest) const {
//     memcpy(dest, source->name.data(), 40);
//     memcpy(dest + 40, &(source->temp), 4);
// }

// void BTreeDB::deserialize_value(const char* const source, ValueEntry* dest) const {
//     memcpy(dest->name.data(), source, 40);
//     memcpy(&(dest->temp), source + 40, 4);
// }

template <typename T>
long BTreeDB<T>::get_id_from_name(const T& search_val) {
    ValuePageInterface iter(this);
    for (unsigned int i = 0; i < num_entries; ++i) {
        if (iter.get_value(i) == search_val) {
            return iter.get_value(i).id;
        }
    }
}