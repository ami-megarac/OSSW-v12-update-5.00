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

#ifndef _UNCORE_MCA_H_
#define _UNCORE_MCA_H_

// PECI sequence
#define UCM_SEQ_DATA 0x10000
#define UCM_BANK_PARAM 0x1000

#define FIRST_UNCORE_MCA 4
#define LAST_UNCORE_MCA 19

#define UCM_NUM_MCA_DWORDS 10

#define UNCORE_MCA_NAME_LEN 8
#define UNCORE_MCA_REG_NAME "IA32_MC%d_%s"

#define UNCORE_MCA_JSON_STRING_LEN 32
#define UNCORE_MCA_JSON_SOCKET_NAME "socket%d"
#define UNCORE_MCA_JSON_MCA_NAME "MC%d"

#define UNCORE_MCA_FAILED "UA:"

/******************************************************************************
*
*   Structures
*
******************************************************************************/
typedef union {
    struct {
        UINT64 u64UncoreMcaCtl;
        UINT64 u64UncoreMcaStatus;
        UINT64 u64UncoreMcaAddr;
        UINT64 u64UncoreMcaMisc;
        UINT64 u64UncoreMcaCtl2;
    } sReg;
    UINT32 u32Raw[UCM_NUM_MCA_DWORDS];
} UUncoreMcaRegs;

typedef struct {
    UUncoreMcaRegs uRegData;
    BOOL bInvalid;
} SUncoreMcaRawData;

static const char uncoreMcaRegNames[][UNCORE_MCA_NAME_LEN] =
{
    "CTL",
    "CTL2",
    "STATUS",
    "ADDR",
    "MISC"
};

typedef enum
{
    UNCORE_CTL,
    UNCORE_CTL2,
    UNCORE_STATUS,
    UNCORE_ADDR,
    UNCORE_MISC
} EUncoreRegNames;

ESTATUS logUncoreMca(FILE * fpRaw, FILE * fp, cJSON * pJsonChild);
#endif //_UNCORE_MCA_H_
