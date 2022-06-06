#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <iterator>
#include <sstream>
#include <chrono>
#include <unordered_map>

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
unordered_multimap<string, tuple<string, uint64, string>> hashMap;

// 字段空格拆分
vector<string> split(string &str) {
    istringstream iss(str);
    return vector<string>(istream_iterator<string>{iss}, istream_iterator<string>());
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
            "diff_" + file1.substr(0, file1.size() - 4) + "_" + file2.substr(0, file1.size() - 4) + ".txt";
    cout << "result_file_name: " << resultFileName << endl;

    return resultFileName;
}

int main(int argc, char *argv[]) {
    // 读取sam文件
    string samFileName1;
    string samFileName2;

    if (argc == 3) {
        samFileName1 = argv[1];
        samFileName2 = argv[2];
    }

    if (argc != 3) {
        cout << "请输入sam文件1路径, 并按Enter结束 " << endl;
        cin >> samFileName1;

        cout << "请输入sam文件2路径, 并按Enter结束 " << endl;
        cin >> samFileName2;
    }

    getResultFileName(samFileName1, samFileName2);

    cout << "开始比较sam文件：" << endl;
    cout << samFileName1 << endl;
    cout << samFileName2 << endl;

    // 开始计时
    auto start = getStartTime();

    ifstream inFile1(samFileName1);

    // 根据sam文件1建立hashMap
    string line1;
    while (getline(inFile1, line1)) {
        auto field = split(line1);

        // 头部其他字段，跳过
        if (field[0][0] == '@') {
            continue;
        }

        // 正负链取key值
        auto qName = field[0];
        bool isMinus = (stoi(field[1])) & 16;
        if (isMinus) {
            qName = "-" + qName;
        }

        tuple<string, int, string> value = make_tuple(field[2], stoi(field[3]), line1);
        hashMap.insert({qName, value});
    }

    // 读取sam文件2开始比对
    ifstream inFile2(samFileName2);
    string line2;
    while (getline(inFile2, line2)) {
        auto field = split(line2);

        // 头部其他字段，跳过
        if (field[0][0] == '@') {
            continue;
        }

        // 正负链取key值
        auto qName = field[0];
        bool isMinus = (stoi(field[1])) & 16;
        if (isMinus) {
            qName = "-" + qName;
        }

        auto it = hashMap.find(qName);

        if (it == hashMap.end()) {
            cout << "未找到匹配key值" << endl;
            continue;
        }

        // 如相同，则移除
        cout << it->first << endl;
        cout << field[2] << " " << stoi(field[3]) << endl;
        cout << get<0>(it->second) << " " << get<1>(it->second) << endl;

        if (get<0>(it->second) == field[2] && get<1>(it->second) == stoi(field[3])) {
            hashMap.erase(it);
        } else {
            // 如不同，则填入
            tuple<string, uint64, string> value = make_tuple(field[2], stoi(field[3]), line2);
            hashMap.insert({qName, value});
        }
    }

    // 统计结果写入文件
    string resFileName = getResultFileName(samFileName1, samFileName2);
    ofstream out(resFileName);
    auto it = hashMap.begin();
    while (it != hashMap.end()) {
        auto chr = it->first; // QName
        auto line = get<2>(it->second); // 存在差异的行
        out << line << endl;
        it++;
    }

    // 结束计时
    auto end = getEndTime();
    cout << "程序比较共消耗时间： " << getElapsed(start, end).count() << "s" << endl;

    cout << "处理结束，结果在./" << resFileName << endl;

    return 0;
}