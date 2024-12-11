cmake_minimum_required(VERSION 3.10)
project(DatabaseGUI)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(MARIADB REQUIRED mariadb)

add_executable(DatabaseGUI main.cpp databaseApp.cpp databaseConnector.cpp)

target_link_libraries(DatabaseGUI 
    Qt5::Widgets 
    ${MARIADB_LIBRARIES}
)