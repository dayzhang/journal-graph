#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include <unordered_map>
#include <sstream>
#include <charconv>
#include <chrono>
#include <mutex>
#include <thread>
#include "boost/bind/bind.hpp"
#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"

using namespace boost::placeholders;

using dblp_id_t = std::pair<uint64_t, uint64_t>;

struct author_t
{
    dblp_id_t id{0,0};
    std::string name;
    std::string org;
};

struct venue_t
{
    std::string raw;
};

struct fos_t
{
    std::string name;
    double w{0.0};
};

struct index_entry_t
{
    std::string word;
    std::vector<int> indexes;
};

struct indexed_abstract_t
{
    int index_length{0};
    std::vector<index_entry_t> inverted_index;
};

struct v12_author_t
{
    std::string name;
    std::string org;
    int id{0};
};

struct article_t
{
    dblp_id_t id{0,0};
    std::string title;
    std::vector<author_t> authors;
    venue_t venue;
    int year{0};
    std::vector<std::string> keywords;
    std::vector<fos_t> fos;
    std::vector<dblp_id_t> references;
    int n_citation{0};
    std::string page_start;
    std::string page_end;
    std::string doc_type;
    std::string lang;
    std::string volume;
    std::string issue;
    std::string issn;
    std::string isbn;
    std::string doi;
    std::vector<std::string> url;
    std::string abstract;
    indexed_abstract_t indexed_abstract;
    int v12_id{0};
    std::vector<v12_author_t> v12_authors;
};

struct pair_hash
{
    template <class T1, class T2>
    std::size_t operator () (const std::pair<T1,T2> &p) const
    {
        auto h1 = std::hash<T1>{}(p.first);
        auto h2 = std::hash<T2>{}(p.second);
        return h1 ^ h2;  
    }
};

std::unordered_map<dblp_id_t, article_t, pair_hash> dblp;
std::mutex dblp_mutex;

bool id_string_to_id(const std::string& id_string, dblp_id_t& id)
{
    auto data_ptr = id_string.data();
    auto data_length = id_string.length();

    if (data_length > 16)
    {
        auto offset = data_length - 16;
        auto [ptr, ec] {std::from_chars(data_ptr + offset, data_ptr + offset + 16, id.first, 16)};
        if (ec != std::errc())
        {
            return false;
        }
        auto [ptr1, ec1] {std::from_chars(data_ptr, data_ptr + offset, id.second, 16)};
        if (ec1 != std::errc())
        {
            return false;
        }
    }
    else
    {
        auto [ptr, ec] {std::from_chars(data_ptr, data_ptr + data_length, id.first, 16)};
        if (ec != std::errc())
        {
            return false;
        }
        id.second = 0;
    }

    return true;
}

std::string id_to_id_string(const dblp_id_t& id)
{
    std::stringstream ss;
    ss << std::hex << id.second << id.first << std::dec;
    return ss.str();
}

bool parse_author(const boost::property_tree::ptree& sub_tree, author_t& author)
{
    bool has_name = false;
    bool has_org = false;
    bool has_id = false;

    {
        const auto& node = sub_tree.get_optional<std::string>("name");

        if (node.has_value())
        {
            author.name = node.value();
            has_name = true;
        }
    }

    {
        const auto& node = sub_tree.get_optional<std::string>("org");

        if (node.has_value())
        {
            author.org = node.value();
            has_org = true;
        }
    }

    {
        const auto& node = sub_tree.get_optional<std::string>("id");

        if (node.has_value())
        {
            if (id_string_to_id(node.value(), author.id))
            {
                has_id = true;
            }
        }
    }

    return has_name && has_org && has_id;
}

bool parse_fos(const boost::property_tree::ptree& sub_tree, fos_t& fos)
{
    bool has_name = false;
    bool has_w = false;

    {
        const auto& node = sub_tree.get_optional<std::string>("name");

        if (node.has_value())
        {
            fos.name = node.value();
            has_name = true;
        }
    }

    {
        const auto& node = sub_tree.get_optional<double>("w");

        if (node.has_value())
        {
            fos.w = node.value();
            has_w = true;
        }
    }

    return has_name && has_w;
}

