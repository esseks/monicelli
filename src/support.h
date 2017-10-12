// Copyright 2017 the Monicelli project authors. All rights reserved.
// Use of this source code is governed by a GPLv3 license, see LICENSE.txt.

#ifndef MONICELLI_SUPPORT_H
#define MONICELLI_SUPPORT_H

#include <iostream>
#include <string>

namespace monicelli {

#define USE(x) ((void)(x))

template<typename First> static void print(std::ostream& stream, const First& first) {
  stream << first << ".\n";
}

template<typename First, typename... Tail>
static void print(std::ostream& stream, const First& first, Tail... tail) {
  stream << first << ' ';
  print(stream, tail...);
}

std::string basename(std::string input_filename);

} // namespace monicelli

#endif
