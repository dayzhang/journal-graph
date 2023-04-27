#include "authorGraph.h"

void add_same_paper_authors(const std::vector<std::string>& authors_in_paper) {
    
}

void add_ref_edge(const std::string& base_author, const std::string& referenced_author) {

}

void add_referenced_authors(const std::vector<std::string>& authors_in_paper, const std::vector<std::string>& authors_referenced) {
    //Typically, authors are listed in decreasing contribution with the last as the supervisor.
    //Effectively constant time, max 9 operations
    for (int auth_no = 0; auth_no < authors_in_paper.size() && auth_no < author_edge_limit; auth_no++) {
        for (int ref_no = 0; ref_no < authors_referenced.size() && ref_no < author_edge_limit; ref_no++) {
            add_ref_edge(authors_in_paper[auth_no], authors_referenced[ref_no]);
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