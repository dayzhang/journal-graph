#include <iostream>
#include <iomanip>
#include <limits>
#include <stdexcept>
#include <string>
#include <fstream>
#include <exception>

#include "../storage/btree_db_v2.hpp"
#include "../storage/btree_types.cpp"

using std::cout;
using std::endl;
using std::cin;

int main(int argc, char* argv[]) {
    if (argc != 5) {
        cout << "Invalid number of arguments passed." << endl;
        cout << "Usage: ./db_interface [key db filename] [value db filename] [type of the database (test, paper, or author)] [whether or not to create a new db (0 for no, 1 for yes)]" << endl;;

        return 0;
    }

    if (std::string(argv[4]) == "1") {
        cout << "Creating a new database file" << endl;
        BTreeDB<test::Entry> db(argv[1], argv[2], true);
    }

    cout << "Note: the key and value databases should be compatible, or an error will be thrown." << endl;
    cout << "We don't recommend insertion if you are working with the full graph/paper databases to prevent any data corruption." << endl;

    cout << "DO NOT ctrl + C to exit the prompt; only use the quit command to prevent data corruption." << endl;

    if (std::string(argv[3]) == "test") {
        BTreeDB<test::Entry> db(argv[1], argv[2]);

        while (true) {
            cout << ">> ";

            std::string input;
            cin >> input;

            if (input == "quit") {
                return 0;
            } else if (input == "insert") {
                cout << "Please provide the x:" << endl;

                int x;
                cin >> x;

                cout << "Please provide the str:" << endl;
                std::string str;
                cin >> str;

                cout << "Please provide the id:" << endl;
                long id;
                cin >> id;

                test::Entry entry(x, str, id);

                db.insert(entry.id, entry);
            } else if (input == "find") {
                cout << "Please provide the id you want to search for:" << endl;

                long id;
                cin >> id;

                test::Entry entry = db.find(id);

                if (entry.id == -1) {
                    cout << "Entry not found" << endl;
                } else {
                    cout << "x: " << entry.x << ", str: " << std::string(entry.str.data()) << endl; 
                }
            } else if (input == "get_id") {
                cout << "Please provide the x you want to find the id for:" << endl;

                int x;
                cin >> x;

                test::Entry entry(x);

                long found = db.get_id_from_name(entry);

                if (found == -1) {
                    cout << "entry not found" << endl;
                } else {
                    cout << "id: " << found << endl;
                }
            } else {
                cout << "Invalid command entered. Valid commands include insert, find, get_id, and quit" << endl;
            }
            cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    } else if (std::string(argv[3]) == "paper") {
        BTreeDB<paper::Entry> db(argv[1], argv[2]);

        while (true) {
            cout << ">> ";

            std::string input;
            cin >> input;

            if (input == "q") {
                return 0;
            } else if (input == "insert") {
                cout << "Please provide the title:" << endl;

                std::string title;
                cin >> title;

                cout << "Please provide the keywords space separated:" << endl;
                std::string keywords;
                cin >> keywords;

                cout << "Please provide me the number of authors:" << endl;

                unsigned int num_authors;
                cin >> num_authors;

                std::array<long, 8> authors;
                authors.fill(0);

                for (unsigned int i = 0; i < num_authors; ++i) {
                    cout << "Please provide the id of author number:" << i << endl;
                    long author_id;
                    cin >> author_id;
                    authors[i] = author_id;

                }

                cout << "Please provide the number of citations of this paper:" << endl;

                unsigned int n_citations;
                cin >> n_citations;

                cout << "Please provide the publication year of this paper:" << endl;

                unsigned int year;
                cin >> year;
    
                cout << "Please provide the paper id:" << endl;
                long id;
                cin >> id;

                paper::Entry entry(title, keywords, n_citations, year, authors, id);

                db.insert(entry.id, entry);

            } else if (input == "find") {
                cout << "Please provide the id you want to search for:" << endl;

                long id;
                cin >> id;

                paper::Entry entry = db.find(id);

                if (entry.id == -1) {
                    cout << "Entry not found" << endl;
                } else {
                    cout << "title: " << std::string(entry.title.data()) << ", keywords: " << std::string(entry.keywords.data()) << ", authors: ";

                    int j = 0;
                    while (entry.authors[j] != 0) {
                        cout << entry.authors[j];
                        ++j;
                    }

                    cout << ", number of citations: " << entry.n_citations << ", publication year: " << entry.pub_year << endl;
                }
            } else if (input == "get_id") {
                cout << "Please provide the title you want to find the id for:" << endl;

                std::string title;
                cin >> title;

                paper::Entry entry(title);

                long found = db.get_id_from_name(entry);

                if (found == -1) {
                    cout << "entry not found" << endl;
                } else {
                    cout << "id: " << found << endl;
                }
            } else {
                cout << input << endl;
                cout << "Invalid command entered. Valid commands include insert, find, get_id, and quit" << endl;
            }
            cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    } else if (std::string(argv[3]) == "author") {
        BTreeDB<author::Entry> db(argv[1], argv[2]);

        while (true) {
            cout << ">> ";

            std::string input;
            cin >> input;

            if (input == "q") {
                return 0;
            } else if (input == "insert") {
                cout << "Please provide the author name:" << endl;

                std::string name;
                cin >> name;

                cout << "Please provide the organization:" << endl;

                std::string org;
                cin >> org;

                cout << "Please provide me the id:" << endl;

                long id;
                cin >> id;

                author::Entry entry(name, org, id);

                db.insert(entry.id, entry);

            } else if (input == "find") {
                cout << "Please provide the id you want to search for:" << endl;

                long id;
                cin >> id;

                author::Entry entry = db.find(id);

                if (entry.id == -1) {
                    cout << "Entry not found" << endl;
                } else {
                    cout << "name: " << std::string(entry.name.data()) << ", organization: " << std::string(entry.organization.data()) << endl;
                }
            } else if (input == "get_id") {
                cout << "Please provide the name you want to find the id for: " << endl;

                std::string title;
                cin >> title;

                author::Entry entry(title);

                long found = db.get_id_from_name(entry);

                if (found == -1) {
                    cout << "entry not found" << endl;
                } else {
                    cout << "id: " << found << endl;
                }
            } else {
                cout << "Invalid command entered. Valid commands include insert, find, get_id, and quit" << endl;
            }
            cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }

    cout << "Invalid type provided; only test, paper, and author are valid options at the moment" << endl;


    return 0;
}