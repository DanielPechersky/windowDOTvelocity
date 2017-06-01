cmake_minimum_required(VERSION 3.7)
project(windowDOTvelocity)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp)
add_executable(windowDOTvelocity ${SOURCE_FILES})

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules")


if(WIN32)
    set(SFML_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/SFML-2.4.2")
endif()

find_package(SFML REQUIRED system window graphics)
if (SFML_FOUND)
    include_directories(${SFML_INCLUDE_DIR})
    target_link_libraries(windowDOTvelocity ${SFML_LIBRARIES})
endif()

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/config.cfg
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

if(WIN32)
    add_custom_command(TARGET windowDOTvelocity POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/dlls ${CMAKE_CURRENT_BINARY_DIR})
endif()