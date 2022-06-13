#include "lib.hpp"




int main(int argc, char *argv[]) {
    // 命令行参数
    getInput(argc, argv);

    // 开始计时
    auto start = getStartTime();

    // 读取file1 建HashMap
    ifstream inFile1(gSamFileName1);
    buildMap(inFile1);
    allLines = gHashMap.size();

    // 读取file2 开始对比
    ifstream inFile2(gSamFileName2);
    compare(inFile2, gThreshold);

    // 统计结果，并保存
    getResult();

    // 结束计时
    auto end = getEndTime();
    cout << "程序比较共消耗时间： " << getElapsed(start, end).count() << "s" << endl;

    return 0;
}