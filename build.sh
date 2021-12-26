#!/bin/bash

BUILDDIR=build

rm -r $BUILDDIR/
mkdir -p $BUILDDIR

OSTYPE=$(uname)

CXX=${CXX-g++}
CCSRC="src/capi.cc \
       src/Watcher.cc \
       src/Options.cc \
       src/Backend.cc \
       src/DirTree.cc \
       src/shared/BruteForceBackend.cc"

CXXFLAGS=${CXXFLAGS-}

# TODO: enable watchman support ?
CXXFLAGS="$CXXFLAGS -std=c++11 -Iinclude -DBRUTE_FORCE"

echo "Building for $OSTYPE"

if [[ $OSTYPE == "Darwin" ]]; then

  $CXX $CCSRC \
    src/macos/FSEventsBackend.cc \
    $CXXFLAGS -dynamiclib -undefined dynamic_lookup -fPIC -o $BUILDDIR/libwatcher.dylib \
    -DFS_EVENTS

elif [[ $OSTYPE == "Linux" ]]; then

  $CXX $CCSRC \
    src/linux/InotifyBackend.cc \
    src/unix/legacy.cc \
    $CXXFLAGS -o $BUILDDIR/libwatcher.so -shared -fPIC \
    -DINOTIFY

else # Windows

  # TODO: Make sure to link with libuv.dll.a and not libuv.dll
  LDFLAGS="${LDFLAGS} ${prefix}/lib/libuv.dll.a"
  $CXX $CCSRC \
    src/windows/WindowsBackend.cc \
    src/windows/win_utils.cc \
    $CXXFLAGS $LDFLAGS -o $BUILDDIR/libwatcher.dll -shared \
    -DWINDOWS -Wl,-no-undefined

fi

echo "done building"
