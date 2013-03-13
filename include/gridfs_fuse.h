#ifndef GRIDFS_FUSE_H
#define GRIDFS_FUSE_H

#define _FILE_OFFSET_BITS 64
#define FUSE_USE_VERSION  26
#include <fuse.h>

#include <string>
#include <stddef.h>
#include <stdio.h>
#include <syslog.h>


namespace mongo
{
  class ConnectionString;
}

struct memcached_server_st;
struct memcached_st;
struct memcached_pool_st;


namespace gridfs 
{

  enum {
    KEY_HELP,
    KEY_VERSION,
  };

  //config containing options from commandline
  struct 
  gridfs_config 
  {
    char* mongo_conn_string;
    char* mongo_user;
    char* mongo_password;
    char* mongo_db;
    char* mongo_collection_prefix;
    char* path_prefix;
    char* log_level;
    char* log_file;
    unsigned int default_uid;
    unsigned int default_gid;
    unsigned int mongo_chunk_size;
  };

  class Fuse;

  class Memcache
  {
  protected:
    friend class Fuse;
    memcached_st* m;

  public:
    Memcache();

    ~Memcache();

    bool
    get(const std::string& aPath, struct stat* aBuf);

    void
    set(const std::string& aPath, struct stat* aBuf);

    void
    remove(const std::string& aPath);
  };


  class Fuse {
  public:
    //all command line args
    fuse_args args;

    //config contains options for commandline
    gridfs_config config;  

    // point filesystem operations to the right callback functions
    fuse_operations filesystem_operations;

    // options to configure gridfs
    // here: mapping to config struct
    static fuse_opt opts[];
 
    static int 
    opt_proc(void *data, const char *arg, int key, struct fuse_args *outargs);

    void
    init(int argc, char **argv);

    void
    initSyslog();

    void
    createRootDir();
    
    Fuse(); 

    ~Fuse();

    const mongo::ConnectionString&
    connection_string();

    memcached_st*
    master() const { return theMaster; }

    memcached_pool_st*
    pool() const { return theMemcachePool; }

  protected:
    friend class Memcache;
    memcached_st*
    cache();

    void
    release(memcached_st*);

    memcached_pool_st*   theMemcachePool;
    memcached_st*        theMaster;
    memcached_server_st* theServers;
  };

  extern Fuse FUSE;

}
#endif
