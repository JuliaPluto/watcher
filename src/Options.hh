#ifndef OPTIONS_HH
#define OPTIONS_HH

#include "Glob.hh"
#include <string>
#include <unordered_set>

#define DEFAULT_BACKEND "default"

struct Options {
  std::unordered_set<std::string> ignores = std::unordered_set<std::string>();
  std::unordered_set<Glob> ignoreGlobs = std::unordered_set<Glob>();
  char *backend = nullptr;

  Options();
  ~Options();

  void addIgnore(const char *ignore);
  void addIgnoreGlob(const char *glob);
  void setBackend(const char *backend);
};

#endif
