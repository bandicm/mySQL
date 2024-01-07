#include "../lib/sqlqa.hpp"

using namespace marcelb;

sqlQA& marcelb::sqlQA::select(const string _columns) {
   if (_columns != "*") {
      parse_columns(_columns);
   }
   isSelect = true;
   cmd += "SELECT " + _columns + " ";
   return *this;
}

sqlQA& marcelb::sqlQA::from(const string _table) {
   table = _table;
   cmd += "FROM " + _table + " ";
   return *this;
}

sqlQA& marcelb::sqlQA::where(const string _condition) {
   cmd += "WHERE " + _condition + " ";
   return *this;
}

sqlQA& marcelb::sqlQA::limit(const uint _limit) {
   cmd += "LIMIT " + to_string(_limit) + " ";
   return *this;
}

sqlQA& marcelb::sqlQA::insertInTo(const string _tablename, const string _columns) {
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

sqlQA& marcelb::sqlQA::values(const string _values) {
   cmd += "VALUES (" + _values + ") ";
   return *this;
}

sqlQA& marcelb::sqlQA::update(const string _table) {
   isUpdate = true;
   cmd += "UPDATE " + _table + " ";
   return *this;
}

sqlQA& marcelb::sqlQA::set(const string _column_value_pairs) {
   cmd += "SET " + _column_value_pairs + " ";
   return *this;
}

sqlQA& marcelb::sqlQA::deleteFrom(const string _table) {
   isUpdate = true;
   cmd += "DELETE FROM " + _table + " ";
   return *this;
}

void marcelb::sqlQA::print(bool withDetail) {
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

void marcelb::sqlQA::parse_columns(const string _columns) {
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