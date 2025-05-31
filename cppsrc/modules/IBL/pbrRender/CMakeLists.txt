cmake_minimum_required(VERSION 3.4.1)

add_library(
        pbrRender
        SHARED
        pbrRender.cpp
)

if(NOT TARGET miscUtils)
    add_subdirectory(
            ${CPP_SOURCE_DIR}/modules/miscUtils miscUtils.out
    )
endif()

target_include_directories(pbrRender PRIVATE ${CPP_SOURCE_DIR}/modules/miscUtils/
                                             ${CPP_SOURCE_DIR}/modules/heads/)

target_link_libraries(
        pbrRender
        miscUtils
)