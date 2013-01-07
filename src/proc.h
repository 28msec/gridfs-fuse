/*
 * Copyright 2012 28msec, Inc.
 */
#pragma once

#include "gridfs_fuse.h"
#include <mongo/client/gridfs.h>
#include <mongo/client/connpool.h>
#include <sys/stat.h>
#include <string>

namespace gridfs {

  class Proc
  {
  public:
    Proc(const std::string& aPath)
      : thePath(aPath)
    {
      static std::string lProc
        = std::string(FUSE.config.path_prefix) + "/proc";
      static std::string lProcInst
        = std::string(FUSE.config.path_prefix) + "/proc/instances";
      static std::string lProcInstList
        = std::string(FUSE.config.path_prefix) + "/proc/instances/";

      // needed for substr
      thePrefixLength = lProcInstList.length();

      if (aPath == lProc)
        theType = ROOT;
      else if (aPath == lProcInst)
        theType = INSTANCES;
      else if (aPath.find(lProcInstList) == 0)
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
    int thePrefixLength;
  };

}

