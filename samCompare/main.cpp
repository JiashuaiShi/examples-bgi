#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <fstream>
#include <vector>
#include <iterator>
#include <sstream>
#include <time.h>
#include <chrono>
#include <unordered_map>
#include <unordered_set>

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
    ofstream out("./result.txt");
    auto it = hashMap.begin();
    while (it != hashMap.end()) {
        auto chr = it->first; // QName
        auto line = get<2>(it->second); // 存在差异的行
        cout << line << endl;
        out << line << endl;
        it++;
    }

    // 结束计时
    auto end = getEndTime();
    cout << "程序比较共消耗时间： " << getElapsed(start, end).count() << "s" << endl;

    cout << "处理结束，结果在 ./result.txt" << endl;

    return 0;
}