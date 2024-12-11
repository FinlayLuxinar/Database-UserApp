#ifndef DATABASE_CONNECTOR_H
#define DATABASE_CONNECTOR_H

#include <string>
#include <mariadb/mysql.h>

class DatabaseConnector {
public:
    DatabaseConnector(const std::string& host, const std::string& user,
                      const std::string& pass, const std::string& db);
    ~DatabaseConnector();

    MYSQL* getConnection();

private:
    MYSQL* conn;
    // Prevent copying
    DatabaseConnector(const DatabaseConnector&) = delete;
    DatabaseConnector& operator=(const DatabaseConnector&) = delete;
};

#endif // DATABASE_CONNECTOR_H