bool parse_v12_author(const boost::property_tree::ptree& sub_tree, v12_author_t& author)
{
    bool has_name = false;
    bool has_org = false;
    bool has_id = false;

    {
        const auto& node = sub_tree.get_optional<std::string>("name");

        if (node.has_value())
        {
            author.name = node.value();
            has_name = true;
        }
    }

    {
        const auto& node = sub_tree.get_optional<std::string>("org");

        if (node.has_value())
        {
            author.org = node.value();
            has_org = true;
        }
    }

    {
        const auto& node = sub_tree.get_optional<int>("id");

        if (node.has_value())
        {
            author.id = node.value();
            has_id = true;
        }
    }

    return has_name && has_org && has_id;
}

bool parse_indexed_abstract(const boost::property_tree::ptree& sub_tree, indexed_abstract_t& indexed_abstract)
{
    bool has_index_length = false;
    bool has_inverted_index = false;

    {
        const auto& node = sub_tree.get_optional<int>("IndexLength");

        if (node.has_value())
        {
            indexed_abstract.index_length = node.value();
            has_index_length = true;
        }
    }

    {
        const auto& node = sub_tree.get_child_optional("InvertedIndex");

        if (node)
        {
            indexed_abstract.inverted_index.reserve(node.get().size());
            for (const auto& kv : node.get())
            {
                index_entry_t index_entry;
                index_entry.word = kv.first;
                index_entry.indexes.reserve(kv.second.size());
                for (const auto& kv1 : kv.second)
                {
                    index_entry.indexes.push_back(kv1.second.get_value<int>());
                }
                indexed_abstract.inverted_index.push_back(index_entry);
            }
            has_inverted_index = true;
        }
    }

    return has_index_length && has_inverted_index;
}

bool parse_json_string_for_article(const std::string& json_string, article_t& article)
{
    boost::property_tree::ptree tree;
    std::stringstream ss(json_string);

    try
    {
        boost::property_tree::json_parser::read_json(ss, tree);
    }
    catch(const std::exception& e)
    {
        std::cerr << "Caught exception while parsing json string:\n";
        std::cerr << json_string << "\n";
        std::cerr << e.what() << "\n";
        return false;
    }

    bool retval = false;

    {
        const auto& node = tree.get_optional<std::string>("id");

        if (node.has_value())
        {
            if (id_string_to_id(node.value(), article.id))
            {
                retval = true;
            }
        }
    }

    {
        const auto& node = tree.get_optional<std::string>("title");

        if (node.has_value())
        {
            article.title = node.value();
        }
    }

    {
        const auto& node = tree.get_child_optional("authors");

        if (node)
        {
            article.authors.reserve(node.get().size());
            for (const auto& kv : node.get())
            {
                author_t author;
                if (parse_author(kv.second, author))
                {
                    article.authors.push_back(author);
                }
            }
        }
    }

    {
        const auto& node = tree.get_optional<std::string>("venue.raw");

        if (node.has_value())
        {
            article.venue.raw = node.value();
        }
    }

    {
        const auto& node = tree.get_optional<int>("year");

        if (node.has_value())
        {
            article.year = node.value();
        }
    }

    {
        const auto& node = tree.get_child_optional("keywords");

        if (node)
        {
            article.keywords.reserve(node.get().size());
            for (const auto& kv : node.get())
            {
                article.keywords.push_back(kv.second.data());
            }
        }
    }

    {
        const auto& node = tree.get_child_optional("fos");

        if (node)
        {
            article.fos.reserve(node.get().size());
            for (const auto& kv : node.get())
            {
                fos_t fos;
                if (parse_fos(kv.second, fos))
                {
                    article.fos.push_back(fos);
                }
            }
        }
    }

    {
        const auto& node = tree.get_child_optional("references");

        if (node)
        {
            article.references.reserve(node.get().size());
            for (const auto& kv : node.get())
            {
                dblp_id_t id;
                if (id_string_to_id(kv.second.data(), id))
                {
                    article.references.push_back(id);
                }
            }
        }
    }

    {
        const auto& node = tree.get_optional<int>("n_citation");

        if (node.has_value())
        {
            article.n_citation = node.value();
        }
    }

    {
        const auto& node = tree.get_optional<std::string>("page_start");

        if (node.has_value())
        {
            article.page_start = node.value();
        }
    }

    {
        const auto& node = tree.get_optional<std::string>("page_end");

        if (node.has_value())
        {
            article.page_end = node.value();
        }
    }

    {
        const auto& node = tree.get_optional<std::string>("doc_type");

        if (node.has_value())
        {
            article.doc_type = node.value();
        }
    }

    {
        const auto& node = tree.get_optional<std::string>("lang");

        if (node.has_value())
        {
            article.lang = node.value();
        }
    }

    {
        const auto& node = tree.get_optional<std::string>("volume");

        if (node.has_value())
        {
            article.volume = node.value();
        }
    }

    {
        const auto& node = tree.get_optional<std::string>("issue");

        if (node.has_value())
        {
            article.issue = node.value();
        }
    }

    {
        const auto& node = tree.get_optional<std::string>("issn");

        if (node.has_value())
        {
            article.issn = node.value();
        }
    }

    {
        const auto& node = tree.get_optional<std::string>("isbn");

        if (node.has_value())
        {
            article.isbn = node.value();
        }
    }

    {
        const auto& node = tree.get_optional<std::string>("doi");

        if (node.has_value())
        {
            article.doi = node.value();
        }
    }

    {
        const auto& node = tree.get_child_optional("url");

        if (node)
        {
            article.url.reserve(node.get().size());
            for (const auto& kv : node.get())
            {
                article.url.push_back(kv.second.data());
            }
        }
    }

    {
        const auto& node = tree.get_optional<std::string>("abstract");

        if (node.has_value())
        {
            article.abstract = node.value();
        }
    }

    /*
    {
        const auto& node = tree.get_child_optional("indexed_abstract");

        if (node)
        {
            parse_indexed_abstract(node.get(), article.indexed_abstract);
        }
    }
    */
   
    {
        const auto& node = tree.get_optional<int>("v12_id");

        if (node.has_value())
        {
            article.v12_id = node.value();
        }
    }

    {
        const auto& node = tree.get_child_optional("v12_authors");

        if (node)
        {
            article.v12_authors.reserve(node.get().size());
            for (const auto& kv : node.get())
            {
                v12_author_t author;
                if (parse_v12_author(kv.second, author))
                {
                    article.v12_authors.push_back(author);
                }
            }
        }
    }

    return retval;
}

