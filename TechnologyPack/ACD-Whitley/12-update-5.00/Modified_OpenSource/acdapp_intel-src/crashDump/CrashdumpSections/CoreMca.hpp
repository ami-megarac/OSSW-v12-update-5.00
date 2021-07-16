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

extern "C" {
#ifndef SPX_BMC_ACD
#include <cjson/cJSON.h>
#endif
#include <stdint.h>
}

/******************************************************************************
 *
 *   Common Defines
 *
 ******************************************************************************/
#define CORE_MCA_NAME_LEN 16

#define CORE_MCA_JSON_STRING_LEN 32
#define CORE_MCA_JSON_CORE_NAME "core%d"
#define CORE_MCA_JSON_THREAD_NAME "thread%d"

#define CORE_MCA_NA "N/A"
#define CORE_MCA_UA "UA:0x%x"
#define CORE_MCA_DF "DF:0x%x"
#define CORE_MCA_UA_DF "UA:0x%x,DF:0x%x"
#define CORE_MCA_UINT64_FMT "0x%" PRIx64 ""
#define CORE_MCA_VALID true

/******************************************************************************
 *
 *   CPX1 Defines
 *
 ******************************************************************************/
// PECI sequence
#define CM_BANK_PARAM 0x1000

#define FIRST_CORE_MCA 0
#define LAST_CORE_MCA 3

#define CM_NUM_MCA_DWORDS 10

#define CORE_MCA_REG_NAME "mc%d_%s"

#define CORE_MCA_JSON_MCA_NAME "MC%d"

#define CORE_MCA_UA_CPX "0x%llx,UA:0x%x"

#define CORE_MCA_UA_DF_CPX "0x0,UA:0x%x,DF:0x%x"

/******************************************************************************
 *
 *   CPX1 Structures
 *
 ******************************************************************************/
typedef union
{
    struct
    {
        uint64_t u64CoreMcaCtl;
        uint64_t u64CoreMcaStatus;
        uint64_t u64CoreMcaAddr;
        uint64_t u64CoreMcaMisc;
        uint64_t u64CoreMcaCtl2;
    } sReg;
    uint32_t u32Raw[CM_NUM_MCA_DWORDS];
} UCoreMcaRegs;

typedef struct
{
    UCoreMcaRegs uRegData;
    bool bInvalid;
    int ret;
    uint8_t cc;
} SCoreMcaRawData;

static const char coreMcaRegNames[][CORE_MCA_NAME_LEN] = {
    "ctl", "ctl2", "status", "addr", "misc"};

typedef enum
{
    CORE_CTL,
    CORE_CTL2,
    CORE_STATUS,
    CORE_ADDR,
    CORE_MISC
} ECoreRegNames;

/******************************************************************************
 *
 *   ICX1 Defines
 *
 ******************************************************************************/
#define CORE_MCA_THREADS_PER_CORE 2

/******************************************************************************
 *
 *   ICX1 Structures
 *
 ******************************************************************************/
typedef struct
{
    char bankName[CORE_MCA_NAME_LEN];
    char regName[CORE_MCA_NAME_LEN];
    uint16_t addr;
} SCoreMcaReg;

typedef struct
{
    crashdump::cpu::Model cpuModel;
    int (*logCoreMcaVx)(crashdump::CPUInfo& cpuInfo, cJSON* pJsonChild);
} SCoreMcaLogVx;

int logCoreMca(crashdump::CPUInfo& cpuInfo, cJSON* pJsonChild);
