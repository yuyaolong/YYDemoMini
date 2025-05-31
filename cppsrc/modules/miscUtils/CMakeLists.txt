cmake_minimum_required(VERSION 3.4.1)

add_library(
        miscUtils
        SHARED
        miscUtils.cpp
        GRTSetting.cpp
)

if(NOT TARGET glm_shared)
    add_subdirectory(
            ${CPP_SOURCE_DIR}/thirdPart/glm/ glm.out
    )
endif()

target_include_directories(miscUtils PRIVATE ${CPP_SOURCE_DIR}/thirdPart/stbImage)

target_link_libraries(
        miscUtils
        glm_shared
        android
        EGL
        GLESv3
        log
)