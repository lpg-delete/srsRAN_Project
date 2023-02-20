/*
 *
 * Copyright 2021-2023 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#pragma once

#include "srsran/adt/span.h"
#include <fstream>
#include <vector>

namespace srsran {

template <typename T>
class file_sink
{
private:
  std::ofstream binary_file;

public:
  /// Default constructor. It does not open file.
  file_sink() = default;

  /// Constructs a file sink using a file name.
  file_sink(std::string file_name) : binary_file(file_name, std::ios::out | std::ios::binary)
  {
    report_fatal_error_if_not(binary_file.is_open(), "Failed to open file.");
  }

  /// Checks if the file is open.
  bool is_open() const { return binary_file.is_open(); }

  /// Writes
  void write(span<const T> data)
  {
    report_fatal_error_if_not(binary_file.is_open(), "File not opened.");

    binary_file.write(reinterpret_cast<const char*>(data.data()), sizeof(T) * data.size());
  }
};

} // namespace srsran