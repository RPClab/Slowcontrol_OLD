########################################################
# Top-level CMakeLists.txt to build Raspberry Libraries
# @author LAGARDE Francois
cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
########################################################

# ----- Project name and version -----
project( RaspberrySensors)
set( ${PROJECT_NAME}_VERSION_MAJOR 1 )
set( ${PROJECT_NAME}_VERSION_MINOR 0 )
set( ${PROJECT_NAME}_VERSION_PATCH 0 )

# set the C++ standard to C++ 11
set(CMAKE_CXX_STANDARD 11)


set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake/)
SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wl,-rpath=${CMAKE_SOURCE_DIR}/bin/lib/" )
SET( CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath=${CMAKE_SOURCE_DIR}/bin/lib/" )
set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_PATH ${CMAKE_SOURCE_DIR}/bin/lib)


##I2C Sources
set(I2C_SRC ${PROJECT_SOURCE_DIR}/com/I2C/soft_i2c.cpp )
include_directories(${PROJECT_SOURCE_DIR}/com/I2C)

link_directories(${PROJECT_SOURCE_DIR}/bin/lib)

add_library(I2C STATIC ${I2C_SRC} ${INCLUDE_FILES})

## Install executable
install(TARGETS I2C ARCHIVE DESTINATION ${PROJECT_SOURCE_DIR}/bin/lib LIBRARY DESTINATION ${PROJECT_SOURCE_DIR}/bin/lib)

macro( set_if_not )
    if( NOT ${ARGV0} )
        set( ${ARGN} )
    endif()
endmacro()

set_if_not( mariadbpp_repository "https://github.com/RPClab/mariadbpp" )
set_if_not( mariadbpp_version "master" )

include(ExternalProject)

    # dependencies
    find_package(MariaDBClient REQUIRED)

    # ----- mariadb++ package -----
    ExternalProject_Add( 
                        mariadbpp
                        GIT_REPOSITORY ${mariadbpp_repository}
                        GIT_TAG ${mariadbpp_version}
                        GIT_PROGRESS TRUE
                        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
                        PREFIX ${CMAKE_CURRENT_BINARY_DIR}/mariadbpp-prefix
                        SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/mariadbpp
                        INSTALL_DIR ${CMAKE_INSTALL_PREFIX}
                        LIST_SEPARATOR %
                        UPDATE_COMMAND ""
                        )

add_subdirectory(source/)















