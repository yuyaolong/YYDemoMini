cmake_minimum_required(VERSION 3.4.1)

add_library(
        specularBRDFIntegrationMap
        SHARED
        specularBRDFIntegrationMap.cpp
)

if(NOT TARGET miscUtils)
    add_subdirectory(
            ${CPP_SOURCE_DIR}/modules/miscUtils miscUtils.out
    )
endif()

target_include_directories(specularBRDFIntegrationMap PRIVATE ${CPP_SOURCE_DIR}/modules/miscUtils/
                                                    ${CPP_SOURCE_DIR}/modules/heads/)


target_link_libraries(
        specularBRDFIntegrationMap
        miscUtils
)