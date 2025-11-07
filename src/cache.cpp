#include "cache.hpp"

using namespace std;

LRUCache::LRUCache(size_t cap) : capacity(cap) {}

string LRUCache::get(const string &key) {
    lock_guard<mutex> lock(mtx);
    if (cacheMap.find(key) == cacheMap.end()) return "";

    moveToFront(key);
    return cacheList.front().second;
}

void LRUCache::remove(const string& key) {
    auto it = cacheMap.find(key);
    if (it != cacheMap.end()) {
        cacheList.erase(it->second);  
        cacheMap.erase(it);           
    } 
}


void LRUCache::put(const string &key, const string &value) {
    lock_guard<mutex> lock(mtx);

    if (cacheMap.find(key) != cacheMap.end()) {
        cacheMap[key]->second = value;
        moveToFront(key);
        return;
    }

    if (cacheList.size() >= capacity) {
        auto last = cacheList.back();
        cacheMap.erase(last.first);
        cacheList.pop_back();
    }

    cacheList.push_front({key, value});
    cacheMap[key] = cacheList.begin();
}

bool LRUCache::exists(const string &key) {
    lock_guard<mutex> lock(mtx);
    return cacheMap.find(key) != cacheMap.end();
}

void LRUCache::moveToFront(const string &key) {
    auto it = cacheMap[key];
    auto kv = *it;
    cacheList.erase(it);
    cacheList.push_front(kv);
    cacheMap[key] = cacheList.begin();
}
