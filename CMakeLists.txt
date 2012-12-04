# Copyright 2012 28msec Inc.

PROJECT(GridFS)

CMAKE_MINIMUM_REQUIRED(VERSION 2.8.0)

# enable testing
INCLUDE(CTest)

IF (WIN32)
    MESSAGE(FATAL_ERROR "This is a fuse based filesystem that is not supported on windows. Sorry.")
ENDIF (WIN32)

IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE)
ENDIF(NOT CMAKE_BUILD_TYPE)

SET(SAUSALITO_BUILD_DIR $CMAKE_BUILD_DIR)
INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/CMakeConfiguration.txt)

########################################################################
# libfuse
########################################################################
INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules/FindFUSE.cmake)
IF(FUSE_FOUND)
    MESSAGE(STATUS "Found fuse library -- " ${FUSE_LIBRARIES})
    INCLUDE_DIRECTORIES(${FUSE_INCLUDE_DIR})
ELSE(FUSE_FOUND)
    MESSAGE(FATAL_ERROR "The fuse library is required in order to build GridFs.")
ENDIF(FUSE_FOUND)

########################################################################
# libmemcached
########################################################################
INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules/FindLibMemcached.cmake)
IF (LIBMEMCACHED_FOUND)
  MESSAGE(STATUS "Found libmemcached libraries -- " ${LIBMEMCACHED_LIBRARIES})
  INCLUDE_DIRECTORIES(${LIBMEMCACHED_INCLUDE_DIR})
ELSE(LIBMEMCACHED_FOUND)
    MESSAGE(FATAL_ERROR "The libmemcached library is required in order to build GridFs.")
ENDIF (LIBMEMCACHED_FOUND)

########################################################################
# mongodriver
########################################################################
INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules/FindMongoDriver.cmake)
IF(MONGO_FOUND)
    MESSAGE(STATUS "Found mongo library -- " ${MONGO_LIBRARIES})
    INCLUDE_DIRECTORIES(${MONGO_INCLUDE_DIR})
ELSE()
    MESSAGE(FATAL_ERROR "The mongo driver is required in order to build GridFs.")
ENDIF()

########################################################################
# boost
########################################################################
FIND_PACKAGE(Boost 1.42.0 COMPONENTS system)
IF(Boost_SYSTEM_FOUND)
  MESSAGE(STATUS "Found BOOST SYSTEM library -- " ${Boost_SYSTEM_LIBRARY})
  LIST(APPEND required-boost-libs ${Boost_SYSTEM_LIBRARY})
ELSE(Boost_SYSTEM_FOUND)
  # boost system is not required by all boost versions. Hence, we do not throw an error here.
  MESSAGE(FATAL_ERROR "The boost system library is required.")
ENDIF(Boost_SYSTEM_FOUND)

########################################################################
# MAIN BUILD
########################################################################

ADD_SUBDIRECTORY(bin)
ADD_SUBDIRECTORY(test)
ADD_SUBDIRECTORY(ppa)

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
ENDIF (UNIX AND NOT APPLE)

IF (APPLE)
  # use variable defined a few levels up
  SET(GridFs_PACKAGE_VERSION_ADDITION "-OSX_${OSX_VERSION}")
ENDIF (APPLE)

INCLUDE(CPack) # important to include that at the end (don't move up)
