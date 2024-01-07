#ifndef _SQLQA_
#define _SQLQA_

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>

using namespace std;

namespace marcelb {

/**
 * A class for creating sql queries and responses
*/
class sqlQA {
    public:
    // query variable list

    // SQL Command
    string cmd;
    // Table name
    string table;
    // Columns name list
    vector<string> columns;
    // Query is update type
    bool isUpdate = false;
    // Query is select type
    bool isSelect = false;

    // answer

    // Number of updates caught
    uint updateCatch = 0;
    // Executing status
    bool executed = false;
    // Answer
    map<string, vector<string>> result;
    // Number of returned rows
    uint num_rows = 0;
    // Number of returned columns
    uint num_columns = 0;

    // query methods

    /**
     * SELECT
     * accept columns names, comma separated  
     * defualt *
    */
    sqlQA& select(const string _select = "*");

    /**
     * FROM
     * accept table name
    */
    sqlQA& from(const string _tablename);

    /**
     * WHERE
     * accept string sql condition
    */
    sqlQA& where(const string _condition);

    /**
     * LIMIT
     * set limit result
    */
    sqlQA& limit(const uint _limit);

    /**
     * INSERT IN TO
     * accept table name, and columns
    */
    sqlQA& insertInTo(const string _tablename, const string _columns = "");

    /**
     * VALUES
     * accept values
    */
    sqlQA& values(const string _values);

    /**
     * UPDATE
     * accept tablename for update query
    */
    sqlQA& update(const string _tablename);

    /**
     * SET
     * accept column and value pairs
    */
    sqlQA& set(const string _column_value_pairs);

    /**
     * DELETE FROM
     * accept table name
    */
    sqlQA& deleteFrom(const string _table);

    /**
     * Print SQLQA
    */
    void print(bool withDetail = false);

    // intern methods
    private:

    /**
     * Parse column names
    */
    void parse_columns(const string _cloumns);
};


}

#endif