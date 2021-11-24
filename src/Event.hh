#ifndef EVENT_H
#define EVENT_H

#include <string>
#include <mutex>
#include <map>
#include <vector>

// using namespace Napi;

struct Event {
  std::string path;
  bool isCreated;
  bool isDeleted;
  Event(std::string path) : path(path), isCreated(false), isDeleted(false) {}

  struct JLEvent {
    const char *path;
    size_t path_len;
    bool isCreated;
    bool isDeleted;
  };

  Event::JLEvent toJL() {
    Event::JLEvent jlevent = JLEvent {
      path.c_str(),
      path.size(),
      isCreated,
      isDeleted,
    };
    return jlevent;
  }
};

class EventList {
public:
  void create(std::string path) {
    std::lock_guard<std::mutex> l(mMutex);
    Event *event = internalUpdate(path);
    event->isCreated = true;
  }

  Event *update(std::string path) {
    std::lock_guard<std::mutex> l(mMutex);
    return internalUpdate(path);
  }

  void remove(std::string path) {
    std::lock_guard<std::mutex> l(mMutex);
    Event *event = internalUpdate(path);
    if (event->isCreated) {
      mEvents.erase(path);
    } else {
      event->isDeleted = true;
    }
  }

  size_t size() {
    std::lock_guard<std::mutex> l(mMutex);
    return mEvents.size();
  }

  std::vector<Event> getEvents() {
    std::lock_guard<std::mutex> l(mMutex);
    std::vector<Event> eventsCloneVector;
    for(auto it = mEvents.begin(); it != mEvents.end(); ++it) {
      eventsCloneVector.push_back(it->second);
    }
    return eventsCloneVector;
  }

  void clear() {
    std::lock_guard<std::mutex> l(mMutex);
    mEvents.clear();
  }

private:
  mutable std::mutex mMutex;
  std::map<std::string, Event> mEvents;
  Event *internalUpdate(std::string path) {
    auto found = mEvents.find(path);
    if (found == mEvents.end()) {
      auto it = mEvents.emplace(path, Event(path));
      return &it.first->second;
    }

    return &found->second;
  }
};

#endif
