#ifndef CAPI_H
#define CAPI_H

#include "Backend.hh"
#include "Options.hh"
#include "Watcher.hh"

typedef struct watcher_error {
  char *msg;
} watcher_error_t;

#ifdef __cplusplus
extern "C" {
#endif

void watcher_delete_error(watcher_error_t *err);

Options *watcher_new_options();
void watcher_options_add_ignore(Options *options, const char *ignore);
void watcher_options_set_backend(Options *options, const char *backend);
void watcher_delete_options(Options *options);

watcher_error_t *watcher_write_snapshot(const char *dir, const char *snapshot);
watcher_error_t *watcher_get_events_since(const char *dir, const char *snapshot,
                                          watcher_events_t *watcher_events);

watcher_error_t *watcher_subscribe(const char *dir, uv_async_t *async_handle,
                                   Options *options);
watcher_error_t *watcher_unsubscribe(const char *dir);

watcher_error_t *watcher_watcher_get_events(Watcher *watcher,
                                            watcher_events_t *watcher_events);
Watcher *watcher_get_watcher(const char *dir, Options *options);

#ifdef __cplusplus
}
#endif

#endif
