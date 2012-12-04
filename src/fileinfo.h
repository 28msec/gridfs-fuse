/*
 * Copyright 2012 28msec, Inc.
 * 
 */
#pragma once

namespace gridfs {

  class Proc;
  class Directory;
  class File;

  struct FileInfo
  {
    FileInfo()
      : type(DIRECTORY),
        directory(0)
    {}

    // takes ownership
    FileInfo(Directory* aDir)
      : type(DIRECTORY),
        directory(aDir)
    {}

    // takes ownership
    FileInfo(Proc* aProc)
      : type(PROC),
        proc(aProc)
    {}

    FileInfo(File* aFile)
      : type(FILE),
        file(aFile)
    {}

    ~FileInfo();

    enum Type {
      PROC = 0,
      DIRECTORY = 1,
      FILE = 2
    } type;
    union {
      Proc* proc;
      Directory* directory;
      File* file;
    };
  }; 

}


