#include "btree_db.h"

#include <iostream>
#include <cassert>
#include <stack>
#include <cstdint>
#include <stdexcept>
#include <exception>

unsigned int ENTRIES_PER_PAGE = PAGE_SIZE / ENTRY_SIZE - 1;

BTreeDB::BTreeDB(const std::string& key_filename, const std::string& values_filename) {
    std::fstream fs_keys(key_filename, std::ios::binary | std::ios::in | std::ios::out);
    std::fstream fs_values(values_filename, std::ios::binary | std::ios::in | std::ios::out);
    
    if (!fs_keys.is_open()) {
        fs_keys.open(key_filename, std::ios::binary | std::ios::trunc | std::fstream::out);
        fs_keys.close();
        fs_keys.open(key_filename, std::ios::binary | std::ios::in | std::ios::out);
    }
    if (!fs_keys.is_open()) {
        throw std::runtime_error("error reading/creating keys file");
    }

    if (!fs_values.is_open()) {
        fs_values.open(values_filename, std::ios::binary | std::ios::trunc | std::fstream::out);
        fs_values.close();
        fs_values.open(values_filename, std::ios::binary | std::ios::in | std::ios::out);
    }
    if (!fs_values.is_open()) {
        throw std::runtime_error("error reading/creating values file");
    }

    value_handler = std::move(fs_values);
    key_handler = std::move(fs_keys);

    value_handler.seekg(0, std::ios::end);
    key_handler.seekg(0, std::ios::end);
    unsigned int values_len = value_handler.tellg();
    unsigned int keys_len = key_handler.tellg();

    std::fstream fs_metadata(key_filename + values_filename, std::ios::in | std::ios::out);

    if (!fs_metadata.is_open()) {
        fs_metadata.open(key_filename + values_filename, std::ios::trunc| std::fstream::out);
        fs_metadata.close();
        fs_metadata.open(key_filename + values_filename, std::ios::in | std::ios::out);

        assert(values_len == 0 && keys_len == 0);

        num_entries = num_value_pages = num_key_pages = 0;
        key_root = 0;
    } else {
        unsigned int curr = 0;

        fs_metadata >> curr;
        num_entries = curr;
        fs_metadata >> curr;
        num_value_pages = curr;
        fs_metadata >> curr;
        num_key_pages = curr;
        fs_metadata >> curr;
        key_root = curr;
    }

    fs_metadata.close();
    fs_metadata.open(key_filename + values_filename, std::ios::out | std::ios::trunc);

    meta_handler = std::move(fs_metadata);
}

