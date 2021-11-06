##===----------------------------------------------------------------------===//
##
##                              Img4helper
##
##  This  document  is the property of "Is This On?" It is considered to be
##  confidential and proprietary and may not be, in any form, reproduced or
##  transmitted, in whole or in part, without express permission of Is This
##  On?.
##
##  Copyright (C) 2021, Is This On? Holdings
##  
##  Harry Moulton <me@h3adsh0tzz.com>
##
##===----------------------------------------------------------------------===//

cmake_minimum_required (VERSION 3.15)

############################ CONFIGURATION #####################################

# Include a file from config/defaults.cmake which has default values.

# The default build type should be DEBUG. The build type can be set with
# -DCMAKE_BUILD_TYPE.
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "DEBUG" CACHE STRING "Build type: RELEASE, DEBUG" FORCE)
endif()

#   TODO: https://stackoverflow.com/questions/7900661/how-to-read-a-cmake-variable-in-c-source-code
#
set(IMG4HELPER_VERSION_NUMBER "2.0.0" CACHE STRING "Build Version" FORCE)
set(IMG4HELPER_VERSION_TAG "DEVELOPMENT" CACHE STRING "Build Type" FORCE)

############################## PROJECT #########################################

# Declare the project
project("Img4helper" VERSION ${IMG4HELPER_VERSION_NUMBER} LANGUAGES C)

# Declare the executable
add_executable (img4helper)

# Add Libhelper
add_subdirectory(libhelper)

# HTool public headers.
target_include_directories(img4helper
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        #${CMAKE_SOURCE_DIR}/libhelper/include
)
target_link_libraries(img4helper libhelper)

# All the source files are located in htool/
add_subdirectory(src/)

# Add other subdirectories
#add_subdirectory(tools)
#add_subdirectory(tests)

#include(config/install.cmake)
