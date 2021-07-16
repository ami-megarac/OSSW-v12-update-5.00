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
#include <map>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

using json = nlohmann::json;

struct MCAData
{
    union
    {
        struct
        {
            uint64_t reserved0 : 63, valid : 1;
        };
        uint64_t mc_status;
    };

    uint64_t address;
    uint64_t misc;
    uint64_t ctl;
    uint32_t core;
    uint32_t thread;
    uint32_t bank;
    bool cbo;
};

struct TscData
{
    std::string pcu_first_ierr_tsc_lo_cfg = "0x0";
    std::string pcu_first_ierr_tsc_hi_cfg = "0x0";
    std::string pcu_first_mcerr_tsc_lo_cfg = "0x0";
    std::string pcu_first_mcerr_tsc_hi_cfg = "0x0";
    uint64_t pcu_first_ierr_tsc_cfg = 0;
    uint64_t pcu_first_mcerr_tsc_cfg = 0;
    bool ierr_on_socket = false;
    bool mcerr_on_socket = false;
    bool ierr_occured_first = false;
    bool mcerr_occured_first = false;
};

struct TscVariablesNames
{
    std::string pcu_first_ierr_tsc_lo_cfg_varname;
    std::string pcu_first_ierr_tsc_hi_cfg_varname;
    std::string pcu_first_mcerr_tsc_lo_cfg_varname;
    std::string pcu_first_mcerr_tsc_hi_cfg_varname;
};

using MCA = std::map<uint32_t, std::vector<MCAData>>;

class McaDecoder
{
  public:
    McaDecoder(const MCAData& mca) : mca(mca){};

    virtual json decode() = 0;

    virtual json decode_status() = 0;

    const MCAData& mca;
};