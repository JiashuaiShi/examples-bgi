#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <iterator>
#include <sstream>
#include <chrono>
#include <unordered_map>
#include <stdio.h>
#include <cmath>

using namespace std;
using namespace chrono;

// 比较两个sam文件版本

// 基本类型定义
typedef char int8;
typedef unsigned char uint8;

typedef int int32;
typedef unsigned int uint32;

typedef long long int64;
typedef unsigned long long uint64;

typedef long long int64;
typedef unsigned long long uint64;

// 统计字段定义
unordered_multimap<string, string> hashMap;  // key - line

// 字段空格拆分
vector<string> split(string &str) {
    istringstream iss(str);
    return vector<string>(istream_iterator<string>{iss}, istream_iterator<string>());
}

// Qname是有后缀
bool isQnameHasSuffix(string qName) {
    int len = qName.size();
    if (len < 3)
        return false;

    return qName.substr(len - 2) == "/1" || qName.substr(len - 2) == "/2";
}

// 去掉非标准qname的后缀
string trimQname(string s) {
    return s.substr(0, s.size() - 2);
}

string trimLine(string qName, string line) {
    return qName.append(line.substr(qName.size() + 2));
}

time_point<system_clock> getStartTime() {
    time_point<system_clock> start = system_clock::now();
    return start;
}

time_point<system_clock> getEndTime() {
    time_point<system_clock> end = system_clock::now();
    return end;
}

std::chrono::duration<double> getElapsed(time_point<system_clock> start, time_point<system_clock> end) {
    std::chrono::duration<double> elapsed = end - start;
    return elapsed;
}

constexpr const char *str_end(const char *str) {
    return *str ? str_end(str + 1) : str;
}

constexpr bool str_slant(const char *str) {
    return *str == '/' ? true : (*str ? str_slant(str + 1) : false);
}

constexpr const char *r_slant(const char *str) {
    return *str == '/' ? (str + 1) : r_slant(str - 1);
}

constexpr const char *file_name(const char *str) {
    return str_slant(str) ? r_slant(str_end(str)) : str;
}

// 截取输出文件名
string getResultFileName(string s1, string s2) {
    auto file1 = string(file_name(s1.c_str()));
    auto file2 = string(file_name(s2.c_str()));

    auto resultFileName =
            "diff__" + file1.substr(0, file1.size() - 4) + "__" + file2.substr(0, file2.size() - 4) + ".txt";
    cout << "result_file_name: " << resultFileName << endl;

    return resultFileName;
}

bool isSame(int v1, int v2, int threshold) {
    return abs(v1 - v2) <= threshold;
}

unordered_multimap<string, string> getHashMap(string fileName) {
    ifstream inFile(fileName);
    string line;

    bool isTestFlag = true;
    bool isNeedTrim = false;

    while (getline(inFile, line)) {
        auto field = split(line);

        // 头部其他字段，跳过
        if (field[0][0] == '@') {
            continue;
        }

        // 正负链取key值
        auto qName = field[0];

        // 测试Qname是否标准，只做1次
        if (isTestFlag) {
            isNeedTrim = isQnameHasSuffix(qName);
            isTestFlag = !isTestFlag;
        }

        // 对不标准qname进行转换
        if (isNeedTrim) {
            qName = trimQname(qName);
            line = trimLine(qName, line);
        }

//        bool isMinus = (stoi(field[1])) & 16;
//        if (isMinus) {
//            qName = "-" + qName;
//        }



        // 同条Qname的多条正链或者负链，都会添加到hashMap
        auto it = hashMap.find(qName);
        pair<string, string> value = make_pair(qName, line);
        hashMap.insert(value);
    }

    return hashMap;
}

int main(int argc, char *argv[]) {
    // 读取sam文件
    string samFileName1;
    string samFileName2;

    if (argc == 3) {
        samFileName1 = argv[1];
        samFileName2 = argv[2];
    } else {
        cout << "输入错误！" << endl;
        return 0;
    }

    // 根据sam文件1建立hashMap
    auto hashMap1 = getHashMap(samFileName1);
    auto hashMap2 = getHashMap(samFileName2);

    string query;

    while (cin >> query) {
        cout << "== file1 ==" << endl;

        auto itr = hashMap.equal_range(query);
        for (auto p = itr.first; p != itr.second; p++) {
            cout << p->second << endl;
        }

        cout << endl << "== file2 ==" << endl;
        itr = hashMap.equal_range(query);
        for (auto p = itr.first; p != itr.second; p++) {
            cout << p->second << endl;
        }
    }

    return 0;
}