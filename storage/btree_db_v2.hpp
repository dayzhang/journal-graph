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
#include <unordered_map>

#define ORDER 337 // max entries in key page (assuming int pointer sand long ids and 4096 byte pages), db designed for odd orders
#define PAGE_SIZE 4096 // size of a page in bytes

#define KEYENTRY_SIZE 12 // size of a key entry (long + int)
#define HEADER_SIZE 16 // size of header

#define DEFAULT_VAL 0 // default val to initialize arrays to 

/**
    This is a helper enum to switch between function behavior for key pages and value pages
*/
enum FileType {
    Key,
    Value
};

/**
    this class is templated on a ValueEntry-type struct. Such a struct must have at least a field for a long id, a deserialization function, a serialization function, a equality operator, a default constructor, and a size variable (as an unsigned int)
*/ 
template <typename T>
class BTreeDB {
    private: 
        /**
            A page is a 4096 byte (4096 chars) region of data.
        */
        typedef std::array<char, PAGE_SIZE> Page;
        
        /**
            This struct is how cache blocks are stored. data is the data the cache block holds. dirty refers to whether the block should be written back on destruction.
        */
        struct CacheBlock {
            char* data = nullptr;
            bool dirty = false;
        };

        /**
            This is the amount of values that can fit on a single page. One is subtracted for safety. NOTE: the size of the struct being stored should be less than 4092 bytes or underflow may occur.
        */
        const unsigned int values_per_page = (PAGE_SIZE - 4) / T::size - 1;

        /**
            This struct is how the header of key pages are structured, although it is unused. num_cells is the number of key-pointer pairs are in the page, node_type is whether the page is internal (1) or a leaf (not 1), is_root is whether the page is the root (1) or not (not 1), parent_ptr is the page number of the parent pointer or CHAR_MAX if it doesn't have one, and next_ptr is the next node on the same level the page points to (if this page is a leaf).
        */
        struct Header {
            unsigned int num_cells;
            char node_type;
            char is_root;
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
            Cache for the key pages. Has infinite capacity to be more performant.
        */
        std::unordered_map<unsigned int, CacheBlock> key_cache;
        
        /**
            Cache for the value pages. Has infinite capacity to be more performant.
        */
        std::unordered_map<unsigned int, CacheBlock> value_cache;

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

        /**
            Member variable to determine if the current instance is read only. If true, nothing will be written to disk. 
        */
        bool read_only;

    public:
        /**
            Constructor for the BTree database. Either creates a new database if the filename doesn't refer to anything or instantitates a previously created database if the filenames do refer to something. The key and value databases should be compatible with each other.

            @param key_filename The filename for the key database
            @param values_filename The filename for the value database
            @param create_new Whether or not new files should be created to store the dbs (will overwrite existing ones)
            @param read_only_opt Whether or not this instance should be read only
        */
        BTreeDB(const std::string& key_filename, const std::string& values_filename, bool create_new=false, bool read_only_opt=false);

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

        /**
            Retrieves an id from the database according to an implemented operator== function for the template struct. For authors, it searches for a name, and for papers, it searches for a paper title.

            @param search_val A struct that at minimum has the members needed for its operator== function to work.
            @return the id of the struct that matches the search val if found, -1 if not
        */
        long get_id_from_name(const T& search_val);

    private:
    
        /**
            This class is a wrapper for working with individual key pages. Supports both reading data and writing data to the key page.

            NOTE: For a given page number, only one interface should ever exist in memory at once, or the interfaces will be out of sync.
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
                    Splits this interface's keypage according to the BTree logic. This page should not have a parent. Should only be called on a full key page. 

                    Splits the latter half of the keys to a new key page and creates a new parent page to store the middle key.
                */
                void split_page();


                /**
                    Splits a key page according to the BTree logic. Should only be called on a full key page. This page should have a parent.

                    Splits the latter half of the keys to a new key page and pushes the middle key to the parent. 

                    @param parent An interface to the parent of the key page that is being split.
                */
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
        KeyPageInterface create_new_keypage(unsigned int num_cells, bool internal, bool is_root);

