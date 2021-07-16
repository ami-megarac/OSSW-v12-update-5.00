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
#include <string.h>

#define AM_REG_NAME_LEN 34

#define AM_JSON_STRING_LEN 32

#define AM_NA "N/A"
#define AM_UA "UA:0x%x"
#define AM_UA_CPX "0x%" PRIx64 ",UA:0x%x"
#define AM_DF "DF:0x%x"
#define AM_UA_DF "UA:0x%x,DF:0x%x"
#define AM_UA_DF_CPX "0x0,UA:0x%x,DF:0x%x"
#define AM_UINT64_FMT "0x%" PRIx64 ""
#define SIZE_FAILURE 7
#define AM_PCI_SEG 0

/******************************************************************************
 *
 *   Structures
 *
 ******************************************************************************/
enum AM_REG_SIZE
{
    AM_REG_BYTE = 1,
    AM_REG_WORD = 2,
    AM_REG_DWORD = 4,
    AM_REG_QWORD = 8
};

typedef union
{
    uint64_t u64;
    uint32_t u32[2];
} UAddrMapRegValue;

typedef struct
{
    UAddrMapRegValue uValue;
    uint8_t cc;
    bool bInvalid;
} SAddressMapRegRawData;

typedef struct
{
    char regName[AM_REG_NAME_LEN];
    uint8_t u8Bus;
    uint8_t u8Dev;
    uint8_t u8Func;
    uint16_t u16Reg;
    uint8_t u8Size;
} SAddrMapEntry;

typedef struct
{
    crashdump::cpu::Model cpuModel;
    int (*logAddrMapVx)(crashdump::CPUInfo& cpuInfo, cJSON* pJsonChild);
} SAddrMapVx;

int logAddressMap(crashdump::CPUInfo& cpuInfo, cJSON* pJsonChild);
