```
CXX="zig c++ -target x86_64-windows-gnu" ./build.sh
```

# watcher

A native C++ &nbsp;
    <a href="https://julialang.org">
        <img src="https://raw.githubusercontent.com/JuliaLang/julia-logo-graphics/master/images/julia.ico" width="16em">
        Julia
    </a> module for querying and subscribing to filesystem events. Ported over from the Javascript library [parcel-bundler/watcher](https://github.com/parcel-bundler/parcel) which powers the [parcel](https://github.com/parcel-bundler/parcel) Javascript bundler.

## Features

- **Watch** - subscribe to realtime recursive directory change notifications when files or directories are created, updated, or deleted.
- **Query** - performantly query for historical change events in a directory, even when your program is not running.
- **Native** - implemented in C++ for performance and low-level integration with the operating system.
- **Cross platform** - includes backends for macOS, Linux, Windows, and Watchman.
- **Performant** - events are throttled in C++ so the JavaScript thread is not overwhelmed during large filesystem changes (e.g. `git checkout` or `npm install`).
- **Scalable** - tens of thousands of files can be watched or queried at once with good performance.

## Example

See [BetterFileWatching.jl](https://github.com/JuliaPluto/BetterFileWatching.jl).

## Watching

`@parcel/watcher` supports subscribing to realtime notifications of changes in a directory. It works recursively, so changes in sub-directories will also be emitted.

Events are throttled and coalesced for performance during large changes like `git checkout` or `npm install`, and a single notification will be emitted with all of the events at the end.

Only one notification will be emitted per file. For example, if a file was both created and updated since the last event, you'll get only a `create` event. If a file is both created and deleted, you will not be notifed of that file. Renames cause two events: a `delete` for the old name, and a `create` for the new name.

Events have three properties:
- `path` - the absolute realpath to the file.
- `is_created` - whether this is a creation event
- `is_deleted` - whether this is deleted (this is modified is neither is true).

To unsubscribe from change notifications, call the `unsubscribe` method on the returned subscription object.

```javascript
unsubscribe(watcher);
```

`@parcel/watcher` has the following watcher backends, listed in priority order:

- [FSEvents](https://developer.apple.com/documentation/coreservices/file_system_events) on macOS
- [Watchman](https://facebook.github.io/watchman/) if installed
- [inotify](http://man7.org/linux/man-pages/man7/inotify.7.html) on Linux
- [ReadDirectoryChangesW](https://msdn.microsoft.com/en-us/library/windows/desktop/aa365465%28v%3Dvs.85%29.aspx) on Windows

You can specify the exact backend you wish to use by passing the `backend` option. If that backend is not available on the current platform, the default backend will be used instead. See below for the list of backend names that can be passed to the options.

## Querying

`@parcel/watcher` also supports querying for historical changes made in a directory, even when your program is not running. This makes it easy to invalidate a cache and re-build only the files that have changed, for example. It can be **significantly** faster than traversing the entire filesystem to determine what files changed, depending on the platform.

In order to query for historical changes, you first need a previous snapshot to compare to. This can be saved to a file with the `writeSnapshot` function, e.g. just before your program exits.

```javascript
write_snapshot(dirPath, snapshotPath)
```

When your program starts up, you can query for changes that have occurred since that snapshot using the `get_events_since` function.

```javascript
get_event_since(dirPath, snapshotPath)
```

The events returned are exactly the same as the events that would be passed to the `subscribe` callback (see above).

`@parcel/watcher` has the following watcher backends, listed in priority order:

- [FSEvents](https://developer.apple.com/documentation/coreservices/file_system_events) on macOS
- [Watchman](https://facebook.github.io/watchman/) if installed
- [fts](http://man7.org/linux/man-pages/man3/fts.3.html) (brute force) on Linux
- [FindFirstFile](https://docs.microsoft.com/en-us/windows/desktop/api/fileapi/nf-fileapi-findfirstfilea) (brute force) on Windows

The FSEvents (macOS) and Watchman backends are significantly more performant than the brute force backends used by default on Linux and Windows, for example returning results in miliseconds instead of seconds for large directory trees. This is because a background daemon monitoring filesystem changes on those platforms allows us to query cached data rather than traversing the filesystem manually (brute force).

macOS has good performance with FSEvents by default. For the best performance on other platforms, install [Watchman](https://facebook.github.io/watchman/) and it will be used by `@parcel/watcher` automatically.

You can specify the exact backend you wish to use by passing the `backend` option. If that backend is not available on the current platform, the default backend will be used instead. See below for the list of backend names that can be passed to the options.

## Options

All of the APIs in `@parcel/watcher` support the following options, which are passed as an object as the last function argument.

- `ignore` - an array of paths or glob patterns to ignore. uses [`is-glob`](https://github.com/micromatch/is-glob) to distinguish paths from globs. glob patterns are parsed with [`micromatch`](https://github.com/micromatch/micromatch) (see [features](https://github.com/micromatch/micromatch#matching-features)).
  - paths can be relative or absolute and can either be files or directories. No events will be emitted about these files or directories or their children. 
  - glob patterns match on relative paths from the root that is watched. No events will be emitted for matching paths.
- `backend` - the name of an explicitly chosen backend to use. Allowed options are `"fs-events"`, `"watchman"`, `"inotify"`, `"windows"`, or `"brute-force"` (only for querying). If the specified backend is not available on the current platform, the default backend will be used instead.

## Parcel/Javascript version

<<<<<<< HEAD
- [Parcel 2](https://parceljs.org/)
- [VSCode](https://code.visualstudio.com/updates/v1_62#_file-watching-changes)
- [Tailwind CSS Intellisense](https://github.com/tailwindlabs/tailwindcss-intellisense)
- [Gatsby Cloud](https://twitter.com/chatsidhartha/status/1435647412828196867)
- [Nx](https://nx.dev)
=======
This repository is built on the [awesome work](https://github.com/parcel-bundler/watcher) from the parcel bundler team.
>>>>>>> 18d62d7 (readme)

## License

MIT
