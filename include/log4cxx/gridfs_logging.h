#pragma once

#include <iostream>
#include <log4cxx/config.h>
#ifdef GRIDFS_LOG4CXX
#  include <log4cxx/logger.h>
#  include <log4cxx/log4cxx.h>
#  include <log4cxx/logstring.h>
#  include <stdlib.h>
#  include "log4cxx/propertyconfigurator.h"
#  include "log4cxx/basicconfigurator.h"
#  include <log4cxx/helpers/exception.h>
#  include <log4cxx/ndc.h>
#  include <log4cxx/stream.h>
#  include <locale.h>
#  include <log4cxx/helpers/properties.h>
#endif

#ifdef GRIDFS_LOG4CXX
#  define GRIDFS_LOGCONFIG(key,value) \
    properties.setProperty(LOG4CXX_STR(key),LOG4CXX_STR(value)); 

#else
#  define GRIDFS_LOGCONFIG(key,value)
#endif

#ifdef GRIDFS_LOG4CXX
#  define GRIDFS_CONFIGURE_LOGGING(logfile, loglevel) \
      log4cxx::helpers::Properties properties; \
      std::string _log_level = loglevel; \
      std::string _log_file = logfile; \
      if (_log_file == "") { \
        GRIDFS_LOGCONFIG("log4j.rootLogger", "OFF ,A1" ) \
      } else { \
        GRIDFS_LOGCONFIG("log4j.rootLogger", _log_level + " ,A1" ) \
      } \
      if( _log_file == "console" || _log_file == "" ){\
        GRIDFS_LOGCONFIG("log4j.appender.A1","org.apache.log4j.ConsoleAppender") \
      }else if ( _log_file == "syslog" ) { \
        GRIDFS_LOGCONFIG("log4j.appender.A1","org.apache.log4j.SyslogAppender") \
      }else { \
        GRIDFS_LOGCONFIG("log4j.appender.A1","org.apache.log4j.DailyRollingFileAppender") \
        GRIDFS_LOGCONFIG("log4j.appender.A1.File",logfile) \
        GRIDFS_LOGCONFIG("log4j.appender.A1.DatePattern","'.'yyyy-MM-dd") \
      } \
      GRIDFS_LOGCONFIG("log4j.appender.A1.layout","org.apache.log4j.PatternLayout") \
      GRIDFS_LOGCONFIG("log4j.appender.A1.layout.ConversionPattern","%-5p %d{dd/MM HH:mm:ss,S} T:%t (%F:%L) %n   [%c{2}] - %m%n") \
      GRIDFS_LOGCONFIG("log4j.logger", _log_level) \
      log4cxx::PropertyConfigurator::configure(properties); \
      if( _log_level.compare("ALL") == 0 ) \
        log4cxx::Logger::getRootLogger()->setLevel(log4cxx::Level::getAll()); \
      if(_log_level.compare("DEBUG") == 0 ) \
        log4cxx::Logger::getRootLogger()->setLevel(log4cxx::Level::getDebug()); \
      if(_log_level.compare("INFO") == 0 ) \
        log4cxx::Logger::getRootLogger()->setLevel(log4cxx::Level::getInfo()); \
      if(_log_level.compare("WARN") == 0 ) \
        log4cxx::Logger::getRootLogger()->setLevel(log4cxx::Level::getWarn()); \
      if(_log_level.compare("ERROR") == 0 ) \
        log4cxx::Logger::getRootLogger()->setLevel(log4cxx::Level::getError()); \
      if(_log_level.compare("FATAL") == 0 ) \
        log4cxx::Logger::getRootLogger()->setLevel(log4cxx::Level::getFatal()); \
      if(_log_level.compare("OFF") == 0 ) \
        log4cxx::Logger::getRootLogger()->setLevel(log4cxx::Level::getOff()); \
      if(_log_file.compare("") == 0 ) \
        log4cxx::Logger::getRootLogger()->setLevel(log4cxx::Level::getOff()); 
#else
#  define GRIDFS_CONFIGURE_LOGGING(filename,loglevel) 
#endif

#ifdef GRIDFS_LOG4CXX
#  define GRIDFS_INIT_LOGGER(name) \
    log4cxx::LoggerPtr logger = log4cxx::Logger::getLogger(name);

#  define GRIDFS_INIT_LOGGER2(logger, name) \
    logger = log4cxx::Logger::getLogger(name);

#else
#  define GRIDFS_INIT_LOGGER(name)
#  define GRIDFS_INIT_LOGGER2(logger, name)
#endif

#ifdef GRIDFS_LOG4CXX
#  define _LOG(logger, message, level) \
    do { \
        log4cxx::logstream logstream(logger, level); \
        logstream << message << LOG4CXX_ENDMSG; \
    } while (0);

#endif

#ifdef GRIDFS_LOG4CXX
#  define _LOG_ERROR2(logger, message) \
  _LOG(logger, message, log4cxx::Level::getError());

#  define _LOG_ERROR(message) \
  _LOG_ERROR2(logger, message);

#else
#  define _LOG_ERROR(message)
#endif

#ifdef GRIDFS_LOG4CXX
#  define _LOG_INFO2(logger, message) \
  _LOG(logger, message, log4cxx::Level::getInfo());

#  define _LOG_INFO(message) \
  _LOG_INFO2(logger, message)

#else
#  define _LOG_INFO(message)
#  define _LOG_INFO2(logger, message)
#endif

#if (! defined NDEBUG) && (defined GRIDFS_LOG4CXX)
#  define _LOG_DEBUG2(logger, message) \
  _LOG(logger, message, log4cxx::Level::getDebug());

#  define _LOG_DEBUG(message) \
   _LOG_DEBUG2(logger, message)
#else
#  define _LOG_DEBUG(message)
#  define _LOG_DEBUG2(logger, message)
#endif

