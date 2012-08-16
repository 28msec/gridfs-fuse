#pragma once

#include <filesystem_entry.h>
#include <config.h>

#include <sys/stat.h>

namespace gridfs {

  class Symlink : public FilesystemEntry
  {
    public:
      Symlink(const std::string& aPath):FilesystemEntry(aPath){};     

      void
      read(char* link, size_t size);
 
  }; 

}

