#include "databaseApp.h"
#include <iostream>
#include <stdexcept>
#include <mariadb/mysql.h>
#include <limits>
#include <sstream>

DatabaseApp::DatabaseApp(DatabaseConnector* dbConnector)
    : dbConnector(dbConnector), 
      dataHandler(new DataHandler(dbConnector)), // Initialize DataHandler
      storageThreshold(80), 
      dataRemovalAmount(30) {}



void DatabaseApp::displayMenu() {
    std::cout << "\n===== Database Application Menu =====\n";
    std::cout << "1. Run Query\n";
    std::cout << "2. Configure Program\n";
    std::cout << "3. Mathmatical Operations\n";
    std::cout << "4. Exit\n";
    std::cout << "Please select an option: ";
}

void DatabaseApp::run() {
    int choice;
    do {
        // Clear any previous error flags and input buffer
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        displayMenu();
        
        // Robust input handling
        if (!(std::cin >> choice)) {
            std::cout << "Invalid input. Please enter a number." << std::endl;
            continue;
        }

        switch (choice) {
            case 0:
                std::cout << "Returning to previous menu..." << std::endl;
                break;
            case 1:
                runQueryMenu();
                break;
            case 2:
                configureProgram();
                break;
            case 3:
                analyseData();
                break;
            case 4:
                std::cout << "Exiting the program..." << std::endl;
                break;
            default:
                std::cout << "Invalid choice. Please select a valid option (0-4)." << std::endl;
        }
    } while (choice != 4);
}

void DatabaseApp::analyseData() {
    if (dataHandler) {
        dataHandler->analyzeField();
    } else {
        std::cerr << "Data handler not initialized." << std::endl;
    }
}



void DatabaseApp::runQueryMenu() {
    std::string query;
    std::cout << "\nEnter SQL Query: ";
    std::cin.ignore();  // To clear the input buffer
    std::getline(std::cin, query);  // Allow spaces in the query

    executeQuery(query);
}

void DatabaseApp::executeQuery(const std::string& query) {
    if (!dbConnector) {
        std::cerr << "No database connection" << std::endl;
        return;
    }

    try {
        MYSQL* conn = dbConnector->getConnection();
        if (mysql_query(conn, query.c_str())) {
            throw std::runtime_error(mysql_error(conn));
        }

        MYSQL_RES* res = mysql_store_result(conn);
        if (!res) {
            // Check if the query was a non-SELECT query (INSERT, UPDATE, DELETE)
            if (mysql_field_count(conn) == 0) {
                std::cout << "Query executed successfully. Rows affected: " 
                          << mysql_affected_rows(conn) << std::endl;
                return;
            }
            throw std::runtime_error(mysql_error(conn));
        }

        // Print query results
        int num_fields = mysql_num_fields(res);
        MYSQL_FIELD* fields = mysql_fetch_fields(res);

        // Print column headers
        for (int i = 0; i < num_fields; ++i) {
            std::cout << fields[i].name << "\t";
        }
        std::cout << std::endl;

        // Print rows of data
        MYSQL_ROW row;
        while ((row = mysql_fetch_row(res))) {
            for (int i = 0; i < num_fields; ++i) {
                std::cout << (row[i] ? row[i] : "NULL") << "\t";
            }
            std::cout << std::endl;
        }

        mysql_free_result(res);  // Free the result set
    } catch (const std::exception& e) {
        std::cerr << "Query Error: " << e.what() << std::endl;
    }
}

void DatabaseApp::configureProgram() {
    // Fetch the current values of the settings from the database
    int currentStorageThreshold;
    int currentDataRemovalAmount;

    try {
        currentStorageThreshold = fetchDatabaseSetting("maxStorage");
        currentDataRemovalAmount = fetchDatabaseSetting("monthsToRemove");
    } catch (const std::exception& e) {
        std::cerr << "Error fetching database settings: " << e.what() << std::endl;
        currentStorageThreshold = 80;  // Default values
        currentDataRemovalAmount = 3;
    }

    while (true) {
        std::cout << "\n===== Program Configuration =====\n";
        std::cout << "Current Settings:\n";
        std::cout << "1. Storage Threshold: " << currentStorageThreshold << "%\n";
        std::cout << "2. Data Removal Amount: " << currentDataRemovalAmount << " days\n";
        std::cout << "3. Cancel/Return to Previous Menu\n";
        std::cout << "Please select an option: ";

        int configChoice;
        
        // Clear any previous error flags and input buffer
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        // Robust input handling
        if (!(std::cin >> configChoice)) {
            std::cout << "Invalid input. Please enter a number." << std::endl;
            continue;
        }

        switch (configChoice) {
            case 0:
                std::cout << "Configuration cancelled." << std::endl;
                return;
            case 1:
                try {
                    setStorageThreshold();
                } catch (const std::exception& e) {
                    std::cerr << "Error setting storage threshold: " << e.what() << std::endl;
                }
                break;
            case 2:
                try {
                    setDataRemovalAmount();
                } catch (const std::exception& e) {
                    std::cerr << "Error setting data removal amount: " << e.what() << std::endl;
                }
                break;
            case 3:
                return;
            default:
                std::cout << "Invalid option. Please select a valid option (0-3)." << std::endl;
        }
    }
}

