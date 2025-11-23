#include "db.hpp"
#include <iostream>
#include <cstdlib>

using namespace std;

Database::Database(const string& conninfo) 
    : conn(nullptr), connectionInfo(conninfo) {}

Database::~Database() {
    close();
}

bool Database::connect() {
    conn = PQconnectdb(connectionInfo.c_str());

    if (!conn || PQstatus(conn) != CONNECTION_OK) {
        cerr << "[DB] Connection failed: " 
                  << (conn ? PQerrorMessage(conn) : "null") << "\n";
        return false;
    }

    cout << "Connected to PostgreSQL\n";
    return true;
}

void Database::close() {
    if (conn) {
        PQfinish(conn);
        conn = nullptr;
    }
}

bool Database::set(const string& key, const string& value) {
    if (!conn) return false;

    const char* params[2] = {key.c_str(), value.c_str()};

    PGresult* res = PQexecParams(
        conn,
        "INSERT INTO kv_store (\"key\", value) VALUES ($1, $2) "
        "ON CONFLICT (\"key\") DO UPDATE SET value = EXCLUDED.value;",
        2, nullptr, params, nullptr, nullptr, 0);

    if (!res) return false;

    bool ok = PQresultStatus(res) == PGRES_COMMAND_OK;
    PQclear(res);
    return ok;
}

string Database::get(const string& key) {
    if (!conn) return "";

    const char* params[1] = {key.c_str()};

    PGresult* res = PQexecParams(
        conn,
        "SELECT value FROM kv_store WHERE \"key\" = $1;",
        1, nullptr, params, nullptr, nullptr, 0);

    if (!res) return "";

    if (PQresultStatus(res) != PGRES_TUPLES_OK || PQntuples(res) == 0) {
        PQclear(res);
        return "";
    }

    string val = PQgetvalue(res, 0, 0);
    PQclear(res);
    return val;
}

bool Database::remove(const string& key) {
    if (!conn) return false;

    const char* params[1] = {key.c_str()};

    PGresult* res = PQexecParams(
        conn,
        "DELETE FROM kv_store WHERE \"key\" = $1;",
        1, nullptr, params, nullptr, nullptr, 0);

    if (!res) return false;

    bool ok = atoi(PQcmdTuples(res)) > 0;
    PQclear(res);
    return ok;
}
