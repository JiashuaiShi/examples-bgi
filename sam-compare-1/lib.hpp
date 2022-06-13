#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <iterator>
#include <sstream>
#include <chrono>
#include <unordered_map>
#include <cstdio>
#include <cmath>

using namespace std;
using namespace chrono;

// 基本类型定义
typedef char int8;
typedef unsigned char uint8;

typedef int int32;
typedef unsigned int uint32;

typedef long long int64;
typedef unsigned long long uint64;

typedef long long int64;
typedef unsigned long long uint64;

// 配置开关
bool isAutoRenameDiffName = false;  // 自动根据比对的两个文件命名结果文件
bool isSavaHashUnMatchFile = false; // 是否保存Hash未命中记录
bool isOpenEnhanceRules = false; // 是否启用增强比较规则

// 命令行读取参数
string gSamFileName1;
string gSamFileName2;
int gThreshold = 0;  // 位置比较阈值
bool gIsSaveResult = true; // 是否保存结果文件

// 全局变量
unordered_multimap<string, tuple<string, int, string>> gHashMap;
uint64 allLines = 0; // HashMap全部行数

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

// 程序时间统计
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

// 辅助函数，文件命名
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

    string resultFileName = "diffResult.txt";

    if (isAutoRenameDiffName) {
        resultFileName =
                "diff__" + file1.substr(0, file1.size() - 4) + "__" + file2.substr(0, file2.size() - 4) + ".txt";
        cout << "result_file_name: " << resultFileName << endl;
    }

    return resultFileName;
}

// pos阈值比较
bool isSame(int v1, int v2, int threshold) {
    return abs(v1 - v2) <= threshold;
}

// thread-buildMap
void buildMap(ifstream &inFile1) {
    bool isTestFlag = true;
    bool isNeedTrim = false; // 是否需要去除qname的后缀 （以 '/1'或者'/2'结尾）
    string line1;

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
            isTestFlag = false;
        }

        // 对不标准qname进行转换
        if (isNeedTrim) {
            qName = trimQname(qName);
            line1 = trimLine(qName, line1);
        }

        bool isMinus = (stoi(field[1])) & 16;
        if (isMinus) {
            qName = qName.append("-");
        }

        tuple<string, int, string> value = make_tuple(field[2], stoi(field[3]), line1);
        gHashMap.insert({qName, value});
    }
}

// thread-compare
void compare(ifstream &inFile2, int threshold) {
    string line2;
    bool isTestFlag = true;
    bool isNeedTrim = false; // 是否需要去除qname的后缀 （以 '/1'或者'/2'结尾）
    ofstream hashFile("./diffMisHashHit.txt");     // hash没有命中的文件保存

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
            isTestFlag = false;
        }

        // 对不标准qname进行转换
        if (isNeedTrim) {
            qName = trimQname(qName);
            line2 = trimLine(qName, line2);
        }

        // 通过flag判断正负链会有异常情况，比如两条记录都是16或者两条记录都是0
        bool isMinus = (stoi(field[1])) & 16;
        if (isMinus) {
            qName = qName.append("-");
        }

        auto it = gHashMap.find(qName);
        tuple<string, uint64, string> value = make_tuple(field[2], stoi(field[3]), line2);

        // 在此处可以根据flag，增加比对规则
        if (isOpenEnhanceRules) {
            int flag = stoi(field[1]);

            bool unMatch1 = flag & 0x40;  // read1匹配上
            bool unMatch2 = flag & 0x80;  // read2匹配上

            // 增加规则1： 没有匹配上的记录，直接判断不匹配
            if (unMatch1 || unMatch2) {
                gHashMap.insert({qName, value});
                continue;
            }
        }

        // key不命中，插入HashMap
        if (it == gHashMap.end()) {
            // 单独保存Hash未命中记录
            if (isSavaHashUnMatchFile) {
                hashFile << line2 << endl;
            }
            continue;
        }

        // 规则： 如相同，则移除
        if (get<0>(it->second) == field[2]
            && isSame(get<1>(it->second), stoi(field[3]), threshold)) {
            gHashMap.erase(it);
        } else {
            // 如不同，则填入
            gHashMap.insert({qName, value});
        }

        hashFile.close();
    }
}

void getInput(int argc, char *argv[]) {
    // 命令行参数
    if (argc == 2) {
        // 使用提示：
        string v = argv[1];
        if (v == "-h") {
            cout << "使用方法：" << endl;
            cout << "参数1： 基准Base.sam" << endl;
            cout << "参数2： 验证query.sam" << endl;
            cout << "参数3： pos校验阈值， 默认为0" << endl;
            cout << "参数4： 是否输出文件 0 不保存结果 1 输出结果到文件, 默认为1" << endl;
            cout << endl;

            exit(0);
        }
    }

    if (argc == 3) {
        gSamFileName1 = argv[1];
        gSamFileName2 = argv[2];
    } else if (argc == 4) {
        gSamFileName1 = argv[1];
        gSamFileName2 = argv[2];
        gThreshold = stoi(argv[3]);
    } else if (argc == 5) {
        gSamFileName1 = argv[1];
        gSamFileName2 = argv[2];
        gThreshold = stoi(argv[3]);
        gIsSaveResult = (argv[4] == "1");
    } else {
        cout << "请输入sam文件1路径, 并按Enter结束 " << endl;
        cin >> gSamFileName1;

        cout << "请输入sam文件2路径, 并按Enter结束 " << endl;
        cin >> gSamFileName2;

        cout << "请输入误差阈值, 并按Enter结束 " << endl;
        cin >> gThreshold;

        cout << "是否输出文件( 0 or 1), 并按Enter结束 " << endl;
        cin >> gIsSaveResult;
    }

    getResultFileName(gSamFileName1, gSamFileName2);

    cout << "开始比较sam文件：" << endl;
    cout << "  " << gSamFileName1 << endl;
    cout << "  " << gSamFileName2 << endl;
    cout << "误差阈值：" << gThreshold << endl;
    cout << "是否保存文件： " << gIsSaveResult << endl;
}

void getResult() {
    uint64 diffLines = 0; // 差异行数
    uint64 sameLines = 0; // 相同行数
    diffLines = gHashMap.size();
    sameLines = allLines - diffLines;

    char samePercent[10];
    sprintf(samePercent, "%.2f", sameLines * 100.0 / allLines);

    char diffPercent[10];
    sprintf(diffPercent, "%.2f", diffLines * 100.0 / allLines);

    string sum = "相同行数： " + to_string(sameLines) + "  " + "百分比：" + samePercent + "%" + '\n';
    sum += "不同行数： " + to_string(diffLines) + "  " + "百分比：  " + diffPercent + "%" + '\n';
    sum += "总共行数： " + to_string(allLines) + '\n' + '\n';

    cout << sum;

    // 统计结果写入文件
    if (gIsSaveResult) {
        string resFileName = "./" + getResultFileName(gSamFileName1, gSamFileName2);
        FILE *fp = fopen(resFileName.c_str(), "w");
        fwrite(sum.c_str(), sizeof(char), sum.size(), fp);
        auto it = gHashMap.begin();
        while (it != gHashMap.end()) {
            auto chr = it->first; // QName
            auto line = get<2>(it->second) + '\n'; // 存在差异的行
            fwrite(line.c_str(), sizeof(char), line.size(), fp);
            it++;
        }
        fclose(fp);
        cout << "处理结束，结果在./" << resFileName << endl;
    }
}