        /**
            Helper function to write all dirty pages to disk.
        */
        void write_all();

        /**
            Helper function to set the page_num page dirty for writeback.

            @param page_num the page number to set dirty
            @param type which cache to set dirty
        */
        void set_dirty(unsigned int page_num, FileType type);

        /**
            Helper function to write a page_num to the file database.

            @param page_num the page number to write
            @param type Specifies which database to write to
        */
        void write_page(unsigned int page_num, FileType type);

        /**
            Gets the actual data of a given page number. Handles cache misses and cache hits implicitly.

            @param page_num The page being requested
            @param type Whwether a key or value page is being requested
            @return the character array representing the page's binary data
        */
        char* get_page(unsigned int page_num, FileType type);
};

template <typename T>
BTreeDB<T>::BTreeDB(const std::string& key_filename, const std::string& values_filename, bool create_new, bool read_only_opt): read_only(read_only_opt) {
    // decare read/write streams for the db files and the metadata file (that stores the important member variables)
    std::fstream fs_keys;
    std::fstream fs_values;
    std::fstream fs_meta;

    // construct name for metadata file from filenames
    std::string metadata_file = key_filename.substr(0, key_filename.size() - 3) + values_filename.substr(0, values_filename.size() - 3) + ".txt";

    if (create_new) {
        // if creating a new BTreeDB, open with std::ios::trunc to create fresh files, overwriting any existing things
        fs_keys.open(key_filename, std::ios::binary | std::ios::in | std::ios::out | std::ios::trunc);
        fs_values.open(values_filename, std::ios::binary | std::ios::in | std::ios::out | std::ios::trunc);
        fs_meta.open(metadata_file, std::ios::binary | std::ios::in | std::ios::out | std::ios::trunc);

        // everything should be zero on new db
        num_entries = num_key_pages = key_root = num_value_pages = 0;
    } else {
        // open normally without overwriting if not creating a new one
        fs_keys.open(key_filename, std::ios::binary | std::ios::in | std::ios::out);
        fs_values.open(values_filename, std::ios::binary | std::ios::in | std::ios::out);
        fs_meta.open(metadata_file, std::ios::binary | std::ios::in | std::ios::out);
    }
    
    // error handling for io errors
    if (!fs_keys.is_open()) {
        throw std::runtime_error("error reading/creating keys file");
    }

    if (!fs_values.is_open()) {
        throw std::runtime_error("error reading/creating values file");
    }

    if (!fs_meta.is_open()) {
        throw std::runtime_error("error reading/creating metadata file");
    }

    // store handlers for values/keys using move semantics
    value_handler = std::move(fs_values);
    key_handler = std::move(fs_keys);

    if (!create_new) {
        // if not creating a new file, read member variables from the metadata file
        fs_meta >> num_entries;
        fs_meta >> num_value_pages;
        fs_meta >> num_key_pages;
        fs_meta >> key_root;
    }

    if (!read_only) {
        // if not read only, reset the metadata file to allow the storing of the new ones once finished
        fs_meta.close();
        fs_meta.open(metadata_file, std::ios::out | std::ios::trunc);
    }

    // std::cout << num_entries << ' ' << num_value_pages << ' ' << num_key_pages << ' ' << key_root << std::endl;

    // store meta handler with move semantics
    meta_handler = std::move(fs_meta);

    // initialize empty array
    for (unsigned int i = 0; i < PAGE_SIZE; ++i) {
        empty_array[i] = 0;
    }
}

template <typename T>
void BTreeDB<T>::write_all() {
    // don't run if read only
    if (read_only) return;

    // iterate over all key pages and write if dirty; also delete allocated data
    for (const auto& entry : key_cache) {
        if (entry.second.data != nullptr && entry.second.dirty) {
            write_page(entry.first, Key);
        } else if (entry.second.data != nullptr) {
            delete[] entry.second.data;
        }
        
    }
    
    // iterate over all value pages and write if dirty; also delete allocated data
    for (const auto& entry : value_cache) {
        if (entry.second.data != nullptr && entry.second.dirty) {
            write_page(entry.first, Value);
        } else if (entry.second.data != nullptr) {
            delete[] entry.second.data;
        }
    }

    // close handlers
    key_handler.close();
    value_handler.close();

    // store metadata member variables
    meta_handler << num_entries << std::endl;
    meta_handler << num_value_pages << std::endl;
    meta_handler << num_key_pages << std::endl;
    meta_handler << key_root << std::endl;

    meta_handler.close();
}

template <typename T>
BTreeDB<T>::~BTreeDB() {
    // on destruction, write all
    write_all();
}

template <typename T>
void BTreeDB<T>::insert(long key, T& value) {
    // if read only, don't allow insertions
    if (read_only) return;

    // if there are no key pages, create a new keypage and push the value
    if (num_key_pages == 0) {
        KeyPageInterface key_iter = create_new_keypage(0, false, true);
        ValuePageInterface value_iter(this);

        key_iter.push(key, value_iter.push(value));

        return;
    }

    // base page to perform binary search on
    unsigned int curr_page = key_root;
    KeyPageInterface key_iter(curr_page, this);

    // keep track of call stack in case node splits are needed
    std::stack<unsigned int> traversal;

    // continue binary searching until we have reached a leaf node (which store values)
    while (key_iter.is_internal()) {
        traversal.push(key_iter.get_page_num());
        // get the id to search w.r.t. binray search
        unsigned int target = key_iter.find_pos(key);
        // change the curr page
        unsigned int next_page = key_iter.get_child_ptr(target);
        key_iter = KeyPageInterface(next_page, this);
    }

    // get the final position to insert to
    unsigned int target = key_iter.find_pos(key);
    ValuePageInterface value_iter(this);

    // check if id already exists; if so, overwrite it
    if (target < key_iter.get_size() && key_iter.get_key(target) == key) {
        value_iter.set_value(value, key_iter.get_child_ptr(target + 1));

        return;
    }

    // id doesn't exist--make a new entry
    key_iter.push(key, value_iter.push(value));

    // if we have reached the max key size, split
    if (key_iter.get_size() == ORDER) {
        // go through the call stack of nodes
        while (!traversal.empty()) {
            // stop when no more splitting needed
            if (key_iter.get_size() != ORDER) {
                break;
            }
            // do the splitting logic for nodes with a parent
            KeyPageInterface curr(traversal.top(), this);
            traversal.pop();
            key_iter.split_page(curr);
            key_iter = curr;
        }
        // split a node that doesn't have a parent -- only happens when call stack is fully traversed
        if (key_iter.get_size() == ORDER) {
                key_iter.split_page();
        }
    }
}

template <typename T>
T BTreeDB<T>::find(long key) {
    // can't find on an empty datbaase
    if (num_entries == 0) {
        write_all();
        throw std::runtime_error("database is empty");
    }

    // base page to do binary search on
    unsigned int curr_page = key_root;
    KeyPageInterface iter(curr_page, this);

    // continue binary search until we have reached a leaf node (which stores values)
    while (iter.is_internal()) {
        unsigned int target = iter.find_pos(key);
        unsigned int next_page = iter.get_child_ptr(target);

        iter = KeyPageInterface(next_page, this);
    }

    // target id to search
    unsigned int target = iter.find_pos(key);

    ValuePageInterface value_iter(this);

    // if id is present at the id, return it
    if (target < iter.get_size() && iter.get_key(target) == key) {
        return value_iter.get_value(iter.get_child_ptr(target + 1));
    }

    // if not, return a default struct
    return T();
}

template <typename T>
BTreeDB<T>::KeyPageInterface::KeyPageInterface(unsigned int page_num, BTreeDB* tree): page_num_(page_num), tree_(tree) {}

template <typename T>
unsigned int BTreeDB<T>::KeyPageInterface::get_size() const {
    // extract size from key page (first 4 bytes of header)
    unsigned int res;
    memcpy(&res, get_data(), 4);
    return res;
}

template <typename T>
void BTreeDB<T>::KeyPageInterface::set_size(unsigned int x) {
    // set size from key page (first 4 bytes of header)
    memcpy(get_data(), &x, 4);
    // set dirty
    handle_set();
}

template <typename T>
bool BTreeDB<T>::KeyPageInterface::is_internal() const {
    // get whether the page is internal (5th byte of header)
    char res;
    memcpy(&res, get_data() + 4, 1);
    return res == 1;
}

template <typename T>
void BTreeDB<T>::KeyPageInterface::set_internal(bool x) {
    // set whether the page is internal (5th byte of header)
    char temp = x;
    memcpy(get_data() + 4, &temp, 1);
    handle_set();
}

template <typename T>
bool BTreeDB<T>::KeyPageInterface::is_root() const {
    // get whether the page is the root (6th byte)
    char res;
    memcpy(&res, get_data() + 5, 1);
    return res == 1;
}

template <typename T>
void BTreeDB<T>::KeyPageInterface::set_root(bool x) {
    // set whether the page is the root (6th byte)
    char temp = x;
    memcpy(get_data() + 5, &temp, 1);
    handle_set();
}

template <typename T>
long BTreeDB<T>::KeyPageInterface::get_key(unsigned int entry_num) const {
    // if trying to get an entry that is out of bounds of the current page, throw an exception
    if (entry_num >= get_size()) {
        tree_->write_all();
        std::cout << entry_num << ' ' << get_size() << std::endl;
        throw std::runtime_error("entry num out of bounds key");
    }

    // get the the address of the desired key
    char* curr = get_data();
    curr += HEADER_SIZE;
    curr += 4;
    curr += entry_num * KEYENTRY_SIZE;

    // copy in the key data into a dummy long and return it
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
    // get current size
    unsigned int num_cells = get_size();

    // move to where we need to insert
    char* curr = get_data();
    curr += HEADER_SIZE;
    curr += 4;

    curr += loc * KEYENTRY_SIZE;

    // move entries where we are tryign to insert to the right (using memmove to account for overlaps)
    memmove(curr + KEYENTRY_SIZE, curr, (num_cells - loc) * KEYENTRY_SIZE);

    // copying in the data to be inserted into its right position
    memcpy(curr, &key, 8);
    memcpy(curr + 8, &child_ptr, 4);
    
    // increment size
    set_size(num_cells + 1);
}

template <typename T>
void BTreeDB<T>::KeyPageInterface::push(long key, unsigned int child_ptr) {
    // getting size
    unsigned int num_cells = get_size();

    if (num_cells == 0) {
        // if page is empty, just write to first key/ptr position
        char* curr = get_data();
        curr += HEADER_SIZE;
        curr += 4;

        memcpy(curr, &key, 8);
        memcpy(curr + 8, &child_ptr, 4);
    } else {
        // otherwise, get the correct binary search position and insert there
        unsigned int pos = find_pos(key);
        insert(key, child_ptr, pos);
    }

    // increment size
    set_size(num_cells + 1);
}

template <typename T>
unsigned int BTreeDB<T>::KeyPageInterface::get_child_ptr(unsigned int entry_num) const {
    // if trying to query out of bounds, throw an exception
    if (entry_num > get_size()) {
        tree_->write_all();
        throw std::runtime_error("entry num out of bounds childptr get");
    }

    // get to the address of the desired child ponter
    char* curr = get_data();
    curr += HEADER_SIZE; // get past headers

    if (entry_num != 0) {
        // 1st child pointer is already here; other wise need to do an offset to reach the desired pointer address
        curr += 4;
        curr += (entry_num - 1) * KEYENTRY_SIZE;
        curr += 8;
    }

    // copy the pointer number into a dummy variable and return it
    unsigned int res;
    memcpy(&res, curr, 4);
    return res;
}

template <typename T>
void BTreeDB<T>::KeyPageInterface::set_child_ptr(unsigned int target, unsigned int entry_num) {
    // if trying to access out of bounds, throw an exception
    if (entry_num > get_size()) {
        tree_->write_all();
        throw std::runtime_error("entry num out of bounds childptr set");
    }

    //get to the address of the desired pointer
    char* curr = get_data();
    curr += HEADER_SIZE; // get past headers

    if (entry_num != 0) {
        curr += 4;
        curr += (entry_num - 1) * KEYENTRY_SIZE;
        curr += 8;
    }

    // overwrite the existing value
    memcpy(curr, &target, 4);
    handle_set();
}

template <typename T>
void BTreeDB<T>::KeyPageInterface::move_keys(KeyPageInterface& other) {
    // the offset to reach the middle of the key page, which is the pivot of the moving
    static const unsigned int offset = HEADER_SIZE + 4 + (ORDER / 2) * KEYENTRY_SIZE;
    
    // get pointers to the two pages
    char* source = get_data();
    char* target = other.get_data();

    if (is_internal()) {
        // copy the latter half of the key/children to the other key page (skipping the first key for alignment)
        memcpy(target + HEADER_SIZE, source + offset + 8, PAGE_SIZE - (offset + 8));
        // replace the moved key files in the original key page with nothing, erase the middle key
        memcpy(source + offset, tree_->empty_array, PAGE_SIZE - (offset + 8));

        // update size
        set_size(ORDER / 2);
        other.set_size(ORDER / 2);
    } else {
        // copy the latter half of the key/children to the other key page (skipping the first pointer)
        memcpy(target + HEADER_SIZE + 4, source + offset + KEYENTRY_SIZE, PAGE_SIZE - offset - KEYENTRY_SIZE);
        // replace the moved key files in the original key page with nothing; keep the middle key
        memcpy(source + offset + KEYENTRY_SIZE, tree_->empty_array, PAGE_SIZE - (offset + KEYENTRY_SIZE));

        // update size
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
    // set the cache block to dirty for writeback later
    tree_->set_dirty(page_num_, Key);
}

template <typename T>
BTreeDB<T>::ValuePageInterface::ValuePageInterface(BTreeDB* tree): tree_(tree) {}

template <typename T>
T BTreeDB<T>::ValuePageInterface::get_value(unsigned int entry_num) const {
    // if querying out of bounds, throw an excpetion
    // page num is the entry num divided by the number of values per page
    unsigned int page_num = entry_num / tree_->values_per_page;
    if (page_num >= tree_->num_value_pages) {
        tree_->write_all();
        throw std::runtime_error("entry num out of bounds for pages value get");
    }

    // specific entry num in a page is the entry num moduloed by the number of values per page
    unsigned int target = entry_num % tree_->values_per_page;
    if (target >= get_size(page_num)) {
        tree_->write_all();
        std::cout << "data: " << page_num << ' ' << entry_num << ' ' << target << ' ' << get_size(page_num) << std::endl;
        throw std::runtime_error("entry_num out of bounds for specific page value get");
    }

    // get to the desired address
    char* data = tree_->get_page(page_num, Value);
    data += 4; // get past size header

    data += target * T::size;

    // deserialize into a dummy result and return it
    T res;
    T::deserialize_value(data, &res);

    return res;
}

template <typename T>
void BTreeDB<T>::ValuePageInterface::set_value(T& entry, unsigned int entry_num) {
    // if querying out of bounds, throw an exception
    unsigned int page_num = entry_num / tree_->values_per_page;
    if (page_num >= tree_->num_value_pages) {
        tree_->write_all();
        throw std::runtime_error("entry num out of bounds value set");
    }

    // get to the target address
    unsigned int target = entry_num % tree_->values_per_page;
    char* data = tree_->get_page(page_num, Value);
    data += 4; // get past size header

    data += target * T::size;

    // serialize the target address data with the passed in data
    T::serialize_value(&entry, data);
    // set the cache block to dirty
    tree_->set_dirty(page_num, Value);
}

template <typename T>
unsigned int BTreeDB<T>::ValuePageInterface::push(T& entry) {
    // increase the number of entries
    tree_->num_entries += 1;

    if (tree_->num_value_pages == 0 || get_size(tree_->num_value_pages - 1) == tree_->values_per_page) {
        // if db is empty or the current last page of the values db is full, make a new page and 0-initialize it
        std::array<char, PAGE_SIZE> new_value_page;
        new_value_page.fill(DEFAULT_VAL);

        //allocate heap memory to store this newly created page
        tree_->value_cache[tree_->num_value_pages].data = new char[PAGE_SIZE];
        memcpy(tree_->value_cache[tree_->num_value_pages].data, new_value_page.data(), PAGE_SIZE);

        // increment number of values pages
        tree_->num_value_pages += 1;
    } 
    
    // set the size and value of this new page and set it to dirty for writeback
    set_size(get_size(tree_->num_value_pages - 1) + 1, tree_->num_value_pages - 1);
    set_value(entry, tree_->num_entries - 1);
    tree_->set_dirty(tree_->num_value_pages - 1, Value);

    // return the entry num of the pushed value
    return tree_->num_entries - 1;
}

template <typename T>
unsigned int BTreeDB<T>::ValuePageInterface::get_size(unsigned int page_num) const {
    // if indexing out of bounds, throw an exception
    if (page_num >= tree_->num_value_pages) {
        tree_->write_all();
        std::cout << page_num << ' ' << tree_->num_value_pages << std::endl;
        throw std::runtime_error("entry num out of bounds size get");
    }

    // get ot hte desired address and read the page num from it to return
    char* data = tree_->get_page(page_num, Value);
    unsigned int res;
    memcpy(&res, data, 4);
    return res;
}

template <typename T>
void BTreeDB<T>::ValuePageInterface::set_size(unsigned int size, unsigned int page_num) {
    // if querying out of bounds, throw an exception
    if (page_num >= tree_->num_value_pages) {
        tree_->write_all();
        throw std::runtime_error("entry num out of bounds size set");
    }

    // navigate to the desired address and change it to reflect hte passed in size; set dirty
    char* data = tree_->get_page(page_num, Value);
    memcpy(data, &size, 4);
    tree_->set_dirty(page_num, Value);
}

template <typename T>
void BTreeDB<T>::KeyPageInterface::split_page() {
    // splitting a node without a parent (the new root)

    // create a new root node and new right node
    KeyPageInterface new_root = tree_->create_new_keypage(0, true, true);
    KeyPageInterface right = tree_->create_new_keypage(0, is_internal(), false);

    // add the middle element to the new root and update its child pointers
    new_root.push(get_key(ORDER / 2), right.get_page_num());
    new_root.set_child_ptr(get_page_num(), 0);

    // update root and move keys from left to right
    set_root(false);
    move_keys(right);

    // setting the key root page number to its new value
    tree_->key_root = new_root.get_page_num();
}

template <typename T>
void BTreeDB<T>::KeyPageInterface::split_page(KeyPageInterface& parent) {
    // splitting a page with a parent

    // getting interface for the new right node
    KeyPageInterface right = tree_->create_new_keypage(0, is_internal(), false);

    // getting the middle key
    long middle_key = get_key(ORDER / 2);

    // move keys from left to right and push the middle key/the right pointer to the parent
    move_keys(right);
    parent.push(middle_key, right.get_page_num());
}

template <typename T>
typename BTreeDB<T>::KeyPageInterface BTreeDB<T>::create_new_keypage(unsigned int num_cells, bool internal, bool is_root) {
    // create new page and 0-initialize it
    std::array<char, PAGE_SIZE> new_page;
    new_page.fill(DEFAULT_VAL);

    // adding in some dummy indicators for binary viewing
    char temp[8] = {'n', 'e', 'w', 'b', 'l', 'o', 'c', 'k'};
    memcpy(new_page.data() + HEADER_SIZE - 8, temp, 8);
    new_page[6] = -1;
    new_page[7] = -1;

    // write the new key page and allocate data to store it
    key_handler.seekg(0, std::ios::end);
    key_handler.write(new_page.data(), PAGE_SIZE);
    key_cache[num_key_pages].data = new char[PAGE_SIZE];
    memcpy(key_cache[num_key_pages].data, new_page.data(), PAGE_SIZE);

    // incremenet number of key pages
    ++num_key_pages;

    // update the new key pages values using the passed in parameters
    KeyPageInterface curr(num_key_pages - 1, this);
    curr.set_size(num_cells);
    curr.set_internal(internal);
    curr.set_root(is_root);

    return curr;
}

template <typename T>
unsigned int BTreeDB<T>::KeyPageInterface::find_pos(long key) const {
    // get size; if 0, return 0
    unsigned int size = get_size();
    if (size == 0) {
        return 0;
    }

    // initialize binary search trackers
    int left = 0;
    int right = size - 1;

    while (left <= right) {
        int middle = (left + right) / 2; // get the middle value
        if (get_key(middle) == key) { // return middle if found
            return middle;
        }

        // logic for binary search
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
    switch (type) {
        case Key: {
            // throw exception of page num not in cache
            if (key_cache.find(page_num) == key_cache.end()) {
                throw std::runtime_error("page_num not in key cache");
            }

            // set the cache block to dirty
            key_cache.at(page_num).dirty = true;
            return;
        }
        case Value: {
            // same as for keys
            if (value_cache.find(page_num) == value_cache.end()) {
                throw std::runtime_error("page_num not in value cache");
            }
            value_cache.at(page_num).dirty = true;
            return;
        }
    }
}

template <typename T>
void BTreeDB<T>::write_page(unsigned int page_num, FileType type) {
    switch (type) {
        case Key: {
            // get to the right positio nfor writing and write the data to it
            key_handler.seekg(page_num * PAGE_SIZE, std::ios::beg);
            char* curr_page = get_page(page_num, type);
            key_handler.write(curr_page, PAGE_SIZE);
            break;
        }
        case Value: {
            // same as key
            value_handler.seekg(page_num * PAGE_SIZE, std::ios::beg);
            char* curr_page = get_page(page_num, type);
            value_handler.write(curr_page, PAGE_SIZE);
            break;
        }
    }
}

template <typename T>
char* BTreeDB<T>::get_page(unsigned int page_num, FileType type) {
    switch(type) {
        case Key: {
            // if page num already in cache, return it; cache hit
            if (key_cache.find(page_num) != key_cache.end()) {
                return key_cache.at(page_num).data;
            }

            // cache miss; allocate data for new block and manually copy it with data read in from teh db file
            key_cache[page_num] = CacheBlock();
            key_cache[page_num].data = new char[PAGE_SIZE];
            key_handler.seekg(page_num * PAGE_SIZE);
            key_handler.read(key_cache.at(page_num).data, PAGE_SIZE);
            return key_cache.at(page_num).data;
        }
        case Value: {
            // same as key
            if (value_cache.find(page_num) != value_cache.end()) {
                return value_cache.at(page_num).data;
            }

            value_cache[page_num] = CacheBlock();
            value_cache[page_num].data = new char[PAGE_SIZE];
            value_handler.seekg(page_num * PAGE_SIZE);
            value_handler.read(value_cache.at(page_num).data, PAGE_SIZE);
            return value_cache.at(page_num).data;
        }
    }
    throw std::runtime_error("something went wrong");
}

template <typename T>
long BTreeDB<T>::get_id_from_name(const T& search_val) {
    // iterate over all values until we reach the entry that matches the passed in entry
    ValuePageInterface iter(this);
    for (unsigned int i = 0; i < num_entries; ++i) {
        if (iter.get_value(i) == search_val) {
            return iter.get_value(i).id;
        }
    }
    return -1;
}