cmake_minimum_required(VERSION 3.16)
project(TamilEditor)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(TamilEditor
    main.cpp
    mainwindow.cpp
    englishtotamilconverter.cpp
    suggestionengine.cpp
)

target_link_libraries(TamilEditor Qt6::Core Qt6::Widgets)