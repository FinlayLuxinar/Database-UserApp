#ifndef DATA_HANDLER_H
#define DATA_HANDLER_H

#include "databaseConnector.h"
#include <string>
#include <vector>
#include <chrono>

class DataHandler {
public:
    DataHandler(DatabaseConnector* dbConnector);

    // Graph data generation methods
    void generateData();
    void analyzeField();

    void chooseGraphData();
    static std::string lastGraphType;

private:
    // Data preprocessing and timestamp parsing
    std::vector<std::pair<double, double>> preprocessData(
        const std::vector<std::pair<std::string, double>>& rawData);

    std::chrono::system_clock::time_point parseTimestamp(const std::string& timestampStr);

   
    void printTableHeaders();
    
    
    // Fetch data for graphing
    std::vector<std::pair<std::string, double>> fetchPowerData();
    std::vector<std::pair<std::string, double>> fetchFlowRateData();
    std::vector<std::pair<std::string, double>> fetchFrequencyData();

    void calculateRange(const std::vector<double>& values);
    void calculateMean(const std::vector<double>& values);
    void calculateMedian(std::vector<double>& values);
    void calculateStandardDeviation(const std::vector<double>& values);
    void identifyOutliers(const std::vector<double>& values);
    
    std::vector<std::pair<std::string, double>> fetchDataFromDatabase(const std::string& query);

    // Pointer to database connector
    DatabaseConnector* dbConnector;
};

#endif // DATA_HANDLER_H