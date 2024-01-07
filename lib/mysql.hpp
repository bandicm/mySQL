#ifndef _MYSQL_
#define _MYSQL_

#include <deque>
#include <mutex>
#include <thread>
#include <future>

#include "sqlqa.hpp"

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

namespace marcelb {

class mySQL {
    mutex io;
    MySQL_Driver *drv;
    deque<Connection*> con;
    string path, username, password, db;
    uint available;
    uint reconTrys = 3;
    bool runBot = true;
    future<void> bot;
    
    /**
     * Get column names for a table
    */
    void getColumns(const string _table, vector<string> &_columns, Connection *ptr_con); // privatno
    
    /**
     * Open one database
    */
    bool open_one(Connection* con_ptr);

    /**
     * Open one database server connection
    */
    Connection* create_con();

    /**
     * Close one database connection
    */
    bool disconnect_one(Connection* con_ptr);

    /**
     * Take an available database connection
    */
    Connection* shift_con();

    public:

    /**
     * mySQL constructor,
     * receive the path to the mysql server, 
     * username, password, database name,
     * and number of active connections (optional)
    */
    mySQL(const string _path, const string _username, const string _password, const string _db, const uint _available = 1);

    /**
     * Disconnect all connections to server
    */
    bool disconnect();

    /**
     * Define the maximum number of attempts to
     * reconnect to the server 
    */
    void reconnectTrys(const uint _trys);

    /**
     * Execute SQLQA
    */
    void exec(sqlQA &sql_qa);

    /**
     * Destruktor
     * close all connections
    */
    ~mySQL();

};


}

#endif