#include "myQueue.hpp"
#include <thread>

// thread-buildMap
void buildMap(ifstream &inFile) {
    bool isTestFlag = true;
    bool isNeedTrim = false; // 是否需要去除qname的后缀 （以 '/1'或者'/2'结尾）

    MyQueue mq;

    auto TaskReadLine = [&]() {
        cout << "TaskReadLine start" << endl;

        // 获取对象
        string line1;
        while (getline(inFile, line1)) {
            mq.push((line1));
        }

        string s;
        mq.push(s);
        cout << "TaskReadLine end" << endl;
    };

    auto TaskBuildMap = [&]() {
        cout << "TaskBuildMap start" << endl;

        while (true) {
            if (mq.size()) {
                string line2 = mq.pop();

                if (line2.empty()) {
                    break;
                }

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

                bool isMinus = (stoi(field[1])) & 16;
                if (isMinus) {
                    qName = qName.append("-");
                }

                tuple<string, int, string> value = make_tuple(field[2], stoi(field[3]), line2);
                gHashMap.insert({qName, value});
            }
        }
        cout << "TaskBuildMap end" << endl;
    };

    std::thread threadReadLine(TaskReadLine);
    std::thread threadBuildMap(TaskBuildMap);

    threadReadLine.join();
    threadBuildMap.join();

    cout << "buildMap End!!!" << endl;
}