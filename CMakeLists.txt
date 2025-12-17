cmake_minimum_required(VERSION 3.16)
project(automata)

set(CMAKE_CXX_STANDARD 17)

# Qt install path (change if needed)
set(CMAKE_PREFIX_PATH "C:/Qt/6.10.1/mingw_64")

# Find Qt packages
find_package(Qt6 COMPONENTS Core Widgets REQUIRED)

# Enable AUTOMOC and AUTOUIC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

# Add executable
add_executable(MyQtApp
    main.cpp
    mainwindow.cpp
    mainwindow.h
    ProjectOverviewTab.cpp
    ProjectOverviewTab.h
    LexicalAnalysis.cpp
    LexicalAnalysis.h
    SyntaxAnalysisTab.cpp
    SyntaxAnalysisTab.h
)

# Link Qt libraries
target_link_libraries(MyQtApp Qt6::Core Qt6::Widgets)
