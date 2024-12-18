#include "dataHandler.h"
#include <iostream>
#include <stdexcept>
#include <mariadb/mysql.h>
#include <limits>
#include <ctime>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <functional>
#include <numeric>

DataHandler::DataHandler(DatabaseConnector* dbConnector) : dbConnector(dbConnector) {}

void DataHandler::printTableHeaders() {
    try {
        MYSQL* conn = dbConnector->getConnection();
        if (!conn) {
            std::cerr << "Database connection is null." << std::endl;
            return;
        }

        const std::string query = "SHOW COLUMNS FROM laser_data"; // Modify the table name if needed
        if (mysql_query(conn, query.c_str())) {
            std::cerr << "Query failed: " << mysql_error(conn) << "\nQuery: " << query << std::endl;
            return;
        }

        MYSQL_RES* res = mysql_store_result(conn);
        if (!res) {
            std::cerr << "Failed to retrieve result: " << mysql_error(conn) << std::endl;
            return;
        }

        std::cout << "Available fields in the table:\n";
        MYSQL_ROW row;
        while ((row = mysql_fetch_row(res))) {
            std::cout << row[0] << std::endl; // Column name
        }

        mysql_free_result(res);
    } catch (const std::exception& e) {
        std::cerr << "Unexpected error in printTableHeaders: " << e.what() << std::endl;
    }
}


std::string DataHandler::lastGraphType = "";

std::vector<std::pair<std::string, double>> DataHandler::fetchPowerData() {
    auto rawData = fetchDataFromDatabase("SELECT timestamp, powerReading FROM laser_data ORDER BY timestamp ASC LIMIT 1000");

    // Convert W to mW
    for (auto& entry : rawData) {
        entry.second /= 1000.0;  // 1 W = 1000 mW
    }

    return rawData;
}

std::vector<std::pair<std::string, double>> DataHandler::fetchFlowRateData() {
    auto rawData = fetchDataFromDatabase("SELECT timestamp, flowRate FROM laser_data ORDER BY timestamp ASC LIMIT 1000");

    // Convert ml to L
    for (auto& entry : rawData) {
        entry.second /= 1000.0;  // 1 L = 1000 ml
    }

    return rawData;
}

std::vector<std::pair<std::string, double>> DataHandler::fetchFrequencyData() {
    return fetchDataFromDatabase("SELECT timestamp, frequency FROM laser_data ORDER BY timestamp ASC LIMIT 1000");
}

std::vector<std::pair<std::string, double>> DataHandler::fetchDataFromDatabase(const std::string& query) {
    std::vector<std::pair<std::string, double>> data;

    try {
        MYSQL* conn = dbConnector->getConnection();
        if (!conn) {
            std::cerr << "Database connection is null." << std::endl;
            return data;
        }

        if (mysql_query(conn, query.c_str())) {
            std::cerr << "Query failed: " << mysql_error(conn)
                    << "\nQuery: " << query << std::endl;
            return data;
        }

        MYSQL_RES* res = mysql_store_result(conn);
        if (!res) {
            std::cerr << "Failed to retrieve result: " << mysql_error(conn) << std::endl;
            return data;
        }

        MYSQL_ROW row;
        while ((row = mysql_fetch_row(res))) {
            try {
                if (!row[0] || !row[1]) {
                    std::cerr << "Null or invalid row encountered. Skipping..." << std::endl;
                    continue;
                }

                std::string timestamp = row[0];
                double value = std::stod(row[1]); // Directly convert; assumes clean query results
                data.emplace_back(timestamp, value);
            } catch (const std::exception& e) {
                std::cerr << "Invalid data format encountered: " << e.what() << ". Skipping row." << std::endl;
            }
        }

        mysql_free_result(res);
    } catch (const std::exception& e) {
        std::cerr << "Unexpected error in fetchDataFromDatabase: " << e.what() << std::endl;
    }

    return data;
}



std::vector<std::pair<double, double>> DataHandler::preprocessData(
    const std::vector<std::pair<std::string, double>>& rawData) {
    std::vector<std::pair<double, double>> processedData;

    if (rawData.empty()) return processedData;

    // Parse the first timestamp to use as a reference point
    auto firstTimestamp = parseTimestamp(rawData.front().first);  // Parse the string timestamp into time_point

    for (const auto& entry : rawData) {
        try {
            auto currentTimestamp = parseTimestamp(entry.first);  // Parse each timestamp to time_point

            // Calculate time difference in seconds
            double timeDiff = std::chrono::duration<double>(
                currentTimestamp - firstTimestamp
            ).count();

            processedData.emplace_back(timeDiff, entry.second);
        } catch (const std::exception& e) {
            std::cerr << "Error processing timestamp: " << e.what()
                      << " for timestamp: " << entry.first << std::endl;
        }
    }

    return processedData;
}

