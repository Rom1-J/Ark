cmake_minimum_required(VERSION 3.8)

project(sfml)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

find_package(SFML 2.5 REQUIRED graphics window system network audio)

include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${Ark_SOURCE_DIR}/thirdparty
    ${Ark_SOURCE_DIR}/include
)

file(GLOB_RECURSE SOURCE_FILES
    ${PROJECT_SOURCE_DIR}/src/*.cpp
)

list(REMOVE_ITEM SOURCE_FILES "${PROJECT_SOURCE_DIR}/../../src/main.cpp")

add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} PRIVATE ArkReactor)
target_link_libraries(${PROJECT_NAME} PRIVATE ${MPIR_LIBRARIES})
target_link_libraries(${PROJECT_NAME} PRIVATE ${SFML_LIBRARIES})
if (CMAKE_COMPILER_IS_GNUCXX)
    target_link_libraries(${PROJECT_NAME} PRIVATE stdc++fs)
endif()
if (UNIX OR LINUX)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${CMAKE_DL_LIBS})
endif()

set_target_properties(
    ${PROJECT_NAME}
    PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
)

if (MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
endif()

if (DEBUG MATCHES "true")
    if (CMAKE_COMPILER_IS_GNUCXX)
        set(CMAKE_CXX_FLAGS "-pg -g -no-pie")
    endif()
endif()

add_custom_command(TARGET ${PROJECT_NAME}
    COMMAND ${CMAKE_COMMAND} -E copy
        "$<TARGET_FILE:${PROJECT_NAME}>" ${PROJECT_SOURCE_DIR}/../../lib
)
