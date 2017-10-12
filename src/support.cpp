// Copyright 2017 the Monicelli project authors. All rights reserved.
// Use of this source code is governed by a GPLv3 license, see LICENSE.txt.

#include "support.h"

namespace monicelli {

std::string basename(std::string input_filename) {
  auto base_name_start = input_filename.find_last_of("\\/");
  if (base_name_start != std::string::npos) {
    input_filename = input_filename.substr(base_name_start + 1);
  }
  if (input_filename.empty()) return "";
  return input_filename;
}

} // namespace monicelli
