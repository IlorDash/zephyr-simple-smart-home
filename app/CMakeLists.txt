cmake_minimum_required(VERSION 3.20.1)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(app LANGUAGES C)

add_subdirectory(src/read_queue)

target_sources(app PRIVATE src/main.c)
