# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(func_demo)

FILE(GLOB devices_sources devices/src/*.c)

target_sources(app PRIVATE main.c ${devices_sources})

target_include_directories(app PRIVATE ./ devices/inc)
