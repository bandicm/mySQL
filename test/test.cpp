#include <iostream>

#include "../lib/mysql.hpp"

using namespace std;

int main() {

    mySQL mydb("tcp://192.168.2.10:3306", "dinio", "H€r5elfInd1aH@nds", "dinio", true);

    try {

        auto res = mydb.query("SELECT * FROM users");

        for (auto i : res) {
            for (auto j: i.second) {
                cout << i.first << " : " << j << endl;
            }
        }

        if (mydb.change("UPDATE records SET enabled = 0 WHERE domain = 'bitelex.ml'")) {
            cout << "Update sucessfuly" << endl;
        }

    }
    catch (const SQLException error) {
        cout << error.what() << endl;
    }
    catch (const string error) {
        cout << error << endl;
    }


    return 0;
}