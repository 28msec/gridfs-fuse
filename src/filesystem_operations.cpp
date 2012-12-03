/*
 * Copyright 2012 28msec, Inc.
 */
#include "filesystem_operations.h"
#include <syslog.h>
#include "gridfs_fuse.h"
#include "filesystem_entry.h"
#include "directory.h"
#include "file.h"
#include "proc.h"
#include "symlink.h"
#include "fileinfo.h"

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <iostream>
#include <cassert>
#include <memory>


/**
 * macro that should be used instead of catch
 */
#define GRIDFS_CATCH \
  catch (mongo::UserException& u) { \
    syslog(LOG_ERR, "mongo exception: %s", u.getInfo().toString().c_str()); \
    result = -EIO;\
  }catch (std::exception& e) { \
    syslog(LOG_ERR, "std::exception: %s", e.what()); \
    result =- EIO;\
  }catch(...){ \
    syslog(LOG_ERR, "unknown exception"); \
    return -EIO; \
  }

namespace gridfs
{

// ############################################
  /********************************************* 
   * Helper functions
   */
  bool
  is_proc(const std::string& aPath, struct stat * aStBuf)
  {
    static std::string lProcPrefix = std::string(FUSE.config.path_prefix) + "/proc";
    return !aPath.compare(0, lProcPrefix.size(), lProcPrefix, 0, lProcPrefix.size());
  }

  void
  configure_path(const char* path, std::string& aRes)
  {
    aRes.reserve(256);
    aRes.append(FUSE.config.path_prefix);
    aRes.append(path);
    
    // remove trailing / otherwise the path is not found in mongo
    if (aRes.length() && aRes.at(aRes.length() - 1) == '/')
    {
      aRes.resize(aRes.length()-1);
    }
  }


// ############################################
  /********************************************* 
   * Get file/folder attributes.
   * 
   * Similar to stat(). The 'st_dev' and 'st_blksize' fields are ignored. 
   * The 'st_ino' field is ignored except if the 'use_ino' mount option is given.
   * 
   */
  int
  getattr(const char *aPath, struct stat *aStBuf)
  {
    int result = 0;
    std::string lPath;
    configure_path(aPath, lPath);

    Memcache m;
    if (!m.get(lPath, aStBuf))
    {
      try
      {
        if (is_proc(lPath, aStBuf))
        {
          Proc lProc(lPath);
          lProc.stat(aStBuf); 
        }
        else
        {
          // load information about the path   
          FilesystemEntry lEntry(lPath);

          //check if path to file, dir or link does exist
          if (!lEntry.exists())
          {
            syslog(LOG_DEBUG, "getattr: entry does not exists %s",
                lPath.c_str());
            result = -ENOENT;
          }
          else
          {
            syslog(LOG_DEBUG, "getattr: entry exists %s", lPath.c_str());
            lEntry.stat(aStBuf); 
          }
        }
        m.set(lPath, aStBuf);

      } GRIDFS_CATCH
    }
    else
    {
      syslog(LOG_DEBUG, "getattr: cache hit for %s", lPath.c_str());
    }

    return result;
  }

// ############################################
  /********************************************* 
   *
   */
  int
  mkdir(const char *path, mode_t mode)
  {
    int result = 0;
    std::string lPath;
    configure_path(path, lPath);

    try
    {
      // init parameters to create dir
      Directory lDir(lPath);
      uid_t st_uid = FUSE.config.default_uid;
      gid_t st_gid = FUSE.config.default_gid;
      
      // create it
      lDir.create(S_IFDIR | mode, st_uid, st_gid, "");

    } GRIDFS_CATCH

    return result;
  }


// ############################################
  /********************************************* 
   * delete a directory
   */
  int
  rmdir(const char *path)
  {
    int result = 0;
    std::string lPath;
    configure_path(path, lPath);

    try
    {
      Directory lDir(lPath);

      // first check if it exists
      if (!lDir.exists())
      {
        syslog(LOG_INFO, "rmdir: entry does not exists %s", lPath.c_str());
        return -ENOENT;
      }

      // then check wheither it is empty
      if (!lDir.isEmpty())
      {
        syslog(LOG_INFO, "rmdir: directory not empty %s", lPath.c_str());
        return -ENOTEMPTY;
      }

      lDir.remove();      
      Memcache m;
      m.remove(lPath);

    } GRIDFS_CATCH

    return result;
  }


// ############################################
  /********************************************* 
   * Read directory
   * 
   * The filesystem may choose between two modes of operation:
   * 
   * 1) The readdir implementation ignores the offset parameter, and passes zero 
   * to the filler function's offset. The filler function will not return '1' 
   * (unless an error happens), so the whole directory is read in a single readdir 
   * operation. This works just like the old getdir() method.
   * 
   * 2) The readdir implementation keeps track of the offsets of the directory 
   * entries. It uses the offset parameter and always passes non-zero offset to
   * the filler function. When the buffer is full (or an error happens) the filler
   *  function will return '1'.
   * 
   */
  int
  readdir(
    const char *path,
	  void *buf,
	  fuse_fill_dir_t filler,
	  off_t offset,
	  struct fuse_file_info *fileinfo)
  {
    int result = 0;
    FileInfo* lInfo = reinterpret_cast<FileInfo*>(fileinfo->fh);

