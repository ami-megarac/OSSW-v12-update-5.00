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
#include <cstdint>
#endif

extern "C" {
#ifndef SPX_BMC_ACD
#include <cjson/cJSON.h>
#endif
}

/******************************************************************************
 *
 *   Common Defines
 *
 ******************************************************************************/
#define CD_REG_NAME_LEN 32

#define CD_JSON_STRING_LEN 256
#define CD_JSON_THREAD_NAME "thread%d"
#define CD_JSON_THREAD_1 "thread1"
#define CD_JSON_UA "UA:0x%x"
#define CD_JSON_DF "DF:0x%x"
#define CD_JSON_UA_DF "UA:0x%x,DF:0x%x"
#define SIZE_FAILURE 7

/******************************************************************************
 *
 *   CPX1 Defines
 *
 ******************************************************************************/
// Header
#define CD_DWORDS_HEADER 4

// Uncore
#define CD_REGS_UNCORE 5
#define CD_DWORDS_UNCORE 5

// Core
#define CD_REGS_CORE 33
#define CD_REGS_CORE_THREAD 76
#define CD_DWORDS_CORE 247
#define CD_NUM_GROUPS_CORE 19
#define CD_NUM_COREGROUP_CORE 2
enum CD_CORE_GROUP_SIZES
{
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
#define CD_HEADER_PARAM 0x1020

// Other
#define CD_CORES_IN_MASK 32
#define CD_THREADS_PER_CORE 2

#define CD_JSON_HEADER_ENTRY "header%d"
#define CD_JSON_DATA_SIZE_NAME "data_size"
#define CD_JSON_UNCORE_NAME "uncore"
#define CD_JSON_CORE_NAME "core%d"

#define CORE_GROUP0_HIGH 16
#define CORE_GROUP0_LOW 0
#define CORE_GROUP1_HIGH 28
#define CORE_GROUP1_LOW 16

/******************************************************************************
 *
 *   CPX1 Structures
 *
 ******************************************************************************/
typedef enum
{
    CORE_SCOPE,
    THREAD_SCOPE
} ECrashdumpRegScope;

typedef struct
{
    char name[CD_REG_NAME_LEN];
    uint8_t dwords;
    ECrashdumpRegScope scope;
} SCrashdumpRegCPX1;

typedef struct
{
    uint32_t size;
    uint32_t data[CD_DWORDS_HEADER];
} SCrashdumpHeader;

typedef struct
{
    SCrashdumpHeader header;
    uint32_t* data;
} SCrashdump;

/******************************************************************************
 *
 *   ICX1 Defines
 *
 ******************************************************************************/
#define CD_MT_THREADS_PER_CORE 2
#define CD_ST_THREADS_PER_CORE 1

#define CD_LBRS_PER_CORE 32
#define CD_ENTRIES_PER_LBR 3
#define CD_LBR_SIZE 8

#define CD_JSON_NUM_SQ_ENTRIES 64
#define CD_JSON_SQ_ENTRY_SIZE 8

#define CD_JSON_CORE_NAME "core%d"

#define CD_JSON_LBR_NAME_FROM "LBR%d_FROM"
#define CD_JSON_LBR_NAME_TO "LBR%d_TO"
#define CD_JSON_LBR_NAME_INFO "LBR%d_INFO"

#define CD_JSON_SQ_ENTRY_NAME "entry%d"
#define CRASHDUMP_KNOWN_VESION 0x401e002
#define CRASHDUMP_MAX_SIZE 0x7f8

#define IERR_BIT 30
#define MSMI_IERR_BIT 22
#define IERR_INTERNAL_BIT 27
#define MSMI_IERR_INTERNAL 19
#define MCERR_INTERNAL_BIT 26
#define MSMI_MCERR_INTERNAL 18

/******************************************************************************
 *
 *   ICX1 Structures
 *
 ******************************************************************************/
typedef struct
{
    char name[CD_REG_NAME_LEN];
    uint8_t size;
} SCrashdumpRegICX1;

typedef union
{
    uint64_t raw;
    struct
    {
        uint32_t version;
        uint32_t regDumpSize : 16;
        uint32_t sqDumpSize : 16;
    } field;
} UCrashdumpVerSize;

typedef union
{
    uint64_t raw;
    struct
    {
        uint32_t whoami;
        uint32_t multithread : 1;
        uint32_t sgxMask : 1;
        uint32_t reserved : 30;
    } field;
} UCrashdumpWhoMisc;
#define CD_WHO_MISC_OFFSET 3

typedef struct
{
    crashdump::cpu::Model cpuModel;
    int (*logCrashdumpVx)(crashdump::CPUInfo& cpuInfo, cJSON* pJsonChild);
} SCrashdumpVx;

int logCrashdump(crashdump::CPUInfo& cpuInfo, cJSON* pJsonChild);

#define ICX_A0_FRAME_BYTE_OFFSET 32
#define ICX_A0_CRASHDUMP_DISABLED 1
#define ICX_A0_CRASHDUMP_ENABLED 0
#define SIZE_OF_0x0 3