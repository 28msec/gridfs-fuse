/*
 * Copyright 2012 28msec, Inc.
 */
#include "gridfs_fuse.h"
#include "proc.h"

#include <syslog.h>
#include <cassert>
#include <sstream>
#include <stdlib.h>
#include <libmemcached/memcached.h>
#include <libmemcached/util/pool.h>

namespace gridfs {

  void
  Proc::list(void* buf, fuse_fill_dir_t filler) const
  {
    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);
    switch (theType)
    {
    case ROOT:
      filler(buf, "instances", NULL, 0);
      break;
    case INSTANCES:
      listServers(buf, filler);
      break;
    default: assert(false);
    }

  }

  void
  Proc::listServers(void* buf, fuse_fill_dir_t filler) const
  {
    memcached_st* lMaster = FUSE.master();
    uint32_t lNumServers = memcached_server_count(lMaster);
    for (uint32_t i = 0; i < lNumServers; ++i)
    {
      memcached_server_instance_st lInstance =
        memcached_server_instance_by_position(lMaster, i);
      std::ostringstream lServerName;
      lServerName
        << memcached_server_name(lInstance)
        << ":"
        << memcached_server_port(lInstance);
      filler(buf, lServerName.str().c_str(), NULL, 0);
    }
  }

  void
  Proc::stat(struct stat* aStBuf) const
  {
    switch (theType)
    {
    case ROOT:
    case INSTANCES:
      aStBuf->st_mode  = S_IFDIR;
      break;
    case LIST_INSTANCES:
      aStBuf->st_mode  = S_IFREG | 0000;
      break;
    default: assert(false);
    }

    aStBuf->st_uid = FUSE.config.default_uid;
    aStBuf->st_gid = FUSE.config.default_gid;
  }

  bool
  Proc::create()
  {
    std::string lNewServer = thePath.substr(16);
    int lNewPort = 11211;

    size_t lIndexOfColon = lNewServer.find_last_of(':');
    if (lIndexOfColon != std::string::npos)
    {
      if (lIndexOfColon >= lNewServer.length() - 1)
        return false;

      std::string lTmp = lNewServer.substr(lIndexOfColon + 1);
      lNewServer = lNewServer.substr(0, lIndexOfColon);
      lNewPort = atoi(lTmp.c_str());
    }

    syslog(LOG_DEBUG, "adding server %s and port %i",
        lNewServer.c_str(), lNewPort);

    memcached_st* lMaster = FUSE.master();
    uint32_t lNumServers = memcached_server_count(lMaster);
    for (uint32_t i = 0; i < lNumServers; ++i)
    {
      memcached_server_instance_st lInstance =
        memcached_server_instance_by_position(lMaster, i);

      const char* lName = memcached_server_name(lInstance);
      int lPort =  memcached_server_port(lInstance);

      if (lNewServer == lName && lNewPort == lPort)
        return false;
    }
    memcached_pool_st* lPool = FUSE.pool();
    memcached_server_add(lMaster, lNewServer.c_str(), lNewPort);
    memcached_pool_behavior_set(lPool, MEMCACHED_BEHAVIOR_SORT_HOSTS, 1);
    return true;
  }

}
