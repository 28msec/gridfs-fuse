#pragma once

#include <config.h>
#include <sys/stat.h>
#include <pthread.h>

#include <filesystem_entry.h>


namespace gridfs {

  class File : public FilesystemEntry
  {
    public:
      File(const std::string& aPath);
 
      virtual
      ~File();

      // does not write to mongo, writes only to virtual memory
      size_t
      write(const char * data, size_t size,off_t offset);

      // writes data to mongo
      void
      store();

      bool
      hasChanges(){ return theHasChanges; }

      size_t
      read(char *data, size_t size, off_t offset);

      void
      truncate();

    private:
     
      // this funkction must only be called with reads aligned to fit into one chunk
      size_t
      read(int chunkN, char *data, size_t size, off_t offset);
   
      void 
      init_memory();

      void 
      extend_memory();

      void
      free_memory();

      size_t theFileLength;
      unsigned int theChunkSize;
      size_t theWritten;
      size_t theCurrentDataSize;
      void* theData;
      bool theHasChanges;
      int theCachedChunkN;
      mongo::GridFSChunk theCachedChunk;

      pthread_mutex_t mutex_read;
  }; 

}

