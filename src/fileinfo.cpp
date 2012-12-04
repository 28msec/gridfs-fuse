#include "fileinfo.h"

#include <cassert>
#include "directory.h"
#include "proc.h"
#include "file.h"

namespace gridfs
{

  FileInfo::~FileInfo()
  {
    switch (type)
    {
    case PROC: delete proc; break;
    case DIRECTORY: delete directory; break;
    case FILE: delete file; break;
    default: assert(false);
    }
  }
}
