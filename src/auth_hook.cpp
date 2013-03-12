#include <auth_hook.h>

namespace gridfs {

  AuthHook::AuthHook(char* aDBName, char* aUsername, char* aPassword):
    theUsername(aUsername),
    thePassword(aPassword),
    theDBName(aDBName)
  {}

  void AuthHook::onCreate(mongo::DBClientBase *aConn)
  {
    std::string lErrMsg;
    try
    {
      if (aConn->auth(theDBName, theUsername, thePassword, lErrMsg) == false)
      {
        std::cerr << "[Failure] Couldn't authenticate connection to '"
                  << theDBName << "' with user '" << theUsername << "'. Reason: "
                  << lErrMsg << std::endl;
        exit(3);
      }
    }
    catch (std::exception &ex)
    {
      std::cerr << "[Exception] Couldn't authenticate connection to '"
                << theDBName << "' with user '" << theUsername << "'. Reason: "
                << ex.what() << std::endl;
      exit(4);
    }
  }

};
