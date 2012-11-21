#include "gridfs_fuse.h"

int
main(int argc, char **argv)
{
  using namespace gridfs;

  FUSE.init(argc, argv);
  FUSE.createRootDir();

  return fuse_main(
      FUSE.args.argc,
      FUSE.args.argv,
      &FUSE.filesystem_operations,
      NULL);
}
