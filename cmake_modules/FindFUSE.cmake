# Find the FUSE includes and library
#
#  FUSE_INCLUDE_DIR - where to find sqlite3.h, etc.
#  FUSE_LIBRARIES   - List of libraries when using SQLite.
#  FUSE_FOUND       - True if FUSE lib is found.

# check if already in cache, be silent
IF (FUSE_INCLUDE_DIR)
	SET (FUSE_FIND_QUIETLY TRUE)
ENDIF (FUSE_INCLUDE_DIR)

# find includes
FIND_PATH (FUSE_INCLUDE_DIR fuse.h
  /usr/local/include
  /usr/include
  /usr/local/include/osxfuse
  /opt/local/include
)

# find lib
IF (APPLE)
	SET(CMAKE_FIND_FRAMEWORK LAST)
	SET(FUSE_NAMES osxfuse fuse)
ELSE (APPLE)
	SET(FUSE_NAMES fuse)
ENDIF (APPLE)
FIND_LIBRARY(FUSE_LIBRARY
	NAMES ${FUSE_NAMES}
	PATHS /usr/lib /usr/local/lib /opt/local/lib
)

# check if lib was found and include is present
IF (FUSE_INCLUDE_DIR AND FUSE_LIBRARY)
	SET (FUSE_FOUND TRUE)
	SET (FUSE_LIBRARIES ${FUSE_LIBRARY})
ELSE (FUSE_INCLUDE_DIR AND FUSE_LIBRARY)
	SET (FUSE_FOUND FALSE)
	SET (FUSE_LIBRARIES)
ENDIF (FUSE_INCLUDE_DIR AND FUSE_LIBRARY)

# let world know the results
IF (FUSE_FOUND)
	IF (NOT FUSE_FIND_QUIETLY)
		MESSAGE(STATUS "Found FUSE: ${FUSE_LIBRARY}")
	ENDIF (NOT FUSE_FIND_QUIETLY)
ELSE (FUSE_FOUND)
	IF (FUSE_FIND_REQUIRED)
		MESSAGE(STATUS "Looked for FUSE libraries named ${FUSE_NAMES}.")
		MESSAGE(FATAL_ERROR "Could NOT find FUSE library")
	ENDIF (FUSE_FIND_REQUIRED)
ENDIF (FUSE_FOUND)
