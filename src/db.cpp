#include "db.hpp"
#include <iostream>

using namespace std;

Database::Database(const string& conninfo) : conn(nullptr), connectionInfo(conninfo) {}

Database::~Database() {
    close();
}

bool Database::connect() {
    conn = PQconnectdb(connectionInfo.c_str());
    if (PQstatus(conn) != CONNECTION_OK) {
        cerr << "Connection failed: " << PQerrorMessage(conn);
        return false;
    }
    cout << "Connected to PostgreSQL\n";
    return true;
}

void Database::close() {
    if (conn) PQfinish(conn);
    conn = nullptr;
}

bool Database::set(const string& key, const string& value) {
    string query = "INSERT INTO kv_store (key, value) VALUES ('" + key + "', '" + value +
                   "') ON CONFLICT (key) DO UPDATE SET value = EXCLUDED.value;";
    PGresult* res = PQexec(conn, query.c_str());
    bool success = PQresultStatus(res) == PGRES_COMMAND_OK;
    PQclear(res);
    return success;
}

string Database::get(const string& key) {
    string query = "SELECT value FROM kv_store WHERE key='" + key + "';";
    PGresult* res = PQexec(conn, query.c_str());

    if (PQresultStatus(res) != PGRES_TUPLES_OK || PQntuples(res) == 0) {
        PQclear(res);
        return "";
    }

    string value = PQgetvalue(res, 0, 0);
    PQclear(res);
    return value;
}

bool Database::remove(const string& key) {
    string query = "DELETE FROM kv_store WHERE key='" + key + "';";
    PGresult* res = PQexec(conn, query.c_str());

    bool success = false;
    if (PQresultStatus(res) == PGRES_COMMAND_OK) {
        int rows = atoi(PQcmdTuples(res));
        success = (rows > 0);
    }

    PQclear(res);
    return success;
}
