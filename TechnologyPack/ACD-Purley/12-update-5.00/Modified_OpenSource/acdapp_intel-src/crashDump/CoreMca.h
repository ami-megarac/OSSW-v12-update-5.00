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

#ifndef _CORE_MCA_H_
#define _CORE_MCA_H_

// PECI sequence
#define CM_SEQ_DATA 0x10000
#define CM_BANK_PARAM 0x1000

#define FIRST_CORE_MCA 0
#define LAST_CORE_MCA 3

#define CM_NUM_MCA_DWORDS 10

#define CORE_MCA_NAME_LEN 8
#define CORE_MCA_REG_NAME "IA32_MC%d_%s"

#define CORE_MCA_JSON_STRING_LEN 32
#define CORE_MCA_JSON_SOCKET_NAME "socket%d"
#define CORE_MCA_JSON_CORE_NAME "core%d"
#define CORE_MCA_JSON_MCA_NAME "MC%d"

#define CORE_MCA_FAILED "UA:"

/******************************************************************************
*
*   Structures
*
******************************************************************************/
typedef union {
    struct {
        UINT64 u64CoreMcaCtl;
        UINT64 u64CoreMcaStatus;
        UINT64 u64CoreMcaAddr;
        UINT64 u64CoreMcaMisc;
        UINT64 u64CoreMcaCtl2;
    } sReg;
    UINT32 u32Raw[CM_NUM_MCA_DWORDS];
} UCoreMcaRegs;

typedef struct {
    UCoreMcaRegs uRegData;
    BOOL bInvalid;
} SCoreMcaRawData;

static const char coreMcaRegNames[][CORE_MCA_NAME_LEN] =
{
    "CTL",
    "CTL2",
    "STATUS",
    "ADDR",
    "MISC"
};

typedef enum
{
    CORE_CTL,
    CORE_CTL2,
    CORE_STATUS,
    CORE_ADDR,
    CORE_MISC
} ECoreRegNames;

ESTATUS logCoreMca(FILE * fpRaw, FILE * fp, cJSON * pJsonChild);
#endif //_CORE_MCA_H_
