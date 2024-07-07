cmake_minimum_required(VERSION 3.20.1)

project(lib_os_cbprintf)

find_package(Zephyr COMPONENTS unittest REQUIRED HINTS $ENV{ZEPHYR_BASE})
target_sources(testbinary PRIVATE main.c)