project(CLICHARM)
cmake_minimum_required(VERSION 2.4.0)

# defaults
set(BOOKMARK_TASKS_MAX 10 CACHE INT "Maximum number of bookmark tasks")
set(CHARM_DB_DEBUG "Charm_debug.db" CACHE STRING "Default database filename in debug mode")
set(CHARM_DB_RELEASE "Charm.db" CACHE STRING "Default database filename in release mode")
set(DEBUG_VERBOSE OFF CACHE BOOL "Print out debug messages")
set(RECENT_TASKS_MAX 10 CACHE INT "Maximum number of recent tasks")

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/modules)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(SQLITE_MIN_VERSION 3.6.23)
find_package(Sqlite)

include_directories(AFTER SYSTEM ${PROJECT_BINARY_DIR})

configure_file(config.h.in ${PROJECT_BINARY_DIR}/config.h)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

if (CMAKE_C_COMPILER MATCHES ".*clang" OR CMAKE_C_COMPILER MATCHES ".*ccc-analyzer")
	set(CMAKE_COMPILER_IS_CLANGCC 1)
endif()
if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_CLANGCC)
	set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wall -Wwrite-strings -pedantic -W -Wconversion -Wshadow -Wcast-qual" CACHE STRING "" FORCE)
endif()

add_subdirectory(src)
