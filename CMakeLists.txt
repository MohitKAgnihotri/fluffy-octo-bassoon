cmake_minimum_required(VERSION 3.16)
project(fluffy_octo_bassoon C)

set(CMAKE_C_STANDARD 11)
link_libraries(pthread)

add_executable(vowelizer_client vowelizer_client.c)
add_executable(vowelizer_server vowelizer_server.c)
