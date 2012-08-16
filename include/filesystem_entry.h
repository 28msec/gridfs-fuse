#pragma once

#include <mongo/client/gridfs.h>
#include <mongo/client/connpool.h>
#include <sys/stat.h>

/**
 * macro that should be used to synchonize updates
 * if updates are not synchronized errors will be raised by the filesystem. E.g. if the 
 * filesystem creates a file and checks the attributes right after that the file eventually
 * won't exist because the write operation didn't finish. Therefore, updates must be 
 * synchronized.
 *
 * the getLastErrorDetailed function can be used for synchronization because it waits for
 * the operation to finish.
 */
#define GRIDFS_SYNCHRONIZE_UPDATE \
  mongo::BSONObj lErrorObj = client().getLastErrorDetailed(); \
  const bool lHasError = ! lErrorObj.getField("err").isNull(); \
  if (lHasError){ \
    std::stringstream lErrorMsg; \
    lErrorMsg << "An update operation failed: " << lErrorObj.jsonString(); \
    throw std::runtime_error(lErrorMsg.str()); \
  } \
 
namespace gridfs {

  class FilesystemEntry
  {
    public:
      FilesystemEntry(const std::string& aPath);

      virtual
      ~FilesystemEntry();

      const std::string&
      path() { return thePath; }

      bool
      exists(){ return gridfile().exists(); } 

      void
      stat(struct stat *stbuf);

      void
      stat(mongo::GridFile& gridfile, struct stat *stbuf);

      void 
      create(mode_t mode, uid_t uid, gid_t gid, const std::string content="");

      void 
      remove();

      void 
      chown(uid_t uid, gid_t gid);

      void
      chmod(mode_t mode);

      void
      utimes(time_t time);

      void
      force_reload();

    protected:

      mongo::GridFile&
      gridfile();

      mongo::GridFS&
      gridfs(){ return theGridFS; };

      void 
      stat(mongo::GridFile& gridfile, mode_t& mode, uid_t& uid, gid_t& gid, time_t& time);

      void 
      updateContentType(mongo::GridFile& gridfile, mode_t mode, uid_t uid, gid_t gid, time_t time);

      mongo::DBClientBase&
      client(){ return theClient; }

      std::string
      files_collection();

    private:
      // forbid copying because of the scoped connedction
      FilesystemEntry(const FilesystemEntry&);
      FilesystemEntry& operator=(const FilesystemEntry&);

      // no default constructor
      FilesystemEntry();
  
      const std::string               thePath;
      mongo::ScopedDbConnection       theConnection;
      mongo::DBClientBase&            theClient;
      mongo::GridFS                   theGridFS;
      std::auto_ptr<mongo::GridFile>  theGridFile; // can also be a link or dir
  };

}
