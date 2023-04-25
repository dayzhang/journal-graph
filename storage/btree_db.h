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

/**
    This struct details how values are stored in the values database.
*/
struct ValueEntry {
    char name[40];
};

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
                page.fill(CHAR_MAX);
            }
        };

        /**
            This struct is how the header of key pages are structured, although it is unused. num_cells is the number of key-pointer pairs are in the page, node_type is whether the page is internal (1) or a leaf (not 1), is_root is whether the page is the root (1) or not (not 1), parent_ptr is the page number of the parent pointer or CHAR_MAX if it doesn't have one, and next_ptr is the next node on the same level the page points to (if this page is a leaf).
        */
        struct Header {
            unsigned int num_cells;
            char node_type;
            char is_root;
            unsigned int parent_ptr;
            unsigned int next_ptr;
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
            Cache for the key pages.
        */
        std::array<CacheBlock, CACHE_SIZE> key_cache;
        /**
            Cache for the value pages.
        */
        std::array<CacheBlock, CACHE_SIZE> value_cache;

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

    public:
        /**
            Constructor for the BTree database. Either creates a new database if the filename doesn't refer to anything or instantitates a previously created database if the filenames do refer to something. The key and value databases should be compatible with each other.

            @param key_filename The filename for the key database
            @param values_filename The filename for the value database
        */
        BTreeDB(const std::string& key_filename, const std::string& values_filename);

        /**
            Destructor for the BTree database. Iterates through the cache block and writebacks any still-dirty blocks of data to the databases and closes the file handlers.
        */
        ~BTreeDB();

        /**
            Inserts a key-value pair into the database according to the BTree structure.

            @param key The key
            @param value The value the key is associated with
        */
        void insert(long key, const std::string& value);
        /**
            Retrieves a value from the database according to a key.

            @param key The key to lookup
            @return The ValueEntry the key is associated with, or the default ValueEntry if it was not found.
        */
        ValueEntry find(long key);

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

                /**
                    Returns the parent pointer of the current page. 

                    @return parent pointer (bucket number) of parent; UINT_MAX if null
                */
                unsigned int get_parent_ptr() const;
                /**
                    Sets the parent pointer of the current page to a given value.

                    @param x The parent pointer (page number) to set
                */
                void set_parent_ptr(unsigned int x);

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
                    Returns the next pointer of this page

                    @return next pointer (page number), UINT_MAX if null
                */
                unsigned int get_next_ptr() const;
                /**
                    Set the next pointer of this page

                    @param ptr pointer (page number) to set this page's next pointer to
                */
                void set_next_ptr(unsigned int ptr);

                /**
                    Inserts a key entry (key + child_ptr) at the given location, moving everything else to the right accordingly.

                    @param key Key of the entry to insert
                    @param child_ptr child pointer that follows this key
                    @param loc location to insert this entry into; 0 to size
                */
                void insert(long key, unsigned int child_ptr, unsigned int loc);
                /**
                    Pushes a new key/child_ptr key pair to the end of the page.

                    @param key Key to insert
                    @param child_ptr child pointer to insert following the key
                */
                void push(long key, unsigned int child_ptr);
                /**
                    Changes the page this interface refers to.

                    @param page_num The page number to switch this interface to
                */
                void change_page(unsigned int page_num);
                /**
                    Moves the latter half of the keys in this interface to another interface. Used for splitting nodes.

                    @param other A reference to the interface to copy keys to (should be empty)
                */
                void move_keys(KeyPageInterface& other);

            private:
                /**
                    Helper variable to mark the current page number as dirty for writeback later. Used in all non-const functions in this class.
                */
                void handle_set();

                /**
                    The page number this interface refers to.
                */
                unsigned int page_num_;
                /**
                    A pointer to the tree this interface is serving. Used to access functions.
                */
                BTreeDB* tree_;
                /**
                    An array of char data representing the page data of the current page. 
                */
                char* data;
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
                ValueEntry get_value(unsigned int entry_num) const;
                /**
                    Sets the nth value in the value database to the passed value entry.

                    @param entry The ValueEntry to copy into the value database
                    @param entry_num The number value entry to change
                */
                void set_value(ValueEntry& entry, unsigned int entry_num);
                /**
                    Push a new value entry record into the value database. Inserts into the last page, if possible, or creates a new page if the last page is full.

                    @param entry The entry to insert into the value database
                    @return The entry number that represents the newly inserted record
                */
                unsigned int push(ValueEntry& entry);

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
            Splits a key page according to the BTree logic. Should only be called on a full key page. 

            Splits the latter half of the keys to a new key page and pushes the middle key to the parent. If there is no parent, creates a new parent key page.

            @param iter An interface to the key page to split
        */
        unsigned int split_page(KeyPageInterface& iter);

        /**
            Creates a new keypage.

            @return A keypage interface for the newly created keypage
        */
        KeyPageInterface create_new_keypage();

        /**
            Helper function to do binary search on a given keypage interface.

            @param iter A reference to the interface for the target key page
            @param key The key to do binary search on

            @return The key index that is the first key index that is greater than the passed in key
        */
        unsigned int find_pos(KeyPageInterface& iter, long key) const;

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
        CacheBlock& get_cache_block(unsigned int page_num, FileType type);

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
        */
        void serialize_value(const ValueEntry* const source, char* dest) const;
        /**
            Helper function to convert binary data to a human readable ValueEntry

            @param source The source pointer to read binary data from
            @param dest The ValueEntry pointer to write to
        */
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