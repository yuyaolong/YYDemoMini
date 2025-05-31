# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)
include(cppsrc/cmake/global.cmake)
include(cppsrc/cmake/utils.cmake)
message(WARNING "${CMAKE_VS_PLATFORM_NAME_DEFAULT}!!")
message(WARNING "${CMAKE_GENERATOR_PLATFORM}!!")
set(CPP_SOURCE_DIR ${CMAKE_SOURCE_DIR}/cppsrc)

set(GLOBAL_LINK_OS_LIBS)
set(GLOBAL_INCLUDE_OS_FOLDER)

# Check for x86 or x64 architecture
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(STATUS "64-bit architecture detected")
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    message(STATUS "32-bit architecture detected")
endif()
 
if(${BUILD_TARGET_PLATFORM} MATCHES "Android")
    message(WARNING "!!Android build")

    
    project(ndkDemos)
    add_compile_options(-Werror=return-type)
    
    add_definitions(-DGR_USE_MOCK_AAFT_SLAM_BLEND)
    
    if(NOT TARGET glm_shared)
    add_subdirectory(
            ${CPP_SOURCE_DIR}/thirdPart/glm/ glm.out
    )
    endif()
    list(APPEND GLOBAL_LINK_OS_LIBS android EGL GLESv3 log glm_shared)
    list(APPEND GLOBAL_INCLUDE_OS_FOLDER ${CPP_SOURCE_DIR}/thirdPart/)
    include_directories(${CPP_SOURCE_DIR}/thirdPart/glm/)

    add_subdirectory(${CPP_SOURCE_DIR}/apps/foveation)

    add_subdirectory(${CPP_SOURCE_DIR}/apps/iblCube)

    add_subdirectory(${CPP_SOURCE_DIR}/apps/gaussianSplattingTest)

endif()

if(${BUILD_TARGET_PLATFORM} MATCHES "Windows")
    # cmake -G "Visual Studio 17 2022" -S ./cppsrc/ -B ./vsProject/
    # -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake already in global.cmake
    message(WARNING "!!Windows build")
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
    set(CMAKE_TOOLCHAIN_FILE "vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
    set(PROJECT_NAME YYDemo)
    project(${PROJECT_NAME} LANGUAGES CXX)

    find_package(glfw3 CONFIG REQUIRED)
    find_package(glad CONFIG REQUIRED)
    find_package(glm CONFIG REQUIRED)   
    find_package(assimp CONFIG REQUIRED)
    list(APPEND GLOBAL_LINK_OS_LIBS glfw glad::glad glm::glm assimp::assimp)

    add_subdirectory(${CPP_SOURCE_DIR}/apps/winDeferShadingTest)
    add_subdirectory(${CPP_SOURCE_DIR}/apps/winMainTest)
endif()
