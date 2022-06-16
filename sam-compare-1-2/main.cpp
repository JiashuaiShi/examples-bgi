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
#include <sys/types.h>
#include <sys/stat.h>
#include <iostream>
#include <cstdio>
#include <memory.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <cstdlib>
#include <unistd.h>
#include <sys/mman.h>

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

unordered_multimap<string, tuple<string, int, string>> hashMap;

// 配置开关定义
bool isAutoRenameDiffName = false;  // 自动根据比对的两个文件命名结果文件
bool isSaveHashDisMatchFile = false; // 是否保存hash未命中的记录
bool isOpenEnhanceRulers = false; // 是否开启增强规则
bool isSaveResult = true; // 是否保存结果文件

// 统计字段定义
uint64 allLines = 0; // 全部行数
uint64 diffLines = 0; // 差异行数
uint64 sameLines = 0; // 相同行数

// 比较文件
string samFileName1;
string samFileName2;

// 按space/Tab拆分
vector<string> split(const string &str) {
    istringstream iss(str);
    return vector<string>(istream_iterator<string>{iss}, istream_iterator<string>());
}

// 按space拆分
vector<string> split_s(const string &s) {
    vector<string> res;
    for (int j = 0, i = 0; i < s.size();) {
        while (i < s.size() && s[i] == ' ') {
            i++;
        }

        j = i;
        while (i < s.size() && s[i] != ' ') {
            i++;
        }

        res.emplace_back(s.substr(j, i - j));
    }
    return res;
}

// 按Tab拆分
vector<string> split_t(const string &s, const string &delimiters = "\t") {
    vector<string> tokens;
    string::size_type lastPos = s.find_first_not_of(delimiters, 0);
    string::size_type pos = s.find_first_of(delimiters, lastPos);
    while (string::npos != pos || string::npos != lastPos) {
        tokens.push_back(s.substr(lastPos, pos - lastPos)); // use emplace_back after C++11
        lastPos = s.find_first_not_of(delimiters, pos);
        pos = s.find_first_of(delimiters, lastPos);
    }
    return tokens;
}

// Qname是有后缀
bool isQnameHasSuffix(const string &qName) {
    int len = qName.size();
    if (len < 3)
        return false;

    return qName.substr(len - 2) == "/1" || qName.substr(len - 2) == "/2";
}

// 去掉非标准qname的后缀
string trimQname(string s) {
    return s.substr(0, s.size() - 2);
}

string trimLine(string qName, const string &line) {
    return qName.append(line.substr(qName.size() + 2));
}

// 程序时间统计
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

