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