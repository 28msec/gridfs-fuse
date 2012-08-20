# Copyright 2010 28msec Inc.

INCLUDE_DIRECTORIES(BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/../include)
INCLUDE_DIRECTORIES(BEFORE ${CMAKE_CURRENT_BINARY_DIR}/../include)
INCLUDE_DIRECTORIES(${APR_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${MONGO_INCLUDE_DIR})

# apache ${CMAKE_SOURCE_DIR}/src/log4cxx
# only build it if apr was found
IF (APR_FOUND)
  OPTION(GRIDFS_LOG4CXX "Enable logging for GridFS-FUSE" ON)
  SET(LOG4CXX_SRCS
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/action.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/appenderattachableimpl.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/appenderskeleton.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/aprinitializer.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/asyncappender.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/basicconfigurator.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/bufferedwriter.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/bytearrayinputstream.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/bytearrayoutputstream.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/bytebuffer.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/cacheddateformat.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/charsetdecoder.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/charsetencoder.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/class.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/classnamepatternconverter.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/classregistration.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/condition.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/configurator.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/consoleappender.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/cyclicbuffer.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/dailyrollingfileappender.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/datagrampacket.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/datagramsocket.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/date.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/dateformat.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/datelayout.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/datepatternconverter.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/defaultconfigurator.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/defaultloggerfactory.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/defaultrepositoryselector.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/domconfigurator.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/exception.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/fallbackerrorhandler.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/file.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/fileappender.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/filedatepatternconverter.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/fileinputstream.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/filelocationpatternconverter.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/fileoutputstream.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/filerenameaction.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/filewatchdog.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/filter.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/filterbasedtriggeringpolicy.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/fixedwindowrollingpolicy.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/formattinginfo.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/fulllocationpatternconverter.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/gzcompressaction.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/hierarchy.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/htmllayout.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/inetaddress.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/inputstream.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/inputstreamreader.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/integer.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/integerpatternconverter.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/layout.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/level.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/levelmatchfilter.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/levelpatternconverter.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/levelrangefilter.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/linelocationpatternconverter.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/lineseparatorpatternconverter.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/literalpatternconverter.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/loader.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/locale.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/locationinfo.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/logger.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/loggerpatternconverter.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/loggingevent.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/loggingeventpatternconverter.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/loglog.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/logmanager.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/logstream.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/manualtriggeringpolicy.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/mdc.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/messagebuffer.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/messagepatternconverter.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/methodlocationpatternconverter.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/mutex.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/nameabbreviator.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/namepatternconverter.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/ndc.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/ndcpatternconverter.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/objectimpl.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/objectoutputstream.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/objectptr.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/obsoleterollingfileappender.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/odbcappender.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/onlyonceerrorhandler.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/optionconverter.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/outputstream.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/outputstreamwriter.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/patternconverter.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/patternlayout.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/patternparser.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/pool.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/properties.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/propertiespatternconverter.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/propertyconfigurator.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/propertyresourcebundle.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/propertysetter.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/reader.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/relativetimedateformat.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/relativetimepatternconverter.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/resourcebundle.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/rollingfileappender.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/rollingpolicy.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/rollingpolicybase.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/rolloverdescription.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/rootlogger.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/serversocket.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/simpledateformat.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/simplelayout.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/sizebasedtriggeringpolicy.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/smtpappender.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/socket.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/socketappender.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/socketappenderskeleton.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/sockethubappender.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/socketoutputstream.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/strftimedateformat.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/stringhelper.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/stringmatchfilter.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/stringtokenizer.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/synchronized.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/system.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/systemerrwriter.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/systemoutwriter.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/telnetappender.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/threadcxx.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/threadlocal.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/threadpatternconverter.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/threadspecificdata.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/throwableinformationpatternconverter.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/timebasedrollingpolicy.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/timezone.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/transcoder.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/transform.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/triggeringpolicy.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/ttcclayout.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/writer.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/writerappender.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/xmllayout.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/xmlsocketappender.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/zipcompressaction.cpp
  )
  
  IF (UNIX)
    LIST(APPEND LOG4CXX_SRCS
                ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/syslogappender.cpp
                ${CMAKE_CURRENT_SOURCE_DIR}/../src/log4cxx/syslogwriter.cpp)
  ENDIF (UNIX)
ENDIF (APR_FOUND)

SET(SRCS
  ${LOG4CXX_SRCS}
  ${CMAKE_CURRENT_SOURCE_DIR}/../src/filesystem_operations.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../src/config.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../src/filesystem_entry.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../src/directory.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../src/file.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../src/symlink.cpp
  main.cpp)

SET(GRIDFS_LIBS ${MONGO_LIBRARIES} ${FUSE_LIBRARIES} ${required-boost-libs})
IF(APR_FOUND)
  SET(GRIDFS_LIBS ${GRIDFS_LIBS} ${APR_LIBRARIES})
ENDIF(APR_FOUND)

ADD_EXECUTABLE(gridfs ${SRCS})
TARGET_LINK_LIBRARIES(gridfs ${GRIDFS_LIBS})

INSTALL(
  TARGETS gridfs 
  DESTINATION bin 
  COMPONENT GridFsFuse
)


