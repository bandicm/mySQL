#include <iostream>

#include "../lib/mysql.hpp"

using namespace std;

int main() {

    mySQL mydb("tcp://192.168.2.10:3306", "dinio", "H€r5elfInd1aH@nds", "dinio", true);

    try {

        sqlQA test_qa;
        // id,user_id,zone_id,domain,record_type,auth_key,last_update,enabled
        // test_qa.select().from("records").where("enabled = 0").limit(2);
        // mydb.exec(test_qa);

        // for (auto i : test_qa.result) {
        //     for (auto j: i.second) {
        //         cout << i.first << " : " << j << endl;
        //     }
        // }


        // test_qa.update("records").set("enabled = 1").where("domain = 'bitelex.test'");
        // mydb.exec(test_qa);
        // if (test_qa.executed) {
        //     cout << "Num rows affect " << test_qa.updateCatch << endl;
        // }

        // cout << "Num rows " << test_qa.num_rows << " num columns " << test_qa.num_columns << " executed " << test_qa.executed << endl;


        // test_qa.insertInTo("records", "id,user_id,zone_id,domain,record_type,auth_key,last_update,enabled").values("'5',2,2,'www.bitelex.test','AAAA','jebiga',NULL,1");
        test_qa.deleteFrom("records").where("record_type = AAAA");
        // test_qa.update("records").set("enabled = 0").where("record_type = 'AAAA'");
        cout << test_qa.cmd << endl;
        mydb.exec(test_qa);
        cout << "Num rows " << test_qa.num_rows << " num columns " << test_qa.num_columns << " catch " << test_qa.updateCatch <<  " executed " << test_qa.executed << endl;



    }
    catch (const SQLException error) {
        cout << error.what() << endl;
    }
    // catch (const string error) {
    //     cout << error << endl;
    // }
    catch (...) {
        cout << "Jebi ga" << endl;
    }


    return 0;
}