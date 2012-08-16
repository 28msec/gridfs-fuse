#include <pthread.h>

namespace gridfs {

  class Lock
  {
    public:
      Lock(pthread_mutex_t& aMutex):
        theMutex(aMutex)
      {
        pthread_mutex_lock(&theMutex);
      }

      ~Lock()
      {
        pthread_mutex_unlock(&theMutex);
      }

    private:
      pthread_mutex_t& theMutex;
  };

}
