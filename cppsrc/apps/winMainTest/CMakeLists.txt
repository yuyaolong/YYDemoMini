cmake_minimum_required(VERSION 3.13 FATAL_ERROR)
get_filename_component(CURRENT_FOLDER_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
add_executable(${CURRENT_FOLDER_NAME} main.cpp asyncLock.h)

#Set the working directory to the source of the project so developer dont have to
set_target_properties(${CURRENT_FOLDER_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/assets/)
#Configure output paths
set_target_properties(${CURRENT_FOLDER_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY  ${CMAKE_BINARY_DIR})
set_target_properties(${CURRENT_FOLDER_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY  ${CMAKE_BINARY_DIR})

if(NOT TARGET cppUtils)
    add_subdirectory(${CPP_SOURCE_DIR}/cppUtils cppUtils.out)
endif()

if(NOT TARGET yySimpleDraw)
add_subdirectory(${CPP_SOURCE_DIR}/cppModules/yySimpleDraw yySimpleDraw.out)
endif()

target_link_libraries(${CURRENT_FOLDER_NAME} PRIVATE glfw glad::glad cppUtils yySimpleDraw)
