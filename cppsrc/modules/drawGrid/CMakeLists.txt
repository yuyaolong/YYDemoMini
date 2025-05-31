cmake_minimum_required(VERSION 3.4.1)

add_library(
        drawGrid
        SHARED
        drawGrid.cpp
)

if(NOT TARGET miscUtils)
    add_subdirectory(
            ${CPP_SOURCE_DIR}/modules/miscUtils miscUtils.out
    )
endif()

target_include_directories(drawGrid PRIVATE ${CPP_SOURCE_DIR}/modules/miscUtils/)

target_link_libraries(
        drawGrid
        miscUtils
)