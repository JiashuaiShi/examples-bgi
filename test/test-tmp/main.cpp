#include <string>
#include <fstream>
#include <iostream>

using namespace std;

int main() {
    setvbuf(stdout, NULL, _IONBF, 0);

    ifstream inFile("/mnt/d/lines.txt");
    string line;

    // 按行分割
//#pragma omp parallel for
    auto c = getline(inFile, line);
    while (c) {
        cout << line << endl;
    }

    return 0;
}