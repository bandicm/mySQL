#include "../lib/mysql.hpp"


marcelb::mySQL::mySQL(const string _path, const string _username, const string _password, const string _db, const uint _available) {
   path = _path;
   username = _username;
   password = _password;
   db = _db;
   available = _available > 0 ? _available : 1;

   drv = get_mysql_driver_instance();

   bot = async(launch::async, [&](){
      while (runBot) {
         sleep(1);
         while (available>con.size() && runBot) {
            try {
               Connection* new_con_ptr = create_con();
               if (!db.empty()) {
                  if (open_one(new_con_ptr)) {
                     throw string("[ERROR] Unable to open database " + db);
                  }
               }
               io.lock();
               con.push_back(new_con_ptr);       
               io.unlock();
            } catch (const SQLException except) {
               cout << except.what() << endl;
            } catch (const string except) {
               cout << except << endl;
            }     
         }

         for (int i=0; i<con.size() && runBot; i++) {
            if (!con[i]->isValid()) {
               io.lock();
               con.erase(con.begin()+i);
               io.unlock();
               i--;
            }
         }
      }
      return;
   });  

}


Connection* marcelb::mySQL::create_con() {
   uint trys = 0;
   bool status = true;
   Connection* new_con = NULL;

   while (reconTrys == unlimited ? status : (trys <= reconTrys && status)) {
      try {
         Connection* con_can = drv->connect(path, username, password);
         status = !con_can->isValid();
         if (!status) {
            new_con = con_can;
         }
         else if (!con_can->isClosed()) {
            disconnect_one(con_can);
         }
      }
      catch (const SQLException &error) {
         cout << error.what() << endl;
         usleep(reconnectSleep);
         reconTrys == unlimited ? trys : trys++;
      }         
   }

   return new_con;
}

bool marcelb::mySQL::disconnect() {
   io.lock();
   bool status = true;

   for (uint i=0; i<con.size(); i++) {
      status = disconnect_one(con[i]) ;
   }

   io.unlock();
   return status;
}

bool marcelb::mySQL::disconnect_one(Connection* con_ptr) {
   bool status = !con_ptr->isClosed();

   if (status) {
      try {
         con_ptr->close();
         status = !con_ptr->isClosed();
      } 
      catch (const SQLException &error) {
         cout << error.what() << endl;
         status = true;
      }
   }

   else {
      status = false; // već je zatvorena
   }

   delete con_ptr;
   return status;
}

bool marcelb::mySQL::open_one(Connection* con_ptr) {
   bool status = true; // ako true greška je
   uint trys = 0;

   while (reconTrys == unlimited ? status : (trys <= reconTrys && status)) {
      try {
         if (con_ptr->isValid()) {
            con_ptr->setSchema(db);
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

void marcelb::mySQL::reconnectTrys(const uint _trys) {
   io.lock();
   reconTrys = _trys;
   io.unlock();
}


void marcelb::mySQL::exec(sqlQA &sql_qa) {
   Connection* con_ptr = shift_con(); 

   try {
      vector<string> columns = sql_qa.columns;
      
      if (columns.empty() && !sql_qa.table.empty()) {
         getColumns(sql_qa.table, columns, con_ptr);
      }

      Statement *stmt;
      stmt = con_ptr->createStatement();

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
      disconnect_one(con_ptr);
   }
   catch (const SQLException &error) {
      cout << error.what() << endl;
      sql_qa.executed = false;
   }
   catch (const string error) {
      throw error;
   }

}

void marcelb::mySQL::getColumns(const string _table, vector<string> &_columns, Connection *ptr_con) {
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

Connection* marcelb::mySQL::shift_con() {
   while (true) {
      while(con.size()) {
         io.lock();
         Connection* con_ptr = con[0];
         con.pop_front();
         if (con_ptr->isValid()) {
            io.unlock();
            return con_ptr;
         } 
         io.unlock();
      }
      usleep(1000);
   }
}

marcelb::mySQL::~mySQL() {
   runBot = false;
   bot.get();
   disconnect();
}