cmake_minimum_required(VERSION 3.16)

project(stakku LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(CTest)

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(INTERP_SOURCES
    src/interpreter/compiler.cpp
    src/interpreter/vm.cpp
    src/interpreter/stack.cpp
)

add_library(stakku_lib STATIC ${INTERP_SOURCES})

target_compile_options(stakku_lib PRIVATE -Wall -Wextra -Wpedantic)

target_include_directories(stakku_lib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include/stakku
        ${CMAKE_CURRENT_SOURCE_DIR}/include/stakku/interpreter
)

set(APP_SOURCES
    src/main.cpp
    src/repl.cpp
    src/help.cpp
)

add_executable(stakku ${APP_SOURCES})
target_link_libraries(stakku PRIVATE stakku_lib)
target_compile_options(stakku PRIVATE -Wall -Wextra -Wpedantic)


install(TARGETS stakku DESTINATION bin)

if(BUILD_TESTING)
    include(FetchContent)

    FetchContent_Declare(
      Catch2
      GIT_REPOSITORY https://github.com/catchorg/Catch2.git
      GIT_TAG        v3.4.0
    )
    FetchContent_MakeAvailable(Catch2)

    set(TEST_SOURCES
        tests/test_stack.cpp
        tests/test_integration.cpp
        tests/test_advanced.cpp
        tests/test_comprehensive.cpp
        tests/test_vm_ops.cpp
        tests/test_compiler_errors.cpp
        tests/test_variables.cpp
    )

    add_executable(stakku_tests ${TEST_SOURCES})

    target_link_libraries(stakku_tests PRIVATE Catch2::Catch2WithMain stakku_lib)

    target_compile_options(stakku_tests PRIVATE -Wall -Wextra -Wpedantic)

    list(APPEND CMAKE_MODULE_PATH "${catch2_SOURCE_DIR}/extras")
    include(Catch)
    catch_discover_tests(stakku_tests)

endif()
