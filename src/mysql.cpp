#include "../lib/mysql.hpp"

sqlQA& sqlQA::select(const string _columns) {
   if (_columns != "*") {
      parse_columns(_columns);
   }
   isSelect = true;
   cmd += "SELECT " + _columns + " ";
   return *this;
}

sqlQA& sqlQA::from(const string _table) {
   table = _table;
   cmd += "FROM " + _table + " ";
   return *this;
}

sqlQA& sqlQA::where(const string _condition) {
   cmd += "WHERE " + _condition + " ";
   return *this;
}

sqlQA& sqlQA::limit(const uint _limit) {
   cmd += "LIMIT " + to_string(_limit) + " ";
   return *this;
}

sqlQA& sqlQA::insertInTo(const string _tablename, const string _columns) {
   isUpdate = true;
   cmd += "INSERT INTO " + _tablename;
   if (_columns.empty()) {
      cmd += " ";
   }
   else {
      cmd += " (" + _columns + ") ";
   }
   return *this;
}

sqlQA& sqlQA::values(const string _values) {
   cmd += "VALUES (" + _values + ") ";
   return *this;
}

sqlQA& sqlQA::update(const string _table) {
   isUpdate = true;
   cmd += "UPDATE " + _table + " ";
   return *this;
}

sqlQA& sqlQA::set(const string _column_value_pairs) {
   cmd += "SET " + _column_value_pairs + " ";
   return *this;
}

sqlQA& sqlQA::deleteFrom(const string _table) {
   isUpdate = true;
   cmd += "DELETE FROM " + _table + " ";
   return *this;
}


void sqlQA::parse_columns(const string _columns) {
   istringstream iss(_columns);
   string columnName;

   while (getline(iss, columnName, ',')) {
      size_t startPos = columnName.find_first_not_of(" ");
      size_t endPos = columnName.find_last_not_of(" ");
      
      if (startPos != string::npos && endPos != string::npos) {
         columns.push_back(columnName.substr(startPos, endPos - startPos + 1));
      }
   }
}

mySQL::mySQL(const string _path, const string _username, const string _password, const string _db, const bool _isPersistent, const uint _numOfCon) {
   path = _path;
   username = _username;
   password = _password;
   db = _db;
   isPersistent = _numOfCon > 1 ? true : _isPersistent;
   numOfCon = _numOfCon;

   drv = get_mysql_driver_instance();

   if (isPersistent) {
      if (connect()) {
         throw string("[ERROR] Unable to connect database ");
      }

      if (!db.empty()) {
         if (open()) {
            throw string("[ERROR] Unable to open database " + db);
         }
      }
   }

}

bool mySQL::open(const string _db, const int con_idx) {
   io.lock();
   db = _db.empty() ? db : _db;
   bool status = true; // ako true greška je

   for (uint i=0; i<con.size(); i++) {
      con_idx != -1 ? i = con_idx : i;
      try {
         con[i]->setSchema(db);
         status = false;
         io.unlock();
      }
      catch (const SQLException &error) {
         cout << error.what() << endl;
         io.unlock();
      }
      con_idx != -1 ? i = con.size() : i;
   }
   
   return status;
}

bool mySQL::connect(const int con_idx) {
   io.lock();
   uint trys = 0;
   bool status = true;

   for (uint i=0; i<numOfCon; i++) {
      con_idx != -1 ? i = con_idx : i;
      while (reconTrys == 0 ? status : (trys < reconTrys && status)) {
         try {
            if (con_idx == -1) {
               con.push_back(drv->connect(path, username, password));
            }
            else {
               con[i] = drv->connect(path, username, password);
            }
            status = false;
            io.unlock();
         }
         catch (const SQLException &error) {
            cout << error.what() << endl;
            usleep(reconnectSleep);
            reconTrys == 0 ? trys : trys++;
            io.unlock();
         }
      }
   }

   return status;
}

bool mySQL::disconnect() {
   io.lock();
   bool status = true;

   for (uint i=0; i<con.size(); i++) {
      if (con[i]->isValid() && !con[i]->isClosed()) {
         try {
            con[i]->close();
            status = false;
            io.unlock();
         } 
         catch (const SQLException &error) {
            cout << error.what() << endl;
            status = true;
            io.unlock();
         }
      }

      else {
         status = false; // već je zatvorena
      }
   }

   return status;
}

