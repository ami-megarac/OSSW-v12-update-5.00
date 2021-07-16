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

#ifndef _CRASHDUMP_H_
#define _CRASHDUMP_H_

// Header
#define CD_DWORDS_HEADER 4

// Uncore
#define CD_REGS_UNCORE 5
#define CD_DWORDS_UNCORE 5

// Core
#define CD_DWORDS_CORE 247
#define CD_NUM_GROUPS_CORE 19
enum CD_CORE_GROUP_SIZES {
    CD_CORE_GROUP_1_SIZE = 1,   // IA32_X2APIC_CUR_COUNT
    CD_CORE_GROUP_2_SIZE = 1,   // EXT_XAPIC_SVR
    CD_CORE_GROUP_3_SIZE = 10,  // IA32_X2APIC_LVT_CMCI
    CD_CORE_GROUP_4_SIZE = 1,   // EXT_XAPIC_LDR
    CD_CORE_GROUP_5_SIZE = 28,  // IA32_X2APIC_ISR0
    CD_CORE_GROUP_6_SIZE = 1,   // ML3_CR_PIC_DESTINATION_FORMAT
    CD_CORE_GROUP_7_SIZE = 1,   // EXT_XAPIC_PPR
    CD_CORE_GROUP_8_SIZE = 1,   // IA32_APIC_BASE
    CD_CORE_GROUP_9_SIZE = 1,   // CR4
    CD_CORE_GROUP_10_SIZE = 1,  // CR3
    CD_CORE_GROUP_11_SIZE = 1,  // CR0
    CD_CORE_GROUP_12_SIZE = 7,  // IA32_FIXED_CTR0
    CD_CORE_GROUP_13_SIZE = 19, // MSR_LASTBRANCH_TOS
    CD_CORE_GROUP_14_SIZE = 1,  // BAC_CR_CS_BASE
    CD_CORE_GROUP_15_SIZE = 1,  // CORE_CR_EPTP
    CD_CORE_GROUP_16_SIZE = 1,  // IA32_EFER
    CD_CORE_GROUP_17_SIZE = 1,  // MSR_MCG_CONTAIN
    CD_CORE_GROUP_18_SIZE = 29, // IA32_MTRR_PHYSBASE0
    CD_CORE_GROUP_19_SIZE = 3,  // IA32_MTRR_DEF_TYPE
};

// GP
#define CD_REGS_GP_THREAD 36
#define CD_DWORDS_GP 72

// PECI sequence
#define CD_SEQ_DATA 0x10038
#define CD_HEADER_PARAM 0x1020

// Other
#define CD_CORES_IN_MASK 32
#define CD_THREADS_PER_CORE 2

#define CD_REG_NAME_LEN 32

#define CD_JSON_STRING_LEN 64
#define CD_JSON_SOCKET_NAME "socket%d"
#define CD_JSON_HEADER_NAME "crashdump_info"
#define CD_JSON_HEADER_ENTRY "header%d"
#define CD_JSON_DATA_SIZE_NAME "crashdump_data_size"
#define CD_JSON_UNCORE_NAME "uncore_regs"
#define CD_JSON_UNCORE_HEADER_NAME "uncore_regs"
#define CD_JSON_CORE_NAME "core%d_regs"
#define CD_JSON_GP_NAME "core%d_GP_regs"
#define CD_JSON_THREAD_NAME "thread%d"

/******************************************************************************
*
*   Structures
*
******************************************************************************/
typedef enum {
    CORE_SCOPE,
    THREAD_SCOPE
} ECrashdumpRegScope;

typedef struct {
    char name[CD_REG_NAME_LEN];
    UINT8 dwords;
    ECrashdumpRegScope scope;
} SCrashdumpReg;

typedef struct {
    UINT32 size;
    UINT32 data[CD_DWORDS_HEADER];
    UINT32 u32NumReads;
} SCrashdumpHeader;

typedef struct {
    SCrashdumpHeader header;
    UINT32 * data;
} SCrashdump;

ESTATUS logCrashdump(FILE * fpRaw, FILE * fp, cJSON * pJsonChild);
#endif //_CRASHDUMP_H_
