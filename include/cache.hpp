#pragma once
#include <string>
#include <unordered_map>
#include <list>
#include <mutex>
using namespace std;

class LRUCache {
public:
    LRUCache(size_t capacity);
    string get(const string &key);
    void put(const string &key, const string &value);
    bool exists(const string &key);
    void remove(const string& key);


private:
    size_t capacity;
    list<pair<string, string>> cacheList; 
    unordered_map<string, list<pair<string, string>>::iterator> cacheMap;
    mutex mtx;
    void moveToFront(const string &key);
};
