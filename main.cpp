#define _WIN32_WINNT 0x0A00
#include "include/httplib.h"
#include "include/db.hpp"
#include "include/cache.hpp"
#include <iostream>

using namespace std;
using namespace httplib;

int main() {
    Database db("host=localhost port=5432 dbname=mydb user=postgres password=nk123456");
    if (!db.connect()) {
        cerr << "Failed to connect to database.\n";
        return 1;
    }

    LRUCache cache(5);

    Server svr;

    svr.Put(R"(/set/(.+))", [&](const Request &req, Response &res) {
        string key = req.matches[1];
        string value = req.body;

        if (db.set(key, value)) {
            cache.put(key, value);
            res.set_content("Stored successfully\n", "text/plain");
        } else {
            res.status = 500;
            res.set_content("Failed to store value\n", "text/plain");
        }
    });

    svr.Get(R"(/get/(.+))", [&](const Request &req, Response &res) {
        string key = req.matches[1];

        if (cache.exists(key)) {
            res.set_content("Cache hit: " + cache.get(key) + "\n", "text/plain");
            return;
        }

        string value = db.get(key);
        if (value.empty()) {
            res.status = 404;
            res.set_content("❌ Key not found\n", "text/plain");
        } else {
            cache.put(key, value);
            res.set_content("DB hit: " + value + "\n", "text/plain");
        }
    });

    svr.Delete(R"(/delete/(.+))", [&](const Request &req, Response &res) {
        string key = req.matches[1];

        if (db.remove(key)) {
            cache.remove(key);
            res.set_content("Deleted successfully\n", "text/plain");
        } else {
            res.status = 404;
            res.set_content("Key not found\n", "text/plain");
        }
    });

    cout << "Server running at http://localhost:8080\n";
    svr.listen("localhost", 8080);

    db.close();
}
