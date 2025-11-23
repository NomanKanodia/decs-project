#pragma once
#include <memory>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include "db.hpp"

using namespace std;

class DatabaseConnectionPool {
private:
    string connectionInfo;
    vector<shared_ptr<Database>> connections;
    mutex pool_mutex;
    condition_variable condition;
    size_t max_pool_size;

public:
    DatabaseConnectionPool(const string& conninfo, size_t pool_size = 10)
        : connectionInfo(conninfo), max_pool_size(pool_size) {
        
        for (size_t i = 0; i < pool_size; ++i) {
            auto db = make_shared<Database>(connectionInfo);
            if (db->connect()) {
                connections.push_back(db);
            }
        }
        cout << "Connection pool ready with " << connections.size() << " connections\n";
    }

    shared_ptr<Database> get_connection() {
        unique_lock<mutex> lock(pool_mutex);
        
        if (condition.wait_for(lock, chrono::seconds(1), [this]() {
            return !connections.empty();
        })) {
            auto db = connections.back();
            connections.pop_back();
            return db;
        }
        
        return nullptr;
    }

    void return_connection(shared_ptr<Database> db) {
        if (db) {
            unique_lock<mutex> lock(pool_mutex);
            connections.push_back(db);
            condition.notify_one();
        }
    }

    size_t available_connections() {
        lock_guard<mutex> lock(pool_mutex);
        return connections.size();
    }
};