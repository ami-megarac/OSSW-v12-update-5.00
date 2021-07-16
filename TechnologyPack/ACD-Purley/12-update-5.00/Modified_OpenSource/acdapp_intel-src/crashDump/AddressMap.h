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

#ifndef _ADDRESS_MAP_H_
#define _ADDRESS_MAP_H_

// Local PCI Access
enum AM_PCI_BUS {
    AM_LOCAL_BUS_0 = 0,
    AM_LOCAL_BUS_1,
    AM_LOCAL_BUS_2,
    AM_LOCAL_BUS_3,
    AM_LOCAL_BUS_4,
    AM_LOCAL_BUS_5
};

enum AM_PCI_DEV {
    AM_DEV_CHA_29 = 0x1d,
    AM_DEV_MMAP_5 = 0x05
};

enum AM_PCI_FUNC {
    AM_FUNC_MMAP_0 = 0,
    AM_FUNC_SAD_ALL_0 = 0,
    AM_FUNC_UTIL_ALL_1 = 1
};

enum AM_PCI_REG {
    MESEG_BASE = 0x50,
    MESEG_LIMIT = 0x58,
    MMCFG_Rule = 0xc0,
    MMCFG_BASE = 0x90,
    MMCFG_LIMIT = 0x98,
    TSEG = 0xa8,
    TOLM = 0xd0,
    TOHM = 0xd8,
    MMIO_RULE_CFG_BASE = 0x40,
};

enum AM_REG_DWORDS {
    AM_REG_DWORD = 1,
    AM_REG_QWORD = 2
};

#define AM_REG_NAME_LEN 32
#define AM_NUM_SYS 11
#define AM_NUM_MMIO 16

#define AM_JSON_STRING_LEN 32
#define AM_MMIO_REG_NAME_BASE "MMIO_RULE_CFG_"
#define AM_JSON_SYS_MEM_NAME "sys_mem"
#define AM_JSON_MMIO_MEM_NAME "mmio_mem"
#define AM_JSON_OVERALL_ENTRY_NAME "Overall"
#define AM_JSON_SOCKET_NAME "socket%d"
#define AM_JSON_ENTRY_NAME "entry%d"
#define AM_JSON_REGION_NAME "region"

/******************************************************************************
*
*   Structures
*
******************************************************************************/
typedef struct {
    char regionName[AM_REG_NAME_LEN];
    UINT8 u8NumRegs;
} SSysMemRegion;

typedef union {
    UINT64 u64;
    UINT32 u32[2];
} UAddrMapRegValue;

typedef struct {
    char regName[AM_REG_NAME_LEN];
    UINT8 u8Bus;
    UINT8 u8Dev;
    UINT8 u8Func;
    UINT16 u16Reg;
    UINT8 u8Dwords;
    UAddrMapRegValue uValue;
} SAddrMapReg;

typedef struct {
    SAddrMapReg sysAddr[AM_NUM_SYS];
    SAddrMapReg mmioAddr[MAX_CPU][AM_NUM_MMIO];
} SAddrMapRawData;

ESTATUS logAddressMap(FILE * fpRaw, FILE * fp, cJSON * pJsonChild);
#endif //_ADDRESS_MAP_H_
