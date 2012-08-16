#include <config.h>

#include <directory.h>
#include <log4cxx/gridfs_logging.h>

#include <sstream>
#include <set>

namespace gridfs {

  void
  Directory::list(void* buf, fuse_fill_dir_t filler)
  {
    GRIDFS_INIT_LOGGER("gridfs.dir.Directory.list")

    // get all entries
    std::auto_ptr<mongo::DBClientCursor> lFileEntries(list());
    size_t filename_pos = path().length() + 1; // +1 because of the path will have a trailing /
    std::set<std::string> lEntries; // we store the names in here to eliminate stored duplicates in mongo
    std::set<std::string>::iterator iter;

    // default fileentries
    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);

    // eliminate duplicates
    while(lFileEntries->more()){
      std::string lFilePath = lFileEntries->next().getStringField("filename");
      lEntries.insert(lFilePath);
    }

    // iterate over feched fileentries and fill result buffer
    for( iter = lEntries.begin() ; iter != lEntries.end(); iter++ ){
      std::string lFilePath = *iter;
      std::string lFileName = lFilePath.substr(filename_pos);
      _LOG_DEBUG("found filepath: " << lFilePath << " (name: " << lFileName << ")")

      // TODO DK: we can also fill the stat but I don't see the benefit of it (getattr is called 
      // anyway). So, for performance reasons we don't do it right now
      /*mongo::GridFile lFile = gridfs().findFile(lFilePath.c_str()); 
      struct stat lStat;
      memset(&lStat, 0, sizeof(struct stat));
      stat(lFile,&lStat);
     
      // we pass 0 as offset to the filler function -> The filler function will not 
      // return '1' (unless an error happens), so the whole directory is read in a 
      // single iteration operation. This works just like the old getdir() method.
      filler(buf, lFileName.c_str(), &lStat, 0);*/
      filler(buf, lFileName.c_str(), NULL, 0);
    }

  }

  bool
  Directory::isEmpty()
  {
    // get all entries
    std::auto_ptr<mongo::DBClientCursor> lFileEntries(list());
    return !lFileEntries->more();
  }
 
  mongo::DBClientCursor*
  Directory::list()
  {
    // filter by regular expression form dir path to the next /
    const std::string lRegex = "^" + pathregex() + "/[^/]*$";
    mongo::BSONObj lQuery = BSON("filename" <<
                              BSON( "$regex" << lRegex ));
    std::auto_ptr<mongo::DBClientCursor> lFileEntries;
    lFileEntries = gridfs().list(lQuery);
    return lFileEntries.release(); // release pointer ownership
  }
  
  const std::string
  Directory::pathregex()
  {
    // we have to escape all special characters (e.g. '.' -> '\\.') from the path
    int len = path().length();
    std::stringstream regex;
    for (int i = 0; i < len; i++) {
      char ch = path().at(i);
      switch(ch)
      {
        case '.':
        case '^':
        case '$':
        case '|':
        case '(':
        case ')':
        case '[':
        case ']':
        case '*':
        case '+':
        case '?':
        case '\\':
          regex << '\\' << ch; break;
        default:
          regex << ch; break;
      }
    }
    return regex.str();
  }

}
