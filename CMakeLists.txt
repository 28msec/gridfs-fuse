# Copyright 2010 28msec Inc.

IF (WIN32)
    MESSAGE(FATAL_ERROR "This is a fuse based filesystem that is not supported on windows. Sorry.")
ENDIF (WIN32)

SET(SAUSALITO_BUILD_DIR $CMAKE_BUILD_DIR)
INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/CMakeConfiguration.txt)


################################################################################
# find all required libraries
################################################################################
SET(DEBIAN_DEPENDENCIES)

########################################################################
# libfuse
########################################################################
INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules/FindFUSE.cmake)
IF(FUSE_FOUND)
    MESSAGE(STATUS "Found fuse library -- " ${FUSE_LIBRARIES})
    INCLUDE_DIRECTORIES(${FUSE_INCLUDE_DIR})
    LIST(APPEND DEBIAN_DEPENDENCIES "libfuse2")
ELSE(FUSE_FOUND)
    MESSAGE(FATAL_ERROR "The fuse library is required in order to build GridFs.")
ENDIF(FUSE_FOUND)

########################################################################
# MAIN BUILD
########################################################################

ADD_SUBDIRECTORY(include)
ADD_SUBDIRECTORY(bin)
ADD_SUBDIRECTORY(test)

########################################################################
# CPACK
########################################################################

IF(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
  SET(GridFs_PACKAGE_DEBUG_ADDITION "-debug")
ELSE(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
  SET(GridFs_PACKAGE_DEBUG_ADDITION "")
ENDIF(${CMAKE_BUILD_TYPE} STREQUAL "Debug")

IF (UNIX AND NOT APPLE)
  # use variables defined a few levels up
  SET(GridFs_PACKAGE_VERSION_ADDITION "-${LSB_DISTRIBUTION_ID}_${LSB_RELEASE_ID}_${ARCHITECTURE}")
  STRING(REPLACE ";" "," GridFs_DEBIAN_REQUIRED_DEPENDENCIES "${DEBIAN_DEPENDENCIES}")
ENDIF (UNIX AND NOT APPLE)

IF (APPLE)
  # use variable defined a few levels up
  SET(GridFs_PACKAGE_VERSION_ADDITION "-OSX_${OSX_VERSION}")
ENDIF (APPLE)

SET(GridFs_PACKAGE_FILE_NAME "GridFsFuse-@GRIDFS_MAJOR_VERSION@.@GRIDFS_MINOR_VERSION@.@GRIDFS_PATCH_VERSION@@GridFs_PACKAGE_DEBUG_ADDITION@@GridFs_PACKAGE_VERSION_ADDITION@")

CONFIGURE_FILE(
  "${CMAKE_CURRENT_SOURCE_DIR}/CPackGridFs.cpack.in" 
  "${CMAKE_CURRENT_BINARY_DIR}/CPackGridFs.cpack"
  IMMEDIATE
  @ONLY
)

INCLUDE(CPack) # important to include that at the end (don't move up)
