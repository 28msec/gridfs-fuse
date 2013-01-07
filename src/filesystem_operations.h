#pragma once
/*
 * Copyright 2008 28msec, Inc.
 * 
 */

#include "gridfs_fuse.h"

#include <sys/stat.h>
#include <stdlib.h>


namespace gridfs
{

  int
  getattr(const char *path, struct stat *stbuf);

  int 
  truncate(const char * path, off_t offset);

  int
  mkdir(const char *path, mode_t mode);

  int
  rmdir(const char *path);

  int
  readdir(const char *path,
	     void *buf,
	     fuse_fill_dir_t filler,
	     off_t offset,
	     struct fuse_file_info *fileinfo);

  int
  create(const char *path, mode_t mode, struct fuse_file_info *fileinfo);

  int
  unlink(const char * path);

  int
  open(const char *path, 
	  struct fuse_file_info *fileinfo);

  int
  write(const char * path, const char * data, size_t size, off_t offset, struct fuse_file_info * fileinfo);

  int
  release(const char *path, struct fuse_file_info *fileinfo);

  int
  read(const char *path,
	  char *buf,
	  size_t size,
	  off_t offset,
	  struct fuse_file_info *fileinfo);

  int
  opendir(const char *path, struct fuse_file_info *fileinfo);

  int
  releasedir(const char *path, struct fuse_file_info *fileinfo);

  int
  symlink(const char * targetpath, const char * newpath);

  int
  readlink(const char * path, char * link, size_t size);

  int
  chmod(const char * path, mode_t mode);

  int
  chown(const char * path, uid_t uid, gid_t gid);

  int
  truncate(const char * path, off_t offset);

  int
  utimens(const char *, const struct timespec tv[2]);

}
