
#include <config.h>
#include <filesystem_operations.h>
#include <log4cxx/gridfs_logging.h>

int
main(int argc, char **argv)
{
  
  using namespace gridfs;
  FUSE.init(argc, argv);

  GRIDFS_CONFIGURE_LOGGING(FUSE.config.log_file, FUSE.config.log_level)
  GRIDFS_INIT_LOGGER("main")
  _LOG_INFO("starting gridfs with configuration:\n"
    << "         mongo_host: " << FUSE.config.mongo_host << "\n"
    << "         mongo_port: " << FUSE.config.mongo_port << "\n"
    << "         mongo_user: " << FUSE.config.mongo_user << "\n"
    << "         mongo_password: " << FUSE.config.mongo_password << "\n"
    << "         mongo_db: " << FUSE.config.mongo_db << "\n"
    << "         mongo_collection_prefix: " << FUSE.config.mongo_collection_prefix << "\n"
    << "         path_prefix: " << FUSE.config.path_prefix << "\n"
    << "         log_level: " << FUSE.config.log_level << "\n"
    << "         log_file: " << FUSE.config.log_file << "\n"
    << "         default_uid: " << FUSE.config.default_uid << "\n"
    << "         default_gid: " << FUSE.config.default_gid << "\n"
//    << "         chache_size_mb: " << FUSE.config.chache_size_mb << "\n"
    << "         mongo_chunk_size: " << FUSE.config.mongo_chunk_size << "\n"
    )
   return fuse_main(FUSE.args.argc, FUSE.args.argv, &FUSE.filesystem_operations, NULL);
}
