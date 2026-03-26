#
# Copyright (c) 2020 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#
cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(openthread_coap_client_button)

# NORDIC SDK APP START
target_sources(app PRIVATE src/coap_client_button.c src/coap_client_utils.c)

# NORDIC SDK APP END

target_sources_ifdef(CONFIG_BT_NUS app PRIVATE src/ble_utils.c)
target_include_directories(app PRIVATE src)
