cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS
    $ENV{ZEPHYR_BASE}
    ${CMAKE_CURRENT_LIST_DIR}/../../zephyr/share/zephyr-package/cmake
)
project(ZephyrWatch)

target_sources(app PRIVATE
    src/main.c
    src/platform/display.c
    src/ui/watch_face.c
    src/core/app_manager.c
    src/core/app_registry.c
    src/services/imu_services.c
    src/services/pedometer_service.c
    src/services/time_service.c
    src/utils/string_utils.c
    src/applications/pedometer/pedometer_app.c
    src/applications/watchface/watchface_app.c
)

target_include_directories(app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

