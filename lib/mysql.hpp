#ifndef _MYSQL_
#define _MYSQL_

#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <iostream>
#include <mutex>
#include <thread>

#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/driver.h>
#include <cppconn/connection.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>

#define unlimited 0
#define reconnectSleep 100000 // in us

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

    // answer methods

    private:
    void parse_columns(const string _cloumns);
};

class mySQL {
    public:
    mutex io;
    MySQL_Driver *drv;
    // Connection *con;
    vector<Connection*> con;
    string path, username, password, db;
    bool isPersistent;
    uint numOfCon;
    uint reconTrys = 3;
    
    mySQL(const string _path, const string _username, const string _password, const string _db, const bool _isPersistent = false, const uint _numOfCon = 1);
    bool open(const string _db = "", const int con_idx = -1);
    bool connect(const int con_idx = -1);
    bool disconnect();
    void reconnectTrys(const uint _trys);
    void exec(sqlQA &sql_qa);
    void getColumns(const string _table, vector<string> &_columns);
    ~mySQL();
};

// class mySQLPool {
//     public:

//     struct Drop {
//         mySQL* instance;
//         bool used = false;
//     };

//     struct Swimmer {
//         thread instance;
//         bool used = false;
//     };

//     mutex io;
//     uint maxpools = 0;
//     vector<struct Drop> droplets;
//     bool fixServer = false;
//     bool fixScheme = false;
//     vector<struct Swimmer> swimmers;

//     mySQLPool(const uint _maxpools);
//     mySQLPool(const uint _maxpools, const string _path, const string _username, const string _password, const string _db);

//     void exec(sqlQA &sql_qa, const string _db = "");

    
// };


#endif