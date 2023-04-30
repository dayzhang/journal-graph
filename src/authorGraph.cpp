#include "authorGraph.h"
bool AuthorGraph::addEdge(float weight, const std::string& first, const std::string& second) {
    weighted_edge new_edge(weight, second);
    graph[first].push_back(new_edge);
}

void AuthorGraph::add_same_paper_authors(const std::vector<std::string>& authors_in_paper) {
    for (int auth_no = 0; auth_no < authors_in_paper.size() && auth_no < author_edge_limit; auth_no++) {
        for (int auth2_no = 0; auth2_no < authors_in_paper.size() && auth2_no < author_edge_limit; auth2_no++) {
            if (auth_no != auth2_no) {
                addEdge(same_paper_weight, authors_in_paper[auth_no], authors_in_paper[auth2_no]);
            }
        }
    }
}

void AuthorGraph::add_referenced_authors(const std::vector<std::string>& authors_in_paper, const std::vector<std::string>& authors_referenced) {
    //Typically, authors are listed in decreasing contribution with the last as the supervisor.
    //Effectively constant time, max 9 operations
    for (int auth_no = 0; auth_no < authors_in_paper.size() && auth_no < author_edge_limit; auth_no++) {
        for (int ref_no = 0; ref_no < authors_referenced.size() && ref_no < author_edge_limit; ref_no++) {
            addEdge(ref_author_weight, authors_in_paper[auth_no], authors_referenced[ref_no]);
        }
    }
}


AuthorGraph::AuthorGraph(const std::vector<std::pair<std::vector<std::string>, std::vector<std::string>>>& node_data) {
    //if authors in same paper, set weight to 1/5. Otherwise, if related by citation, let weight be 5
    //maps node to authors by index

    std::unordered_map<std::string, std::vector<std::string>> static_author_mapping;

    for (size_t paper = 0; paper < node_data.size(); paper++) {
        const std::string& root = node_data[paper].first[0];
        static_author_mapping[root] = node_data[paper].second;
    }

    for (const std::pair<std::vector<std::string>, std::vector<std::string>>& paper : node_data) {
        add_same_paper_authors(paper.second);
        for (size_t reference = 1; reference < paper.first.size(); reference++) {
            add_referenced_authors(paper.second, static_author_mapping[paper.first[reference]]);
        }
    }
}   

void AuthorGraph::print_graph() {
    for (auto& entry : graph) {
        std::cout << entry.first << " | ";
        for (auto& ele : entry.second) {
            std::cout << ele.destination << " " << ele.weight << "\t";
        }
        std::cout << "\n";
    }
}