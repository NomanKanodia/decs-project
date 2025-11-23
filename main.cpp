#define _WIN32_WINNT 0x0A00
#include "include/httplib.h"
#include "include/db_connection_pool.hpp"
#include "include/cache.hpp"
#include <iostream>
#include <memory>

using namespace std;
using namespace httplib;

int main() {

    DatabaseConnectionPool db_pool("host=localhost port=5432 dbname=mydb user=postgres password=nk123456", 8);
    
    LRUCache cache(1000);
    Server svr;

    
    svr.new_task_queue = [] { 
        return new ThreadPool(8); 
    };

    svr.Put(R"(/set/(.+))", [&](const Request &req, Response &res) {
        string key = req.matches[1];
        string value = req.body;

        auto db = db_pool.get_connection();
        if (!db) {
            res.status = 503;
            res.set_content("Error in connection pool\n", "text/plain");
            return;
        }

        if (db->set(key, value)) {
            cache.put(key, value);
            res.set_content("Stored successfully\n", "text/plain");
        } else {
            res.status = 500;
            res.set_content("Failed to store value\n", "text/plain");
        }
        db_pool.return_connection(db);
    });

    svr.Get(R"(/get/(.+))", [&](const Request &req, Response &res) {
        string key = req.matches[1];

        if (cache.exists(key)) {
            res.set_content("Cache hit: " + cache.get(key) + "\n", "text/plain");
            return;
        }

        auto db = db_pool.get_connection();
        if (!db) {
            res.status = 503;
            res.set_content("Error\n", "text/plain");
            return;
        }

        string value = db->get(key);
        db_pool.return_connection(db);

        if (value.empty()) {
            res.status = 404;
            res.set_content("Key not found\n", "text/plain");
        } else {
            cache.put(key, value);
            res.set_content("DB hit: " + value + "\n", "text/plain");
        }
    });

    svr.Delete(R"(/delete/(.+))", [&](const Request &req, Response &res) {
        string key = req.matches[1];

        auto db = db_pool.get_connection();
        if (!db) {
            res.status = 503;
            res.set_content("Service unavailable - database pool exhausted\n", "text/plain");
            return;
        }

        if (db->remove(key)) {
            cache.remove(key);
            res.set_content("Deleted successfully\n", "text/plain");
        } else {
            res.status = 404;
            res.set_content("Key not found\n", "text/plain");
        }
        db_pool.return_connection(db);
    });


    svr.Get("/status", [&](const Request &req, Response &res) {
        res.set_content("Available connections: " + 
                       to_string(db_pool.available_connections()) + "\n", "text/plain");
    });

    cout << "Server running at http://localhost:8080 \n";
    svr.listen("localhost", 8080);

    return 0;
}