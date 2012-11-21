#include "symlink.h"

#include <sstream>

namespace gridfs {

  void
  Symlink::read(char* link, size_t size)
  {
    size_t length = gridfile().getContentLength();
    std::stringstream linkstream;
    gridfile().write(linkstream);
    
    // If the linkname is too long to fit in the buffer, it should be truncated.
    // The buffer size argument includes the space for the terminating null character.
    size_t aligned_length = (length >= size)?( size - 1 ):length;

    memcpy(link, linkstream.str().c_str(), aligned_length);
    link[aligned_length] = '\0';
  }

}