    try
    {
      switch (lInfo->type)
      {
        case FileInfo::DIRECTORY:
          lInfo->directory->list(buf, filler); break;
        case FileInfo::PROC:
          lInfo->proc->list(buf, filler); break;
        default: assert(false);
      }
    } GRIDFS_CATCH

    return result;
  }


// ############################################
  /********************************************* 
   * Create and open a file
   * 
   * If the file does not exist, first create it with the specified 
   * mode, and then open it.
   * 
   * If this method is not implemented or under Linux kernel versions 
   * earlier than 2.6.15, the mknod() and open() methods will be called 
   * instead.
   * 
   */
  int
  create(const char *path, mode_t mode, struct fuse_file_info *fileinfo)
  {
    memset(fileinfo, 0, sizeof(struct fuse_file_info));
    int result = 0;
    std::string lPath;
    configure_path(path, lPath);

    try
    {
      std::auto_ptr<FileInfo> lInfo;

      syslog(LOG_DEBUG, "create: %s", lPath.c_str());

      if (is_proc(lPath, 0))
      {
        lInfo.reset(new FileInfo(new Proc(lPath)));
        lInfo->proc->create();
      }
      else
      {
        lInfo.reset(new FileInfo(new File(lPath)));

        //check if file already exists
        if (lInfo->file->exists())
        {
	        result = -EEXIST;
        }
        else
        {
          // init parameters to create file or symlink
          uid_t st_uid = FUSE.config.default_uid;
          gid_t st_gid = FUSE.config.default_gid;

          // create it in mongo
          lInfo->file->create(S_IFREG | mode, st_uid, st_gid, "");

        }
        // put pointer into fileinfo struct
        fileinfo->fh = (uint64_t)lInfo.release();
      }

    } GRIDFS_CATCH

    return result;
  }


// ############################################
  /********************************************* 
   * Remove a file 
   */
  int
  unlink(const char * path)
  {
    int result = 0;
    std::string lPath;
    configure_path(path, lPath);

    try
    {
      // load information about the path   
      FilesystemEntry lEntry(lPath);

      //checd if path to file, dir or link does exist
      if (!lEntry.exists())
      {
        syslog(LOG_DEBUG, "unlink: entry does not exists %s", lPath.c_str());
        return -ENOENT;
      }

      lEntry.remove();

      Memcache m;
      m.remove(lPath);

    } GRIDFS_CATCH

    return result;
  }

// ############################################
  /********************************************* 
   * open a File
   * 
   * No creation, or truncation flags (O_CREAT, O_EXCL, O_TRUNC) will be passed
   * to open(). Open should check if the operation is permitted for the given 
   * flags. Optionally open may also return an arbitrary filehandle in the 
   * fuse_file_info structure, which will be passed to all file operations.
   */
  int
  open(
    const char *path, 
	  struct fuse_file_info *fileinfo)
  {
    syslog(LOG_DEBUG, "open %s", path);

    int result = 0;
    std::string lPath;
    configure_path(path, lPath);

    std::auto_ptr<FileInfo> lInfo;

    try
    {
      if (is_proc(lPath, 0))
      {
        lInfo.reset(new FileInfo(new Proc(lPath)));

        if (!lInfo->proc->create())
          result = -ENAMETOOLONG;
      }
      else
      {
        lInfo.reset(new FileInfo(new File(lPath)));

        // assumption: don't check for lFile->exists() because getattr is always
        // called before open
        assert(lInfo->file->exists());
      }
      
      // put pointer into fileinfo struct
      fileinfo->fh = (uint64_t)lInfo.release();          

    } GRIDFS_CATCH
    
