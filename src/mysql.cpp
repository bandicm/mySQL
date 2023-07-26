#include "../lib/mysql.hpp"

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
   db = _db.empty() ? db : _db;
   bool status = true;

   try {
      con->setSchema(db);
      status = false;
   }
   catch (const SQLException &error) {
      cout << error.what() << endl;
   } 

   return status;
}

bool mySQL::connect() {
   uint trys = 0;
   bool status = true;

   while (trys < CONNECT_TRY_LIMIT && status) {
      try {
         drv = get_mysql_driver_instance();
         con = drv->connect(path, username, password);
         status = false;
      }
      catch (const SQLException &error) {
         cout << error.what() << endl;
         usleep(10000*trys++);
      }
   }

   return status;
}

bool mySQL::close() {
   bool status = true;

   if (con->isValid() && !con->isClosed()) {
      try {
         con->close();
         status = false;
      } 
      catch (const SQLException &error) {
         cout << error.what() << endl;
      }
   }

   return status;
}

map<string, vector<string>> mySQL::query(const string sql_command) {

   if (!isPersistent || !con->isValid() || con->isClosed()) {
      if (connect()) {
         throw string("[ERROR] Unable to connect database ");
      }

      if (open()) {
         throw string("[ERROR] Unable to open database " + db);
      }
   }

   /**/
   map<string, vector<string>> maped;

   try {
      Statement *stmt;
      stmt = con->createStatement();
      
      const string table = getTable(sql_command);
      if (table.empty()) {
         throw string ("[ERROR] SQL command not have table ");
      }

      ResultSet *columnsRes = stmt->executeQuery("SHOW COLUMNS from " + table);
      vector<string> tableFields;

      while (columnsRes->next()) {
         tableFields.push_back(columnsRes->getString("Field"));
      }

      delete columnsRes;

      ResultSet *res = stmt->executeQuery(sql_command);

      while (res->next()) {
         for (uint i=0; i<tableFields.size(); i++) {
            maped[tableFields[i]].push_back(res->getString(tableFields[i]));
         }
      }

      delete res;
      delete stmt;

   }
   catch (const SQLException &error) {
      cout << error.what() << endl;
   }
   catch (const string error) {
      throw error;
   }

   /**/

   if (!isPersistent) {
      if(close()) {
         throw string("[ERROR] Unable to close database ");
      }
   }

   return maped;
}

bool mySQL::change(const string sql_command) {

   if (!isPersistent || !con->isValid() || con->isClosed()) {
      if (connect()) {
         throw string("[ERROR] Unable to connect database ");
      }

      if (open()) {
         throw string("[ERROR] Unable to open database " + db);
      }
   }

   /**/
   bool status = false;
   
   try {
      Statement *stmt;
      stmt = con->createStatement();
      
      uint changeCatch = stmt->executeUpdate(sql_command);
      status = (bool)changeCatch;

      delete stmt;

   }
   catch (const SQLException &error) {
      cout << error.what() << endl;
   }
   catch (const string error) {
      throw error;
   }

   /**/

   if (!isPersistent) {
      if(close()) {
         throw string("[ERROR] Unable to close database ");
      }
   }

   return status;
}


string mySQL::getTable(const string req) {
   size_t from = req.find("FROM") < req.find("from") ? req.find("FROM") : req.find("from");
   if (from > req.length()) {
      return "";
   }
   string table = req.substr(from+5, req.find(" ", from+5));
   return table;
}



mySQL::~mySQL() {
   if(close()) {
      throw string("[ERROR] Unable to close database ");
   }
}