cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS
    $ENV{ZEPHYR_BASE}
    ${CMAKE_CURRENT_LIST_DIR}/../../zephyr/share/zephyr-package/cmake
)
project(ZephyrWatch)

target_sources(app PRIVATE src/main.c)

