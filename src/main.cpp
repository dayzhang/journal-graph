#include <iostream>
#include "../Dataset/parsing.cpp"
#include "journalGraph.cpp"
#include "../storage/vector_db.cpp"
#define exit_failure 0
#define exit_success 1

int main() {
    VectorDatabase db("test.db");

    db.insert(5, "abcd");
    std::cout << "among us" << std::endl;
    std::cout << db.select(5) << std::endl;
    db.insert(100, "deez nuts fdsafdsafkdsalf;dsafjdkasfjkdslajfkdasjfkfdsafdasfdf");
    std::cout << db.select(100) << std::endl;
    std::cout << db.select(-1) << std::endl;
    

    return 1;
}