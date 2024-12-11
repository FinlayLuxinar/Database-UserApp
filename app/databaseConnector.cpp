#include "databaseConnector.h"
#include <stdexcept>

DatabaseConnector::DatabaseConnector(const std::string& host, const std::string& user,
                                     const std::string& pass, const std::string& db)
    : conn(mysql_init(nullptr)) {
    if (!conn) {
        throw std::runtime_error("mysql_init() failed");
    }

    // Connect to the database
    if (!mysql_real_connect(conn, host.c_str(), user.c_str(), pass.c_str(),
                            db.c_str(), 0, nullptr, 0)) {
        std::string error_msg = "mysql_real_connect() failed: " + 
                                std::string(mysql_error(conn));
        mysql_close(conn);
        throw std::runtime_error(error_msg);
    }
}

DatabaseConnector::~DatabaseConnector() {
    if (conn) {
        mysql_close(conn);
    }
}

MYSQL* DatabaseConnector::getConnection() { 
    return conn; 
}