    return result;
  }


// ############################################
  /********************************************* 
   * Write data to an open file
   * 
   * Write should return exactly the number of bytes requested except 
   * on error. An exception to this is when the 'direct_io' mount option 
   * is specified (see read operation).
   * 
   * 
   * TODO: evtl optimization: align buffer size to chunksize in mongo with -o max_read=xxx -o max_write=xxx
   */
  int
  write(
      const char * path,
      const char * data,
      size_t size,
      off_t offset,
      struct fuse_file_info * fileinfo)
  {
    assert(fileinfo);
    assert(fileinfo->fh);

    int result = 0;
    std::string lPath;
    configure_path(path, lPath);
    syslog(LOG_DEBUG, "write: context path %s size %i offset %i",
        path, (int) size, (int) offset);

    File* lFile = reinterpret_cast<File*>(fileinfo->fh);

    try
    {
      result = lFile->write(data, size, offset);
    } GRIDFS_CATCH

    return result;
  }


// ############################################
  /********************************************* 
   * Release an open file
   * 
   * Release is called when there are no more references to an open 
   * file: all file descriptors are closed and all memory mappings are 
   * unmapped.
   * 
   * For every open() call there will be exactly one release() call with 
   * the same flags and file descriptor. It is possible to have a file 
   * opened more than once, in which case only the last release will mean, 
   * that no more reads/writes will happen on the file. The return value 
   * of release is ignored.
   * 
   */
  int
  release(const char *path, struct fuse_file_info *fileinfo)
  {
    int result = 0;
    std::string lPath;
    configure_path(path, lPath);

    std::auto_ptr<FileInfo> lInfo(reinterpret_cast<FileInfo*>(fileinfo->fh));

    try
    {
      
      switch (lInfo->type)
      {
        case FileInfo::FILE:
        {
          // write changes to mongo if any
          if (lInfo->file->hasChanges())
          {
            lInfo->file->store();
            Memcache m;
            m.remove(lPath);
          }
          break;
        }
        default:
        {
          Memcache m;
          m.remove(lPath);
          break;
        }
      }

    } GRIDFS_CATCH

    return result;
  }



// ############################################
  /********************************************* 
   * Read data from an open file
   * 
   * Read should return exactly the number of bytes requested except on EOF
   * or error, otherwise the rest of the data will be substituted with zeroes. 
   * An exception to this is when the 'direct_io' mount option is specified, 
   * in which case the return value of the read system call will reflect the 
   * return value of this operation.
   * 
   */
  int
  read(
    const char *aPath,
	  char *buf,
	  size_t size,
	  off_t offset,
	  struct fuse_file_info *fileinfo)
  {
    assert(fileinfo);
    assert(fileinfo->fh);

    int result = 0;

#ifndef NDEBUG
    std::string lPath;
    configure_path(aPath, lPath);
    syslog(LOG_DEBUG, "read path: %s, offset: %i, size: %i",
        lPath.c_str(), (int)offset, (int)size);
#endif

    File* lFile = reinterpret_cast<File*>(fileinfo->fh);

    try
    {
      result = lFile->read(buf, size, offset);
    } GRIDFS_CATCH

    return result;
  }


// ############################################
  /********************************************* 
   * Open directory
   * 
   * This method should check if the open operation is permitted for this directory
   * 
   * it is called before readdir
   */