struct work_item_t
{
    std::string json_string;
};

std::list<work_item_t> work_queue;
std::mutex work_queue_mutex;

bool done = false;

void print_article(const article_t& article)
{
    std::cout << "id: " << id_to_id_string(article.id) << "\n";
    std::cout << "title: " << article.title << "\n";
    std::cout << "authors: [";
    for (size_t i = 0; i < article.authors.size(); i++)
    {
        const auto& author = article.authors.at(i);
        std::cout << "name: " << author.name << " org: " << author.org << " id: " << id_to_id_string(author.id);
        if (i < article.authors.size() - 1)
        {
            std::cout << ", ";
        }
    }
    std::cout << "]\n";
    std::cout << "venue.raw: " << article.venue.raw << "\n";
    std::cout << "year: " << article.year << "\n";
    std::cout << "keywords: [";
    for (size_t i = 0; i < article.keywords.size(); i++)
    {
        std::cout << article.keywords[i];
        if (i < article.keywords.size() - 1)
        {
            std::cout << ", ";
        }
    }
    std::cout << "]\n";
    std::cout << "fos: [";
    for (size_t i = 0; i < article.fos.size(); i++)
    {
        const auto& fos = article.fos.at(i);
        std::cout << "name: " << fos.name << " w: " << fos.w;
        if (i < article.fos.size() - 1)
        {
            std::cout << ", ";
        }
    }
    std::cout << "]\n";
    std::cout << "references: [";
    for (size_t i = 0; i < article.references.size(); i++)
    {
        std::cout << id_to_id_string(article.references[i]);
        if (i < article.references.size() - 1)
        {
            std::cout << ", ";
        }
    }
    std::cout << "]\n";
    std::cout << "n_citation: " << article.n_citation << "\n";
    std::cout << "page_start: " << article.page_start << "\n";
    std::cout << "page_end: " << article.page_end << "\n";
    std::cout << "doc_type: " << article.doc_type << "\n";
    std::cout << "lang: " << article.lang << "\n";
    std::cout << "volume: " << article.volume << "\n";
    std::cout << "issue: " << article.issue << "\n";
    std::cout << "issn: " << article.issn << "\n";
    std::cout << "isbn: " << article.isbn << "\n";
    std::cout << "doi: " << article.doi << "\n";
    std::cout << "url: [";
    for (size_t i = 0; i < article.url.size(); i++)
    {
        std::cout << article.url[i];
        if (i < article.url.size() - 1)
        {
            std::cout << ", ";
        }
    }
    std::cout << "]\n";
    std::cout << "abstract: " << article.abstract << "\n";
    std::cout << "indexed_abstract: IndexLength: " << article.indexed_abstract.index_length << " InvertedIndex: ";
    for (size_t i = 0; i < article.indexed_abstract.inverted_index.size(); i++)
    {
        const auto& index_entry = article.indexed_abstract.inverted_index[i];
        std::cout << index_entry.word << " [";
        for (size_t j = 0; j < index_entry.indexes.size(); j++)
        {
            std::cout << index_entry.indexes[j];
            if (j < index_entry.indexes.size() - 1)
            {
                std::cout << ", ";
            }
        }
        std::cout << "]";
        if (i < article.indexed_abstract.inverted_index.size() - 1)
        {
            std::cout << " ";
        }
    }
    std::cout << "\n";
    std::cout << "v12_id: " << article.v12_id << "\n";
    std::cout << "v12_authors: [";
    for (size_t i = 0; i < article.v12_authors.size(); i++)
    {
        const auto& author = article.v12_authors.at(i);
        std::cout << "name: " << author.name << " org: " << author.org << " id: " << author.id;
        if (i < article.v12_authors.size() - 1)
        {
            std::cout << ", ";
        }
    }
    std::cout << "]\n";
}

