# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

add_library(iblCube SHARED iblCube.cpp)

if(NOT TARGET miscUtils)
    add_subdirectory(
            ${CPP_SOURCE_DIR}/modules/miscUtils miscUtils.out
    )
endif()


if(NOT TARGET glCamera)
add_subdirectory(
        ${CPP_SOURCE_DIR}/modules/glCamera/ glCamera.out
)
endif()

if(NOT TARGET equirToCubemap)
    add_subdirectory(
            ${CPP_SOURCE_DIR}/modules/IBL/equirToCubemap equirToCubemap.out
    )
endif()

if(NOT TARGET irradianceMap)
    add_subdirectory(
            ${CPP_SOURCE_DIR}/modules/IBL/irradianceMap irradianceMap.out
    )
endif()

if(NOT TARGET specularPrefilterMap)
    add_subdirectory(
            ${CPP_SOURCE_DIR}/modules/IBL/specularPrefilterMap specularPrefilterMap.out
    )
endif()

if(NOT TARGET specularBRDFIntegrationMap)
    add_subdirectory(
            ${CPP_SOURCE_DIR}/modules/IBL/specularBRDFIntegrationMap specularBRDFIntegrationMap.out
    )
endif()

if(NOT TARGET pbrRender)
    add_subdirectory(
            ${CPP_SOURCE_DIR}/modules/IBL/pbrRender pbrRender.out
    )
endif()

if(NOT TARGET environmentMap)
    add_subdirectory(
            ${CPP_SOURCE_DIR}/modules/IBL/environmentMap environmentMap.out
    )
endif()

if(NOT TARGET SH9Extract)
    add_subdirectory(
            ${CPP_SOURCE_DIR}/modules/IBL/SH9Extract SH9Extract.out
    )
endif()


if(NOT TARGET showOnScreen)
    add_subdirectory(
            ${CPP_SOURCE_DIR}/modules/showOnScreen/ showOnScreen.out
    )
endif()


target_include_directories(iblCube PRIVATE  ${CPP_SOURCE_DIR}/modules/miscUtils/
                                            ${CPP_SOURCE_DIR}/modules/glCamera/
                                            ${CPP_SOURCE_DIR}/modules/showOnScreen/
                                            ${CPP_SOURCE_DIR}/modules/IBL/pbrRender/
                                            ${CPP_SOURCE_DIR}/modules/IBL/environmentMap/
                                            ${CPP_SOURCE_DIR}/modules/IBL/irradianceMap/
                                            ${CPP_SOURCE_DIR}/modules/IBL/specularPrefilterMap/
                                            ${CPP_SOURCE_DIR}/modules/IBL/specularBRDFIntegrationMap/
                                            ${CPP_SOURCE_DIR}/modules/IBL/SH9Extract/
                                            ${CPP_SOURCE_DIR}/modules/IBL/equirToCubemap/)

target_link_libraries(iblCube
                      miscUtils
                      glCamera
                      showOnScreen
                      equirToCubemap
                      irradianceMap
                      environmentMap
                      specularPrefilterMap
                      specularBRDFIntegrationMap
                      SH9Extract
                      pbrRender)
