# Copyright 2010 28msec Inc.

INCLUDE_DIRECTORIES(BEFORE ${CMAKE_SOURCE_DIR}/include)
INCLUDE_DIRECTORIES(BEFORE ${CMAKE_SOURCE_DIR}/src)

SET(SRCS
  ${CMAKE_SOURCE_DIR}/src/filesystem_operations.cpp
  ${CMAKE_SOURCE_DIR}/src/gridfs_fuse.cpp
  ${CMAKE_SOURCE_DIR}/src/auth_hook.cpp
  ${CMAKE_SOURCE_DIR}/src/filesystem_entry.cpp
  ${CMAKE_SOURCE_DIR}/src/directory.cpp
  ${CMAKE_SOURCE_DIR}/src/file.cpp
  ${CMAKE_SOURCE_DIR}/src/proc.cpp
  ${CMAKE_SOURCE_DIR}/src/fileinfo.cpp
  ${CMAKE_SOURCE_DIR}/src/symlink.cpp
  main.cpp)

SET(GRIDFS_LIBS ${MONGO_LIBRARIES} ${FUSE_LIBRARIES} ${required-boost-libs} ${LIBMEMCACHED_LIBRARIES})

ADD_EXECUTABLE(gridfs ${SRCS})
TARGET_LINK_LIBRARIES(gridfs ${GRIDFS_LIBS})

INSTALL(
  TARGETS gridfs 
  DESTINATION bin 
  COMPONENT GridFsFuse
)


