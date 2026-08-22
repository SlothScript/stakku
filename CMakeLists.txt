cmake_minimum_required(VERSION 3.16)

project(stakku LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SOURCES
    src/main.cpp
    src/repl.cpp
    src/interpreter/interpreter.cpp
    src/interpreter/stack.cpp
)

add_executable(stakku ${SOURCES})

target_compile_options(stakku PRIVATE -Wall -Wextra -Wpedantic)

target_include_directories(stakku
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include/stakku
        ${CMAKE_CURRENT_SOURCE_DIR}/include/stakku/interpreter
)

install(TARGETS stakku DESTINATION bin)
