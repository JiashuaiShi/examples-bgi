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
unordered_multimap<string, tuple<string, int, string>> hashMap;

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

int main(int argc, char *argv[]) {
    // 读取sam文件
    string samFileName1;
    string samFileName2;
    int threshold = 0;  // 位置比较阈值
    bool isSaveResult = true; // 是否保存结果文件

    uint64 allLines = 0; // 全部行数
    uint64 diffLines = 0; // 差异行数
    uint64 sameLines = 0; // 相同行数

    // 命令行参数
    if (argc == 2 && argv[1] == "-h") {
        // 使用提示：
        cout << "使用方法：" << endl;
        cout << "参数1： 基准Base.sam" << endl;
        cout << "参数2： 验证query.sam" << endl;
        cout << "参数3： pos校验阈值， 默认为0" << endl;
        cout << "参数4： 是否输出文件 0 不保存结果 1 输出结果到文件, 默认为1" << endl;
        cout << endl;
    } else if (argc == 3) {
        samFileName1 = argv[1];
        samFileName2 = argv[2];
    } else if (argc == 4) {
        samFileName1 = argv[1];
        samFileName2 = argv[2];
        threshold = stoi(argv[3]);
    } else if (argc == 5) {
        samFileName1 = argv[1];
        samFileName2 = argv[2];
        threshold = stoi(argv[3]);
        isSaveResult = (argv[4] == "1");
    } else {
        cout << "请输入sam文件1路径, 并按Enter结束 " << endl;
        cin >> samFileName1;

        cout << "请输入sam文件2路径, 并按Enter结束 " << endl;
        cin >> samFileName2;

        cout << "请输入误差阈值, 并按Enter结束 " << endl;
        cin >> threshold;

        cout << "是否输出文件( 0 or 1), 并按Enter结束 " << endl;
        cin >> isSaveResult;
    }

    getResultFileName(samFileName1, samFileName2);

    cout << "开始比较sam文件：" << endl;
    cout << "  " << samFileName1 << endl;
    cout << "  " << samFileName2 << endl;
    cout << "误差阈值：" << threshold << endl;
    cout << "是否保存文件： " << isSaveResult << endl;

    // 开始计时
    auto start = getStartTime();

    ifstream inFile1(samFileName1);

    // 根据sam文件1建立hashMap
    string line1;
    bool isTestFlag = true;
    bool isNeedTrim = false; // 是否需要去除qname的后缀 （以 '/1'或者'/2'结尾）

    while (getline(inFile1, line1)) {
        auto field = split(line1);

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
            line1 = trimLine(qName, line1);
        }

        bool isMinus = (stoi(field[1])) & 16;
        if (isMinus) {
            qName = "-" + qName;
        }

        tuple<string, int, string> value = make_tuple(field[2], stoi(field[3]), line1);
        hashMap.insert({qName, value});
    }

    allLines = hashMap.size();

    // 读取sam文件2开始比对
    ifstream inFile2(samFileName2);
    string line2;
    isTestFlag = true;
    isNeedTrim = false;

    while (getline(inFile2, line2)) {
        auto field = split(line2);

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
            line2 = trimLine(qName, line2);
        }

        bool isMinus = (stoi(field[1])) & 16;
        if (isMinus) {
            qName = "-" + qName;
        }

        auto it = hashMap.find(qName);
        tuple<string, uint64, string> value = make_tuple(field[2], stoi(field[3]), line2);

        // key不命中，插入HashMap
        if (it == hashMap.end()) {
            hashMap.insert({qName, value});
            continue;
        }

        // 如相同，则移除
        if (get<0>(it->second) == field[2] && isSame(get<1>(it->second), stoi(field[3]), threshold)) {
            hashMap.erase(it);
        } else {
            // 如不同，则填入
            hashMap.insert({qName, value});
        }
    }

    diffLines = hashMap.size();
    sameLines = allLines - diffLines;

    // 统计结果写入文件
    string resFileName = getResultFileName(samFileName1, samFileName2);
    auto it = hashMap.begin();

    char samePercent[10];
    sprintf(samePercent, "%.2f", sameLines * 100.0 / allLines);

    char diffPercent[10];
    sprintf(diffPercent, "%.2f", diffLines * 100.0 / allLines);

    string sum = "相同行数： " + to_string(sameLines) + "  " + "百分比：" + samePercent + "%" + '\n';
    sum += "不同行数： " + to_string(diffLines) + "  " + "百分比：  " + diffPercent + "%" + '\n';
    sum += "总共行数： " + to_string(allLines) + '\n' + '\n';

    if (isSaveResult) {
        FILE *fp = fopen(resFileName.c_str(), "w");
        fwrite(sum.c_str(), sizeof(char), sum.size(), fp);
        while (it != hashMap.end()) {
            auto chr = it->first; // QName
            auto line = get<2>(it->second) + '\n'; // 存在差异的行
            fwrite(line.c_str(), sizeof(char), line.size(), fp);
            it++;
        }
        fclose(fp);
        cout << "处理结束，结果在./" << resFileName << endl;
    }

    // 结束计时
    auto end = getEndTime();

    cout << sum;
    cout << "程序比较共消耗时间： " << getElapsed(start, end).count() << "s" << endl;

    return 0;
}