#include "capi.h"
#include <iostream>

Options *watcher_new_options() {
  Options *options = new Options();
  return options;
}

void watcher_options_add_ignore(Options *options, const char *ignore) {
  options->addIgnore(ignore);
}

void watcher_options_set_backend(Options *options, const char *backend) {
  options->setBackend(backend);
}

void watcher_delete_options(Options *options) {
  delete options;
}


watcher_error_t *watcher_write_snapshot(const char *dir, const char *snapshot) {
  std::unordered_set<std::string> ignores;
  std::shared_ptr<Watcher> watcher = Watcher::getShared(
    std::string(dir),
    nullptr,
    ignores
  );

  std::shared_ptr<Backend> backend = Backend::getShared(DEFAULT_BACKEND);

  std::string snapshot_path(snapshot);
  backend->writeSnapshot(*watcher, &snapshot_path);

  watcher->unref();
  backend->unref();

  return nullptr;
}

watcher_error_t *watcher_get_events_since(const char *dir, const char *snapshot, callback_func callback) {
  std::unordered_set<std::string> ignores;
  std::shared_ptr<Watcher> watcher = Watcher::getShared(
    std::string(dir),
    nullptr,
    ignores
  );

  std::shared_ptr<Backend> backend = Backend::getShared(DEFAULT_BACKEND);

  std::string snapshot_path(snapshot);
  backend->getEventsSince(*watcher, &snapshot_path);

  std::vector<Event> events = watcher->mEvents.getEvents();
  std::vector<Event::JLEvent> jl_events(events.size());
  for (auto it = events.begin(); it != events.end(); it++) {
    jl_events.push_back(it->toJL());
  }

  callback(jl_events.data(), jl_events.size());

  watcher->unref();
  backend->unref();

  return nullptr;
}

watcher_error_t *watcher_subscribe(const char *dir, uv_async_t *handle, Options *options) {
  std::unordered_set<std::string> ignores = std::unordered_set<std::string>();
  std::shared_ptr<Watcher> watcher = Watcher::getShared(
    std::string(dir),
    handle,
    ignores
  );

  std::shared_ptr<Backend> backend = Backend::getShared(DEFAULT_BACKEND);
  backend->watch(*watcher);
  watcher->watch(&dummy_callback, handle);

  return nullptr;
}

watcher_error_t *watcher_unsubscribe(const char *dir) {
  std::unordered_set<std::string> ignores;
  std::shared_ptr<Watcher> watcher = Watcher::getShared(
    std::string(dir),
    nullptr,
    ignores
  );

  std::shared_ptr<Backend> backend = Backend::getShared(DEFAULT_BACKEND);
  bool shouldUnwatch = watcher->unwatch(&dummy_callback);

  if (shouldUnwatch) {
    backend->unwatch(*watcher);
  }

  return nullptr;
}

watcher_error_t *watcher_watcher_get_events(Watcher *watcher, watcher_events_t *watcher_events) {
  watcher->toWatcherEvents(watcher_events);
  return nullptr;
}

Watcher *watcher_get_watcher(const char *dir, Options *options) {
  std::shared_ptr<Watcher> watcher = Watcher::getShared(
    std::string(dir),
    nullptr,
    options->ignores
  );

  return &(*watcher);
}
