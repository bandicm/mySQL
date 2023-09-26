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
   // istražit da se prikaže u tabeli
   for (auto i : result) {
         for (auto j: i.second) {
            cout << i.first << " : " << j << endl;
         }
   }

   if (withDetail) {
      cout << "Is executed: " << (executed ? "true" : "false") << endl;
      cout << "Update catch: " << updateCatch << endl;
      cout << "Num of rows: " << num_rows << endl;
      cout << "Num of columns: " << num_columns << endl;
   }
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

   cout << "Num of con: " << numOfCon << endl;

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

bool mySQL::open(const string _db) {
   io.lock();
   db = _db.empty() ? db : _db;
   bool status = true; // ako true greška je

   for (uint i=0; i<con.size(); i++) {
      try {
         con[i].second->setSchema(db);
         status = false;
      }
      catch (const SQLException &error) {
         cout << error.what() << endl;
      }

   }

   io.unlock();
   return status;
}

bool mySQL::connect() {
   io.lock();
   // uint trys = 0;
   // bool status = true;
   bool Gstatus = true;

   for (uint i=0; i<numOfCon; i++) {
      uint trys = 0;
      bool status = true;
      cout << "Init connection " << i << endl;
      while (reconTrys == unlimited ? status : (trys < reconTrys && status)) {
         cout << "Try connect " << trys << endl;
         try {
            con.push_back(make_pair(new mutex, drv->connect(path, username, password)));
            // con[i].first = new mutex;
            // con[i].second = drv->connect(path, username, password);
            status = false;
            Gstatus *= status;
         }
         catch (const SQLException &error) {
            cout << error.what() << endl;
            usleep(reconnectSleep);
            reconTrys == unlimited ? trys : trys++;
         }         
      }
   }

   cout << "Num of pairs " << con.size() << endl;

   io.unlock();
   // return status;
   return Gstatus;
}

bool mySQL::disconnect() {
   io.lock();
   bool status = true;

   for (uint i=0; i<con.size(); i++) {
      if (con[i].second->isValid() && !con[i].second->isClosed()) {
         try {
            con[i].second->close();
            status = false;
         } 
         catch (const SQLException &error) {
            cout << error.what() << endl;
            status = true;
         }
      }

      else {
         status = false; // već je zatvorena
      }
   }

   io.unlock();
   return status;
}

bool mySQL::disconnect_one(Connection *ptr_con) {
   bool status = true;

   if (ptr_con->isValid() && !ptr_con->isClosed()) {
      try {
         ptr_con->close();
         status = false;
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

bool mySQL::connect_one(Connection *ptr_con) {
   uint trys = 0;
   bool status = true;

   while (reconTrys == unlimited ? status : (trys < reconTrys && status)) {
      try {
         ptr_con = drv->connect(path, username, password);
         status = false;
      }
      catch (const SQLException &error) {
         cout << error.what() << endl;
         usleep(reconnectSleep);
         reconTrys == unlimited ? trys : trys++;
      }         
   }

   return status;
}

bool mySQL::open_one(Connection *ptr_con) {
   bool status = true; // ako true greška je

   try {
      ptr_con->setSchema(db);
      status = false;
   }
   catch (const SQLException &error) {
      cout << error.what() << endl;
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

   pair<mutex*, Connection*> workCon = findFreeCon(); 

   if (!isPersistent || !workCon.second->isValid() || workCon.second->isClosed()) {
      if (connect_one(workCon.second)) {
         throw string("[ERROR] Unable to connect database ");
      }

      if (open_one(workCon.second)) {
         throw string("[ERROR] Unable to open database " + db);
      }
   }

   /**/
   try {
      vector<string> columns = sql_qa.columns;
      
      if (columns.empty() && !sql_qa.table.empty()) {
         getColumns(sql_qa.table, columns, workCon.second);
      }

      Statement *stmt;
      stmt = workCon.second->createStatement();

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
      workCon.first->unlock();
      throw error;
   }

   /**/

   if (!isPersistent) {
      if(disconnect_one(workCon.second)) {
         throw string("[ERROR] Unable to close database ");
      }
   }

   workCon.first->unlock();

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

pair<mutex*, Connection*> mySQL::findFreeCon() {
   io.lock();
   while (true) {
      cout << "Tražim konekciju " << endl;
      for (uint i=0; i<con.size(); i++) {
         cout << "Pokušavam s " << i << " konekciju" << endl;
         if (con[i].first->try_lock()) {
            cout << "Koristim " << i << " konekciju" << endl;
            io.unlock();
            return con[i];
         }
         // else {
         //    usleep(1000);
         // }
      }
   }
}

mySQL::~mySQL() {
   if(disconnect()) {
      throw string("[ERROR] Unable to close database ");
   }
}