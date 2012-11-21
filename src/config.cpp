#include <config.h>
#include <filesystem_operations.h>
#include <stddef.h>
#include <stdio.h>
#include <stdexcept>

#define GRIDFS_OPT(t, p, v) { t, offsetof(struct gridfs_config, p), v }

namespace gridfs 
{
  const unsigned int MONGO_DEFAULT_CHUNK_SIZE = 256 * 1024;

  // options to configure gridfs
  // here: mapping to config struct
  fuse_opt Fuse::opts[] = {
     GRIDFS_OPT("mongo_host=%s", mongo_host, 0),
     GRIDFS_OPT("mongo_port=%s", mongo_port, 0),
     GRIDFS_OPT("mongo_user=%s", mongo_user, 0),
     GRIDFS_OPT("mongo_password=%s", mongo_password, 0),
     GRIDFS_OPT("mongo_db=%s", mongo_db, 0),
     GRIDFS_OPT("mongo_collection_prefix=%s", mongo_collection_prefix, 0),
     GRIDFS_OPT("path_prefix=%s", path_prefix, 0),
     GRIDFS_OPT("log_level=%s", log_level, 0),
     GRIDFS_OPT("log_file=%s", log_file, 0),
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
 
  int 
  Fuse::opt_proc(void *data, const char *arg, int key, struct fuse_args *outargs)
  {
    switch (key) {
    case KEY_HELP:
      fprintf(stderr,
	      "usage: %s mountpoint [options]\n"
	      "\n"
	      "general options:\n"
	      "    -o opt,[opt...]                       mount options (see below)\n"
	      "    -h,-H,--help                          print help\n"
	      "    -v,-V,--version                       print version info\n"
	      "\n"
	      "mandatory GRIDFS options:\n"
	      "    -o mongo_db=STRING                    database name in mongo db in which to save the gridfs collections\n"
	      "\n"
	      "optional GRIDFS options:\n"
	      "    -o mongo_host=STRING                  hostname or ip of the host to connect to (default: localhost)\n"
	      "    -o mongo_port=STRING                  port on which the mongo db is running (default: 27017)\n"
	      "    -o mongo_user=STRING                  user name for mongo db authentication\n"
	      "    -o mongo_password=STRING              password for mongo db authentication\n"
	      "    -o mongo_collection_prefix=STRING     prefix for the gridfs collections (default: fs)\n"
	      "    -o path_prefix=STRING                 this prefix will be prepended to all path stored in mongo (default: \"\")\n"
	      "    -o log_level=STRING                   logging level (ERROR, INFO, DEBUG, OFF) (default: ERROR)\n"
	      "    -o log_file=STRING                    optional log file or \"console\" or \"syslog\" (default: \"console\")\n"
	      "    -o default_uid=INT                     optional default user id (default: userid of user running gridfs)\n"
	      "    -o default_gid=INT                     optional default group id (default: groupid of user running gridfs)\n"
	      "\n"
	      , outargs->argv[0]);
      fuse_opt_add_arg(outargs, "-ho");
      fuse_main(outargs->argc, outargs->argv, &FUSE.filesystem_operations, NULL);
      exit(1);

    case KEY_VERSION:
      fprintf(stderr, "GridFs fuse filesystem version %s\n", "@GRIDFS_VERSION@");
      fuse_opt_add_arg(outargs, "--version");
      fuse_main(outargs->argc, outargs->argv, &FUSE.filesystem_operations, NULL);
      exit(0);

    }
    return 1;
  }

  void
  Fuse::init(int argc, char **argv)
  {

    config.mongo_host = (char*)"localhost";
    config.mongo_port = (char*)"27017";
    config.mongo_user = (char*)"";
    config.mongo_password = (char*)"";
    config.mongo_db = (char*)"";
    config.mongo_collection_prefix = (char*)"fs";
    config.path_prefix = (char*)"";
    config.log_level = (char*)"ERROR";
    config.log_file = (char*)"console";
    config.default_uid = getuid();
    config.default_gid = getgid();
    config.mongo_chunk_size = MONGO_DEFAULT_CHUNK_SIZE; // this is fix for now 

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
    
    // check chunksize < 200 MB
    if(config.mongo_chunk_size >= (200 * 1024 *1024) ){
      fprintf(stderr,"\n[Error] Chunk size is too high. Must be less than 2 GB. See '%s -h' for help.\n",argv[0]);
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

    // check mandatory args
    if(strcmp(config.mongo_db,"") == 0){
      fprintf(stderr,"\n[Error] Mandatory option mongo_db not set. See '%s -h' for help.\n",argv[0]);
      exit(1);
    }
  }

  Fuse::~Fuse()
  {
    fuse_opt_free_args(&args);
  }
 
  const mongo::ConnectionString&
  Fuse::connection_string()
  {
    static mongo::ConnectionString lConString;

    // create and validate connection string
    if(!lConString.isValid()){
      std::string port = config.mongo_port;
      std::string host = config.mongo_host;
      std::string url = host + (port==""?"":":") + port;
      std::string errmsg;
      lConString = mongo::ConnectionString::parse( url , errmsg );
      if ( ! lConString.isValid() ) throw std::runtime_error("Invalid Connection String: " + errmsg); 
    }

    return lConString;
  }

  const std::string
  Fuse::configure_path(const char* path)
  {
    std::string lpath(config.path_prefix);
    lpath.append(path);
    
    // remove trailing / otherwise the path is not found in mongo
    if (lpath.at(lpath.length()-1) == '/'){
      lpath.resize(lpath.length()-1);
    }

    return lpath;
  }

  Fuse FUSE;
}
