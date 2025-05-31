cmake_minimum_required(VERSION 3.4.1)

add_library(
        equirToCubemap
        SHARED
        equirToCubemap.cpp
)

if(NOT TARGET miscUtils)
    add_subdirectory(
            ${CPP_SOURCE_DIR}/modules/miscUtils miscUtils.out
    )
endif()

target_include_directories(equirToCubemap PRIVATE ${CPP_SOURCE_DIR}/modules/miscUtils/
                                                  ${CPP_SOURCE_DIR}/modules/heads/)

target_link_libraries(
        equirToCubemap
        miscUtils
)