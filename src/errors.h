#ifndef MONICELLI_ERRORS_H
#define MONICELLI_ERRORS_H

// Copyright 2017 the Monicelli project authors. All rights reserved.
// Use of this source code is governed by a GPLv3 license, see LICENSE.txt.

#include "location.h"
#include "support.h"

#include <iostream>
#include <string>

namespace monicelli {

[[noreturn]] void UNREACHABLE(const std::string& message);

class ErrorReportingMixin {
protected:
  explicit ErrorReportingMixin(const std::string& source_filename)
      : source_filename_(source_filename) {}

  const std::string& getSourceFilename() const { return source_filename_; }

  void printErrorLocation(std::ostream& stream, const Location& from, const Location& to);

  template<typename Locatable, typename First>
  [[noreturn]] void error(const Locatable& obj, const First& first) {
    printErrorLocation(std::cerr, obj->getFirstLocation(), obj->getLastLocation());
    print(std::cerr, first);
    exit(1);
  }

  template<typename Locatable, typename First, typename... Tail>
  [[noreturn]] void error(const Locatable& obj, const First& first, Tail... tail) {
    printErrorLocation(std::cerr, obj->getFirstLocation(), obj->getLastLocation());
    print(std::cerr, first, tail...);
    exit(1);
  }

  template<typename First>
  [[noreturn]] void error(const Location& where, const First& first) {
    printErrorLocation(std::cerr, where, where);
    print(std::cerr, first);
    exit(1);
  }

  template<typename First, typename... Tail>
  [[noreturn]] void error(const Location& where, const First& first, Tail... tail) {
    printErrorLocation(std::cerr, where, where);
    print(std::cerr, first, tail...);
    exit(1);
  }

  private : std::string source_filename_;
};

} // namespace monicelli

#endif
