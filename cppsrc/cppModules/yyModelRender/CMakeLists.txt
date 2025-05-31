cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

get_filename_component(FOLDER_NAME ${CMAKE_CURRENT_LIST_DIR} NAME)
get_filename_component(PARENT_DIR ${CMAKE_CURRENT_LIST_DIR} PATH)
get_filename_component(CATEGORY_NAME ${PARENT_DIR} NAME)


add_library(
        yyModelRender
        STATIC
        yyModelRender.cpp
        yyModelRender.h
)

if(NOT TARGET cppUtils)
    add_subdirectory(
        ${CPP_SOURCE_DIR}/cppUtils cppUtils.out
    )
endif()

target_include_directories(yyModelRender PUBLIC ${CPP_SOURCE_DIR}/cppModules/
                                                ${CPP_SOURCE_DIR}/cppUtils/
                                                ${CPP_SOURCE_DIR}/cppModules/yyModelRender/)

target_link_libraries(
        yyModelRender
        cppUtils
)

if (MSVC)
set_target_properties(yyModelRender PROPERTIES 
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
endif()