/**
 * Broj pokušaja usljed povezivanja s bazom od 1 do unlimited;
*/

void mySQL::reconnectTrys(const uint _trys) {
   io.lock();
   reconTrys = _trys;
   io.unlock();
}

void mySQL::exec(sqlQA &sql_qa) {

   for (uint i=0; i<con.size(); i++) {
      if (!isPersistent || !con[i]->isValid() || con[i]->isClosed()) {
         if (connect(i)) {
            throw string("[ERROR] Unable to connect database ");
         }

         if (open(i)) {
            throw string("[ERROR] Unable to open database " + db);
         }
      }
   }

   // find free connection

   io.lock();
   /**/
   try {
      vector<string> columns = sql_qa.columns;
      
      if (columns.empty() && !sql_qa.table.empty()) {
         getColumns(sql_qa.table, columns);
      }

      Statement *stmt;
      stmt = con->createStatement();

      if (sql_qa.isSelect) {
         ResultSet *res = stmt->executeQuery(sql_qa.cmd);
         sql_qa.executed = true;
         uint num_raw_columns = 0;
         while (res->next()) {
            for (uint i=0; i<columns.size(); i++) {
               sql_qa.result[columns[i]].push_back(res->getString(columns[i]));
               num_raw_columns++;
            }
         }

         delete res;
         sql_qa.num_columns = columns.size();
         sql_qa.num_rows = num_raw_columns/columns.size();
      }

      if (sql_qa.isUpdate) {
         sql_qa.updateCatch = stmt->executeUpdate(sql_qa.cmd);
         sql_qa.executed = true;
      }

      else {
         sql_qa.executed = stmt->execute(sql_qa.cmd);
      }

      delete stmt;
      io.unlock();
   }
   catch (const SQLException &error) {
      cout << error.what() << endl;
      sql_qa.executed = false;
      io.unlock();
   }
   catch (const string error) {
      throw error;
      io.unlock();
   }

   /**/

   if (!isPersistent) {
      if(disconnect()) {
         throw string("[ERROR] Unable to close database ");
      }
   }

}

void mySQL::getColumns(const string _table, vector<string> &_columns) {
   Statement *stmt;
   stmt = con->createStatement();
   
   ResultSet *columnsRes = stmt->executeQuery("SHOW COLUMNS from " + _table);

   while (columnsRes->next()) {
      _columns.push_back(columnsRes->getString("Field"));
   }

   delete columnsRes;
   delete stmt;
}

mySQL::~mySQL() {
   if(disconnect()) {
      throw string("[ERROR] Unable to close database ");
   }
}

// mySQLPool::mySQLPool(const uint _maxpools) {
//    maxpools = _maxpools;
//    fixServer = false;
//    fixScheme = true;
// }

// mySQLPool::mySQLPool(const uint _maxpools, const string _path, const string _username, const string _password, const string _db) {
//    maxpools = _maxpools;
//    fixServer = true;
//    fixScheme = !_db.empty();
//    for (uint i=0; i<maxpools; i++) {
//       mySQL tmpmysql(_path, _username, _password, _db);
//       // struct Drop tmpdrop(new mySQL(_path, _username, _password, _db), false);
//       droplets.push_back({mySQL(_path, _username, _password, _db, true), false});
//    }
// }

// void mySQLPool::exec(sqlQA &sql_qa, const string _db) {
//    if (!fixScheme && _db.empty()) {
//       throw string("[ERROR] Database is not selected! ");
//    }

//    for (uint i=0; i<droplets.size(); i++) {
//       if (droplets[i].used == false) {
//          droplets[i].used = true;

//          bool isRunned = false;

//          while (!isRunned) {
//             for (uint i=0; i<swimmers.size(); i++) {
//                if (swimmers[i].used == false) {
//                   swimmers[i].used = true;
//                   swimmers[i].instance = thread([&]() {
//                      droplets[i].instance->exec(sql_qa);
//                   });
//                   swimmers[i].instance.join();
//                   swimmers[i].used = false;
//                   isRunned = true;
//                }
//             }
//          }
//          droplets[i].used = false;
//       }
//    }

// }
