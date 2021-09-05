cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

project(SaveMyTelegram VERSION 1.0 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall -Wextra")

set(CMAKE_CXX_FLAGS_DEBUG_INIT "-g -fsanitize=address -fsanitize=undefined")
set(CMAKE_CXX_FLAGS_RELEASE_INIT "-O2")

# Add your included directories here
include_directories("include/"
        "third_party/")

set(HEADERS "include/date_printer.hpp"
        "include/overload.hpp"
        "include/TdParser.hpp")

# Add your source files here
set(SOURCES "src/main.cpp"
        "src/actions.cpp"
        "src/auth.cpp"
        "src/monitoring_chats.cpp"
        "src/update.cpp")

add_subdirectory(third_party/td)
add_executable(SaveMyTelegram ${HEADERS} ${SOURCES})
target_link_libraries(SaveMyTelegram PRIVATE Td::TdStatic)
set_property(TARGET SaveMyTelegram PROPERTY CXX_STANDARD 14)