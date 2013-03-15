#include "filesystem_entry.h"
#include "gridfs_fuse.h"

#include <cassert>
#include <ctime>
#include <stdexcept>
#include <sstream>
#include "mongo/bson/bsonobj.h"

namespace gridfs {

  FilesystemEntry::FilesystemEntry(const std::string& aPath):
    thePath(aPath),
    theConnection(FUSE.connection_string()),
    theGridFS(
        *theConnection.get(),
        FUSE.config.mongo_db,
        FUSE.config.mongo_collection_prefix)
  {
  }

  FilesystemEntry::~FilesystemEntry()
  {
    // put it back into the connection pool
    theConnection.done();
  }
  
  mongo::GridFile&
  FilesystemEntry::gridfile()
  {
    // fetch the file lazyly only if needed
    if (theGridFile.get()==0)
    {
      theGridFile.reset(new mongo::GridFile(theGridFS.findFile(thePath)));
    }
    return *(theGridFile.get());
  }

  void
  FilesystemEntry::stat(struct stat *stbuf)
  {
    stat(gridfile(), stbuf);
  }

  void
  FilesystemEntry::stat(mongo::GridFile& gridfile, struct stat *stbuf)
  {
    // sanity
    assert(gridfile.exists());
    memset(stbuf, 0, sizeof(struct stat));

    // now set the stat fields we know
    time_t lTime;
    stat(gridfile, stbuf->st_mode, stbuf->st_uid, stbuf->st_gid, lTime);
    stbuf->st_mtime = lTime;
    stbuf->st_atime = lTime; // not correct, but for completeness
    stbuf->st_ctime = lTime; // not correct, but for completeness
    
    // set number of links not too accurate
    if (stbuf->st_mode & S_IFDIR)
      stbuf->st_nlink = 2;
    else
      stbuf->st_nlink = 1;

    // set size correctly for files and links otherwise put blocksize for dirs
    if (stbuf->st_mode & S_IFDIR)
      stbuf->st_size = 4096;
    else 
      stbuf->st_size =  gridfile.getContentLength();
  }

  void
  FilesystemEntry::create(
      mode_t mode,
      uid_t uid,
      gid_t gid,
      const std::string& content)
  {
    // we use the content type to store some extra meta data
    std::stringstream lContentType;    
    lContentType << "m:" << mode << "|u:" << uid << "|g:" << gid << "|t:" << time(0);
    const char* data = content.c_str();
    size_t length = content.length();

    gridfs().storeFile(data, length, path(), lContentType.str());    

    synchonizeUpdate();

    //force reload
    force_reload();
  }

  void
  FilesystemEntry::remove()
  {
    gridfs().removeFile(path());

    synchonizeUpdate();
  }

  void
  FilesystemEntry::chown(uid_t uid, gid_t gid)
  {
    mode_t st_mode;
    uid_t dummy_uid;// will not be used
    gid_t dummy_gid;// will not be used
    time_t st_time;
    stat(gridfile(), st_mode, dummy_uid, dummy_gid, st_time); // we need the mode/time to leave it unchanged

    updateContentType(gridfile(), st_mode /*unchanged*/, uid, gid, st_time /*unchanged*/);
  }

  void
  FilesystemEntry::chmod(mode_t mode)
  {
    mode_t dummy_mode;// will not be used
    uid_t st_uid;
    gid_t st_gid;
    time_t st_time;
    stat(gridfile(), dummy_mode, st_uid, st_gid, st_time); // we need the uid/gid/time to leave it unchanged

    updateContentType(gridfile(), mode, st_uid /*unchanged*/, st_gid /*unchanged*/, st_time /*unchanged*/);
  }

  void
  FilesystemEntry::utimes(time_t time)
  {
    mode_t st_mode;
    uid_t st_uid;
    gid_t st_gid;
    time_t dummy_time; // will not be used
    stat(gridfile(), st_mode, st_uid, st_gid, dummy_time); // we need the mode/uid/gid to leave it unchanged

    updateContentType(gridfile(), st_mode /*unchanged*/, st_uid /*unchanged*/, st_gid /*unchanged*/, time);
  }

  void
  FilesystemEntry::force_reload()
  {
    theGridFile.reset();
  }

  void
  FilesystemEntry::stat(mongo::GridFile& gridfile, mode_t& mode, uid_t& uid, gid_t& gid, time_t& time)
  {
    // get mode, uid and gid from contenttype
    // if not (mis)used as such, use defaults
    mode = S_IFREG | 0644; // defaults to a file because that is only supported natively in mongo gridfs
    uid = FUSE.config.default_uid; // default
    gid = FUSE.config.default_gid; // default
    std::string lContentType = gridfile.getContentType();
    char* lContentTypeCopy = new char[lContentType.size()+1]; // strtok wants char* not const char*
    std::copy(lContentType.begin(), lContentType.end(), lContentTypeCopy);
    lContentTypeCopy[lContentType.size()] = '\0';
    char* tok = strtok(lContentTypeCopy,":|"); // we expect e.g. "m:16877|u:0|g:0|t:1321927291"
    while(tok != NULL)
    {
      // should be m, u or g
      char type = tok[0];
      switch (type)
      {
        case 'm':
          // found st_mode -> fetch value:
          tok = strtok(NULL,":|");
          mode = atoi(tok);
          break;
        case 'u':
          // found owner uid -> fetch value:
          tok = strtok(NULL,":|");
          uid = atoi(tok);
          break;
        case 'g':
          // found owner gid -> fetch value:
          tok = strtok(NULL,":|");
          gid = atoi(tok);
          break;
        case 't':
          // found timestamp -> fetch value:
          tok = strtok(NULL,":|");
          time = atoi(tok);
          break;
      }
      tok = strtok(NULL,":|");
    }

    // cleanup
    delete[] lContentTypeCopy;
    lContentTypeCopy = NULL;
  }

  void
  FilesystemEntry::updateContentType(mongo::GridFile& gridfile, 
                                     mode_t mode, 
                                     uid_t uid, 
                                     gid_t gid,
                                     time_t time)
  {
    std::stringstream lContentType;    
    lContentType << "m:" << mode << "|u:" << uid << "|g:" << gid << "|t:" << time;

    // create update filter and query
    mongo::OID id(gridfile.getFileField("_id").OID().str());
    mongo::Query filter = QUERY("_id" << id);
    mongo::BSONObj update = BSON( "$set" 
                               << BSON ( "contentType" << lContentType.str() ));

    // update it
    // TODO DK this is not multi process safe because it doesn't store a new file 
    //         entry, but don't see a better solution yet.
    theConnection->update(filesCollection(), filter, update);

    synchonizeUpdate();
  }

  std::string
  FilesystemEntry::filesCollection()
  {
    return std::string(FUSE.config.mongo_db) + "." + 
      FUSE.config.mongo_collection_prefix +
      ".files";
  }

  /**
   * if updates are not synchronized errors will be raised by the filesystem.
   * e.g. if the filesystem creates a file and checks the attributes right
   * after that the file eventually won't exist because the write operation
   * didn't finish. Therefore, updates must be synchronized.
   *
   * the getLastErrorDetailed function can be used for synchronization
   * because it waits for the operation to finish.
   */
  void
  FilesystemEntry::synchonizeUpdate()
  {
    mongo::BSONObj lErrorObj = theConnection->getLastErrorDetailed();
    if (lErrorObj.getField("err").ok() && !lErrorObj.getField("err").isNull())
    {
      std::stringstream lErrorMsg;
      lErrorMsg << "An update operation failed: " << lErrorObj.jsonString();
      throw std::runtime_error(lErrorMsg.str());
    }
  }

}
