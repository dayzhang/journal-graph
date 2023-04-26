#include "btree_db.h"

#include <iostream>
#include <cassert>
#include <stack>
#include <cstdint>
#include <stdexcept>
#include <exception>
#include <cstring>

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

    num_entries = num_value_pages = num_key_pages = key_root = 0;

    std::fstream fs_metadata(key_filename + values_filename + ".txt", std::ios::in | std::ios::out);

    if (!fs_metadata.is_open()) {
        fs_metadata.open(key_filename + values_filename + ".txt", std::ios::trunc| std::fstream::out);
        fs_metadata.close();
        fs_metadata.open(key_filename + values_filename + ".txt", std::ios::in | std::ios::out);

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
    fs_metadata.open(key_filename + values_filename + ".txt", std::ios::out | std::ios::trunc);

    std::cout << num_entries << ' ' << num_value_pages << ' ' << num_key_pages << ' ' << key_root << std::endl;

    meta_handler = std::move(fs_metadata);
    for (unsigned int i = 0; i < PAGE_SIZE; ++i) {
        empty_array[i] = 0;
    }
}

void BTreeDB::write_all() {
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

BTreeDB::~BTreeDB() {
    write_all();
}

void BTreeDB::insert(long key, ValueEntry& value) {
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

ValueEntry BTreeDB::find(long key) {
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

    return ValueEntry();
}

BTreeDB::KeyPageInterface::KeyPageInterface(unsigned int page_num, BTreeDB* tree): page_num_(page_num), tree_(tree) {}

unsigned int BTreeDB::KeyPageInterface::get_size() const {
    unsigned int res;
    memcpy(&res, get_data(), 4);
    return res;
}

void BTreeDB::KeyPageInterface::set_size(unsigned int x) {
    memcpy(get_data(), &x, 4);
    handle_set();
}

bool BTreeDB::KeyPageInterface::is_internal() const {
    char res;
    memcpy(&res, get_data() + 4, 1);
    return res == 1;
}

void BTreeDB::KeyPageInterface::set_internal(bool x) {
    char temp = x;
    memcpy(get_data() + 4, &temp, 1);
    handle_set();
}

bool BTreeDB::KeyPageInterface::is_root() const {
    char res;
    memcpy(&res, get_data() + 5, 1);
    return res == 1;
}

void BTreeDB::KeyPageInterface::set_root(bool x) {
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

long BTreeDB::KeyPageInterface::get_key(unsigned int entry_num) const {
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
unsigned int BTreeDB::KeyPageInterface::get_page_num() const  {
    return page_num_;
}

void BTreeDB::KeyPageInterface::insert(long key, unsigned int child_ptr, unsigned int loc) {
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

void BTreeDB::KeyPageInterface::push(long key, unsigned int child_ptr) {
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

unsigned int BTreeDB::KeyPageInterface::get_child_ptr(unsigned int entry_num) const {
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

void BTreeDB::KeyPageInterface::set_child_ptr(unsigned int target, unsigned int entry_num) {
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

void BTreeDB::KeyPageInterface::move_keys(KeyPageInterface& other) {
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

char* BTreeDB::KeyPageInterface::get_data() const {
    return tree_->get_page(page_num_, Key);
}

void BTreeDB::KeyPageInterface::handle_set()  {
    tree_->set_dirty(page_num_, Key);
}

BTreeDB::ValuePageInterface::ValuePageInterface(BTreeDB* tree): tree_(tree) {}

ValueEntry BTreeDB::ValuePageInterface::get_value(unsigned int entry_num) const {
    unsigned int page_num = entry_num / ENTRIES_PER_PAGE;
    if (page_num >= tree_->num_value_pages) {
        tree_->write_all();
        throw std::runtime_error("entry num out of bounds for pages value get");
    }
    unsigned int target = entry_num % ENTRIES_PER_PAGE;
    if (target >= get_size(page_num)) {
        tree_->write_all();
        std::cout << "data: " << page_num << ' ' << entry_num << ' ' << target << ' ' << get_size(page_num) << std::endl;
        throw std::runtime_error("entry_num out of bounds for specific page value get");
    }
    char* data = tree_->get_page(page_num, Value);
    data += 4; // get past size header

    data += target * ENTRY_SIZE;

    ValueEntry res;
    int temp;
    memcpy(&temp, data + 40, 4);
    tree_->deserialize_value(data, &res);

    return res;
}

void BTreeDB::ValuePageInterface::set_value(ValueEntry& entry, unsigned int entry_num) {
    unsigned int page_num = entry_num / ENTRIES_PER_PAGE;
    if (page_num >= tree_->num_value_pages) {
        tree_->write_all();
        throw std::runtime_error("entry num out of bounds value set");
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
        tree_->num_value_pages += 1;
        std::array<char, PAGE_SIZE> new_value_page;
        new_value_page.fill(DEFAULT_VAL);
        tree_->value_handler.write(new_value_page.data(), PAGE_SIZE);
    } 
    set_size(get_size(page_num) + 1, page_num);
    set_value(entry, tree_->num_entries - 1);
    tree_->set_dirty(page_num, Value);

    return tree_->num_entries - 1;
}

unsigned int BTreeDB::ValuePageInterface::get_size(unsigned int page_num) const {
    if (page_num >= tree_->num_value_pages) {
        tree_->write_all();
        throw std::runtime_error("entry num out of bounds size get");
    }
    char* data = tree_->get_page(page_num, Value);
    unsigned int res;
    memcpy(&res, data, 4);
    return res;
}

void BTreeDB::ValuePageInterface::set_size(unsigned int size, unsigned int page_num) {
    if (page_num >= tree_->num_value_pages) {
        tree_->write_all();
        throw std::runtime_error("entry num out of bounds size set");
    }
    char* data = tree_->get_page(page_num, Value);
    memcpy(data, &size, 4);
    tree_->set_dirty(page_num, Value);
}

void BTreeDB::KeyPageInterface::split_page() {
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

void BTreeDB::KeyPageInterface::split_page(KeyPageInterface& parent) {
    KeyPageInterface right = tree_->create_new_keypage(0, is_internal(), false);

    long middle_key = get_key(ORDER / 2);

    move_keys(right);
    parent.push(middle_key, right.get_page_num());
}

BTreeDB::KeyPageInterface BTreeDB::create_new_keypage(unsigned int num_cells, bool internal, bool is_root, unsigned int parent_ptr, unsigned int next_ptr) {
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

unsigned int BTreeDB::KeyPageInterface::find_pos(long key) const {
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

void BTreeDB::set_dirty(unsigned int page_num, FileType type) {
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
            break;
        }
    }
}

BTreeDB::CacheSet& BTreeDB::get_cache_set(unsigned int page_num, FileType type) {
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

char* BTreeDB::get_page(unsigned int page_num, FileType type) {
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

void BTreeDB::serialize_value(const ValueEntry* const source, char* dest) const {
    memcpy(dest, source->name.data(), 40);
    memcpy(dest + 40, &(source->temp), 4);
}

void BTreeDB::deserialize_value(const char* const source, ValueEntry* dest) const {
    memcpy(dest->name.data(), source, 40);
    memcpy(&(dest->temp), source + 40, 4);
}
