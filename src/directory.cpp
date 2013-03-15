#include "gridfs_fuse.h"

#include "directory.h"

#include <sstream>
#include <set>

namespace gridfs {

  void
  Directory::list(void* buf, fuse_fill_dir_t filler)
  {
    // get all entries
    std::auto_ptr<mongo::DBClientCursor> lFileEntries(list());

    // +1 because of the path will have a trailing /
    size_t filename_pos = path().length() + 1;
    std::set<std::string> lEntries;

    // default fileentries
    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);

    // eliminate duplicates
    while (lFileEntries->more())
    {
      std::string lFilePath = lFileEntries->next().getStringField("filename");
      const std::pair<std::set<std::string>::iterator, bool>& lRes =
        lEntries.insert(lFilePath);

      if (lRes.second) // no dup
      {
        std::string lFileName = (*lRes.first).substr(filename_pos);
        filler(buf, lFileName.c_str(), NULL, 0);
      }
    }
  }

  bool
  Directory::isEmpty()
  {
    std::auto_ptr<mongo::DBClientCursor> lFileEntries(list());
    return !lFileEntries->more();
  }
 
  mongo::DBClientCursor*
  Directory::list()
  {
    // filter by regular expression form dir path to the next /
    const std::string lRegex = "^" + pathregex() + "/[^/]*$";
    mongo::BSONObj lQuery = BSON("filename" << BSON("$regex" << lRegex));

    return gridfs().list(lQuery).release();
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