void worker()
{
    while (!done)
    {
        work_item_t work_item;

        {
            std::lock_guard lock(work_queue_mutex);
            if (work_queue.size() > 0)
            {
                work_item = work_queue.front();
                work_queue.pop_front();
            }
        }

        if (work_item.json_string.empty())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        else
        {
            article_t article;

            if (!parse_json_string_for_article(work_item.json_string, article))
            {
                std::cerr << "Failed to parse json string:\n";
                std::cerr << work_item.json_string << "\n";
                break;
            }

            {
                std::lock_guard lock(dblp_mutex);
                dblp.emplace(article.id, article);
            }
        }
    }
}

//tested on local machine
/*
int main(int argc, char* argv[])
{
    const char* json_file_name = "dblp_v14.json";
    bool verbose = false;

    for (size_t i = 1; i < argc; i++)
    {
        if (argv[i][0] == '-')
        {
            if (std::string(argv[i]) == "-v")
            {
                verbose = true;
            }
        }
        else
        {
            json_file_name = argv[i];
        }
    }

    std::ifstream in_file(json_file_name);

    if (!in_file.is_open())
    {
        std::cerr << "Cannot open file\n";
        return -1;
    }

    std::vector<std::thread> worker_threads;

    auto num_worker_threads = std::thread::hardware_concurrency() - 1;

    for (size_t i = 0; i < num_worker_threads; i++)
    {
        worker_threads.push_back(std::thread(worker));
    }

    size_t num_lines_parsed = 0;
    auto start_time = std::chrono::system_clock::now();

    while (in_file.good())
    {
        std::string line;

        std::getline(in_file, line);

        if (!line.empty())
        {
            if (line.length() == 1 && line[0] == '[')
            {
                continue;
            }
            else
            {
                auto last_char = line[line.length() -1];
                if (last_char == ',' || last_char == ']')
                {
                    line.pop_back();
                }
            }

            bool item_added_to_queue = false;

            while (!item_added_to_queue)
            {
                {
                    std::lock_guard lock(work_queue_mutex);
                    if (work_queue.size() < 100)
                    {
                        work_queue.push_back({line});
                        item_added_to_queue = true;
                    }
                }

                if (!item_added_to_queue)
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }
            }

            num_lines_parsed++;

            if ((num_lines_parsed % 10000) == 0)
            {
                std::cout << "num lines parsed = " << num_lines_parsed << "\n";
            }
        }
    }

    // wait until all work items have been pulled out of the queue
    while (1)
    {
        {
            std::lock_guard lock(work_queue_mutex);
            if (work_queue.size() == 0)
            {
                break;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    // tell worker threads to terminate
    done = true;

    // joining worker threads
    for (auto& worker_thread : worker_threads)
    {
        worker_thread.join();
    }

    auto end_time = std::chrono::system_clock::now();
    
    auto seconds_elapsed = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time);

    std::cout << "num lines parsed = " << num_lines_parsed << "\n";
    std::cout << "took " << seconds_elapsed.count() << " seconds\n";
    std::cout << "\n";

    if (verbose)
    {
        for (const auto& [id, article] : dblp)
        {
            print_article(article);
            std::cout << "\n";
        }
    }

    std::cout << "press a key to exit...";
    std::cin.get();

    return 0;
}
*/