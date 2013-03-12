#pragma once

#include <mongo/client/connpool.h>

namespace gridfs {

  class AuthHook : public mongo::DBConnectionHook
  {
    private:
      AuthHook(){}

    public:
      AuthHook(char* aDBName, char* aUsername, char* aPassword);

      virtual void onCreate(mongo::DBClientBase *aConn);

      virtual void onHandedOut(mongo::DBClientBase* /*aConn*/) {}

      virtual void onDestroy(mongo::DBClientBase* /*aConn*/) {}

    private:
      const std::string theUsername;
      const std::string thePassword;
      const std::string theDBName;

  };

}
