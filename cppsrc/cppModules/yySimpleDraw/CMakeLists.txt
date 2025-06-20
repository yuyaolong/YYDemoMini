cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

get_filename_component(FOLDER_NAME ${CMAKE_CURRENT_LIST_DIR} NAME)
get_filename_component(PARENT_DIR ${CMAKE_CURRENT_LIST_DIR} PATH)
get_filename_component(CATEGORY_NAME ${PARENT_DIR} NAME)


add_library(
        yySimpleDraw
        STATIC
        yySimpleDraw.cpp
        yySimpleDraw.h
)

if(NOT TARGET cppUtils)
    add_subdirectory(
        ${CPP_SOURCE_DIR}/cppUtils cppUtils.out
    )
endif()

target_include_directories(yySimpleDraw PUBLIC ${CPP_SOURCE_DIR}/cppModules/
                                                ${CPP_SOURCE_DIR}/cppUtils/
                                                ${CPP_SOURCE_DIR}/cppModules/yySimpleDraw/)

target_link_libraries(
        yySimpleDraw
        cppUtils
)

if (MSVC)
set_target_properties(yySimpleDraw PROPERTIES 
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
endif()