#include <iostream>
#include "json.hpp"
#include <fstream>
using namespace std;
using json = nlohmann::json;
// https://medium.com/ml2b/a-guide-to-json-using-c-a48039124f3a
int main()
{
std::ifstream f(“data.json”);
json data = json::parse(f);
// Access the values existing in JSON data
string name = data.value(“name”, “not found”);
string grade = data.value(“grade”, “not found”);
// Access a value that does not exist in the JSON data
string email = data.value(“email”, “not found”);
// Print the values
cout << “Name: “ << name << endl;
cout << “Grade: “ << grade << endl;
cout << “Email: “ << email << endl;
return 0;}
