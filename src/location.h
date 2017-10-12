#ifndef MONICELLI_LOCATION_H
#define MONICELLI_LOCATION_H

// Copyright 2017 the Monicelli project authors. All rights reserved.
// Use of this source code is governed by a GPLv3 license, see LICENSE.txt.

#include <iostream>

namespace monicelli {

class Location final {
public:
  Location() : line_(1), column_(1) {}

  int getLine() const { return line_; }
  int getColumn() const { return column_; }

private:
  Location(int line, int column) : line_(line), column_(column) {}

  void advanceColumn(int amount) { column_ += amount; }
  void newLine() {
    column_ = 1;
    line_ += 1;
  }

  int line_;
  int column_;

  friend class Lexer;
};

class LocationMixin {
public:
  Location getFirstLocation() const { return first_location_; }
  Location getLastLocation() const { return last_location_; }

protected:
  LocationMixin() {}

  LocationMixin(Location first, Location last) : first_location_(first), last_location_(last) {}

  Location first_location_;
  Location last_location_;

  friend class Parser;
};

static inline std::ostream& operator<<(std::ostream& stream, const Location& location) {
  return stream << location.getLine() << ':' << location.getColumn();
}

static inline bool operator==(const Location& a, const Location& b) {
  return a.getLine() == b.getLine() && a.getColumn() == b.getColumn();
}

} // namespace monicelli

#endif
