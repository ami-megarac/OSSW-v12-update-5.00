/******************************************************************************
 *
 * INTEL CONFIDENTIAL
 *
 * Copyright 2020 Intel Corporation.
 *
 * This software and the related documents are Intel copyrighted materials, and
 * your use of them is governed by the express license under which they were
 * provided to you ("License"). Unless the License provides otherwise, you may
 * not use, modify, copy, publish, distribute, disclose or transmit this
 * software or the related documents without Intel's prior written permission.
 *
 * This software and the related documents are provided as is, with no express
 * or implied warranties, other than those that are expressly stated in
 * the License.
 *
 ******************************************************************************/

#pragma once
#include <chrono>
#include <iomanip>
#include <ios>
#include <iostream>
#include <map>
#include <optional>
#include <sstream>
#include <stdio.h>
#include <string>

[[nodiscard]] static const inline bool startsWith(const std::string &input,
                                                  std::string &&search) {
  return (input.substr(0, search.length()) == search);
}

template <typename T>
[[nodiscard]] bool str2uint(const std::string &stringValue, T &output,
                            uint8_t base = 16) {
  try {
    output = std::stoull(stringValue, nullptr, base);
  } catch (...) {
    std::cerr << "Cannot convert to unsigned int: " << stringValue << "\n";
    return false;
  }
  return true;
}

template <typename T>
[[nodiscard]] inline std::string int_to_hex(T val, size_t width = sizeof(T),
                                            bool prefix = true) {
  std::stringstream ss;
  if (prefix)
    ss << "0x";
  ss << std::setfill('0') << std::setw(width) << std::hex << (val | 0);
  return ss.str();
}

template <typename T>
[[nodiscard]] std::optional<std::string>
getDecoded(std::map<T, const char *> decodingTable, T toDecode) {
  const auto &item = decodingTable.find(toDecode);
  if (item != decodingTable.cend()) {
    return item->second;
  }
  return {};
}

[[nodiscard]] bool checkInproperValue(const std::string &input) {
  if (startsWith(input, "UA") || startsWith(input, "N/A")) {
    return true;
  }
  return false;
}

[[nodiscard]] std::string countDuration(
                                    std::chrono::steady_clock::time_point end,
                                    std::chrono::steady_clock::time_point start)
{
    auto durationSeconds =
        std::chrono::duration_cast<std::chrono::seconds> (end - start);
    auto durationMiliseconds =
        std::chrono::duration_cast<std::chrono::milliseconds> (end - start);
    std::stringstream ss;
    ss << durationSeconds.count() << "." << durationMiliseconds.count() << "s";
    return ss.str();
}
