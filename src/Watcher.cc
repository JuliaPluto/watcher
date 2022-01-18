#include "Watcher.hh"
#include <unordered_set>
#include <iostream>

struct WatcherHash {
  std::size_t operator() (std::shared_ptr<Watcher> const &k) const {
    return std::hash<std::string>()(k->mDir);
  }
};

struct WatcherCompare {
  size_t operator() (std::shared_ptr<Watcher> const &a, std::shared_ptr<Watcher> const &b) const {
    return *a == *b;
  }
};

static std::unordered_set<std::shared_ptr<Watcher>, WatcherHash, WatcherCompare> sharedWatchers;

void Watcher::deleteShared(std::shared_ptr<Watcher> watcher) {
  auto found = sharedWatchers.find(watcher);
  if (found != sharedWatchers.end()) {
    sharedWatchers.erase(found);
  }
}

std::shared_ptr<Watcher> Watcher::getShared(std::string dir, uv_async_t *handle, std::unordered_set<std::string> ignore) {
  std::shared_ptr<Watcher> watcher = std::make_shared<Watcher>(dir, ignore, handle);
  auto found = sharedWatchers.find(watcher);
  if (found != sharedWatchers.end()) {
    return *found;
  }

  sharedWatchers.insert(watcher);
  return watcher;
}

void removeShared(Watcher *watcher) {
  for (auto it = sharedWatchers.begin(); it != sharedWatchers.end(); it++) {
    if (it->get() == watcher) {
      sharedWatchers.erase(it);
      break;
    }
  }
}

Watcher::Watcher(std::string dir, std::unordered_set<std::string> ignore, uv_async_t *async_handle)
  : mDir(dir),
    mIgnore(ignore),
    mWatched(false),
    mAsync(async_handle), // We provide our own uv_async_t since Julia override the close callback
    mCallingCallbacks(false) {
      mDebounce = Debounce::getShared();
      mDebounce->add(this, [this] () {
        triggerCallbacks();
      });
    }

Watcher::~Watcher() {
  mDebounce->remove(this);
}

void Watcher::wait() {
  std::unique_lock<std::mutex> lk(mMutex);
  mCond.wait(lk);
}

void Watcher::notify() {
  std::unique_lock<std::mutex> lk(mMutex);
  mCond.notify_all();

  if (mCallbacks.size() > 0 && mEvents.size() > 0) {
    mDebounce->trigger();
  }
}

void Watcher::notifyError(std::exception &err) {
  std::unique_lock<std::mutex> lk(mMutex);
  if (mCallingCallbacks) {
    mCallbackSignal.wait();
    mCallbackSignal.reset();
  }

  mError = err.what();
  triggerCallbacks();
}

void Watcher::triggerCallbacks() {
  std::lock_guard<std::mutex> l(mCallbackEventsMutex);
  if (mCallbacks.size() > 0 && (mEvents.size() > 0 || mError.size() > 0)) {
    if (mCallingCallbacks) {
      mCallbackSignal.wait();
      mCallbackSignal.reset();
    }

    mCallbackEvents = mEvents.getEvents();
    mEvents.clear();

    uv_async_send(mAsync);
  }
}

// TODO: Doesn't this need some kind of locking?
void Watcher::clearCallbacks() {
  mCallbacks.clear();
}

void Watcher::fireCallbacks(uv_async_t *handle) {
  Watcher *watcher = (Watcher *)handle->data;
  watcher->mCallingCallbacks = true;

  watcher->mCallbacksIterator = watcher->mCallbacks.begin();
  while (watcher->mCallbacksIterator != watcher->mCallbacks.end()) {
    auto it = watcher->mCallbacksIterator;

    auto callback = *it;
    size_t n_events = watcher->mCallbackEvents.size();
    std::vector<Event::JLEvent> jl_events(n_events);
    for (auto it = watcher->mCallbackEvents.begin(); it != watcher->mCallbackEvents.end(); it++) {
      Event::JLEvent jl_event = it->toJL();
      jl_events.push_back(std::move(jl_event));
    }

    callback(jl_events.data(), n_events); // call callback

    // If the iterator was changed, then the callback trigged an unwatch.
    // The iterator will have been set to the next valid callback.
    // If it is the same as before, increment it.
    if (watcher->mCallbacksIterator == it) {
      watcher->mCallbacksIterator++;
    }
  }

  watcher->mCallingCallbacks = false;

  if (watcher->mError.size() > 0) {
    // watcher->clearCallbacks();
  }

  if (watcher->mCallbacks.size() == 0) {
    watcher->unref();
  } else {
    watcher->mCallbackSignal.notify();
  }
}

void Watcher::toWatcherEvents(watcher_events_t *watcher_events) {
  Event::JLEvent *jl_events = new Event::JLEvent[mCallbackEvents.size()];

  int i = 0;
  for (auto it = mCallbackEvents.begin(); it != mCallbackEvents.end(); it++) {
    jl_events[i] = it->toJL();
    i++;
  }

  watcher_events->n_events = mCallbackEvents.size();
  watcher_events->events = jl_events;
}

bool Watcher::watch(callback_func callback, uv_async_t *handle) {
  std::unique_lock<std::mutex> lk(mMutex);
  auto res = mCallbacks.insert(std::move(callback));
  if (res.second && !mWatched) {
    if (mAsync == nullptr) {
      if (handle != nullptr) {
        mAsync = handle;
      } else {
        mAsync = new uv_async_t;
        mAsync->data = (void *)this;
        uv_async_init(uv_default_loop(), mAsync, Watcher::fireCallbacks);
      }
    }
    mWatched = true;
    return true;
  }

  return false;
}

bool Watcher::unwatch(callback_func callback) {
  std::unique_lock<std::mutex> lk(mMutex);

  bool removed = false;
  for (auto it = mCallbacks.begin(); it != mCallbacks.end(); it++) {
    if (*it == callback) {
      mCallbacksIterator = mCallbacks.erase(it);
      removed = true;
      break;
    }
  }

  if (removed && mCallbacks.size() == 0) {
    unref();
    return true;
  }

  return false;
}

void Watcher::unref() {
  if (mCallbacks.size() == 0 && !mCallingCallbacks) {
    if (mWatched) {
      mWatched = false;
      // uv_close((uv_handle_t *)mAsync, Watcher::onClose);
    }

    removeShared(this);
  }
}

void Watcher::onClose(uv_handle_t *handle) {
  delete (uv_async_t *)handle;
}

bool Watcher::isIgnored(std::string path) {
  for (auto it = mIgnore.begin(); it != mIgnore.end(); it++) {
    auto dir = *it + DIR_SEP;
    if (*it == path || path.compare(0, dir.size(), dir) == 0) {
      return true;
    }
  }

  return false;
}
