/*
 * Copyright 2012 28msec, Inc.
 */
#pragma once

#include <mongo/client/gridfs.h>
#include <mongo/client/connpool.h>
#include <sys/stat.h>
#include <string>
#include "gridfs_fuse.h"

namespace gridfs {

  class Proc
  {
  public:
    Proc(const std::string& aPath)
      : thePath(aPath)
    {
      if (aPath == "/proc")
        theType = ROOT;
      else if (aPath == "/proc/instances")
        theType = INSTANCES;
      else if (aPath.find("/proc/instances/") == 0)
        theType = LIST_INSTANCES;
    }

    void
    list(void* buf, fuse_fill_dir_t filler) const;

    void
    stat(struct stat* aStBuf) const;

    bool
    create();


  private:
    void
    listServers(void* buf, fuse_fill_dir_t filler) const;

    enum Type
    {
      ROOT = 0,
      INSTANCES = 1,
      LIST_INSTANCES = 2
    } theType;

    const std::string thePath;
  };

}

