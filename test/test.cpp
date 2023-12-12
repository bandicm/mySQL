#include <iostream>
#include <thread>
#include <chrono>

#include "../lib/mysql.hpp"

using namespace std;
using namespace chrono;

int main() {
    try {
        // mySQL mydb("tcp://192.168.2.10:3306", "dinio", "H€r5elfInd1aH@nds", "dinio", 1);
        mySQL mydb("tcp://192.168.2.10:3306", "dinio", "H€r5elfInd1aH@nds", "dinio", 5);

        // sleep(3600*10);
        sleep(20);


        auto start = high_resolution_clock::now();


        // thread t1([&](){
        //     try {
        //         sqlQA test_qa;
        //         test_qa.select().from("records").where("enabled = 1");
        //         mydb.exec(test_qa);
        //         test_qa.print(true);
        //     } catch (const string err) {
        //         cout << err << endl;
        //     }
        // });

        // // sleep(2);

        // thread t2([&](){
        //     try {
        //         sqlQA test_qa;
        //         test_qa.select().from("zones");
        //         mydb.exec(test_qa);
        //         test_qa.print(true);
        //     } catch (const string err) {
        //         cout << err << endl;
        //     }
        // });

        // // sleep(3);

        // thread t3([&](){
        //     try {
        //         sqlQA test_qa;
        //         test_qa.select().from("users");
        //         mydb.exec(test_qa);
        //         test_qa.print(true);
        //     } catch (const string err) {
        //         cout << err << endl;
        //     }
        // });

        // // sleep(1);

        // thread t4([&](){
        //     try {
        //         sqlQA test_qa;
        //         test_qa.select().from("records").where("enabled = 1");
        //         mydb.exec(test_qa);
        //         test_qa.print(true);
        //     } catch (const string err) {
        //         cout << err << endl;
        //     }
        // });

        // thread t5([&](){
        //     try {
        //         sqlQA test_qa;
        //         test_qa.select().from("zones");
        //         mydb.exec(test_qa);
        //         test_qa.print(true);
        //     } catch (const string err) {
        //         cout << err << endl;
        //     }
        // });

        // thread t6([&](){
        //     try {
        //         sqlQA test_qa;
        //         test_qa.select().from("users");
        //         mydb.exec(test_qa);
        //         test_qa.print(true);
        //     } catch (const string err) {
        //         cout << err << endl;
        //     }
        // });

        // t1.join();
        // t2.join();
        // t3.join();
        // t4.join();
        // t5.join();
        // t6.join();

//        one by one
        try {
            sqlQA test_qa;
            test_qa.select().from("records").where("enabled = 1");
            mydb.exec(test_qa);
            test_qa.print(true);
        } catch (const string err) {
            cout << err << endl;
        }

        sleep(20);
    
        try {
            sqlQA test_qa;
            test_qa.select().from("users");
            mydb.exec(test_qa);
            test_qa.print(true);
        } catch (const string err) {
            cout << err << endl;
        }

        sleep(20);

        try {
            sqlQA test_qa;
            test_qa.select("zone_id,record_type,enabled").from("records").where("domain = 'bitelex.test'");
            mydb.exec(test_qa);
            test_qa.print(true);
        } catch (const string err) {
            cout << err << endl;
        }


        auto end = high_resolution_clock::now();
        auto duration = duration_cast<microseconds>(end - start);
        cout << "-------------Izvršilo se za: " << (double)(duration.count() / 1000.0) << " ms"<< endl;
        
        // sleep(100);
        
        
    } catch (const SQLException error) {
        cout << error.what() << endl;
    } catch (const string error) {
        cout << error << endl;
    } catch (...) {
        cout << "Jebi ga" << endl;
    }

    sleep(600);

    return 0;
}