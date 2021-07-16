/*
// Copyright (C) 2018 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions
// and limitations under the License.
//
//
// SPDX-License-Identifier: Apache-2.0
*/

#ifndef _UNCORE_STATUS_H_
#define _UNCORE_STATUS_H_

#define US_JSON_STRING_LEN 64
#define US_JSON_SOCKET_NAME "socket%d"

enum US_REG_SIZE {
    US_REG_BYTE = 1,
    US_REG_WORD = 2,
    US_REG_DWORD = 4,
    US_REG_QWORD = 8
};

enum US_MMIO_SIZE {
    US_MMIO_BYTE = 0,
    US_MMIO_WORD = 1,
    US_MMIO_DWORD = 2,
    US_MMIO_QWORD = 3
};

// PECI sequence
#define US_MCA_PARAM 0x1000
#define US_MMIO_PARAM 0x0012
#define US_UCRASH_START 0x3003
#define US_UCRASH_PARAM 0

#define US_REG_NAME_LEN 64
#define US_NUM_MCA_DWORDS 10
#define US_NUM_MCA_QWORDS (US_NUM_MCA_DWORDS / 2)

#define US_BASE_CBO_BANK 9
#define US_NUM_CBO_BANKS 3

#define US_BASE_IIO_BANK 6
#define US_NUM_IIO 5

#define US_MCA_UNMERGE (1 << 22)

#define US_MCA_NAME_LEN 8
#define US_CBO_MCA_REG_NAME "cbo%d_mc_%s"
#define US_UNCORE_CRASH_DW_NAME "uncore_crashdump_dw%d"

#define US_FAILED "UA:"

/******************************************************************************
*
*   Structures
*
******************************************************************************/
typedef union {
    UINT64 u64;
    UINT32 u32[2];
} UUncoreStatusRegValue;

typedef struct {
    UUncoreStatusRegValue uValue;
    BOOL bInvalid;
} SUncoreStatusRegRawData;

typedef struct {
    char regName[US_REG_NAME_LEN];
    UINT8 u8Bus;
    UINT8 u8Dev;
    UINT8 u8Func;
    UINT16 u16Reg;
    UINT8 u8Size;
} SUncoreStatusRegPci;

typedef struct {
    char regName[US_REG_NAME_LEN];
    union {
        struct {
            UINT32 lenCode : 2;
            UINT32 bar : 2;
            UINT32 bus : 3;
            UINT32 rsvd : 1;
            UINT32 func : 3;
            UINT32 dev : 5;
            UINT32 reg : 16;
        } fields;
        UINT32 raw;
    } uMmioReg;
} SUncoreStatusRegPciMmio;

typedef union {
    UINT64 u64Raw[US_NUM_MCA_QWORDS];
    UINT32 u32Raw[US_NUM_MCA_DWORDS];
} UUncoreStatusMcaRegs;

typedef struct {
    UUncoreStatusMcaRegs uRegData;
    BOOL bInvalid;
} SUncoreStatusMcaRawData;

typedef struct {
    char regName[US_REG_NAME_LEN];
    UINT8 u8IioNum;
} SUncoreStatusRegIio;

static const char uncoreStatusMcaRegNames[][US_MCA_NAME_LEN] =
{
    "ctl",
    "status",
    "addr",
    "misc",
    "ctl2"
};

typedef ESTATUS (*UncoreStatusRead)(FILE * fpRaw, cJSON * pJsonChild);

ESTATUS logUncoreStatus(FILE * fpRaw, FILE * fp, cJSON * pJsonChild);
#endif //_UNCORE_STATUS_H_
