#ifndef OPTIONS_HH
#define OPTIONS_HH

#include <unordered_set>
#include <string>

#define DEFAULT_BACKEND "default"

struct Options {
  std::unordered_set<std::string> ignores = std::unordered_set<std::string>();
  char *backend = nullptr;

  Options();
  ~Options();

  void addIgnore(const char *ignore);
  void setBackend(const char *backend);
};

#endif