std::chrono::system_clock::time_point DataHandler::parseTimestamp(const std::string& timestampStr) {
    std::tm tm = {};
    std::istringstream ss(timestampStr);

    // Use strptime for more reliable parsing (POSIX method)
    if (strptime(timestampStr.c_str(), "%Y-%m-%d %H:%M:%S", &tm) == nullptr) {
        throw std::runtime_error("Failed to parse timestamp: " + timestampStr);
    }

    // Convert to time_t
    tm.tm_isdst = -1;  // Let the system determine daylight saving time
    std::time_t time = std::mktime(&tm);

    // Handle microseconds if present
    size_t dotPos = timestampStr.find('.');
    int microseconds = 0;
    if (dotPos != std::string::npos) {
        try {
            microseconds = std::stoi(timestampStr.substr(dotPos + 1));
        } catch (...) {
            // Ignore if microseconds can't be parsed
        }
    }

    // Convert to system_clock time point
    auto timePoint = std::chrono::system_clock::from_time_t(time);
    timePoint += std::chrono::microseconds(microseconds);

    return timePoint;
}

void DataHandler::analyzeField() {
    // First, print out available table headers
    printTableHeaders();

    // Prompt user to select a field
    std::string field;
    std::cout << "\nEnter the field name you want to analyze: ";
    std::cin >> field;

    std::cout << "Testing analysis for field: " << field << std::endl;

    // Construct a dynamic query based on the selected field
    std::string query = "SELECT timestamp, " + field + " FROM laser_data ORDER BY timestamp ASC LIMIT 1000";
    
    // Fetch data using a generic method
    auto rawData = fetchDataFromDatabase(query);

    if (rawData.empty()) {
        std::cout << "No data found for the specified field." << std::endl;
        return;
    }

    // Extract values from fetched data
    std::vector<double> values;
    for (const auto& entry : rawData) {
        values.push_back(entry.second);
    }

    int analysisChoice;
    std::cout << "\nChoose an analysis option:\n";
    std::cout << "1. Range\n";
    std::cout << "2. Mean\n";
    std::cout << "3. Median\n";
    std::cout << "4. Standard Deviation\n";
    std::cout << "5. Outliers\n";
    std::cout << "Please select an option: ";
    std::cin >> analysisChoice;

    switch (analysisChoice) {
        case 1:
            calculateRange(values);
            break;
        case 2:
            calculateMean(values);
            break;
        case 3:
            calculateMedian(values);
            break;
        case 4:
            calculateStandardDeviation(values);
            break;
        case 5:
            identifyOutliers(values);
            break;
        default:
            std::cout << "Invalid choice." << std::endl;
            break;
    }
}

void DataHandler::calculateRange(const std::vector<double>& values) {
    auto minMax = std::minmax_element(values.begin(), values.end());
    std::cout << "Range: " << *minMax.second - *minMax.first << std::endl;
}

void DataHandler::calculateMean(const std::vector<double>& values) {
    double sum = std::accumulate(values.begin(), values.end(), 0.0);
    double mean = sum / values.size();
    std::cout << "Mean: " << mean << std::endl;
}

void DataHandler::calculateMedian(std::vector<double>& values) {
    std::sort(values.begin(), values.end());
    size_t size = values.size();
    double median = (size % 2 == 0)
                    ? (values[size / 2 - 1] + values[size / 2]) / 2
                    : values[size / 2];
    std::cout << "Median: " << median << std::endl;
}

void DataHandler::calculateStandardDeviation(const std::vector<double>& values) {
    double mean = std::accumulate(values.begin(), values.end(), 0.0) / values.size();
    double sumSquaredDiffs = 0.0;
    for (double value : values) {
        sumSquaredDiffs += (value - mean) * (value - mean);
    }
    double stdDev = std::sqrt(sumSquaredDiffs / values.size());
    std::cout << "Standard Deviation: " << stdDev << std::endl;
}

void DataHandler::identifyOutliers(const std::vector<double>& values) {
    double mean = std::accumulate(values.begin(), values.end(), 0.0) / values.size();
    double stdDev = 0;
    for (double value : values) {
        stdDev += (value - mean) * (value - mean);
    }
    stdDev = std::sqrt(stdDev / values.size());

    std::cout << "Outliers (values greater than 2 standard deviations away from the mean):\n";
    for (double value : values) {
        if (std::abs(value - mean) > 2 * stdDev) {
            std::cout << value << std::endl;
        }
    }
}
