#include <iostream>
#define exit_failure 0
#define exit_success 1
#include "Dataset/parsing.cpp"

int main() {
    std::cout << "CS225 Project by Daniel Zhang, Ian Zhang, Kevin Chen, and Jenny Hu" << "\n";
    if (!dataset_parse()) {
        std::cout << "Failure parsing data \n";
        return exit_failure;
    }
    return exit_success;
}