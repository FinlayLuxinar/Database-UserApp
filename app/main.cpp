#include <iostream>
#include <string>
#include "databaseConnector.h"
#include "databaseApp.h"

int main() {
    std::string host = "127.0.0.1";
    std::string user, pass, database = "my_database";

    // Input username and password
    std::cout << "Enter database username: ";
    std::cin >> user;
    std::cout << "Enter database password: ";
    std::cin >> pass;

    try {
        // Create DatabaseConnector instance
        DatabaseConnector dbConnector(host, user, pass, database);
        DatabaseApp app(&dbConnector);

        // Run the application logic
        app.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}