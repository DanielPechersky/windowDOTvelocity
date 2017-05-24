cmake_minimum_required(VERSION 3.7)
project(windowDOTvelocity)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp)
add_executable(windowDOTvelocity ${SOURCE_FILES})

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake_modules")

set(SFML_ROOT C:/Users/dannyp/Documents/SFML-2.4.2)
find_package(SFML REQUIRED system window graphics)
if (SFML_FOUND)
    include_directories(${SFML_INCLUDE_DIR})
    target_link_libraries(windowDOTvelocity ${SFML_LIBRARIES})
endif()
