cmake_minimum_required(VERSION 3.4.1)

add_library(
        foveationByVRS
        SHARED
        foveationByVRS.cpp
)

if(NOT TARGET miscUtils)
    add_subdirectory(
            ${CPP_SOURCE_DIR}/modules/miscUtils miscUtils.out
    )
endif()

target_include_directories(foveationByVRS PRIVATE ${CPP_SOURCE_DIR}/modules/miscUtils/)


target_link_libraries(
        foveationByVRS
        miscUtils
)