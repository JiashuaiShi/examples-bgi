#include <queue>
#include <shared_mutex>
#include <condition_variable>
#include <shared_mutex>
#include <unordered_map>
#include <iterator>
#include <tuple>

using namespace std;

class MyHashMap {
private:
    mutable std::shared_mutex mut;
    condition_variable cond;
    unordered_multimap<string, tuple<string, int, string>> hashMap;
public:
    MyHashMap() = default;

    MyHashMap(const MyHashMap &) = delete;

    MyHashMap &operator=(const MyHashMap &) = delete;

    void insert(pair<string, tuple<string, int, string>> &e) {
        std::lock_guard<std::shared_mutex> lk(mut);
        hashMap.insert(e);
        cond.notify_one();
    }

    unordered_multimap<string, tuple<string, int, string>>::iterator
    erase(unordered_multimap<string, tuple<string, int, string>>::iterator it) {
        lock_guard<shared_mutex> lk(mut);
        return hashMap.erase(it);;
    }

    unordered_multimap<string, tuple<string, int, string>>::iterator
    find(const string &key) {
        lock_guard<shared_mutex> lk(mut);
        return hashMap.find(key);;
    }

    unordered_multimap<string, tuple<string, int, string>>::iterator
    end() {
        lock_guard<shared_mutex> lk(mut);
        return hashMap.end();
    }

    unordered_multimap<string, tuple<string, int, string>>::iterator
    begin() {
        lock_guard<shared_mutex> lk(mut);
        return hashMap.begin();
    }

    unsigned long size() {
        lock_guard<shared_mutex> lk(mut);
        return hashMap.size();
    }
};

