#ifndef DATABASE_APP_H
#define DATABASE_APP_H

#include "databaseConnector.h"
#include "dataHandler.h"
#include <string>
#include <vector>
#include <chrono>

class DatabaseApp {
public:
    DatabaseApp(DatabaseConnector* dbConnector);
     ~DatabaseApp();
    void run();

private:
    void displayMenu();
    void runQueryMenu();
    void executeQuery(const std::string& query);
    void configureProgram();
    void analyseData(); // Add this method declaration
    void calculateStatistics();
    
    // Database setting helper functions
    int fetchDatabaseSetting(const std::string& settingName);
    int getValidatedIntInput(const std::string& prompt, int minValue, int maxValue);
    void updateDatabaseSetting(const std::string& settingName, int value);
    void setStorageThreshold();
    void setDataRemovalAmount();

    DatabaseConnector* dbConnector;
    DataHandler* dataHandler; // Add a DataHandler pointer
    int storageThreshold;
    int dataRemovalAmount;
};

#endif // DATABASE_APP_H