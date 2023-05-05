#include "authorGraph.h"

#include <fstream>
#include <cstring>
#include <algorithm>

void AuthorGraph::addEdge(int weight, long source, long dest) {

    adj_list[source][dest] += weight;
}

void AuthorGraph::export_to_file(const std::string &filename) {
    std::ofstream ofs(filename, std::ios::trunc | std::ios::binary);
    unsigned int total_size = adj_list.size();
    ofs.write((char*) &total_size, 4);

    for (const auto& elem : adj_list) {
        ofs.write((char*) &(elem.first), 8);
        unsigned int size = elem.second.size();
        ofs.write((char*)(&size), 4);
        for (const auto& edge : elem.second) {
            ofs.write((char*)(&edge.first), 8);
            ofs.write((char*)(&edge.second), 4);
        }
    }

    ofs.close();
}

AuthorGraph::AuthorGraph(const std::string& filename) {
    std::ifstream ifs(filename, std::ios::in | std::ios::binary);
    
    if (!ifs.is_open()) {
        throw std::runtime_error("error opening author graph binary file");
    }

    unsigned int size = 0;
    char buff_4[4];
    char buff_8[8];

    ifs.read(buff_4, 4);
    memcpy(&size, buff_4, 4);

    for (unsigned int i = 0; i < size; ++i) {
        if (i % 100000 == 0) {
            std::cout << i << std::endl;
        }
        // if (i == 100000) break;

        long id = 0;
        ifs.read(buff_8, 8);
        memcpy(&id, buff_8, 8);

        unsigned int num_edges = 0;
        ifs.read(buff_4, 4);
        memcpy(&num_edges, buff_4, 4);

        if (num_edges == 0) {
            adj_list[id];
            continue;
        }

        char* edges = new char[num_edges * 12];
        ifs.read(edges, num_edges * 12);

        for (unsigned int j = 0; j < num_edges; ++j) {
            long edge_id = 0;
            int weight = 0;
            memcpy(&edge_id, edges + j * 12, 8);
            memcpy(&weight, edges + j * 12 + 8, 4);

            adj_list[id][edge_id] = weight;
        }

        delete[] edges;
    }

    ifs.close();
}

void AuthorGraph::add_same_paper_authors(const std::vector<unsigned long>& authors_in_paper, unsigned int n_citation) {
    for (unsigned int i = 0; i < authors_in_paper.size() && i < AUTHOR_EDGE_LIMIT; ++i) {
        for (unsigned int j = i + 1; j < authors_in_paper.size() && j < AUTHOR_EDGE_LIMIT; ++j) {
            addEdge(same_paper_weight * n_citation, authors_in_paper[i], authors_in_paper[j]);
            addEdge(same_paper_weight * n_citation, authors_in_paper[j], authors_in_paper[i]);
        }
    } 
}

void AuthorGraph::add_referenced_authors(const std::vector<unsigned long>& authors_in_paper, const std::array<long, 8>& authors_referenced, unsigned int n_citation_paper, unsigned int n_citation_ref) {
    for (unsigned int i = 0; i < authors_in_paper.size() && i < AUTHOR_EDGE_LIMIT; ++i) {
        for (unsigned int j = 0; j < AUTHOR_EDGE_LIMIT; ++j) {
            if (authors_referenced[j] == 0) {
                break;
            } 
            addEdge(ref_author_weight_orig * n_citation_paper + ref_author_weight_ref * n_citation_ref, authors_in_paper[i], authors_referenced[j]);
        }
    }
}

void AuthorGraph::print_graph() {
    for (auto& entry : adj_list) {
        std::cout << entry.first << " | ";
        for (auto& ele : entry.second) {
            std::cout << ele.first << " " << ele.second << "\t";
            if (ele.second == 5) {
                std::cout << "!!!!";
            }
        }
        std::cout << "\n";
    }
}

//For unit testing: Dont use on full dataset
AuthorGraph::AuthorGraph(const std::vector<author_parse_wrapper>& node_data) {

    std::unordered_map<unsigned long, std::vector<unsigned long>> static_author_mapping;

    for (size_t paper = 0; paper < node_data.size(); paper++) {
        const unsigned long& root = node_data[paper].source;
        static_author_mapping[root] = node_data[paper].authors;
    }

    for (const author_parse_wrapper& paper : node_data) {
        add_same_paper_authors(paper.authors, 1);
        for (const unsigned long& reference : paper.cited) {
            std::array<long, 8> arr;
            std::fill(arr.begin(), arr.end(), 0);
            const std::vector<unsigned long>& to_add = static_author_mapping[reference];
            std::copy_n(to_add.begin(), std::min(to_add.size(), arr.size()), arr.begin());

            add_referenced_authors(paper.authors, arr, 1, 1);
        }
    }

    num_nodes = adj_list.size();
}   

AuthorGraph::~AuthorGraph() {
    std::cout << "\nClosing Author Graph\n";
}