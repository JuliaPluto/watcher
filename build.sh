#!/bin/bash

OSTYPE=$(uname)

if [[ $OSTYPE == "Darwin" ]]; then

  echo "building watcher lib for macos"

  g++ \
    src/capi.c \
    src/Watcher.cc \
    src/Options.cc \
    src/Backend.cc \
    src/DirTree.cc \
    src/shared/BruteForceBackend.cc \
    src/macos/FSEvents/Backend.cc \
    -Linclude -o libwatcher.so -shared -fPIC \
    -DFS_EVENTS -DBRUTE_FORCE

else

  echo "building watcher lib for linux"

  g++ \
    src/capi.c \
    src/Watcher.cc \
    src/Options.cc \
    src/Backend.cc \
    src/DirTree.cc \
    src/shared/BruteForceBackend.cc \
    src/linux/InotifyBackend.cc \
    src/unix/legacy.cc \
    -Linclude -o libwatcher.so -shared -fPIC \
    -DINOTIFY -DBRUTE_FORCE

    # -DWATCHMAN
    # src/watchman/BSER.cc \
    # src/watchman/WatchmanBackend.cc \

fi

echo "done building"
