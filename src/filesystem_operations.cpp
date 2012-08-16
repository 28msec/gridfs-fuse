/*
 * Copyright 2008 28msec, Inc.
 * 
 */
#include <filesystem_operations.h>
#include <config.h>
#include <log4cxx/gridfs_logging.h>
#include <filesystem_entry.h>
#include <directory.h>
#include <file.h>
#include <symlink.h>

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
    _LOG_ERROR("Mongo exception:" << u.getInfo().toString() ); \
    result=-EIO;\
  }catch (std::exception& e) { \
    _LOG_ERROR("An exception was thrown: " << e.what()); \
    result=-EIO;\
  }catch(...){ \
    _LOG_ERROR("An Unexpected Error occured."); \
    return -EIO; \
  }

namespace gridfs
{

// ############################################
  /********************************************* 
   * Get file/folder attributes.
   * 
   * Similar to stat(). The 'st_dev' and 'st_blksize' fields are ignored. 
   * The 'st_ino' field is ignored except if the 'use_ino' mount option is given.
   * 
   */
  int
  getattr(const char *path, struct stat *stbuf)
  {
    GRIDFS_INIT_LOGGER("gridfs.meta.getattr")

    // init
    int result=0;
    const std::string lpath = FUSE.configure_path(path);
    _LOG_DEBUG("context path: " << lpath)

    try{

      // load information about the path   
      gridfs::FilesystemEntry lEntry(lpath);

      // check if the root dir is requested and it doesn't 
      // exist; if so -> create it automatically
      if(!lEntry.exists() &&
         ( strcmp(path, "/") == 0 || strcmp(path, "") == 0 )
        ){

        // create root dir 
        mode_t mode = S_IFDIR | 0755;
        mkdir(path,mode);

        // empty caches
        lEntry.force_reload();
      }

      //check if path to file, dir or link does exist
      if(!lEntry.exists()){
	_LOG_DEBUG("fileentry does not exist: " << lpath)
	return -ENOENT;
      }

      lEntry.stat(stbuf); 

    } GRIDFS_CATCH

    return result;
  }

// ############################################
  /********************************************* 
   *
   */
  int
  mkdir(const char *path, mode_t mode)
  {
    GRIDFS_INIT_LOGGER("gridfs.dir.mkdir")

    // init
    int result=0;
    const std::string lpath = FUSE.configure_path(path);
    _LOG_DEBUG("context path: " << lpath << " mode: " << mode)

    try{

      // init parameters to create dir
      gridfs::Directory lDir(lpath);
      uid_t st_uid = FUSE.config.default_uid;
      gid_t st_gid = FUSE.config.default_gid;
      
      // create it
      lDir.create(S_IFDIR | mode, st_uid, st_gid);
      _LOG_INFO("created directory: " << lpath)

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
    GRIDFS_INIT_LOGGER("gridfs.dir.rmdir")

    //init
    int result=0;
    const std::string lpath = FUSE.configure_path(path);
    _LOG_DEBUG("context path: " << lpath)

    try{

      // init dir
      gridfs::Directory lDir(lpath);

      // first check if it exists
      if(!lDir.exists()){
	_LOG_INFO("directory does not exist: " << lpath)
	return -ENOENT;
      }

      // then check wheither it is empty
      if(!lDir.isEmpty()){
	_LOG_INFO("directory is not empty: " << lpath)
	return -ENOTEMPTY;
      }

      // ok, can be removed
      lDir.remove();      
      _LOG_INFO("removed directory: " << lpath)

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
  readdir(const char *path,
	     void *buf,
	     fuse_fill_dir_t filler,
	     off_t offset,
	     struct fuse_file_info *fileinfo)
  {
    GRIDFS_INIT_LOGGER("gridfs.dir.readdir")

    //sanity
    assert(fileinfo);
    assert(fileinfo->fh);

    // init
    int result=0;
    const std::string lpath = FUSE.configure_path(path);
    _LOG_DEBUG("context path: " << lpath)
    gridfs::Directory* lDir = reinterpret_cast<gridfs::Directory*>(fileinfo->fh);

    try{
      lDir->list(buf, filler);

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
    GRIDFS_INIT_LOGGER("gridfs.file.create")

    // init
    memset(fileinfo, 0, sizeof(struct fuse_file_info));
    int result=0;
    const std::string lpath = FUSE.configure_path(path);
    _LOG_DEBUG("context path: " << lpath << " mode: " << mode)

    try{
      gridfs::File* lFile = new gridfs::File(lpath);

      //check if file already exists
      if(lFile->exists()){
	_LOG_INFO("file does already exist: " << lpath)
	return -EEXIST;
      }

      // init parameters to create file or symlink
      uid_t st_uid = FUSE.config.default_uid;
      gid_t st_gid = FUSE.config.default_gid;

      // create it in mongo
      lFile->create(S_IFREG | mode, st_uid, st_gid);
      _LOG_INFO("created regular file: " << lpath)

      // put pointer into fileinfo struct
      fileinfo->fh = (uint64_t)lFile;          

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
    GRIDFS_INIT_LOGGER("gridfs.file.unlink")

    // init
    int result=0;
    const std::string lpath = FUSE.configure_path(path);
    _LOG_DEBUG("context path: " << lpath)

    try{
      // load information about the path   
      gridfs::FilesystemEntry lEntry(lpath);

      //checd if path to file, dir or link does exist
      if(!lEntry.exists()){
	_LOG_INFO("fileentry does not exist: " << lpath)
	return -ENOENT;
      }

      lEntry.remove();
      _LOG_INFO("Removed fileentry: " << lpath)

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
  open(const char *path, 
	  struct fuse_file_info *fileinfo)
  {
    GRIDFS_INIT_LOGGER("gridfs.file.open")

    // init
    int result=0;
    const std::string lpath = FUSE.configure_path(path);
    _LOG_DEBUG("context path: " << lpath)

    try{
      gridfs::File* lFile = new gridfs::File(lpath);

      //check if file exists
      if(!lFile->exists()){
	_LOG_INFO("file does not exist: " << lpath)
	return -ENOENT;
      }

      // put pointer into fileinfo struct
      fileinfo->fh = (uint64_t)lFile;          

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
  write(const char * path, const char * data, size_t size, off_t offset, struct fuse_file_info * fileinfo)
  {
    GRIDFS_INIT_LOGGER("gridfs.file.write")

    //sanity
    assert(fileinfo);
    assert(fileinfo->fh);

    // init
    int result=0;
    const std::string lpath = FUSE.configure_path(path);
    _LOG_DEBUG("context path: " << lpath << " size: " << size << " offset: " << offset)
    gridfs::File* lFile = reinterpret_cast<gridfs::File*>(fileinfo->fh);

    try{

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
    GRIDFS_INIT_LOGGER("gridfs.file.release")

    //sanity
    assert(fileinfo);
    assert(fileinfo->fh);

    // init
    int result=0;
    const std::string lpath = FUSE.configure_path(path);
    _LOG_DEBUG("context path: " << lpath)
    gridfs::File* lFile = reinterpret_cast<gridfs::File*>(fileinfo->fh);

    try{

      // write changes to mongo if any
      if (lFile->hasChanges())
        lFile->store();

      // close file
      delete lFile;
      lFile = NULL; 

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
  read(const char *path,
	  char *buf,
	  size_t size,
	  off_t offset,
	  struct fuse_file_info *fileinfo)
  {
    GRIDFS_INIT_LOGGER("gridfs.file.read")

    //sanity
    assert(fileinfo);
    assert(fileinfo->fh);

    // init
    int result=0;
    const std::string lpath = FUSE.configure_path(path);
    _LOG_DEBUG("context path: " << lpath << " offset: " << offset << " size: " << size)
    gridfs::File* lFile = reinterpret_cast<gridfs::File*>(fileinfo->fh);

    try{
 
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
    GRIDFS_INIT_LOGGER("gridfs.dir.opendir")

    // init
    memset(fileinfo, 0, sizeof(struct fuse_file_info));
    int result=0;
    const std::string lpath = FUSE.configure_path(path);
    _LOG_DEBUG("context path: " << lpath)

    try{
     
      //TODO DK: do we have to check permissions ourselves here? 
      // http://openbook.galileocomputing.de/c_von_a_bis_z/017_c_dateien_verzeichnisse_001.htm
      gridfs::Directory* lDir = new gridfs::Directory(lpath);

      // check if path to file, dir or link does exist
      if(!lDir->exists()){
        _LOG_DEBUG("dir does not exist: " << lpath)
        delete lDir;
        lDir = NULL;
	return -ENOENT;
      }

      // put pointer into fileinfo struct
      fileinfo->fh = (uint64_t)lDir;          

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
    GRIDFS_INIT_LOGGER("gridfs.dir.releasedir")

    //sanity
    assert(fileinfo);
    assert(fileinfo->fh);

    // init
    int result=0;
    const std::string lpath = FUSE.configure_path(path);
    _LOG_DEBUG("context path: " << lpath)
    gridfs::Directory* lDir = reinterpret_cast<gridfs::Directory*>(fileinfo->fh);

    try{

      // close dir
      delete lDir;
      lDir = NULL; 

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
    GRIDFS_INIT_LOGGER("gridfs.file.symlink")

    // init
    int result=0;
    const std::string lOldPath = oldpath;
    const std::string lNewPath = FUSE.configure_path(newpath);
    struct stat stbuf;
    memset(&stbuf, 0, sizeof(struct stat));
    _LOG_DEBUG("symlink  " << lNewPath << " -> " << lOldPath)
    
    try{
      //TODO DK: do we have to check permissions ourselves here? 
      // http://openbook.galileocomputing.de/c_von_a_bis_z/017_c_dateien_verzeichnisse_001.htm
      gridfs::Symlink lSymlink(lNewPath);

      //check if link already exists
      if(lSymlink.exists()){
	_LOG_INFO("Fileentry does already exist: " << lNewPath)
	return -EEXIST;
      }

      // init parameters to create symlink
      uid_t st_uid = FUSE.config.default_uid;
      gid_t st_gid = FUSE.config.default_gid;

      // create it in mongo
      lSymlink.create(S_IFLNK | 0777, st_uid, st_gid, lOldPath);
      _LOG_INFO("created symlink file: " << lNewPath << " pointing to " << lOldPath)

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
    GRIDFS_INIT_LOGGER("gridfs.file.readlink")

    // init
    int result=0;
    const std::string lpath = FUSE.configure_path(path);
    _LOG_DEBUG("context path: " << lpath << " buffer size: " << size)

    try{
      gridfs::Symlink lSymlink(lpath);

      //check if symlink path does exist
      if(!lSymlink.exists()){
	_LOG_INFO("Symbolic link does not exist: " << lpath)
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
    GRIDFS_INIT_LOGGER("gridfs.file.chmod")

    // init
    int result=0;
    const std::string lpath = FUSE.configure_path(path);
    _LOG_DEBUG("context path: " << lpath << " new mode: " << mode)

    try{
      // load information about the path   
      gridfs::FilesystemEntry lEntry(lpath);

      //check if path to file, dir or link does exist
      if(!lEntry.exists()){
	_LOG_INFO("fileentry does not exist: " << lpath)
	return -ENOENT;
      }

      lEntry.chmod(mode); 

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
    GRIDFS_INIT_LOGGER("gridfs.file.chown")

    // init
    int result=0;
    const std::string lpath = FUSE.configure_path(path);
    _LOG_DEBUG("context path: " << lpath << " new uid: " << uid 
                                         << " new gid: " << gid)

    try{
      // load information about the path   
      gridfs::FilesystemEntry lEntry(lpath);

      //check if path to file, dir or link does exist
      if(!lEntry.exists()){
	_LOG_INFO("fileentry does not exist: " << lpath)
	return -ENOENT;
      }

      lEntry.chown(uid,gid); 

    } GRIDFS_CATCH

    return result; // 0 for success
  }

// ############################################
  /********************************************* 
   * change size of a file
   */ 
  int
  truncate(const char * path, off_t offset)
  {
    GRIDFS_INIT_LOGGER("gridfs.file.truncate")

    // init
    int result=0;
    const std::string lpath = FUSE.configure_path(path);
    _LOG_DEBUG("context path: " << lpath << " offset: " << offset)

    try{
      // load information about the path   
      gridfs::File lFile(lpath);

      //check if path to file does exist
      if(!lFile.exists()){
	_LOG_INFO("fileentry does not exist: " << lpath)
	return -ENOENT;
      }

      if(offset==0){
        lFile.truncate();
      }else{
        // can only truncate all
        _LOG_ERROR("Called truncate with offset. Can only truncate complete file.")
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
  utimens(const char * path, const struct timespec tv[2]) {
    GRIDFS_INIT_LOGGER("gridfs.file.utimens")

    // init
    int result=0;
    const std::string lpath = FUSE.configure_path(path);
    _LOG_DEBUG("context path: " << lpath)

    try{
      // load information about the path
      gridfs::File lFile(lpath);

      //check if path to file does exist
      if(!lFile.exists()){
        _LOG_INFO("fileentry does not exist: " << lpath);
        return -ENOENT;
      }

      lFile.utimes(tv[1].tv_sec);

    } GRIDFS_CATCH

    return result; // 0 for success
  }
}
