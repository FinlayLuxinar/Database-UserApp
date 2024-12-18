#include <iostream>
#include <string>
#include "databaseConnector.h"
#include "databaseApp.h"

int main(int argc, char *argv[]) {
    std::string host = "172.19.76.58";
    std::string user, pass, database = "my_database";
    bool connectionSuccessful = false;
    DatabaseConnector* dbConnector = nullptr;

    // Allow multiple login attempts
    while (!connectionSuccessful) {
        // Input username and password
        std::cout << "Enter database username: ";
        std::cin >> user;
        std::cout << "Enter database password: ";
        std::cin >> pass;

        try {
            // Attempt to create DatabaseConnector instance
            dbConnector = new DatabaseConnector(host, user, pass, database);
            
            // If no exception is thrown, connection is successful
            connectionSuccessful = true;
        } 
        catch (const std::exception& e) {
            std::cerr << "Connection failed: " << e.what() << std::endl;
            std::cout << "Would you like to try again? (y/n): ";
            
            char retry;
            std::cin >> retry;
            
            // If user doesn't want to retry, exit the program
            if (retry != 'y' && retry != 'Y') {
                delete dbConnector;  // Clean up if allocated
                return 1;
            }
        }
    }

    try {
        // Create DatabaseApp with the connector
        DatabaseApp databaseApp(dbConnector);
        
        // Run the application logic
        databaseApp.run();

        // Clean up dynamically allocated connector
        delete dbConnector;
    } 
    catch (const std::exception& e) {
        std::cerr << "Application error: " << e.what() << std::endl;
        
        // Clean up dynamically allocated connector
        delete dbConnector;
        return 1;
    }
}