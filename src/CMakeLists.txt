 #*************************************************************************
 #   CMakeLists.txt  --  This file is part of MKMCXX.                     *
 #                                                                        *
 #   Copyright (C) 2016, Ivo Filot                                        *
 #                                                                        *
 #   MKMCXX is free software: you can redistribute it and/or modify       *
 #   it under the terms of the GNU General Public License as published    *
 #   by the Free Software Foundation, either version 3 of the License,    *
 #   or (at your option) any later version.                               *
 #                                                                        *
 #   MKMCXX is distributed in the hope that it will be useful,            *
 #   but WITHOUT ANY WARRANTY; without even the implied warranty          *
 #   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.              *
 #   See the GNU General Public License for more details.                 *
 #                                                                        *
 #   You should have received a copy of the GNU General Public License    *
 #   along with this program.  If not, see http://www.gnu.org/licenses/.  *
 #                                                                        *
 #*************************************************************************/

# set minimum cmake requirements
cmake_minimum_required(VERSION 2.8)
project (mkmcxx)

# add custom directory to look for .cmake files
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake/modules )

# add OS specific
if(APPLE)
    add_definitions(-D_APPLE)
    SET(BOOST_INCLUDEDIR "/opt/local/include")
    SET(BOOST_LIBRARYDIR "/opt/local/lib")
elseif(UNIX AND NOT APPLE)
    add_definitions(-Wno-literal-suffix)
    SET(BOOST_INCLUDEDIR "/usr/include")
    SET(BOOST_LIBRARYDIR "/usr/lib/x86_64-linux-gnu")
elseif(WIN32)
    SET(BOOST_ROOT "../../../libraries/boost-1.64.0-win-x64")
endif()

# set Boost
set (Boost_NO_SYSTEM_PATHS ON)
set (Boost_USE_MULTITHREADED ON)
set (Boost_USE_STATIC_LIBS ON)
set (Boost_USE_STATIC_RUNTIME OFF)
set (BOOST_ALL_DYN_LINK OFF)

# Include libraries
find_package(Boost COMPONENTS system filesystem regex REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(CAIRO cairo REQUIRED)
pkg_check_modules(EIGEN eigen3 REQUIRED)

# Set include folders
include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_BINARY_DIR}
                    ${Boost_INCLUDE_DIRS}
                    ${CAIRO_INCLUDE_DIRS}
                    ${EIGEN_INCLUDE_DIRS})

if(APPLE)
    link_directories(${CAIRO_LIBDIR})
endif()

# Add sources
file(GLOB SOURCES "*.cpp")

# Set C++11
add_definitions(-std=c++14)
add_definitions(-march=native)
if(UNIX AND NOT APPLE)
    # as of Debian Stretch (9.0), the default building position independent executables, to revert
    # back to the old ways, use the settings below:
    SET( CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -no-pie" )
ENDIF()

# Set executable
add_executable(svg2cairo ${SOURCES})

# Link libraries
target_link_libraries(svg2cairo ${CAIRO_LIBRARIES} ${Boost_LIBRARIES})
