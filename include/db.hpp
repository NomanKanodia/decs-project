#pragma once
#include <string>
#include <libpq-fe.h>
using namespace std;

class Database {
public:
    Database(const string& conninfo);
    ~Database();

    bool connect();
    void close();

    bool set(const string& key, const string& value);
    string get(const string& key);
    bool remove(const string& key);

private:
    PGconn* conn;
    string connectionInfo;
};
