#ifndef WATCHER_H
#define WATCHER_H

#include "Debounce.hh"
#include "DirTree.hh"
#include "Event.hh"
#include "Signal.hh"
#include <condition_variable>
#include <cstring>
#include <errno.h>
#include <functional>
#include <set>
#include <unordered_set>
#include <uv.h>

typedef void (*callback_func)(Event::JLEvent *, size_t);

typedef struct watcher_events {
  size_t n_events;
  Event::JLEvent *events;
} watcher_events_t;

extern "C" {
uv_loop_t *jl_global_event_loop();
}

struct Watcher {
  std::string mDir;
  std::unordered_set<std::string> mIgnore;
  EventList mEvents;
  void *state;
  bool mWatched;

  Watcher(std::string dir, std::unordered_set<std::string> ignore, uv_async_t *async_handle=nullptr);
  ~Watcher();

  bool operator==(const Watcher &other) const {
    return mDir == other.mDir && mIgnore == other.mIgnore;
  }

  void wait();
  void notify();
  void notifyError(std::exception &err);
  bool watch(callback_func callback, uv_async_t *handle=nullptr);
  bool unwatch(callback_func callback);
  void unref();
  bool isIgnored(std::string path);

  static std::shared_ptr<Watcher>
  getShared(std::string dir, uv_async_t *handle, std::unordered_set<std::string> ignore);
  void toWatcherEvents(watcher_events_t *events);

private:
  std::mutex mMutex;
  std::mutex mCallbackEventsMutex;
  std::condition_variable mCond;
  uv_async_t *mAsync;
  std::set<callback_func> mCallbacks;
  std::set<callback_func>::iterator mCallbacksIterator;
  bool mCallingCallbacks;
  std::vector<Event> mCallbackEvents;
  std::shared_ptr<Debounce> mDebounce;
  Signal mCallbackSignal;
  std::string mError;

  // Value callbackEventsToJS(const Env& env);
  void clearCallbacks();
  void triggerCallbacks();
  static void fireCallbacks(uv_async_t *handle);
  static void onClose(uv_handle_t *handle);
};

class WatcherError : public std::runtime_error {
public:
  Watcher *mWatcher;
  WatcherError(std::string msg, Watcher *watcher)
      : std::runtime_error(msg), mWatcher(watcher) {}
  WatcherError(const char *msg, Watcher *watcher)
      : std::runtime_error(msg), mWatcher(watcher) {}
};

#endif
