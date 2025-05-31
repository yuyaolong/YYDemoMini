cmake_minimum_required(VERSION 3.4.1)

file(GLOB SOURCES *.cpp)
file(GLOB HEADERS *.h)  

add_library(cppUtils SHARED ${SOURCES} ${HEADERS})

if (MSVC)
set_target_properties(cppUtils PROPERTIES 
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
endif()

target_compile_definitions(cppUtils PRIVATE CPPUTILS_EXPORTS)

target_include_directories(cppUtils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
                                           ${CPP_SOURCE_DIR}/thirdPart/stbImage/
                                           ${GLOBAL_INCLUDE_OS_FOLDER})


target_link_libraries(cppUtils PUBLIC
        ${GLOBAL_LINK_OS_LIBS})


