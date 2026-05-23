cmake_minimum_required(VERSION 3.16)

project(WhiteBoardServer LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)
find_package(Qt6 REQUIRED COMPONENTS Network)

find_package(Qt6 REQUIRED COMPONENTS Core)

add_executable(WhiteBoardServer
  main.cpp
  server.h server.cpp
)

target_sources(WhiteBoardServer
  PRIVATE
    server.cpp server.h
)


target_link_libraries(WhiteBoardServer PRIVATE Qt${QT_VERSION_MAJOR}::Core)
target_link_libraries(WhiteBoardServer PRIVATE Qt6::Core)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Network)

include(GNUInstallDirs)
install(TARGETS WhiteBoardServer
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
