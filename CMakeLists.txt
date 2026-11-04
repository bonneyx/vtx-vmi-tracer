cmake_minimum_required(VERSION 3.16)
project(vtx_vmi_tracer C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(vtx_vmi_core STATIC
    src/ept_monitor.c
    src/vmx_exit_dispatch.c
)

target_include_directories(vtx_vmi_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

