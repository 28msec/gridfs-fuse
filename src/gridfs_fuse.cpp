#include "gridfs_fuse.h"

#include <stddef.h>
#include <stdio.h>
#include <stdexcept>
#include <mongo/client/dbclient.h>
#include <mongo/client/connpool.h>
#include <libmemcached/util/pool.h>
#include <libmemcached/memcached.h>
#include <syslog.h>

#include "filesystem_operations.h"
#include "filesystem_entry.h"
#include "auth_hook.h"


namespace gridfs 
{
  const unsigned int MONGO_DEFAULT_CHUNK_SIZE = 256 * 1024;

  // options to configure gridfs
  // here: mapping to config struct
#define GRIDFS_OPT(t, p, v) { t, offsetof(struct gridfs_config, p), v }
  fuse_opt Fuse::opts[] = {
     GRIDFS_OPT("mongo_conn_string=%s", mongo_conn_string, 0),
     GRIDFS_OPT("mongo_user=%s", mongo_user, 0),
     GRIDFS_OPT("mongo_password=%s", mongo_password, 0),
     GRIDFS_OPT("mongo_db=%s", mongo_db, 0),
     GRIDFS_OPT("mongo_collection_prefix=%s", mongo_collection_prefix, 0),
     GRIDFS_OPT("path_prefix=%s", path_prefix, 0),
     GRIDFS_OPT("log_level=%s", log_level, 0),
     GRIDFS_OPT("default_uid=%u", default_uid, 0),
     GRIDFS_OPT("default_gid=%u", default_gid, 0),

     FUSE_OPT_KEY("-V",             KEY_VERSION),
     FUSE_OPT_KEY("-v",             KEY_VERSION),
     FUSE_OPT_KEY("--version",      KEY_VERSION),
     FUSE_OPT_KEY("-h",             KEY_HELP),
     FUSE_OPT_KEY("-H",             KEY_HELP),
     FUSE_OPT_KEY("--help",         KEY_HELP),

     {NULL, -1U, 0} // mark the end
  };
#undef GRIDFS_OPT
 
  int 
  Fuse::opt_proc(
    void *data,
    const char *arg,
    int key,
    struct fuse_args *outargs)
  {
    switch (key)
    {
    case KEY_HELP:
      std::cerr 
        << "usage: " << outargs->argv[0] << " mountpoint [options]" << std::endl
        << std::endl
        << "general options:" << std::endl
        << "  -o opt,[opt...]                    mount options (see below)" << std::endl
        << "  -h,-H,--help                       print help" << std::endl
        << "  -v,-V,--version                    print version info" << std::endl
        << "" << std::endl
        << "mandatory GRIDFS options:" << std::endl
        << "  -o mongo_db=STRING                 database name in mongo db in which to save the gridfs collections" << std::endl
        << std::endl
        << "optional GRIDFS options:" << std::endl
        << "  -o mongo_conn_string=STRING        connection string (e.g. replica-set/host:port,host:port; default: localhost:27017)" << std::endl
        << "  -o mongo_user=STRING               user name for mongo db authentication" << std::endl
        << "  -o mongo_password=STRING           password for mongo db authentication" << std::endl
        << "  -o mongo_collection_prefix=STRING  prefix for the gridfs collections (default: fs)" << std::endl
        << "  -o path_prefix=STRING              this prefix will be prepended to all path stored in mongo (default: \"\")" << std::endl
        << "  -o log_level=STRING                logging level (EMERG, ALERT, CRIT, ERR, WARNING, NOTICE, INFO, DEBUG) (default: ERR)" << std::endl
        << "  -o default_uid=INT                 optional default user id (default: userid of user running gridfs)" << std::endl
        << "  -o default_gid=INT                 optional default group id (default: groupid of user running gridfs)"
        << std::endl << std::endl;

      fuse_opt_add_arg(outargs, "-ho");

      fuse_main(
        outargs->argc,
        outargs->argv,
        &FUSE.filesystem_operations, NULL);

      exit(1);

    case KEY_VERSION:
      fuse_opt_add_arg(outargs, "--version");
      fuse_main(outargs->argc, outargs->argv, &FUSE.filesystem_operations, NULL);
      exit(0);
    }
    return 1;
  }

