#include "Options.hh"
#include <cstring>

Options::Options() {
  setBackend(DEFAULT_BACKEND);
}

Options::~Options() {
  delete backend;
}

void Options::addIgnore(const char *ignore) {
  auto str_ignore = std::string(ignore);
  this->ignores.emplace(str_ignore);
}

void Options::addIgnoreGlob(const char *glob) {
    auto str_glob = std::string(glob);
    this->ignoreGlobs.emplace(str_glob);
}

void Options::setBackend(const char *backend) {
  if (this->backend != nullptr) {
    delete this->backend;
  }

  size_t len = strlen(backend)+1;
  this->backend = new char[len];

  strcpy(this->backend, backend);
}
