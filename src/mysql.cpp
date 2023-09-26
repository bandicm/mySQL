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

void sqlQA::print(bool withDetail) {
   cout << "============================================" << endl;

   for (auto i : result) {
      for (auto j: i.second) {
         cout << i.first << " : " << j << endl;
      }
      cout << "--------------------------------------------" << endl;
   }

   if (withDetail) {
      cout << "-----------------DETAILS--------------------" << endl;
      cout << "Is executed: " << (executed ? "true" : "false") << endl;
      cout << "Update catch: " << updateCatch << endl;
      cout << "Num of rows: " << num_rows << endl;
      cout << "Num of columns: " << num_columns << endl;
   }
   cout << "============================================" << endl;

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
   numOfCon = _numOfCon > 0 ? _numOfCon : 1;

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

bool mySQL::connect() {
   io.lock();
   bool status = true;

   for (uint i=0; i<numOfCon; i++) {
      status = connect_one(i);
   }

   io.unlock();
   return status;
}

bool mySQL::connect_one(const uint idx) {
   uint trys = 0;
   bool status = true;

   while (reconTrys == unlimited ? status : (trys <= reconTrys && status)) {
      try {
         if (con.size() < numOfCon) {
            con.push_back(make_pair(new mutex, drv->connect(path, username, password)));
         }
         else {
            con[idx].second = drv->connect(path, username, password);
         }
         status = !con[idx].second->isValid() && con[idx].second->isClosed() || con[idx].second->isClosed();
      }
      catch (const SQLException &error) {
         cout << error.what() << endl;
         usleep(reconnectSleep);
         reconTrys == unlimited ? trys : trys++;
      }         
   }

   return status;
}

bool mySQL::disconnect() {
   io.lock();
   bool status = true;

   for (uint i=0; i<con.size(); i++) {
      status = disconnect_one(i) ;
   }

   io.unlock();
   return status;
}

bool mySQL::disconnect_one(const uint idx) {
   bool status = !con[idx].second->isClosed();

   if (status) {
      try {
         con[idx].second->close();
         status = !con[idx].second->isClosed();
      } 
      catch (const SQLException &error) {
         cout << error.what() << endl;
         status = true;
      }
   }

   else {
      status = false; // već je zatvorena
   }

   return status;
}


bool mySQL::open(const string _db) {
   io.lock();
   db = _db.empty() ? db : _db;
   bool status = true; // ako true greška je

   for (uint i=0; i<con.size(); i++) {
      status = open_one(i);
   }

   io.unlock();
   return status;
}

bool mySQL::open_one(const uint idx) {
   bool status = true; // ako true greška je
   uint trys = 0;

   while (reconTrys == unlimited ? status : (trys <= reconTrys && status)) {
      try {
         if (con[idx].second->isValid()) {
            con[idx].second->setSchema(db);
            status = false;
         }
         else {
            break;
         }
      }
      catch (const SQLException &error) {
         cout << error.what() << endl;
         usleep(reconnectSleep);
         reconTrys == unlimited ? trys : trys++;
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
   const uint idx = findFreeCon(); 

   if (!isPersistent || !con[idx].second->isValid() || con[idx].second->isClosed()) {
      if (connect_one(idx)) {
         throw string("[ERROR] Unable to connect database ");
      }

      if (open_one(idx)) {
         throw string("[ERROR] Unable to open database " + db);
      }
   }

   try {
      vector<string> columns = sql_qa.columns;
      
      if (columns.empty() && !sql_qa.table.empty()) {
         getColumns(sql_qa.table, columns, con[idx].second);
      }

      Statement *stmt;
      stmt = con[idx].second->createStatement();

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

         res->close();
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

      stmt->close();
      delete stmt;
   }
   catch (const SQLException &error) {
      cout << error.what() << endl;
      sql_qa.executed = false;
   }
   catch (const string error) {
      con[idx].first->unlock();
      throw error;
   }

   if (!isPersistent) {
      if(disconnect_one(idx)) {
         throw string("[ERROR] Unable to close database ");
      }
   }

   con[idx].first->unlock();
}

void mySQL::getColumns(const string _table, vector<string> &_columns, Connection *ptr_con) {
   Statement *stmt;
   stmt = ptr_con->createStatement();
   
   ResultSet *columnsRes = stmt->executeQuery("SHOW COLUMNS from " + _table);

   while (columnsRes->next()) {
      _columns.push_back(columnsRes->getString("Field"));
   }

   columnsRes->close();
   stmt->close();
   delete columnsRes;
   delete stmt;
}

uint mySQL::findFreeCon() {
   lock_guard<mutex> master(io);
   while (true) {
      for (uint i=0; i<con.size(); i++) {
         if (con[i].first->try_lock()) {
            return i;
         }
      }
   }
}

mySQL::~mySQL() {
   if(disconnect()) {
      throw string("[ERROR] Unable to close database ");
   }
}