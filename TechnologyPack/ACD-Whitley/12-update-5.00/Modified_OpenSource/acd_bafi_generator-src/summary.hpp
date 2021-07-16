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
#include <array>
#include <map>
#include <string>
#include <aer.hpp>
#include <mca_defs.hpp>
#include <tor_defs_icx.hpp>

class Summary
{
  public:
    std::string cpuType;
    MCA mca;
    UncAer uncAer;
    CorAer corAer;
    std::map<uint32_t, TscData> tsc;
    std::map<std::string, std::array<uint64_t, 2>> memoryMap;
    std::map<uint32_t, PackageThermStatus> thermStatus;
};