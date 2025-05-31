# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

add_library(foveationTest SHARED JNIFoveationeSetting.cpp)


if(NOT TARGET qcomFoveation)
add_subdirectory(
        ${CPP_SOURCE_DIR}/modules/qcomFoveation/ qcomFoveation.out
)
endif()

if(NOT TARGET foveationByVRS)
add_subdirectory(
        ${CPP_SOURCE_DIR}/modules/foveationByVRS/ foveationByVRS.out
)
endif()

if(NOT TARGET drawPoints)
add_subdirectory(
        ${CPP_SOURCE_DIR}/modules/drawPoints/ drawPoints.out
)
endif()

if(NOT TARGET showOnScreen)
add_subdirectory(
        ${CPP_SOURCE_DIR}/modules/showOnScreen/ showOnScreen.out
)
endif()

if(NOT TARGET drawGrid)
add_subdirectory(
        ${CPP_SOURCE_DIR}/modules/drawGrid/ drawGrid.out
)
endif()

if(NOT TARGET showOnScreenMultiview)
    add_subdirectory(
            ${CPP_SOURCE_DIR}/modules/showOnScreenMultiview/ showOnScreenMultiview.out
    )
endif()


target_include_directories(foveationTest PRIVATE ${CPP_SOURCE_DIR}/modules/qcomFoveation/
                                                ${CPP_SOURCE_DIR}/modules/showOnScreenMultiview/
                                                ${CPP_SOURCE_DIR}/modules/foveationByVRS/
                                                ${CPP_SOURCE_DIR}/modules/drawPoints/
                                                ${CPP_SOURCE_DIR}/modules/drawGrid/
                                                ${CPP_SOURCE_DIR}/modules/showOnScreen/
                                                ${CPP_SOURCE_DIR}/modules/miscUtils/)

# Specifies libraries CMake should link to your target library.

# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries(foveationTest
                      drawGrid
                      qcomFoveation
                      showOnScreenMultiview
                      foveationByVRS
                      drawPoints
                      showOnScreen
                      miscUtils
                      android
                      EGL
                      GLESv3
                      log)
