#include "cache.hpp"

using namespace std;

LRUCache::LRUCache(size_t cap) : capacity(cap) {}

string LRUCache::get(const string &key) {
    lock_guard<mutex> lock(mtx);

    auto it = cacheMap.find(key);
    if (it == cacheMap.end()) return "";

    auto node_it = it->second;
    auto kv = *node_it;

    cacheList.erase(node_it);
    cacheList.push_front(kv);
    cacheMap[key] = cacheList.begin();

    return kv.second;
}

void LRUCache::put(const string &key, const string &value) {
    lock_guard<mutex> lock(mtx);

    auto it = cacheMap.find(key);
    if (it != cacheMap.end()) {
        it->second->second = value;

        auto node_it = it->second;
        auto kv = *node_it;

        cacheList.erase(node_it);
        cacheList.push_front(kv);
        cacheMap[key] = cacheList.begin();
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

void LRUCache::remove(const string& key) {
    lock_guard<mutex> lock(mtx);
    auto it = cacheMap.find(key);
    if (it != cacheMap.end()) {
        cacheList.erase(it->second);
        cacheMap.erase(it);
    }
}
