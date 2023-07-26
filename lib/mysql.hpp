#ifndef _MYSQL_
#define _MYSQL_

#include <string>
#include <vector>
#include <map>
#include <iostream>

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

class mySQL {
    public:

    MySQL_Driver *drv;
    Connection *con;
    string path, username, password, db;
    bool isPersistent;
    
    mySQL(const string _path, const string _username, const string _password, const string _db, bool _isPersistent = false);
    bool open(const string _db = "");
    bool connect();
    bool close();
    map<string, vector<string>> query(const string sql_command);
    bool change(const string sql_command);
    string getTable(const string req);
    ~mySQL();

};


#endif