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

// 单线程版本sam文件碱基位点深度统计

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
typedef map<string, vector<int>> WghDepthInfo;
WghDepthInfo wghDepthInfo;

// 字段空格拆分
vector<string> split(string &str) {
    istringstream iss(str);
    return vector<string>(istream_iterator<string>{iss}, istream_iterator<string>());
}

int main(int argc, char *argv[]) {
    // 读取sam文件
    string samFileName;

    if (argc == 2) {
        samFileName = argv[1];
    }

    if (argc != 2) {
        cout << "请输入sam文件路径: " << endl;
        cin >> samFileName;
    }

    cout << "开始统计sam文件位点深度：" << samFileName << endl;
    ifstream inFile(samFileName);

    // 按行解析sam文件
    string line;
    while (getline(inFile, line)) {
        auto field = split(line);

        // 头部@SQ 生成统计直方图空间
        if (field[0] == "@SQ") {
            auto chr = field[1].substr(3);
            auto len = stoi(field[2].substr(3));
            vector<int> p = vector<int>(len);
            wghDepthInfo[chr] = p;

            continue;
        }
        // 头部其他字段，跳过
        if(field[0][0] == '@') {
            continue;
        }

        // 过滤匹配失败
        if (stoi(field[1]) & 4) {
            continue;
        }

        string chr = field[2]; // 染色体名称
        uint32 pos = std::stoi(field[3]);; // 位点
        string cigar = field[5]; // cigar

        // 位点有序统计
        for (auto u = 0, i = 0; i < cigar.size(); i++) {
            while (isdigit(cigar[i])) {
                i++;
            }

            auto t = stoi(cigar.substr(u, i - u)); // 数字区间 [u, i)
            u = i + 1;

            switch (cigar[i]) {
                case 'M': // read匹配到，统计
                    while (t--) {
                        wghDepthInfo[chr][pos++]++;
                    }
                    break;
                case 'I': // read插入字符，表示多统计，Match部分的位置要先计算偏移再进行统计，Inset部分要从插入位置开始统计
                    while (t--) {
                        wghDepthInfo[chr][pos++]++;
                    }
                    break;
                case 'D': // read删除字符，表示跳过
                    pos += t;
                    break;

                default:  // N S H P 跳过
                    break;
            }
        }
    }

    // 统计结果写入文件
    ofstream out("./result.txt");
    auto it = wghDepthInfo.begin();
    while (it != wghDepthInfo.end()) {
        auto chr = it->first;
        auto cur = it->second.begin();

        for (auto i = 0; i < it->second.size(); i++) {
            out << it->first << " " << i << " " << it->second[i] << endl;
            cur++;
        }
        it++;
    }

    cout << "处理结束，结果在 ./result.txt" << endl;

    return 0;
}