  void
  Fuse::initSyslog()
  {
    int logmask = LOG_ERR;

    std::string log_level(config.log_level != 0?config.log_level:"ERR");

    if (log_level == "EMERG")
      logmask = LOG_EMERG;
    else if (log_level == "ALERT")
      logmask = LOG_ALERT;
    else if (log_level == "CRIT")
      logmask = LOG_CRIT;
    else if (log_level == "ERR")
      logmask = LOG_ERR;
    else if (log_level == "WARNING")
      logmask = LOG_WARNING;
    else if (log_level == "NOTICE")
      logmask = LOG_NOTICE;
    else if (log_level == "INFO")
      logmask = LOG_INFO;
    else if (log_level == "DEBUG")
      logmask = LOG_DEBUG;

    setlogmask (LOG_UPTO (logmask));
    openlog ("gridfs", 0, 0);
  }

  void
  Fuse::init(int argc, char **argv)
  {
    config.mongo_conn_string = (char*)"localhost:27017";
    config.mongo_user = (char*)"";
    config.mongo_password = (char*)"";
    config.mongo_db = (char*)"";
    config.mongo_collection_prefix = (char*)"fs";
    config.path_prefix = (char*)"";
    config.log_level = (char*)"ERROR";
    config.log_file = (char*)"console";
    config.default_uid = getuid();
    config.default_gid = getgid();
    config.mongo_chunk_size = MONGO_DEFAULT_CHUNK_SIZE;

    // point filesystem operations to the right callback functions
    filesystem_operations.getattr    = gridfs::getattr;
    filesystem_operations.mkdir      = gridfs::mkdir;
    filesystem_operations.rmdir      = gridfs::rmdir;
    filesystem_operations.opendir    = gridfs::opendir;
    filesystem_operations.readdir    = gridfs::readdir;
    filesystem_operations.releasedir = gridfs::releasedir;
    filesystem_operations.create     = gridfs::create;
    filesystem_operations.unlink     = gridfs::unlink;
    filesystem_operations.read       = gridfs::read;
    filesystem_operations.write      = gridfs::write;
    filesystem_operations.open       = gridfs::open;
    filesystem_operations.release    = gridfs::release;
    filesystem_operations.symlink    = gridfs::symlink;
    filesystem_operations.readlink   = gridfs::readlink;
    filesystem_operations.chmod      = gridfs::chmod;
    filesystem_operations.chown      = gridfs::chown;
    filesystem_operations.truncate   = gridfs::truncate;
    filesystem_operations.utimens    = gridfs::utimens;

    // get all commandline args
    args.argc = argc;
    args.argv = argv;
    args.allocated = 0;

    // init config struct
    fuse_opt_parse(&args, &config, opts, Fuse::opt_proc);

    initSyslog();

    // check chunksize < 200 MB
    if (config.mongo_chunk_size >= (200 * 1024 * 1024))
    {
      std::cerr
        << "chunk size is too high < 2GB (" << argv[0] << " -h)"
        << std::endl;
      exit(1);
    }

    char chunk_size[11];
    sprintf(chunk_size, "%d", config.mongo_chunk_size);
    
    // we set the read and write buffer size by hand to have
    // it aligned with the mongo chunksize
    std::string max_read("max_read=");
    max_read.append(chunk_size);
    fuse_opt_add_arg(&args, "-o");
    fuse_opt_add_arg(&args, max_read.c_str());

    std::string max_write("max_write=");
    max_write.append(chunk_size);
    fuse_opt_add_arg(&args, "-o");
    fuse_opt_add_arg(&args, max_write.c_str());

    std::string big_writes("big_writes");
    fuse_opt_add_arg(&args, "-o");
    fuse_opt_add_arg(&args, big_writes.c_str());

    // check mandatory args
    if (strcmp(config.mongo_db, "") == 0)
    {
      std::cerr
        << "mandatory option mongo_db not set (" << argv[0] << " -h)"
        << std::endl;
      exit(1);
    }
    
    memcached_return rc;
    theMaster = memcached_create(NULL);
    theServers = memcached_server_list_append(theServers, "localhost", 11211, &rc);
    rc = memcached_server_push(theMaster, theServers);

    if (rc != MEMCACHED_SUCCESS)
    {
      std::cerr
        << "could not connect to memcached (" << memcached_strerror(theMaster, rc) << ")"
        << std::endl;
      exit(2);
    }

    theMemcachePool = memcached_pool_create(theMaster, 100, 200);

    // enable mongo authentication if username given
    if (strcmp(config.mongo_user, "") != 0)
    {
      // the incode documentation says that addHook() function passes
      // ownership to the pool
      // https://github.com/mongodb/mongo/blob/master/src/mongo/client/connpool.h
      AuthHook* lAuthHook = new AuthHook(config.mongo_db,
                                         config.mongo_user,
                                         config.mongo_password);
      mongo::pool.addHook(lAuthHook);
    }

  }

