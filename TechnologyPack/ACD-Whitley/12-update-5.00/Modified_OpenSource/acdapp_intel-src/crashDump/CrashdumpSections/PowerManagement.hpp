/******************************************************************************
 *
 * INTEL CONFIDENTIAL
 *
 * Copyright 2019 Intel Corporation.
 *
 * This software and the related documents are Intel copyrighted materials,
 * and your use of them is governed by the express license under which they
 * were provided to you ("License"). Unless the License provides otherwise,
 * you may not use, modify, copy, publish, distribute, disclose or transmit
 * this software or the related documents without Intel's prior written
 * permission.
 *
 * This software and the related documents are provided as is, with no express
 * or implied warranties, other than those that are expressly stated in the
 * License.
 *
 ******************************************************************************/

#pragma once

#include "crashdump.hpp"

#ifndef SPX_BMC_ACD
#include <cjson/cJSON.h>
#endif
#include <stdint.h>

// PECI sequence
#define PM_CSTATE_PARAM 0x4660B4
#define PM_VID_PARAM 0x488004
#define PM_READ_PARAM 0x1019
#define PM_CORE_OFFSET 24

#define PM_JSON_STRING_LEN 32
#define PM_JSON_CORE_NAME "core%d"
#define PM_JSON_CSTATE_REG_NAME "c_state_reg"
#define PM_JSON_VID_REG_NAME "vid_ratio_reg"

#define PM_NA "N/A"
#define PM_UA "UA:0x%x"
#define PM_UA_CPX "0x%x,UA:0x%x"
#define PM_DF "DF:0x%x"
#define PM_UA_DF "UA:0x%x,DF:0x%x"
#define PM_UA_DF_CPX "0x0,UA:0x%x,DF:0x%x"
#define PM_UINT32_FMT "0x%" PRIx32 ""
#define PM_PCS_88 88
#define JSON_FAILURE 8
/******************************************************************************
 *
 *   Input File Defines
 *
 ******************************************************************************/
#define FILE_PM_KEY "_input_file_pm"
#define FILE_PM_ERR "Error parsing power management section"
#define FILE_PLL_KEY "_input_file_pll"
#define FILE_PLL_ERR "Error parsing Phase Lock Loop section"
/******************************************************************************
 *
 *   Structures
 *
 ******************************************************************************/
typedef struct
{
    uint32_t u32CState;
    uint32_t u32VidRatio;
    uint8_t ccCstate;
    int retCstate;
    uint8_t ccVratio;
    int retVratio;
} SCpuPowerState;

typedef struct
{
    uint32_t uValue;
    bool bInvalid;

} SPmRegRawData;

typedef struct
{
    char* secName;
    char* regName;
    bool coreScope;
    uint16_t param;
} SPmEntry;

typedef int (*PowerManagementStatusRead)(crashdump::CPUInfo& cpuInfo,
                                         cJSON* pJsonChild);

enum PM_REG
{
    PM_REG_NAME,
    PM_CORESCOPE,
    PM_PARAM,
};

enum PLL_REG
{
    PLL_SEC_NAME,
    PLL_REG_NAME,
    PLL_PARAM,
};

typedef struct
{
    crashdump::cpu::Model cpuModel;
    int (*logPowerManagementVx)(crashdump::CPUInfo& cpuInfo, cJSON* pJsonChild);
} SPowerManagementVx;

int logPowerManagement(crashdump::CPUInfo& cpuInfo, cJSON* pJsonChild);
