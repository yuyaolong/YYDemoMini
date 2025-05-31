cmake_minimum_required(VERSION 3.4.1)

set(LIB_NAME imgui)

# Set the default library type to static
set(IMGUI_LIB_TYPE "STATIC")
# Optionally let users control the library type
option(IMGUI_BUILD_SHARED_LIB "Build imgui shared library instead of static" OFF)
if(IMGUI_BUILD_SHARED_LIB)
    set(IMGUI_LIB_TYPE "SHARED")
endif()

file(GLOB SOURCES *.cpp)

add_library(${LIB_NAME} ${IMGUI_LIB_TYPE} ${SOURCES})
target_compile_definitions(${LIB_NAME} PRIVATE IMGUI_IMPL_OPENGL_ES3)

message(WARNING ${LIB_NAME})
message(WARNING ${SOURCES})
message(WARNING ${IMGUI_LIB_TYPE})

target_link_libraries(${LIB_NAME} PRIVATE
        android
        EGL
        GLESv3
        log)