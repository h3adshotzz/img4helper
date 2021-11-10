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

# Check this module should be built
#if (NOT LIBHELPER_)

target_include_directories(img4helper
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}../
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_sources(img4helper
    PUBLIC
        main.c
        asn1.c
        image4.c
)
