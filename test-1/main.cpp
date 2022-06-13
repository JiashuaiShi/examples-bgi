#include <string>
#include <vector>
#include <iostream>

using namespace std;

void split(const string &s, vector <string> &tokens, const string &delimiters = " ") {
    string::size_type lastPos = s.find_first_not_of(delimiters, 0);
    string::size_type pos = s.find_first_of(delimiters, lastPos);
    while (string::npos != pos || string::npos != lastPos) {
        tokens.push_back(s.substr(lastPos, pos - lastPos)); // use emplace_back after C++11
        lastPos = s.find_first_not_of(delimiters, pos);
        pos = s.find_first_of(delimiters, lastPos);
    }
}

int main() {
    string s = "1 2 3 4 5";

    vector <string> field;
    split(s, field);

    for (auto i = 0; i < field.size(); i++) {
        cout << field[i] << endl;
    }

    return 0;
}