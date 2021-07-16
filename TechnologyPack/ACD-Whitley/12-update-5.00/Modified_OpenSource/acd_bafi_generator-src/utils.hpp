/******************************************************************************
 *
 * INTEL CONFIDENTIAL
 *
 * Copyright 2021 Intel Corporation.
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

#include <nlohmann/json.hpp>

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
  if (startsWith(input, "UA") || startsWith(input, "N/A") || input == "") {
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

void showBdfDescription(json deviceMap, nlohmann::ordered_json &bdfObject)
{
  if (deviceMap != NULL)
  {
    for (const auto [devKey, devVal] : deviceMap["devicesMap"].items())
    {
      if (devVal["bus"] == std::string(bdfObject["Bus"]) &&
      devVal["device"] == std::string(bdfObject["Device"]) &&
      devVal["function"] == std::string(bdfObject["Function"]))
      {
        bdfObject["Description"] = devVal["description"];
      }
    }
  }
}

void getBdfFromFirstMcerrSection(std::string bdfString,
                                  nlohmann::ordered_json &bdfObj)
{
  if (bdfString != "Please refer to system address map")
  {
    std::size_t left = bdfString.find("Bus");
    std::size_t right = bdfString.find(",");
    bdfObj["Bus"] = bdfString.substr(left, right).substr(5);
    left = bdfString.find("Device");
    right = bdfString.substr(right + 2).find(",");
    bdfObj["Device"] = bdfString.substr(left, right).substr(8);
    left = bdfString.find("Function");
    bdfObj["Function"] =
      bdfString.substr(left, bdfString.size()).substr(10);
  }
}

void getBdfFromIoErrorsSection(std::string bdfString,
                                nlohmann::ordered_json &bdfObj)
{
  if (bdfString != "Please refer to system address map")
  {
    std::size_t left = bdfString.find("Bus");
    std::size_t right = bdfString.find("] ");
    bdfObj["Bus"] = bdfString.substr(left, right).substr(4);
    left = bdfString.find("Device");
    right = bdfString.substr(right + 2).find("]");
    bdfObj["Device"] = bdfString.substr(left, right).substr(7);
    left = bdfString.find("Function");
    bdfObj["Function"] = bdfString
    .substr(left, bdfString.size() - left - 1).substr(9);
  }
}
