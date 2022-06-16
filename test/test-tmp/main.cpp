#include <stdio.h>
#include <string.h>
#include <string>

using namespace std;

const char *strtok_p(const char *p, const char delim, const char **pos) {
    // 分割结束
    if (*p == '\0') {
        return nullptr;
    }

    const char *cur = p;
    int cnt = 0;
    while (*cur != '\0' && *cur != delim) {
        cur++;
        cnt++;
    }

    // 指向下一个字符串开头
    if (*cur != '\0') {
        cur++;
    }

    *pos = cur;

    long len = cur - p;

    char *res = new char[len];
    memcpy(res, p, len);
    res[len - 1] = '\0';

    return res;
}

int main() {
    setvbuf(stdout, NULL, _IONBF, 0);

    const char *s = "Geeks\nfor\nGeeks";
    const char **pos = &s;

    while (const char *line = strtok_p(s, '\n', pos)) {
        printf("%s\n", line);
        delete[] line;
        s = *pos;
    }

    return 0;
}