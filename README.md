
# A library for MySQL that implements a simpler framework for MySQL Connector++

A small framework for basic MySQL database operations via MySQL/Connector++


## Features

- Object oriented
- Active connection pool
- Retries to connect
- Native C++ containers: map, vector
- QA object
- Thread safe
- Exceptions

## Installation

First install dependency MySQL/Connector++

```
sudo apt install libmysqlcppconn-dev
```

Just download the latest release and unzip it into your project. You can turn it on with:

```
#include "mysql/lib/mysql.hpp"
using namespace marcelb;
```

## Usage

```c++
/**
* Init
*/
mySQL mydb("tcp://192.168.2.10:3306", "user_nm", "passss", "my_db", 5);

/**
* Use
*/
sqlQA test_qa;
// build qa
test_qa.select().from("records").where("enabled = 1");
try {
    // execute
    mydb.exec(test_qa);
    // check is execute
    if (test_qa.executed) {
        // print
        test_qa.print(true);
        // access to first result of column id
        cout << test_qa.result["id"][0] << endl;
        // num of returned rows and columns
        cout << test_qa.num_rows << " " << test_qa.num_columns << endl;
    }
} catch (const string err) {
    cout << err << endl;
}
```
## License

[APACHE 2.0](http://www.apache.org/licenses/LICENSE-2.0/)


## Support & Feedback

For support and any feedback, contact the address: marcelb96@yahoo.com.

## Contributing

Contributions are always welcome!

Feel free to fork and start working with or without a later pull request. Or contact for suggest and request an option.

