cmake_minimum_required(VERSION 3.8)
project(ScriptingCPP)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
    scriptingParser.cpp
)

add_executable(scripting_test
    main.cpp
    ${SOURCES}
)

target_include_directories(scripting_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