  int
  opendir(const char *path, struct fuse_file_info *fileinfo)
  {
    int result = 0;
    std::string lPath;
    configure_path(path, lPath);

    try
    {
      std::auto_ptr<FileInfo> lInfo;

      if (is_proc(lPath, 0))
      {
        lInfo.reset(new FileInfo(new Proc(lPath)));
      }
      else
      {
        //TODO DK: do we have to check permissions ourselves here? 
        // http://openbook.galileocomputing.de/c_von_a_bis_z/017_c_dateien_verzeichnisse_001.htm
        lInfo.reset(new FileInfo(new Directory(lPath)));
      
        if (!lInfo->directory->exists())
        {
          syslog(LOG_DEBUG, "opendir: directory does not exist %s",
              lPath.c_str());
          return -ENOENT;
        }
      }

      // put pointer into fileinfo struct
      fileinfo->fh = (uint64_t)lInfo.release();          

    } GRIDFS_CATCH

    return result;
  }

// ############################################
  /********************************************* 
   * Close directory
   */
  int
  releasedir(const char *path, struct fuse_file_info *fileinfo)
  {
    int result = 0;

    FileInfo* lInfo = reinterpret_cast<FileInfo*>(fileinfo->fh);

    try
    {
      // close dir
      delete lInfo;
    } GRIDFS_CATCH

    return result;
  }
   
// ############################################
  /********************************************* 
   * Create a symbolic link
   */
  int
  symlink(const char* oldpath, const char * newpath) 
  {
    int result = 0;
    const std::string lOldPath = oldpath;
    std::string lNewPath;
    configure_path(newpath, lNewPath);
    struct stat stbuf;
    memset(&stbuf, 0, sizeof(struct stat));
    
    try{
      //TODO DK: do we have to check permissions ourselves here? 
      // http://openbook.galileocomputing.de/c_von_a_bis_z/017_c_dateien_verzeichnisse_001.htm
      Symlink lSymlink(lNewPath);

      //check if link already exists
      if(lSymlink.exists())
      {
        syslog(LOG_DEBUG, "symlink: entry does not exists %s",
            lOldPath.c_str());
        return -EEXIST;
      }

      // init parameters to create symlink
      uid_t st_uid = FUSE.config.default_uid;
      gid_t st_gid = FUSE.config.default_gid;

      // create it in mongo
      lSymlink.create(S_IFLNK | 0777, st_uid, st_gid, lOldPath);
      syslog(LOG_INFO, "symlink: created %s pointint to %s",
          lNewPath.c_str(), lOldPath.c_str());

    } GRIDFS_CATCH

    return result;
  }


// ############################################
  /********************************************* 
   * Read the target of a symbolic link
   *
   * The buffer should be filled with a null terminated string. The buffer size 
   * argument includes the space for the terminating null character. If the linkname
   * is too long to fit in the buffer, it should be truncated. The return value should
   * be 0 for success.
   */ 
  int
  readlink(const char * path, char * link, size_t size)
  {
    int result = 0;
    std::string lPath;
    configure_path(path, lPath);

    try
    {
      Symlink lSymlink(lPath);

      if (!lSymlink.exists())
      {
        syslog(LOG_DEBUG, "readlink: entry does not exists %s", lPath.c_str());
        return -ENOENT;
      }

      lSymlink.read(link, size);

    } GRIDFS_CATCH

    return result; // 0 for success
  }

// ############################################
  /********************************************* 
   * change permissions of fileentry
   */ 
  int
  chmod(const char * path, mode_t mode)
  {
    int result = 0;
    std::string lPath;
    configure_path(path, lPath);

    try
    {
      // load information about the path   
      FilesystemEntry lEntry(lPath);

      //check if path to file, dir or link does exist
      if (!lEntry.exists())
      {
        syslog(LOG_DEBUG, "getattr: entry does not exists %s", lPath.c_str());
        return -ENOENT;
      }

      lEntry.chmod(mode); 
      Memcache m;
      m.remove(lPath);

    } GRIDFS_CATCH

    return result; // 0 for success
  }

// ############################################
  /********************************************* 
   * change owner of fileentry
   */ 
  int
  chown(const char * path, uid_t uid, gid_t gid)
  {
    int result = 0;
    std::string lPath;
    configure_path(path, lPath);

    try
    {
      FilesystemEntry lEntry(lPath);

      if (!lEntry.exists())
      {
       result = -ENOENT;
      }
      else
      {
        lEntry.chown(uid,gid); 
        Memcache m;
        m.remove(lPath);
      }


    } GRIDFS_CATCH

    return result;
  }

// ############################################
  /********************************************* 
   * change size of a file
   */ 
  int
  truncate(const char * path, off_t offset)
  {
    int result = 0;
    std::string lPath;
    configure_path(path, lPath);

    try
    {
      // load information about the path   
      File lFile(lPath);

      //check if path to file does exist
      if(!lFile.exists())
      {
        syslog(LOG_DEBUG, "truncate: entry does not exists %s", lPath.c_str());
        return -ENOENT;
      }

      if (offset==0)
      {
        lFile.truncate();
        Memcache m;
        m.remove(lPath);
      }
      else
      {
        // can only truncate all
        syslog(LOG_ERR,
            "truncate called with offset (can only truncate entire files");
        return -EIO;
      }

    } GRIDFS_CATCH

    return result; // 0 for success
  }

// ############################################
  /*********************************************
   * change the access and modification times of a file with
   * nanosecond resolution
   */

  int
  utimens(const char * path, const struct timespec tv[2])
  {
    int result = 0;
    std::string lPath;
    configure_path(path, lPath);

    if (is_proc(lPath, 0))
    {
      return 0;
    }

    try 
    {
      // load information about the path
      File lFile(lPath);

      //check if path to file does exist
      if (!lFile.exists())
      {
        syslog(LOG_DEBUG, "utimens: entry does not exists %s", lPath.c_str());
        return -ENOENT;
      }

      lFile.utimes(tv[1].tv_sec);
      Memcache m;
      m.remove(lPath);

    } GRIDFS_CATCH

    return result; // 0 for success
  }
}
