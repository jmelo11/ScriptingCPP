cmake_minimum_required(VERSION 3.8)
project(ScriptingCPP)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB SOURCES
    "src/*.cpp"
)

if(NOT WIN32)
    list(REMOVE_ITEM SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/src/xlApi.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/xlExport.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/xlcall.cpp
    )
endif()

file(GLOB HEADERS "include/*.h")

add_executable(scripting_test ${SOURCES} ${HEADERS})
target_include_directories(scripting_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

