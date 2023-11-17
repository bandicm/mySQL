#ifndef _MYSQL_
#define _MYSQL_

#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <iostream>
#include <mutex>
#include <thread>
#include <future>

#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/driver.h>
#include <cppconn/connection.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>

#define unlimited 0
#define reconnectSleep 10000 // in us

using namespace std;
using namespace sql;
using namespace mysql;

class sqlQA {
    public:
    // query
    string cmd;
    string table;
    vector<string> columns;
    bool isUpdate = false;
    bool isSelect = false;

    // answer
    uint updateCatch = 0;
    bool executed = false;
    map<string, vector<string>> result;
    uint num_rows = 0;
    uint num_columns = 0;

    // query methods
    sqlQA& select(const string _select = "*");
    sqlQA& from(const string _tablename);
    sqlQA& where(const string _condition);
    sqlQA& limit(const uint _limit);
    sqlQA& insertInTo(const string _tablename, const string _columns = "");
    sqlQA& values(const string _values);
    sqlQA& update(const string _tablename);
    sqlQA& set(const string _column_value_pairs);
    sqlQA& deleteFrom(const string _table);

    void print(bool withDetail = false);

    // answer methods

    private:
    void parse_columns(const string _cloumns);
};

class mySQL {
    private:
    mutex io;
    MySQL_Driver *drv;
    vector<pair<mutex*, Connection*>> con;
    string path, username, password, db;
    bool isPersistent;
    uint numOfCon;
    uint reconTrys = 3;
    bool runBot = true;
    future<void> bot;
    
    void getColumns(const string _table, vector<string> &_columns, Connection *ptr_con); // privatno
    bool open_one(const uint idx);
    bool connect_one(const uint idx);
    bool disconnect_one(const uint idx);
    uint findFreeCon();

    public:
    mySQL(const string _path, const string _username, const string _password, const string _db, const bool _isPersistent = false, const uint _numOfCon = 1);
    bool open(const string _db = "");
    bool connect();
    bool disconnect();
    void reconnectTrys(const uint _trys);
    void exec(sqlQA &sql_qa);
    ~mySQL();

};


#endif