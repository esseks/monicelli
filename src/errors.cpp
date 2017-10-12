// Copyright 2017 the Monicelli project authors. All rights reserved.
// Use of this source code is governed by a GPLv3 license, see LICENSE.txt.

#include "errors.h"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

namespace monicelli {

[[noreturn]] void UNREACHABLE(const std::string& message) {
  std::cerr << message << '\n';
  abort();
}

static std::string getNthLine(std::istream& file, int lineNumber) {
  std::string line;
  for (int i = 0; i < lineNumber; ++i) {
    if (!std::getline(file, line)) {
      return "";
    }
  }
  return line;
}

void ErrorReportingMixin::printErrorLocation(std::ostream& stream, const Location& from,
                                             const Location& to) {
  std::ifstream file{source_filename_};
  auto line = getNthLine(file, from.getLine());

  if (!line.empty()) {
    stream << line << '\n';

    for (int i = 1; i < from.getColumn(); ++i) {
      stream << ' ';
    }
    stream << '^';

    int area_limit = from.getLine() == to.getLine() ? to.getColumn() - 1 : line.size();

    // This one will not get printed if from and to are the same.
    for (int i = from.getColumn(); i < area_limit; ++i) {
      stream << '~';
    }
  }

  stream << '\n' << from << ": error: ";
}

} // namespace monicelli
