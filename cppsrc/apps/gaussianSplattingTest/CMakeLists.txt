# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

set(CUR_TEST_NAME gaussianSplattingTest)
if(NOT TARGET cppUtils)
    add_subdirectory(${CPP_SOURCE_DIR}/cppUtils cppUtils.out)
endif()

if(NOT TARGET imgui)
add_subdirectory(${CPP_SOURCE_DIR}/imgui imgui.out)
endif()

if(NOT TARGET yyGaussianSplatRender)
add_subdirectory(${CPP_SOURCE_DIR}/cppModules/yyGaussianSplatRender yyGaussianSplatRender.out)
endif()

if(NOT TARGET yySimpleDraw)
add_subdirectory(${CPP_SOURCE_DIR}/cppModules/yySimpleDraw yySimpleDraw.out)
endif()

add_library(${CUR_TEST_NAME} SHARED JNIGaussianSplattingTest.cpp)

target_link_libraries(${CUR_TEST_NAME}
                      PRIVATE
                      cppUtils
                      yyGaussianSplatRender
                      yySimpleDraw
                      imgui
                      android
                      EGL
                      GLESv3
                      log)
target_include_directories(${CUR_TEST_NAME} PUBLIC ${CPP_SOURCE_DIR}/imgui/)
