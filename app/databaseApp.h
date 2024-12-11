#ifndef DATABASE_APP_H
#define DATABASE_APP_H

#include "databaseConnector.h"
#include <string>

class DatabaseApp {
public:
    DatabaseApp(DatabaseConnector* dbConnector);
    void run();

private:
    void displayMenu();
    void runQueryMenu();
    void executeQuery(const std::string& query);
    void configureProgram();
    
    // Database setting helper functions
    int fetchDatabaseSetting(const std::string& settingName);
    void updateDatabaseSetting(const std::string& settingName, int value);
    void setStorageThreshold();
    void setDataRemovalAmount();

    DatabaseConnector* dbConnector;
    int storageThreshold;
    int dataRemovalAmount;
};

#endif // DATABASE_APP_H