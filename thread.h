#ifndef _NDB_THREAD_H_
#define _NDB_THREAD_H_

#include <pthread.h>
#include <vector>

#include "macros.h"
#include "util.h"

/**
 * Threads in NuDB
 *
 * Threads in NuDB should never be explicitly created with
 * pthread_create()! This allows us to wrap threads with handlers on
 * thread completion
 *
 * Note that ndb_threads are thin wrappers around pthread's API
 */

class ndb_thread : private util::noncopyable {
public:

  typedef void (*run_t)(void);

  ndb_thread(bool daemon = false)
    : body(NULL), daemon(daemon) {}
  ndb_thread(run_t body, bool daemon = false)
    : body(body), daemon(daemon) {}
  virtual ~ndb_thread() {}

  inline pthread_t
  pthread_id() const
  {
    return p;
  }

  void start();
  void join();
  virtual void run();

  typedef void (*callback_t)(ndb_thread *);

  /**
   * callback registration is assumed to happen at static initialization time,
   * and thus be single-threaded
   */
  static bool register_completion_callback(callback_t callback);

private:
  pthread_t p;
  run_t body;
  const bool daemon;
  static std::vector<callback_t> &completion_callbacks();

  void on_complete();
  static void *pthread_bootstrap(void *p);
};

#define NDB_THREAD_REGISTER_COMPLETION_CALLBACK(fn) \
  static bool _ndb_thread_callback_register_ ## __LINE__ UNUSED = \
    ::ndb_thread::register_completion_callback(fn);

#endif /* _NDB_THREAD_H_ */
