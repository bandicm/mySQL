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

mySQL::mySQL(const string _path, const string _username, const string _password, const string _db, bool _isPersistent) {
   path = _path;
   username = _username;
   password = _password;
   db = _db;
   isPersistent = _isPersistent;

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
   bool status = true;

   try {
      con->setSchema(db);
      status = false;
      io.unlock();
   }
   catch (const SQLException &error) {
      cout << error.what() << endl;
      io.unlock();
   } 
   return status;
}

bool mySQL::connect() {
   io.lock();
   uint trys = 0;
   bool status = true;

   while (trys < CONNECT_TRY_LIMIT && status) {
      try {
         drv = get_mysql_driver_instance();
         con = drv->connect(path, username, password);
         status = false;
         io.unlock();
      }
      catch (const SQLException &error) {
         cout << error.what() << endl;
         usleep(10000*trys++);
         io.unlock();
      }
   }

   return status;
}

bool mySQL::disconnect() {
   io.lock();
   bool status = true;

   if (con->isValid() && !con->isClosed()) {
      try {
         con->close();
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
      status = false;
   }

   return status;
}

void mySQL::exec(sqlQA &sql_qa) {

   if (!isPersistent || !con->isValid() || con->isClosed()) {
      if (connect()) {
         throw string("[ERROR] Unable to connect database ");
      }

      if (open()) {
         throw string("[ERROR] Unable to open database " + db);
      }
   }

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
