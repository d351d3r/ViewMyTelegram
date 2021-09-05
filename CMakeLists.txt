cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

project(SaveMyTelegram VERSION 1.0 LANGUAGES CXX)

add_subdirectory(lib/td)
add_executable(SaveMyTelegram src/auth.cpp src/main.cpp src/chats_to_parse.cpp)
target_link_libraries(SaveMyTelegram PRIVATE Td::TdStatic)
set_property(TARGET SaveMyTelegram PROPERTY CXX_STANDARD 14)