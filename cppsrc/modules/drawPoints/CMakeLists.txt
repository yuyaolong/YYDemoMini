cmake_minimum_required(VERSION 3.4.1)

add_library(
        drawPoints
        SHARED
        drawPoints.cpp
)

if(NOT TARGET miscUtils)
    add_subdirectory(
            ${CPP_SOURCE_DIR}/modules/miscUtils miscUtils.out
    )
endif()

target_include_directories(drawPoints PRIVATE ${CPP_SOURCE_DIR}/modules/miscUtils/)

target_link_libraries(
        drawPoints
        miscUtils
)