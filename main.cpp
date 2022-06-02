#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <fstream>
#include <vector>
#include <iterator>
#include <sstream>

using namespace std;

// 基本类型定义
typedef signed char int8;
typedef unsigned char uint8;

typedef int int16;
typedef unsigned int uint16;

typedef long int32;
typedef unsigned long uint32;

typedef long long int64_t;
typedef unsigned long long uint64_t;

// 全局常量
const unsigned long wghMaxLength = 30000000000; // 全基因组碱基对位点
const unsigned int chrMaxLength = 50000; // 单个染色体碱基对位点
const unsigned int chrNum = 24; // 染色体类型数量

// 统计字段定义
typedef map<int, int> ChrDepthInfo;
typedef map<string, ChrDepthInfo> WghDepthInfo;
WghDepthInfo wghDepthInfo;

// 字段空格拆分
vector<std::string> split(std::string &str) {
    istringstream iss(str);
    return vector<string>(istream_iterator<string>{iss}, istream_iterator<string>());
}

int main() {
    string samFileName;
    cin >> samFileName;
    ifstream inFile(samFileName);

    // 按行读取sam文件

    // 多线程统计
    string line;
    while (getline(inFile, line)) {
        auto field = split(line);
        string chr = field[2]; // 染色体名称
        unsigned int pos = std::stoi(field[3]); ; // 位点
        wghDepthInfo[chr][pos]++;
    }

    // 单线程合并

    std::cout << "Hello, World!" << std::endl;
    return 0;
}
