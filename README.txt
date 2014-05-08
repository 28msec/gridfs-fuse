--------------------------------------------------------------
GridFS Fuse - A FUSE filesystem for MongoDB's GridFS
--------------------------------------------------------------

GridFS Fuse is a user-based filesystem based on a FUSE that is based on
MongoDB's GridFS. Specifically, it allows to mount a MongoDB GridFS
database enabling filesystem like reads and writes for files stored
in MongoDB.

It currently supports files, directories, and symbolic links but not
all system call are available. For example, get-, set-, and listxattr
are not implementd.

In order to provide good performance, the module uses memcached for
caching file attributes such that not every getattr call results in
a read in MongoDB.

Build & Installation
--------------------

In order to build the filesystem module, the following dependencies
need to be met:
- CMake >= 2.6
- libfuse-dev
- libmemcached-dev
- MongoDB Driver >= 2.6.1
- boost system >= 1.49

To build the module, you need to configure a build directory using
CMake and use make to do the actual build.

To build a Debian package you can call

- cmake -P ppa/PPAGridFS.cmake in your build directory or
- debuild -S in the ppa/ppaingGridFS/gridfs-0.8.0 directory (version might be different)


Usage
-----

The module is available in the form of a command-line client. You can get
its help using: ./bin/gridfs  -h

For example, to mount a MongoDB gridfs database named foobar into the local directory foobar, you
could use

./bin/gridfs foobar -o mongo_db=fuse -f

The -f option keeps the process running in the foreground

Some MongoDB related options are:

-o mongo_conn_string=STRING        connection string (e.g. "replica-set/host:port\,host:port"; default: localhost:27017)
-o mongo_user=STRING               user name for mongo db authentication
-o mongo_password=STRING           password for mongo db authentication
-o mongo_collection_prefix=STRING  prefix for the gridfs collections (default: fs)


Testing
-------

Once you have successfully built the module, you can run a simple test using ctest.
The output could as follows:

Test project /home/vagrant/sausalito/gridfs/build
    Start 1: gridfs-fuse-simple
1/2 Test #1: gridfs-fuse-simple ...............   Passed    6.73 sec
    Start 2: gridfs-fuse-parallel
2/2 Test #2: gridfs-fuse-parallel .............   Passed    4.56 sec

100% tests passed, 0 tests failed out of 2

Total Test time (real) =  11.30 sec


General Documentation
---------------------

  Source Code
  -----------
  The most important file is src/filesystem_operations.cpp. It contains the implementation
  of all the fuse functions (e.g. getattr or readdir). Mostly, those functions dispatch
  the work to any of the subclasses of FilesystemEntry (e.g. File, Directory, or Symlink).
  Those subclasses take care of the communication with MongoDB.

  The Fuse class in include/gridfs_fuse.h (implementation in src/gridfs_fuse.cpp) is the
  main class that configures fuse and syslog as well as creating connection pools for
  communicating with MongoDB and Memcached.

  Memcached Administration
  ------------------------
  A special component of each mounted filesystem is the proc filesystem. It works similar
  to the Linux proc filesystem and can currently be used to introspect or add available
  Memcached nodes.

  For example,

  ls foobar/proc/instances
    may be used to show all of the memcached nodes that are used as a cache for filesystem
    attributes. They are returned as "server-or-ip:port".

  Currently, localhost:11211 is always used as a default memcached server.

  A new memcached node can be added by simply touching a file with the name and port of
  the new server. For instance,

  touch foobar/proc/instances/192.168.1.50:11211
    would add the memcached server running at 192.168.1.50:11211 to the cluster.

  The implementation responsible for adding new nodes is located in the Proc::create
  function (see src/proc.cpp).

  Memcached nodes are automatically removed if nodes become unavailable during runtime.

  In the future, other features might be added to the proc filesystem.
  

  Memcached Attributes
  --------------------
  As already mentioned, Memcached is used as a distributed cache for storing filesystem
  attributes. The key for each entry in the cache always starts with "a:" to indicate
  that it's a filesystem _a_ttribute. The value of each entry is the binary representation
  of the stat struct defined by FUSE.
