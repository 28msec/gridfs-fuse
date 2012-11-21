#pragma once

#include "gridfs_fuse.h"
#include <sys/stat.h>

#include "filesystem_entry.h"

namespace gridfs {

  class Directory : public FilesystemEntry
  {
    public:
      Directory(const std::string& aPath) : FilesystemEntry(aPath){};     
 
      void 
      list(void* buf, fuse_fill_dir_t filler);
  
      bool
      isEmpty();

    private:

      const std::string
      pathregex();

      mongo::DBClientCursor*
      list();
  }; 

}

