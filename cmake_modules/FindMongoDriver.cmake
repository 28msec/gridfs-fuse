# Find the Mongo C++ includes includes and library
#
#  MONGO_INCLUDE_DIR - where to find sqlite3.h, etc.
#  MONGO_LIBRARIES   - List of libraries when using SQLite.
#  MONGO_FOUND       - True if MONGO lib is found.

# check if already in cache, be silent
IF (MONGO_INCLUDE_DIR)
  SET (MONGO_FIND_QUIETLY TRUE)
ENDIF (MONGO_INCLUDE_DIR)

# find includes
FIND_PATH (MONGO_INCLUDE_DIR mongo/client/dbclient.h
  /usr/local/include
  /usr/include
  /opt/local/include
)

SET(MONGO_NAMES mongoclient)
FIND_LIBRARY(MONGO_LIBRARY
  NAMES ${MONGO_NAMES}
	PATHS /usr/lib /usr/local/lib /opt/local/lib
)

# check if lib was found and include is present
IF (MONGO_INCLUDE_DIR AND MONGO_LIBRARY)
  SET (MONGO_FOUND TRUE)
  SET (MONGO_LIBRARIES ${MONGO_LIBRARY})
ELSE (MONGO_INCLUDE_DIR AND MONGO_LIBRARY)
  SET (MONGO_FOUND FALSE)
  SET (MONGO_LIBRARIES)
ENDIF (MONGO_INCLUDE_DIR AND MONGO_LIBRARY)

# let world know the results
IF (MONGO_FOUND)
  IF (NOT MONGO_FIND_QUIETLY)
    MESSAGE(STATUS "Found MONGO: ${MONGO_LIBRARY}")
  ENDIF (NOT MONGO_FIND_QUIETLY)
ELSE (MONGO_FOUND)
  IF (MONGO_FIND_REQUIRED)
    MESSAGE(STATUS "Looked for MONGO libraries named ${MONGO_NAMES}.")
    MESSAGE(FATAL_ERROR "Could NOT find MONGO library")
  ENDIF (MONGO_FIND_REQUIRED)
ENDIF (MONGO_FOUND)

