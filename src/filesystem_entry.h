#pragma once

#include <mongo/client/gridfs.h>
#include <mongo/client/connpool.h>
#include <sys/stat.h>

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
      exists() { return gridfile().exists(); } 

      void
      stat(struct stat *stbuf);

      void
      stat(mongo::GridFile& gridfile, struct stat *stbuf);

      void 
      create(
          mode_t mode,
          uid_t uid,
          gid_t gid,
          const std::string& content);

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
      gridfs() { return theGridFS; };

      void 
      stat(
        mongo::GridFile& gridfile,
        mode_t& mode,
        uid_t& uid,
        gid_t& gid,
        time_t& time);

      void 
      updateContentType(
          mongo::GridFile& gridfile,
          mode_t mode,
          uid_t uid,
          gid_t gid,
          time_t time);

      std::string
      filesCollection();

      void
      synchonizeUpdate();

    private:
      // forbid copying because of the scoped connedction
      FilesystemEntry(const FilesystemEntry&);
      FilesystemEntry& operator=(const FilesystemEntry&);

      // no default constructor
      FilesystemEntry();
  
    protected:
      const std::string               thePath;
      mongo::ScopedDbConnection       theConnection;
      mongo::GridFS                   theGridFS;
      std::auto_ptr<mongo::GridFile>  theGridFile;
  };

}