// task-buildMap
void buildMap(const string &filePath) {
    cout << "buildMap start..." << endl;

    bool isTestFlag = true;
    bool isNeedTrim = false; // 是否需要去除qname的后缀 （以 '/1'或者'/2'结尾）

    // 第一步：建立映射
    int fd;
    void *start;
    struct stat sb{};
    fd = open(filePath.c_str(), O_RDONLY);
    fstat(fd, &sb);  /* 取得文件大小 */
    start = mmap(nullptr, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (start == MAP_FAILED) {      /* 判断是否映射成功 */
        cout << "mmap failed!!" << endl;
        exit(-1);
    }

    // 第二步：拷贝内存
    char *buffer = new char[sb.st_size];
    char *mem = buffer;
    memcpy(buffer, start, sb.st_size);

    // 第三步: 解除映射
    munmap(start, sb.st_size);
    close(fd);

    // 按行分割
    while (char *line = strtok_r(buffer, "\n", &buffer)) {
        auto field = split_t(line);

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
        string line_s;
        if (isNeedTrim) {
            qName = trimQname(qName);
            line_s = trimLine(qName, string(line));
        }

        bool isMinus = (stoi(field[1])) & 16;
        if (isMinus) {
            qName = qName.append("-");
        }

        tuple<string, int, string> value = make_tuple(field[2], stoi(field[3]), line_s);
        hashMap.insert({qName, value});
    }

    // 释放内存
    delete[] mem;

    cout << "buildMap end..." << endl;
}

// task-compare
void compare(const string &filePath, int threshold) {
    cout << "compare start..." << endl;

    string line2;
    bool isTestFlag = true;
    bool isNeedTrim = false; // 是否需要去除qname的后缀 （以 '/1'或者'/2'结尾）
    ofstream hashFile;

    // hash没有命中的文件保存
    if (isSaveHashDisMatchFile) {
        hashFile = ofstream("./diffMisHashHit.txt");
    }

    // 第一步：建立映射
    int fd;
    void *start;
    struct stat sb{};
    fd = open(filePath.c_str(), O_RDONLY);
    fstat(fd, &sb);  /* 取得文件大小 */
    start = mmap(nullptr, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (start == MAP_FAILED) {      /* 判断是否映射成功 */
        cout << "mmap failed!!" << endl;
        exit(-1);
    }

    // 第二步：拷贝内存
    char *buffer = new char[sb.st_size];
    char *mem = buffer;
    memcpy(buffer, start, sb.st_size);

    // 第三步: 解除映射
    munmap(start, sb.st_size);
    close(fd);

    // 按行分割
    while ((char * line = strtok_r(buffer, "\n", &buffer))) {
        auto field = split_t(line);

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
        string line_s;
        if (isNeedTrim) {
            qName = trimQname(qName);
            line_s = trimLine(qName, line);
        }

        // 通过flag判断正负链会有异常情况，比如两条记录都是16或者两条记录都是0
        bool isMinus = (stoi(field[1])) & 16;
        if (isMinus) {
            qName = qName.append("-");
        }

        auto it = hashMap.find(qName);
        tuple<string, uint64, string> value = make_tuple(field[2], stoi(field[3]), line_s);

        // 在此处可以根据flag，增加比对规则
        if (isOpenEnhanceRulers) {
            int flag = stoi(field[1]);

            bool unMatch1 = flag & 0x40;  // read1匹配上
            bool unMatch2 = flag & 0x80;  // read2匹配上

            // 增加规则1： 没有匹配上的记录，直接判断不匹配
            if (unMatch1 || unMatch2) {
                hashMap.insert({qName, value});
                continue;
            }
        }

        // key不命中，插入HashMap
        if (it == hashMap.end()) {
            // 单独保存Hash未命中记录
            if (isSaveHashDisMatchFile) {
                hashFile << line_s << endl;
            }
            continue;
        }

        // 规则： 如相同，则移除
        if (get<0>(it->second) == field[2]
            && isSame(get<1>(it->second), stoi(field[3]), threshold)) {
            hashMap.erase(it);
        } else {
            // 如不同，则填入
            hashMap.insert({qName, value});
        }

        // hash没有命中的文件保存
        if (isSaveHashDisMatchFile) {
            hashFile.close();
        }
    }

    delete[] mem;
    cout << "compare end..." << endl;
}

void saveResult() {
    cout << "saveResult start..." << endl;

    // 比对结果统计
    diffLines = hashMap.size();
    sameLines = allLines - diffLines;

    char samePercent[10];
    sprintf(samePercent, "%.2f", sameLines * 100.0 / allLines);

    char diffPercent[10];
    sprintf(diffPercent, "%.2f", diffLines * 100.0 / allLines);

    string sum = string("\n") + "相同行数： " + to_string(sameLines) + "  " + "百分比：" + samePercent + "%" + '\n';
    sum += "不同行数： " + to_string(diffLines) + "  " + "百分比：  " + diffPercent + "%" + '\n';
    sum += "总共行数： " + to_string(allLines) + '\n' + '\n';

    cout << sum;

    // 统计结果写入文件
    if (isSaveResult) {
        string resFileName = "./" + getResultFileName(samFileName1, samFileName2);
        FILE *fp = fopen(resFileName.c_str(), "w");
        fwrite(sum.c_str(), sizeof(char), sum.size(), fp);
        auto it = hashMap.begin();
        while (it != hashMap.end()) {
            auto chr = it->first; // QName
            auto line = get<2>(it->second) + '\n'; // 存在差异的行
            fwrite(line.c_str(), sizeof(char), line.size(), fp);
            it++;
        }
        fclose(fp);
        cout << "处理结束，结果在: " << resFileName << endl;
    }

    cout << "saveResult end..." << endl;
}

int main(int argc, char *argv[]) {
    // 读取sam文件
    int threshold = 0;  // 位置比较阈值

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
            return 0;
        }
    }

    if (argc == 3) {
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

    cout << "开始比较sam文件：" << endl;
    cout << "  " << samFileName1 << endl;
    cout << "  " << samFileName2 << endl;
    cout << "误差阈值：" << threshold << endl;
    cout << "是否保存文件： " << isSaveResult << endl << endl;

    getResultFileName(samFileName1, samFileName2);

    // 开始计时
    auto start = getStartTime();

    // 根据sam文件1建立hashMap
    buildMap(samFileName1);
    allLines = hashMap.size();

    // 读取sam文件2开始比对
    compare(samFileName2, threshold);

    // 保存结果
    saveResult();

    // 结束计时
    auto end = getEndTime();
    cout << "程序比较共消耗时间： " << getElapsed(start, end).count() << "s" << endl;

    return 0;
}