BTreeDB::~BTreeDB() {
    for (unsigned int i = 0; i < CACHE_SIZE; ++i) {
        if (key_cache.at(i).dirty) {
            write_page(key_cache.at(i).page_num, Key);
        }
        if (value_cache.at(i).dirty) {
            write_page(value_cache.at(i).page_num, Value);
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

void BTreeDB::insert(long key, const std::string& value) {
    ValueEntry to_add;
    if (value.size() >= 40) {
        strcpy(to_add.name, value.substr(0, 39).c_str());
    } else {
        strcpy(to_add.name, value.c_str());
    }

    if (num_key_pages == 0) {
        KeyPageInterface key_iter = create_new_keypage();
        ValuePageInterface value_iter(this);

        key_iter.set_root(true);
        key_iter.set_internal(false);
        key_iter.set_num_cells(0);
        key_iter.push(key, value_iter.push(to_add));

        return;
    }

    unsigned int curr_page = key_root;
    KeyPageInterface key_iter(curr_page, this);

    std::stack<unsigned int> traversal;

    while (key_iter.is_internal()) {
        traversal.push(key_iter.get_page_num());
        unsigned int target = find_pos(key_iter, key);
        unsigned int next_page = key_iter.get_child_ptr(target);
        if (key_iter.get_key(target) > target) {
            next_page = key_iter.get_child_ptr(target + 1);
        } 
        key_iter.change_page(next_page);
    }

    unsigned int target = find_pos(key_iter, key);
    ValuePageInterface value_iter(this);

    if (key_iter.get_key(target) == key) {
        value_iter.set_value(to_add, key_iter.get_child_ptr(target + 1));

        return;
    }

    key_iter.insert(key, value_iter.push(to_add), target);

    if (key_iter.get_num_cells() == ORDER) {

    }

}

BTreeDB::KeyPageInterface::KeyPageInterface(unsigned int page_num, BTreeDB* tree): page_num_(page_num), tree_(tree) {
    data = tree->get_page(page_num, Key);
}

unsigned int BTreeDB::KeyPageInterface::get_num_cells() const {
    unsigned int res;
    memcpy(&res, data, 4);
    return res;
}

void BTreeDB::KeyPageInterface::set_num_cells(unsigned int x) {
    memcpy(data, &x, 4);
    handle_set();
}

bool BTreeDB::KeyPageInterface::is_internal() const {
    char res;
    memcpy(&res, data + 4, 1);
    return res == 1;
}

void BTreeDB::KeyPageInterface::set_internal(bool x) {
    char temp = x;
    memcpy(data + 4, &temp, 1);
    handle_set();
}

bool BTreeDB::KeyPageInterface::is_root() const {
    char res;
    memcpy(&res, data + 5, 1);
    return res == 1;
}

void BTreeDB::KeyPageInterface::set_root(bool x) {
    char temp = x;
    memcpy(data + 5, &temp, 1);
    handle_set();
}

unsigned int BTreeDB::KeyPageInterface::get_parent_ptr() const {
    unsigned int res;
    memcpy(&res, data + 8, 4);
    return res;
}

void BTreeDB::KeyPageInterface::set_parent_ptr(unsigned int x) {
    memcpy(data + 8, &x, 4);
    handle_set();
}

long BTreeDB::KeyPageInterface::get_key(unsigned int entry_num) const {
    if (entry_num >= get_num_cells()) {
        throw std::runtime_error("entry num out of bounds");
    }
    char* curr = data;

    curr += HEADER_SIZE; // get past headers
    curr += 4; // get past first child pointer

    curr += entry_num * KEYENTRY_SIZE;
    long res;
    memcpy(&res, curr, 8);
    return res;
}

unsigned int BTreeDB::KeyPageInterface::get_page_num() const  {
    return page_num_;
}

void BTreeDB::KeyPageInterface::insert(long key, unsigned int child_ptr, unsigned int loc) {
    unsigned int num_cells = get_num_cells();
    char* curr = data;
    curr += HEADER_SIZE;
    curr += 4;

    curr += loc * ENTRY_SIZE;

    char swap[KEYENTRY_SIZE];
    memcpy(swap, curr, KEYENTRY_SIZE);
    for (unsigned int i = loc; i < num_cells; ++i) {
        char temp[KEYENTRY_SIZE];
        memcpy(temp, curr + KEYENTRY_SIZE * (i + 1), KEYENTRY_SIZE);
        memcpy(curr + KEYENTRY_SIZE * (i + 1), swap, KEYENTRY_SIZE);
        memcpy(swap, temp, KEYENTRY_SIZE);
    }

    memcpy(curr, &key, 8);
    memcpy(curr + 8, &child_ptr, 4);

    handle_set();
}

void BTreeDB::KeyPageInterface::push(long key, unsigned int child_ptr) {
    unsigned int num_cells = get_num_cells();
    char* curr = data;
    curr += HEADER_SIZE;
    curr += 4;

    curr += num_cells * ENTRY_SIZE;
    memcpy(curr, &key, 8);
    memcpy(curr + 8, &child_ptr, 4);

    set_num_cells(num_cells + 1);
}

unsigned int BTreeDB::KeyPageInterface::get_child_ptr(unsigned int entry_num) const {
    if (entry_num > get_num_cells()) {
        throw std::runtime_error("entry num out of bounds");
    }
    char* curr = data;
    curr += HEADER_SIZE; // get past headers

    if (entry_num != 0) {
        curr += 4;
        curr += (entry_num - 1) * KEYENTRY_SIZE;
    }

    unsigned int res;
    memcpy(&res, curr, 4);
    return res;
}

void BTreeDB::KeyPageInterface::set_child_ptr(unsigned int target, unsigned int entry_num) {
    if (entry_num > get_num_cells()) {
        throw std::runtime_error("entry num out of bounds");
    }
    char* curr = data;
    curr += HEADER_SIZE; // get past headers

    if (entry_num != 0) {
        curr += 4;
        data += (entry_num - 1) * KEYENTRY_SIZE;
    }

    unsigned int res;
    memcpy(data, &target, 4);
    handle_set();
}

unsigned int BTreeDB::KeyPageInterface::get_next_ptr() const {
    unsigned int res;
    memcpy(&res, data + 12, 4);
    return res;
}

void BTreeDB::KeyPageInterface::set_next_ptr(unsigned int ptr) {
    memcpy(data + 12, &ptr, 4);
    handle_set();
}

void BTreeDB::KeyPageInterface::change_page(unsigned int page_num) {
    page_num_ = page_num;
    data = tree_->get_page(page_num, Key);
}

void BTreeDB::KeyPageInterface::move_keys(KeyPageInterface& other) {

    other.handle_set();
    handle_set();
}

void BTreeDB::KeyPageInterface::handle_set()  {
    data = tree_->get_page(page_num_, Key);
    tree_->set_dirty(page_num_, Key);
}

BTreeDB::ValuePageInterface::ValuePageInterface(BTreeDB* tree): tree_(tree) {}

BTreeDB::ValueEntry BTreeDB::ValuePageInterface::get_value(unsigned int entry_num) const {
    unsigned int page_num = entry_num / ENTRIES_PER_PAGE;
    if (page_num >= tree_->num_value_pages) {
        throw std::runtime_error("entry num out of bounds");
    }
    unsigned int target = entry_num % ENTRIES_PER_PAGE;
    char* data = tree_->get_page(page_num, Value);
    data += 4; // get past size header

    data += target * ENTRY_SIZE;

    ValueEntry res;
    tree_->deserialize_value(data, &res);

    return res;
}

void BTreeDB::ValuePageInterface::set_value(ValueEntry& entry, unsigned int entry_num) {
    unsigned int page_num = entry_num / ENTRIES_PER_PAGE;
    if (page_num >= tree_->num_value_pages) {
        throw std::runtime_error("entry num out of bounds");
    }
    unsigned int target = entry_num % ENTRIES_PER_PAGE;
    char* data = tree_->get_page(page_num, Value);
    data += 4; // get past size header

    data += target * ENTRY_SIZE;

    tree_->serialize_value(&entry, data);
    tree_->set_dirty(page_num, Value);
}


unsigned int BTreeDB::ValuePageInterface::push(ValueEntry& entry) {
    tree_->num_entries += 1;
    unsigned int page_num = tree_->num_entries / ENTRIES_PER_PAGE;
    if (page_num == tree_->num_value_pages) {
        std::array<char, PAGE_SIZE> new_value_page;
        new_value_page.fill(0);
        unsigned int temp = 1;
        memcpy(new_value_page.data(), &temp, 4);
        memcpy(new_value_page.data() + 4, &(entry.name), ENTRY_SIZE);
        tree_->write_page(page_num, Value);
    } else {
        char* data = tree_->get_page(page_num, Value);
        unsigned int size;
        memcpy(&size, data, 4);
        tree_->serialize_value(&entry, data + 4 * ENTRY_SIZE);
        memcpy(data + 4 + size * ENTRY_SIZE, &(entry.name), ENTRY_SIZE);
        set_value_size(page_num, size + 1);
    }

    return tree_->num_entries;
}

unsigned int BTreeDB::ValuePageInterface::get_value_size(unsigned int page_num) const {
    if (page_num >= tree_->num_value_pages) {
        throw std::runtime_error("entry num out of bounds");
    }
    char* data = tree_->get_page(page_num, Value);
    unsigned int res;
    memcpy(&res, data, 4);
    return res;
}

void BTreeDB::ValuePageInterface::set_value_size(unsigned int page_num, unsigned int size) {
    if (page_num >= tree_->num_value_pages) {
        throw std::runtime_error("entry num out of bounds");
    }
    char* data = tree_->get_page(page_num, Value);
    memcpy(data, &size, 4);
    tree_->set_dirty(page_num, Value);
}

BTreeDB::KeyPageInterface BTreeDB::create_new_keypage() {
    std::array<char, PAGE_SIZE> new_page;
    new_page.fill(CHAR_MAX);

    key_handler.seekg(0, std::ios::end);
    key_handler.write(new_page.data(), PAGE_SIZE);
    get_page(num_key_pages - 1, Key);
    ++num_key_pages;

    return KeyPageInterface(num_key_pages - 1, this);
}

unsigned int BTreeDB::find_pos(KeyPageInterface& iter, long key) const {
    unsigned int size = iter.get_num_cells();
    if (size == 0) {
        return 0;
    }

    int left = 0;
    int right = size - 1;

    while (left <= right) {
        int middle = (left + right) / 2;
        if (iter.get_key(middle) == key) {
            return middle;
        }

        if (iter.get_key(middle) < key) {
            left = middle + 1;
        } else {
            right = middle - 1;
        }
    }
    return left;
}

void BTreeDB::set_dirty(unsigned int page_num, FileType type) {
    switch (type) {
        case Key: 
            key_cache[get_page_idx(page_num)].dirty = true;
        case Value:
            value_cache[get_page_idx(page_num)].dirty = true;
    }
}

unsigned int BTreeDB::get_page_idx(unsigned int page_num) const {
    return page_num % CACHE_SIZE;
}

void BTreeDB::write_page(unsigned int page_num, FileType type) {
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
        }
    }
}

BTreeDB::CacheBlock& BTreeDB::get_cache_block(unsigned int page_num, FileType type) {
    switch(type) {
        case Key: {
            if (page_num >= num_key_pages) {
                throw std::runtime_error("requested key page out of bounds");
            }
            return key_cache[get_page_idx(page_num)];
        }
        case Value: {
            if (page_num >= num_value_pages) {
                throw std::runtime_error("requested value page out of bounds");
            }
            return value_cache[get_page_idx(page_num)];
        }
    }
}

char* BTreeDB::get_page(unsigned int page_num, FileType type) {
    CacheBlock& cache_get = get_cache_block(page_num, type);

    if (cache_get.page_num != page_num || !cache_get.valid) {
        Page& page = cache_get.page;
        cache_get.valid = true;

        if (cache_get.dirty) {
            write_page(cache_get.page_num, type);
        }

        char buffer[PAGE_SIZE];
        key_handler.seekg(page_num * PAGE_SIZE, std::ios::beg);

        key_handler.read(buffer, PAGE_SIZE);
        memcpy(page.data(), buffer, PAGE_SIZE);

        cache_get.page_num = page_num;
        cache_get.page = page;
        cache_get.dirty = false;
    }

    return cache_get.page.data();
}

void BTreeDB::serialize_value(const ValueEntry* const source, char* dest) const {
    memcpy(dest, &(source->name), ENTRY_SIZE);
}

void BTreeDB::deserialize_value(const char* const source, ValueEntry* dest) const {
    memcpy(&(dest->name), source, ENTRY_SIZE);
}

// #include <cstdint>
// #include <cstring>
// #include <stdexcept>
// #include <string>
// #include <vector>
// #include <fstream>
// #include <exception>
// #include <tuple>
// #include <array>
// #include <iostream>
// #include <cassert>
// #include <stack>
// #include <climits>

// #define CACHE_SIZE 10
// #define ORDER 337
// #define PAGE_SIZE 4096
// #define KEYENTRY_SIZE 12
// #define HEADER_SIZE 16
// #define ENTRY_SIZE 40

// unsigned int ENTRIES_PER_PAGE = PAGE_SIZE / ENTRY_SIZE - 1;

// enum FileType {
//     Key,
//     Value
// };

// class BTreeDB {
//     public:
//         BTreeDB(const std::string& key_filename, const std::string& values_filename) {
//             std::fstream fs_keys(key_filename, std::ios::binary | std::ios::in | std::ios::out);
//             std::fstream fs_values(values_filename, std::ios::binary | std::ios::in | std::ios::out);
            
//             if (!fs_keys.is_open()) {
//                 fs_keys.open(key_filename, std::ios::binary | std::ios::trunc | std::fstream::out);
//                 fs_keys.close();
//                 fs_keys.open(key_filename, std::ios::binary | std::ios::in | std::ios::out);
//             }
//             if (!fs_keys.is_open()) {
//                 throw std::runtime_error("error reading/creating keys file");
//             }

//             if (!fs_values.is_open()) {
//                 fs_values.open(values_filename, std::ios::binary | std::ios::trunc | std::fstream::out);
//                 fs_values.close();
//                 fs_values.open(values_filename, std::ios::binary | std::ios::in | std::ios::out);
//             }
//             if (!fs_values.is_open()) {
//                 throw std::runtime_error("error reading/creating values file");
//             }

//             value_handler = std::move(fs_values);
//             key_handler = std::move(fs_keys);

//             value_handler.seekg(0, std::ios::end);
//             key_handler.seekg(0, std::ios::end);
//             unsigned int values_len = value_handler.tellg();
//             unsigned int keys_len = key_handler.tellg();

//             std::fstream fs_metadata(key_filename + values_filename, std::ios::in | std::ios::out);

//             if (!fs_metadata.is_open()) {
//                 fs_metadata.open(key_filename + values_filename, std::ios::trunc| std::fstream::out);
//                 fs_metadata.close();
//                 fs_metadata.open(key_filename + values_filename, std::ios::in | std::ios::out);

//                 assert(values_len == 0 && keys_len == 0);

//                 num_entries = num_value_pages = num_key_pages = 0;
//                 key_root = 0;
//             } else {
//                 unsigned int curr = 0;

//                 fs_metadata >> curr;
//                 num_entries = curr;
//                 fs_metadata >> curr;
//                 num_value_pages = curr;
//                 fs_metadata >> curr;
//                 num_key_pages = curr;
//                 fs_metadata >> curr;
//                 key_root = curr;
//             }

//             fs_metadata.close();
//             fs_metadata.open(key_filename + values_filename, std::ios::out | std::ios::trunc);

//             meta_handler = std::move(fs_metadata);
//         }

//         ~BTreeDB() {
//             for (unsigned int i = 0; i < CACHE_SIZE; ++i) {
//                 if (key_cache.at(i).dirty) {
//                     write_page(key_cache.at(i).page_num, Key);
//                 }
//                 if (value_cache.at(i).dirty) {
//                     write_page(value_cache.at(i).page_num, Value);
//                 }
//             }

//             key_handler.close();
//             value_handler.close();

//             meta_handler << num_entries << std::endl;
//             meta_handler << num_value_pages << std::endl;
//             meta_handler << num_key_pages << std::endl;
//             meta_handler << key_root << std::endl;

//             meta_handler.close();
//         }

//         void insert(long key, const std::string& value) {
//             ValueEntry to_add;
//             if (value.size() >= 40) {
//                 strcpy(to_add.name, value.substr(0, 39).c_str());
//             } else {
//                 strcpy(to_add.name, value.c_str());
//             }

//             if (num_key_pages == 0) {
//                 KeyPageInterface key_iter = create_new_keypage();
//                 ValuePageInterface value_iter(this);

//                 key_iter.set_root(true);
//                 key_iter.set_internal(false);
//                 key_iter.set_num_cells(0);
//                 key_iter.push(key, value_iter.push(to_add));

//                 return;
//             }

//             unsigned int curr_page = key_root;
//             KeyPageInterface key_iter(curr_page, this);

//             std::stack<unsigned int> traversal;

//             while (key_iter.is_internal()) {
//                 traversal.push(key_iter.get_page_num());
//                 unsigned int target = find_pos(key_iter, key);
//                 unsigned int next_page = key_iter.get_child_pointer(target);
//                 if (key_iter.get_key(target) > target) {
//                     next_page = key_iter.get_child_pointer(target + 1);
//                 } 
//                 key_iter.change_page(next_page);
//             }

//             unsigned int target = find_pos(key_iter, key);
//             ValuePageInterface value_iter(this);

//             if (key_iter.get_key(target) == key) {
//                 value_iter.set_value(to_add, key_iter.get_child_pointer(target + 1));

//                 return;
//             }

//             key_iter.insert(key, value_iter.push(to_add), target);

//             if (key_iter.get_num_cells() == ORDER) {

//             }

//         }

//         std::string find(long key) {
//             return "";
//         }

//     private:
//         struct ValueEntry {
//             char name[40];
//         };

//         typedef std::array<char, PAGE_SIZE> Page;

//         struct CacheBlock {
//             unsigned int page_num;
//             bool dirty;
//             bool valid;
//             Page page;
//             CacheBlock(): page_num(0), dirty(false), valid(false) {
//                 page.fill(CHAR_MAX);
//             }
//         };

//         struct Header {
//             unsigned int num_cells;
//             char node_type;
//             char is_root;
//             unsigned int parent_ptr;
//             unsigned int next_ptr;
//         };

//         unsigned int num_value_pages;
//         unsigned int num_key_pages;
//         unsigned int num_entries;

//         unsigned int key_root;

//         std::array<CacheBlock, CACHE_SIZE> key_cache;
//         std::array<CacheBlock, CACHE_SIZE> value_cache;

//         std::fstream key_handler;
//         std::fstream value_handler;
//         std::fstream meta_handler;

//         class KeyPageInterface {
//             public:
//                 KeyPageInterface(unsigned int page_num, BTreeDB* tree): page_num_(page_num), tree_(tree) {
//                     data = tree->get_page(page_num, Key);
//                 }

//                 unsigned int get_num_cells() {
//                     unsigned int res;
//                     memcpy(&res, data, 4);
//                     return res;
//                 }

//                 void set_num_cells(unsigned int x) {
//                     memcpy(data, &x, 4);
//                     handle_set();
//                 }

//                 bool is_internal() {
//                     char res;
//                     memcpy(&res, data + 4, 1);
//                     return res == 1;
//                 }

//                 void set_internal(bool x) {
//                     char temp = x;
//                     memcpy(data + 4, &temp, 1);
//                     handle_set();
//                 }

//                 bool is_root() {
//                     char res;
//                     memcpy(&res, data + 5, 1);
//                     return res == 1;
//                 }

//                 void set_root(bool x) {
//                     char temp = x;
//                     memcpy(data + 5, &temp, 1);
//                     handle_set();
//                 }

//                 unsigned int get_parent_ptr() {
//                     unsigned int res;
//                     memcpy(&res, data + 8, 4);
//                     return res;
//                 }

//                 void set_parent_ptr(unsigned int x) {
//                     memcpy(data + 8, &x, 4);
//                     handle_set();
//                 }

//                 long get_key(unsigned int entry_num) {
//                     if (entry_num >= get_num_cells()) {
//                         throw std::runtime_error("entry num out of bounds");
//                     }
//                     char* curr = data;

//                     curr += HEADER_SIZE; // get past headers
//                     curr += 4; // get past first child pointer

//                     curr += entry_num * KEYENTRY_SIZE;
//                     long res;
//                     memcpy(&res, curr, 8);
//                     return res;
//                 }

//                 void insert(long key, unsigned int child_ptr, unsigned int loc) {
//                     unsigned int num_cells = get_num_cells();
//                     char* curr = data;
//                     curr += HEADER_SIZE;
//                     curr += 4;

//                     curr += loc * ENTRY_SIZE;

//                     char swap[KEYENTRY_SIZE];
//                     memcpy(swap, curr, KEYENTRY_SIZE);
//                     for (unsigned int i = loc; i < num_cells; ++i) {
//                         char temp[KEYENTRY_SIZE];
//                         memcpy(temp, curr + KEYENTRY_SIZE * (i + 1), KEYENTRY_SIZE);
//                         memcpy(curr + KEYENTRY_SIZE * (i + 1), swap, KEYENTRY_SIZE);
//                         memcpy(swap, temp, KEYENTRY_SIZE);
//                     }

//                     memcpy(curr, &key, 8);
//                     memcpy(curr + 8, &child_ptr, 4);

//                     handle_set();
//                 }

//                 void push(long key, unsigned int child_ptr) {
//                     unsigned int num_cells = get_num_cells();
//                     char* curr = data;
//                     curr += HEADER_SIZE;
//                     curr += 4;

//                     curr += num_cells * ENTRY_SIZE;
//                     memcpy(curr, &key, 8);
//                     memcpy(curr + 8, &child_ptr, 4);

//                     set_num_cells(num_cells + 1);
//                 }

//                 unsigned int get_child_pointer(unsigned int entry_num) {
//                     if (entry_num > get_num_cells()) {
//                         throw std::runtime_error("entry num out of bounds");
//                     }
//                     char* curr = data;
//                     curr += HEADER_SIZE; // get past headers

//                     if (entry_num != 0) {
//                         curr += 4;
//                         data += (entry_num - 1) * KEYENTRY_SIZE;
//                     }

//                     unsigned int res;
//                     memcpy(&res, data, 4);
//                     return res;
//                 }

//                 void set_child_pointer(unsigned int target, unsigned int entry_num) {
//                     if (entry_num > get_num_cells()) {
//                         throw std::runtime_error("entry num out of bounds");
//                     }
//                     char* curr = data;
//                     curr += HEADER_SIZE; // get past headers

//                     if (entry_num != 0) {
//                         curr += 4;
//                         data += (entry_num - 1) * KEYENTRY_SIZE;
//                     }

//                     unsigned int res;
//                     memcpy(data, &target, 4);
//                     handle_set();
//                 }

//                 unsigned int get_next_pointer() {
//                     unsigned int res;
//                     memcpy(&res, data + 12, 4);
//                     return res;
//                 }

//                 void set_next_pointer(unsigned int ptr) {
//                     memcpy(data + 12, &ptr, 4);
//                     handle_set();
//                 }

//                 void change_page(unsigned int page_num) {
//                     page_num_ = page_num;
//                     data = tree_->get_page(page_num, Key);
//                 }

//                 unsigned int get_page_num() {
//                     return page_num_;
//                 }

//                 void move_keys(KeyPageInterface& other) {

//                     other.handle_set();
//                     handle_set();
//                 }

//             private:
//                 void handle_set() {
//                     data = tree_->get_page(page_num_, Key);
//                     tree_->set_dirty(page_num_, Key);
//                 }

//                 unsigned int page_num_;
//                 BTreeDB* tree_;
//                 char* data;
//         };

//         class ValuePageInterface {
//             public:
//                 ValuePageInterface(BTreeDB* tree): tree_(tree) {}

//                 ValueEntry get_value(unsigned int entry_num) {
//                     unsigned int page_num = entry_num / ENTRIES_PER_PAGE;
//                     if (page_num >= tree_->num_value_pages) {
//                         throw std::runtime_error("entry num out of bounds");
//                     }
//                     unsigned int target = entry_num % ENTRIES_PER_PAGE;
//                     char* data = tree_->get_page(page_num, Value);
//                     data += 4; // get past size header

//                     data += target * ENTRY_SIZE;

//                     ValueEntry res;
//                     tree_->deserialize_value(data, &res);

//                     return res;
//                 }

//                 void set_value(ValueEntry& entry, unsigned int entry_num) {
//                     unsigned int page_num = entry_num / ENTRIES_PER_PAGE;
//                     if (page_num >= tree_->num_value_pages) {
//                         throw std::runtime_error("entry num out of bounds");
//                     }
//                     unsigned int target = entry_num % ENTRIES_PER_PAGE;
//                     char* data = tree_->get_page(page_num, Value);
//                     data += 4; // get past size header

//                     data += target * ENTRY_SIZE;

//                     tree_->serialize_value(&entry, data);
//                     tree_->set_dirty(page_num, Value);
//                 }

                
//                 unsigned int push(ValueEntry& entry) {
//                     tree_->num_entries += 1;
//                     unsigned int page_num = tree_->num_entries / ENTRIES_PER_PAGE;
//                     if (page_num == tree_->num_value_pages) {
//                         std::array<char, PAGE_SIZE> new_value_page;
//                         new_value_page.fill(0);
//                         unsigned int temp = 1;
//                         memcpy(new_value_page.data(), &temp, 4);
//                         memcpy(new_value_page.data() + 4, &(entry.name), ENTRY_SIZE);
//                         tree_->write_page(page_num, Value);
//                     } else {
//                         char* data = tree_->get_page(page_num, Value);
//                         unsigned int size;
//                         memcpy(&size, data, 4);
//                         tree_->serialize_value(&entry, data + 4 * ENTRY_SIZE);
//                         memcpy(data + 4 + size * ENTRY_SIZE, &(entry.name), ENTRY_SIZE);
//                         set_value_size(page_num, size + 1);
//                     }

//                     return tree_->num_entries;
//                 }

//                 unsigned int get_value_size(unsigned int page_num) {
//                     if (page_num >= tree_->num_value_pages) {
//                         throw std::runtime_error("entry num out of bounds");
//                     }
//                     char* data = tree_->get_page(page_num, Value);
//                     unsigned int res;
//                     memcpy(&res, data, 4);
//                     return res;
//                 }

//                 void set_value_size(unsigned int page_num, unsigned int size) {
//                     if (page_num >= tree_->num_value_pages) {
//                         throw std::runtime_error("entry num out of bounds");
//                     }
//                     char* data = tree_->get_page(page_num, Value);
//                     memcpy(data, &size, 4);
//                     tree_->set_dirty(page_num, Value);
//                 }
                
//             private:
//                 BTreeDB* tree_;
//         };

//         unsigned int split_page(KeyPageInterface iter) {
//             if (iter.is_root()) {

//                 KeyPageInterface new_root = create_new_keypage();
//                 KeyPageInterface right = create_new_keypage();
                
//                 new_root.set_root(true);
//                 new_root.set_num_cells(1);
//                 new_root.set_internal(true);
//                 new_root.push(iter.get_key(ORDER / 2), right.get_page_num());
//                 new_root.set_child_pointer(iter.get_page_num(), 0);

//                 key_root = new_root.get_page_num();

//                 iter.set_root(false);
//                 iter.set_internal(false);
//                 iter.set_parent_ptr(new_root.get_page_num());
//                 iter.set_num_cells(PAGE_SIZE / 2 + 1);

//                 right.set_root(false);
//                 right.set_internal(false);
//                 right.set_parent_ptr(new_root.get_page_num());
//                 right.set_num_cells(PAGE_SIZE / 2);


                
//             }

//             return 0;
//         }

//         KeyPageInterface create_new_keypage() {
//             std::array<char, PAGE_SIZE> new_page;
//             new_page.fill(CHAR_MAX);

//             key_handler.seekg(0, std::ios::end);
//             key_handler.write(new_page.data(), PAGE_SIZE);
//             get_page(num_key_pages - 1, Key);
//             ++num_key_pages;

//             return KeyPageInterface(num_key_pages - 1, this);
//         }

//         unsigned int find_pos(KeyPageInterface& iter, long key) {
//             unsigned int size = iter.get_num_cells();
//             if (size == 0) {
//                 return 0;
//             }

//             int left = 0;
//             int right = size - 1;

//             while (left <= right) {
//                 int middle = (left + right) / 2;
//                 if (iter.get_key(middle) == key) {
//                     return middle;
//                 }

//                 if (iter.get_key(middle) < key) {
//                     left = middle + 1;
//                 } else {
//                     right = middle - 1;
//                 }
//             }
//             return left;
//         }

//         void set_dirty(unsigned int page_num, FileType type) {
//             switch (type) {
//                 case Key: 
//                     key_cache[get_page_idx(page_num)].dirty = true;
//                 case Value:
//                     value_cache[get_page_idx(page_num)].dirty = true;
//             }
//         }

//         unsigned int get_page_idx(unsigned int page_num) {
//             return page_num % CACHE_SIZE;
//         }

//         void write_page(unsigned int page_num, FileType type) {
//             switch (type) {
//                 case Key: {
//                     key_handler.seekg(page_num * PAGE_SIZE, std::ios::beg);
//                     char* curr_page = get_page(page_num, type);
//                     key_handler.write(curr_page, PAGE_SIZE);
//                     break;
//                 }
//                 case Value: {
//                     value_handler.seekg(page_num * PAGE_SIZE, std::ios::beg);
//                     char* curr_page = get_page(page_num, type);
//                     value_handler.write(curr_page, PAGE_SIZE);
//                 }
//             }
//         }

//         CacheBlock& get_cache_block(unsigned int page_num, FileType type) {
//             switch(type) {
//                 case Key: {
//                     if (page_num >= num_key_pages) {
//                         throw std::runtime_error("requested key page out of bounds");
//                     }
//                     return key_cache[get_page_idx(page_num)];
//                 }
//                 case Value: {
//                     if (page_num >= num_value_pages) {
//                         throw std::runtime_error("requested value page out of bounds");
//                     }
//                     return value_cache[get_page_idx(page_num)];
//                 }
//             }
//         }

//         char* get_page(unsigned int page_num, FileType type) {
//             CacheBlock& cache_get = get_cache_block(page_num, type);

//             if (cache_get.page_num != page_num || !cache_get.valid) {
//                 Page& page = cache_get.page;
//                 cache_get.valid = true;

//                 if (cache_get.dirty) {
//                     write_page(cache_get.page_num, type);
//                 }

//                 char buffer[PAGE_SIZE];
//                 key_handler.seekg(page_num * PAGE_SIZE, std::ios::beg);

//                 key_handler.read(buffer, PAGE_SIZE);
//                 memcpy(page.data(), buffer, PAGE_SIZE);

//                 cache_get.page_num = page_num;
//                 cache_get.page = page;
//                 cache_get.dirty = false;
//             }

//             return cache_get.page.data();
//         }

//         void serialize_value(const ValueEntry* const source, char* dest) {
//             memcpy(dest, &(source->name), ENTRY_SIZE);
//         }

//         void deserialize_value(const char* const source, ValueEntry* dest) {
//             memcpy(&(dest->name), source, ENTRY_SIZE);
//         }

//         // void serialize_header(Header* source, char* dest) {
//         //     memcpy(dest, &(source->num_cells), 4);
//         //     memcpy(dest + 4, &(source->node_type), 1);
//         //     memcpy(dest + 5, &(source->is_root), 1);
//         //     memcpy(dest + 8, &(source->parent_ptr), 4);
//         // }

//         // void deserialize_header(char* source, Header* dest) {
//         //     memcpy(&(dest->num_cells), source, 4);
//         //     memcpy(&(dest->node_type), source + 4, 1);
//         //     memcpy(&(dest->is_root), source + 5, 1);
//         //     memcpy(&(dest->parent_ptr), source + 8, 4);
//         // }
// };