int DatabaseApp::getValidatedIntInput(const std::string& prompt, int minValue, int maxValue) {
    int input;
    while (true) {
        std::cout << prompt;
        
        // Clear any previous error flags and input buffer
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        // Robust input handling
        if (!(std::cin >> input)) {
            std::cout << "Invalid input. Please enter a number." << std::endl;
            continue;
        }

        // Check if input is within valid range
        if (input >= minValue && input <= maxValue) {
            return input;
        }

        std::cout << "Input out of range. Please enter a value between " 
                  << minValue << " and " << maxValue << "." << std::endl;
    }
}

int DatabaseApp::fetchDatabaseSetting(const std::string& settingName) {
    if (!dbConnector) {
        std::cerr << "No database connection" << std::endl;
        return -1;
    }

    MYSQL* conn = dbConnector->getConnection();
    MYSQL_RES* res;
    MYSQL_ROW row;

    // Create the query to fetch the setting
    std::string query = "SELECT " + settingName + " FROM settings WHERE id = 1;";

    if (mysql_query(conn, query.c_str())) {
        std::cerr << "Failed to fetch " << settingName << ": " << mysql_error(conn) << std::endl;
        return -1;
    }

    res = mysql_store_result(conn);
    if (res == nullptr || (row = mysql_fetch_row(res)) == nullptr) {
        std::cerr << "Failed to fetch value for " << settingName << std::endl;
        if (res) mysql_free_result(res);
        return -1;
    }

    int value = std::stoi(row[0]); // Convert the result to an integer
    mysql_free_result(res);
    return value;
}

void DatabaseApp::updateDatabaseSetting(const std::string& settingName, int value) {
    // Use a separate admin connection
    MYSQL* conn = mysql_init(nullptr);

    // Connection parameters for the admin user
    const char* host = "172.19.76.58";
    const char* admin_user = "my_user";  // Separate admin username
    const char* admin_pass = "my_password";  // Secure admin password
    const char* database = "my_database";

    // Establish connection
    if (!mysql_real_connect(conn, host, admin_user, admin_pass, database, 0, nullptr, 0)) {
        std::cerr << "Admin database connection failed: " << mysql_error(conn) << std::endl;
        mysql_close(conn);
        return;
    }

    // Create the update query
    std::string query = "UPDATE settings SET " + settingName + " = " + 
                        std::to_string(value) + ", last_updated = NOW() WHERE id = 1;";

    // Execute the query
    if (mysql_query(conn, query.c_str())) {
        std::cerr << "Failed to update " << settingName << ": " << mysql_error(conn) << std::endl;
        mysql_close(conn);
        return;
    }

    std::cout << settingName << " successfully updated in the database." << std::endl;
    
    // Close the admin connection
    mysql_close(conn);
}

void DatabaseApp::setStorageThreshold() {
    // This method remains mostly the same, but we'll add more robust error checking
    int currentStorageThreshold = fetchDatabaseSetting("maxStorage");
    
    if (currentStorageThreshold == -1) {
        std::cerr << "Error fetching current storage threshold." << std::endl;
        return;
    }

    int newStorageThreshold;
    while (true) {
        std::cout << "\nEnter new storage threshold (current: " << currentStorageThreshold << "%): ";
        
        // Input validation to prevent input failure
        if (!(std::cin >> newStorageThreshold)) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid input. Please enter a numeric value." << std::endl;
            continue;
        }

        if (newStorageThreshold < 0 || newStorageThreshold > 100) {
            std::cout << "Invalid value. Threshold must be between 0 and 100." << std::endl;
            continue;
        }

        // If we've made it here, the input is valid
        updateDatabaseSetting("maxStorage", newStorageThreshold);
        break;
    }
}

void DatabaseApp::setDataRemovalAmount() {
    // Similar to setStorageThreshold, with robust input validation
    int currentDataRemovalAmount = fetchDatabaseSetting("monthsToRemove");
    
    if (currentDataRemovalAmount == -1) {
        std::cerr << "Error fetching current data removal amount." << std::endl;
        return;
    }

    int newDataRemovalAmount;
    while (true) {
        std::cout << "\nEnter new data removal amount in days (current: " 
                  << currentDataRemovalAmount << " days): ";
        
        // Input validation to prevent input failure
        if (!(std::cin >> newDataRemovalAmount)) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid input. Please enter a numeric value." << std::endl;
            continue;
        }

        if (newDataRemovalAmount <= 0) {
            std::cout << "Invalid value. Removal amount must be a positive number." << std::endl;
            continue;
        }

        // If we've made it here, the input is valid
        updateDatabaseSetting("monthsToRemove", newDataRemovalAmount);
        break;
    }
}

DatabaseApp::~DatabaseApp() {
    delete dataHandler;
}