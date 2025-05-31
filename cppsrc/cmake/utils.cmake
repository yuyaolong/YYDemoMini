#[[
 Copyright (c) 2019-2021, Arm Limited and Contributors

 SPDX-License-Identifier: Apache-2.0

 Licensed under the Apache License, Version 2.0 the "License";
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.

 ]]

# auto recursive add folder
function(add_sub_dirs DIRECTORY)
    scan_dirs(
            DIR ${DIRECTORY}
            LIST DIR_LIST)
    foreach(CURR_DIR ${DIR_LIST})
        if(EXISTS "${DIRECTORY}/${CURR_DIR}/CMakeLists.txt")
            add_subdirectory(${DIRECTORY}/${CURR_DIR} ${CURR_DIR}.out)
            list(APPEND TOTAL_SAMPLE_ID_LIST ${CURR_DIR})
        else()
            add_sub_dirs(${DIRECTORY}/${CURR_DIR})
        endif()
        set(TOTAL_SAMPLE_ID_LIST ${TOTAL_SAMPLE_ID_LIST} PARENT_SCOPE)
    endforeach()
endfunction(add_sub_dirs)

function(scan_dirs)
    set(options)
    set(oneValueArgs LIST DIR)
    set(multiValueArgs)

    cmake_parse_arguments(TARGET "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT EXISTS ${TARGET_DIR})
        message(FATAL_ERROR "Directory not found `${TARGET_DIR}`")
    endif()

    # get all file and subfolder from ${TARGET_DIR} store in DIR_FILES
    # file function operate file and folder
    file(GLOB DIR_FILES RELATIVE ${TARGET_DIR} ${TARGET_DIR}/*)

    set(DIR_LIST)

    foreach(FILE_NAME ${DIR_FILES})
        if(IS_DIRECTORY ${TARGET_DIR}/${FILE_NAME})
            # list function operate list variable
            list(APPEND DIR_LIST ${FILE_NAME})
        endif()
    endforeach()

    set(${TARGET_LIST} ${DIR_LIST} PARENT_SCOPE)
endfunction()

function(add_project)
    set(options)
    set(oneValueArgs TYPE ID CATEGORY AUTHOR NAME DESCRIPTION)
    set(multiValueArgs TAGS FILES LIBS INCLUDES SHADERS_GLSL)

    cmake_parse_arguments(TARGET "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    message(STATUS "${TARGET_TYPE} `${TARGET_ID}` - BUILD")

    # create project (object target - reused by app target)
    project(${TARGET_ID} LANGUAGES C CXX)

    source_group("\\" FILES ${TARGET_FILES})

    # Add shaders to project group
    if (TARGET_SHADERS_GLSL)
        source_group("\\Shaders" FILES ${TARGET_SHADERS_GLSL})
    endif()

    # build static libs for module
    add_library(${PROJECT_NAME} STATIC ${TARGET_FILES} ${SHADERS_GLSL})

    get_filename_component(CMAKE_CURRENT_SOURCE_PARENT_DIR "${CMAKE_CURRENT_SOURCE_DIR}" DIRECTORY)
    message(WARNING CMAKE_CURRENT_SOURCE_PARENT_DIR: ${CMAKE_CURRENT_SOURCE_PARENT_DIR})

    message(WARNING "YY!!! ${TARGET_INCLUDES}")
    message(WARNING "YY!!!! ${TARGET_LIBS}")

    # inherit include directories from framework target
    target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
                                                      ${CMAKE_CURRENT_SOURCE_PARENT_DIR}
                                                      ${TARGET_INCLUDES})

    # Link against extra project specific libraries
    if(TARGET_LIBS)
        target_link_libraries(${PROJECT_NAME} PUBLIC ${TARGET_LIBS})
    endif()

    # set sample properties
    set_target_properties(${PROJECT_NAME}
            PROPERTIES
            SAMPLE_CATEGORY ${TARGET_CATEGORY}
            SAMPLE_AUTHOR ${TARGET_AUTHOR}
            SAMPLE_NAME ${TARGET_NAME}
            SAMPLE_DESCRIPTION ${TARGET_DESCRIPTION}
            SAMPLE_TAGS "${TARGET_TAGS}")
endfunction()


function(add_module_with_tags)
    set(options)
    set(oneValueArgs ID CATEGORY AUTHOR NAME DESCRIPTION)
    set(multiValueArgs TAGS FILES LIBS INCLUDES SHADER_FILES_GLSL)

    cmake_parse_arguments(TARGET "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    list(APPEND TARGET_TAGS "any")

    set(SRC_FILES
            ${TARGET_ID}.h
            ${TARGET_ID}.cpp)

    # Append extra files if present
    if (TARGET_FILES)
        list(APPEND SRC_FILES ${TARGET_FILES})
    endif()

    get_filename_component(PROJECT_SOURCE_PARENT_DIR "${PROJECT_SOURCE_DIR}" DIRECTORY)
    # Add GLSL shader files for this sample
    if (TARGET_SHADER_FILES_GLSL)
        list(APPEND SHADER_FILES_GLSL ${TARGET_SHADER_FILES_GLSL})
        foreach(SHADER_FILE_GLSL ${SHADER_FILES_GLSL})
            list(APPEND SHADERS_GLSL "${PROJECT_SOURCE_DIR}/shaders/${SHADER_FILE_GLSL}")
        endforeach()
    endif()

    add_project(
            TYPE "Module"
            ID ${TARGET_ID}
            CATEGORY ${TARGET_CATEGORY}
            AUTHOR ${TARGET_AUTHOR}
            NAME ${TARGET_NAME}
            DESCRIPTION ${TARGET_DESCRIPTION}
            TAGS ${TARGET_TAGS}
            FILES ${SRC_FILES}
            LIBS ${TARGET_LIBS}
            INCLUDES ${TARGET_INCLUDES}
            SHADERS_GLSL ${SHADERS_GLSL})

endfunction()