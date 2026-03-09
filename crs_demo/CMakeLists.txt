# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(crs_demo)

FILE(GLOB devices_sources src/devices/src/*.c)

target_sources(app PRIVATE src/main.c ${devices_sources})

target_include_directories(app PRIVATE src src/devices/inc)
