#include "capi.h"
#include <iostream>

Options *watcher_new_options() {
  Options *options = new Options();
  return options;
}

void dummy_callback(Event::JLEvent *, size_t) {}

void watcher_options_add_ignore(Options *options, const char *ignore) {
  options->addIgnore(ignore);
}

void watcher_options_set_backend(Options *options, const char *backend) {
  options->setBackend(backend);
}

void watcher_delete_options(Options *options) { delete options; }

size_t watcher_watcher_handle_sizeof() { return sizeof(watcher_handle_t); }

watcher_error_t *watcher_write_snapshot(const char *dir, const char *snapshot,
                                        Options *options) {
  std::shared_ptr<Watcher> watcher = Watcher::getShared(
      std::string(dir), nullptr, options->ignores, options->ignoreGlobs);

  std::shared_ptr<Backend> backend = Backend::getShared(options->backend);

  std::string snapshot_path(snapshot);
  backend->writeSnapshot(*watcher, &snapshot_path);

  watcher->unref();
  backend->unref();

  return nullptr;
}

watcher_error_t *watcher_get_events_since(const char *dir, const char *snapshot,
                                          watcher_events_t *watcher_events,
                                          Options *options) {
  std::shared_ptr<Watcher> watcher = Watcher::getShared(
      std::string(dir), nullptr, options->ignores, options->ignoreGlobs);

  std::shared_ptr<Backend> backend = Backend::getShared(options->backend);

  std::string snapshot_path(snapshot);
  backend->getEventsSince(*watcher, &snapshot_path);

  std::vector<Event> events = watcher->mEvents.getEvents();

  Event::JLEvent *jl_events = new Event::JLEvent[events.size()];
  int i = 0;
  for (auto it = events.begin(); it != events.end(); it++) {
    jl_events[i] = it->toJL();
    i++;
  }

  watcher_events->n_events = events.size();
  watcher_events->events = jl_events;

  watcher->unref();
  backend->unref();

  return nullptr;
}

watcher_error_t *watcher_subscribe(const char *dir, uv_async_t *handle,
                                   Options *options,
                                   watcher_handle_t *watcher_handle) {
  std::shared_ptr<Watcher> watcher = Watcher::getShared(
      std::string(dir), handle, options->ignores, options->ignoreGlobs);

  std::shared_ptr<Backend> backend = Backend::getShared(options->backend);
  backend->watch(*watcher);
  watcher->watch(&dummy_callback, handle);

  watcher_handle->watcher = watcher;
  watcher_handle->backend = backend;

  return nullptr;
}

watcher_error_t *watcher_unsubscribe(watcher_handle_t *handle) {
  std::shared_ptr<Watcher> watcher = handle->watcher;
  std::shared_ptr<Backend> backend = handle->backend;
  bool shouldUnwatch = watcher->unwatch(&dummy_callback);

  if (shouldUnwatch) {
    backend->unwatch(*watcher);
    Watcher::deleteShared(watcher);
  }

  return nullptr;
}

watcher_error_t *watcher_watcher_get_events(watcher_handle_t *watcher_handle,
                                            watcher_events_t *watcher_events) {
  watcher_handle->watcher->toWatcherEvents(watcher_events);
  return nullptr;
}

void watcher_delete_events(watcher_events_t *watcher_events) {
  if (watcher_events == nullptr || watcher_events->events == nullptr)
    return;

  for (size_t i = 0; i < watcher_events->n_events; ++i) {
    char *path = watcher_events->events[i].path;
    delete path;
  }

  delete watcher_events->events;
}
