cmake_minimum_required(VERSION 3.10)

project(lab6_queue_adt C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_executable(lab6_queue_adt
    main.c
    warehouse.c
    file_ops.c
)