  void
  Fuse::createRootDir()
  {
    std::string lRootDir = config.path_prefix;
    gridfs::FilesystemEntry lEntry(lRootDir);
    if (!lEntry.exists())
    {
      mode_t mode = S_IFDIR | 0755;
      gridfs::mkdir("", mode);
    }
  }

  Fuse::Fuse()
    : theMemcachePool(0),
      theMaster(0),
      theServers(0)
  {
  }

  Fuse::~Fuse()
  {
    if (theMemcachePool) memcached_pool_destroy(theMemcachePool);
    if (theMaster) memcached_free(theMaster);

    fuse_opt_free_args(&args);

    closelog();
  }
 
  const mongo::ConnectionString&
  Fuse::connection_string()
  {
    static mongo::ConnectionString lConString;

    if (!lConString.isValid())
    {
      std::string errmsg;
      lConString = mongo::ConnectionString::parse( config.mongo_conn_string, errmsg );
      if (!lConString.isValid())
      {
        syslog(LOG_ERR, "invalid connection string (%s)", errmsg.c_str());
        throw std::runtime_error("invalid connection string: " + errmsg); 
      }
    }

    return lConString;
  }

  memcached_st*
  Fuse::cache()
  {
    memcached_return rc;
    memcached_st* m = memcached_pool_pop(theMemcachePool, true, &rc);
    return m;
  }

  void
  Fuse::release(memcached_st* m)
  {
    memcached_pool_push(theMemcachePool, m);
  }

  Fuse FUSE;

  Memcache::Memcache()
    : m(FUSE.cache())
  {}

  Memcache::~Memcache()
  {
    FUSE.release(m);
  }

  bool
  Memcache::get(const std::string& aPath, struct stat* aBuf)
  {
    uint32_t lFlags = 0;
    size_t lLength  = 0;
    memcached_return_t rc;

    std::string lKey = "a:" + aPath;

    char* lResult = memcached_get(m, lKey.c_str(), lKey.size(), &lLength, &lFlags, &rc); 

    if (lResult)
    {
      assert(lLength == sizeof(struct stat));
      memcpy(aBuf, lResult, sizeof(struct stat));
      free(lResult);
      return true;
    }
    else
    {
      return false;
    }
  }

  void
  Memcache::set(const std::string& aPath, struct stat* aBuf)
  {
    uint32_t lFlags = 0;
    memcached_return_t rc;

    std::string lKey = "a:" + aPath;

    rc = memcached_set(m, lKey.c_str(), lKey.size(), (const char*) aBuf, sizeof(struct stat), 0, lFlags);
  }

  void
  Memcache::remove(const std::string& aPath)
  {
    memcached_return_t rc;

    std::string lKey = "a:" + aPath;

    rc = memcached_delete(m, lKey.c_str(), lKey.size(), 0);
  }

}
