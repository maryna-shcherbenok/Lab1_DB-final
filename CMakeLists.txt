cmake_minimum_required(VERSION 3.30)
project(Lab1_DB C)

set(CMAKE_C_STANDARD 11)

add_executable(Lab1_DB main.c
        books.c
        books.h
        publishers.c
        struct.h
        publishers.c
        publishers.h
        file_manager.c
        file_manager.h)
