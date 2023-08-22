#ifndef _MYSQL_
#define _MYSQL_

#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <iostream>
#include <mutex>

#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/driver.h>
#include <cppconn/connection.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>

#define CONNECT_TRY_LIMIT 3

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
    // sqlQA& limit();
    // sqlQA& insertInTo(string _tablename, string _columns);
    // sqlQA& values(); // proizvoljan broj argumenata
    sqlQA& update(const string _tablename);
    sqlQA& set(const string _column_value_pairs);
    // sqlQA& deleteFrom();

    // answer methods


    private:
    void parse_columns(const string _cloumns);
};

class mySQL {
    public:
    mutex io;
    MySQL_Driver *drv;
    Connection *con;
    string path, username, password, db;
    bool isPersistent;
    
    mySQL(const string _path, const string _username, const string _password, const string _db, bool _isPersistent = false);
    bool open(const string _db = "");
    bool connect();
    bool disconnect();
    void exec(sqlQA &sql_qa);
    void getColumns(const string _table, vector<string> &_columns);
    ~mySQL();
};


#endif