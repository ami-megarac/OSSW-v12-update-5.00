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

#include "common.h"
#ifndef SPX_BMC_ACD
#include <cJSON.h>
#endif
#include "Crashdump.h"

#define TMPDUMPLOG	"/tmp/crashdump.log"
#define TMPDUMPLOG1	"/tmp/crashdump1.log"
#define TMPLOGDATALEN	512

extern void GetClockTime(char* buffer, int iSize);

#if defined(BUILD_JSON) || defined(BUILD_TXT)
static SCrashdumpReg sCrashdumpUncoreRegs[CD_REGS_UNCORE] =
{
    { "IerrLoggingReg",         1, CORE_SCOPE },
    { "MCerrLoggingReg",        1, CORE_SCOPE },
    { "THERM_INTERRUPT",        1, CORE_SCOPE },
    { "THERM_STATUS",           1, CORE_SCOPE },
    { "IA32_CLOCK_MODULATION",  1, CORE_SCOPE }
};

static SCrashdumpReg * sCrashdumpCoreRegs[CD_NUM_GROUPS_CORE] =
{
    (SCrashdumpReg[])
    {   // Group 1
        { "IA32_X2APIC_CUR_COUNT",          1, THREAD_SCOPE }
    },
    (SCrashdumpReg[])
    {   // Group 2
        { "EXT_XAPIC_SVR",                  1, THREAD_SCOPE }
    },
    (SCrashdumpReg[])
    {   // Group 3
        { "IA32_X2APIC_LVT_CMCI",           1, THREAD_SCOPE },
        { "IA32_X2APIC_LVT_TIMER",          1, THREAD_SCOPE },
        { "IA32_X2APIC_LVT_THERMAL",        1, THREAD_SCOPE },
        { "IA32_X2APIC_LVT_PMI",            1, THREAD_SCOPE },
        { "IA32_X2APIC_LVT_LINT0",          1, THREAD_SCOPE },
        { "IA32_X2APIC_LVT_LINT1",          1, THREAD_SCOPE },
        { "IA32_X2APIC_LVT_ERROR",          1, THREAD_SCOPE },
        { "IA32_X2APIC_ICR",                2, THREAD_SCOPE },
        { "IA32_X2APIC_INIT_COUNT",         1, THREAD_SCOPE },
        { "IA32_X2APIC_DIV_CONF",           1, THREAD_SCOPE }
    },
    (SCrashdumpReg[])
    {   // Group 4
        { "EXT_XAPIC_LDR",                  1, THREAD_SCOPE }
    },
    (SCrashdumpReg[])
    {   // Group 5
        { "IA32_X2APIC_ISR0",               1, THREAD_SCOPE },
        { "IA32_X2APIC_ISR1",               1, THREAD_SCOPE },
        { "IA32_X2APIC_ISR2",               1, THREAD_SCOPE },
        { "IA32_X2APIC_ISR3",               1, THREAD_SCOPE },
        { "IA32_X2APIC_ISR4",               1, THREAD_SCOPE },
        { "IA32_X2APIC_ISR5",               1, THREAD_SCOPE },
        { "IA32_X2APIC_ISR6",               1, THREAD_SCOPE },
        { "IA32_X2APIC_ISR7",               1, THREAD_SCOPE },
        { "IA32_X2APIC_TMR0",               1, THREAD_SCOPE },
        { "IA32_X2APIC_TMR1",               1, THREAD_SCOPE },
        { "IA32_X2APIC_TMR2",               1, THREAD_SCOPE },
        { "IA32_X2APIC_TMR3",               1, THREAD_SCOPE },
        { "IA32_X2APIC_TMR4",               1, THREAD_SCOPE },
        { "IA32_X2APIC_TMR5",               1, THREAD_SCOPE },
        { "IA32_X2APIC_TMR6",               1, THREAD_SCOPE },
        { "IA32_X2APIC_TMR7",               1, THREAD_SCOPE },
        { "IA32_X2APIC_IRR0",               1, THREAD_SCOPE },
        { "IA32_X2APIC_IRR1",               1, THREAD_SCOPE },
        { "IA32_X2APIC_IRR2",               1, THREAD_SCOPE },
        { "IA32_X2APIC_IRR3",               1, THREAD_SCOPE },
        { "IA32_X2APIC_IRR4",               1, THREAD_SCOPE },
        { "IA32_X2APIC_IRR5",               1, THREAD_SCOPE },
        { "IA32_X2APIC_IRR6",               1, THREAD_SCOPE },
        { "IA32_X2APIC_IRR7",               1, THREAD_SCOPE },
        { "IA32_X2APIC_ESR",                1, THREAD_SCOPE },
        { "PIC_LEGACY_LOCAL_APIC_ID",       1, THREAD_SCOPE },
        { "PIC_LEGACY_LOGICAL_DESTINATION", 1, THREAD_SCOPE },
        { "EXT_XAPIC_LOCAL_APIC_ID",        1, THREAD_SCOPE }
    },
    (SCrashdumpReg[])
    {   // Group 6
        { "ML3_CR_PIC_DESTINATION_FORMAT",  1, THREAD_SCOPE }
    },
    (SCrashdumpReg[])
    {   // Group 7
        { "EXT_XAPIC_PPR",                  1, THREAD_SCOPE }
    },
    (SCrashdumpReg[])
    {   // Group 8
        { "IA32_APIC_BASE",                 2, THREAD_SCOPE }
    },
    (SCrashdumpReg[])
    {   // Group 9
        { "CR4",                            1, THREAD_SCOPE }
    },
    (SCrashdumpReg[])
    {   // Group 10
        { "CR3",                            2, THREAD_SCOPE }
    },
    (SCrashdumpReg[])
    {   // Group 11
        { "CR0",                            1, THREAD_SCOPE }
    },
    (SCrashdumpReg[])
    {   // Group 12
        { "IA32_FIXED_CTR0",                2, THREAD_SCOPE },
        { "IA32_FIXED_CTR1",                2, THREAD_SCOPE },
        { "IA32_FIXED_CTR2",                2, THREAD_SCOPE },
        { "IA32_PMC0",                      2, THREAD_SCOPE },
        { "IA32_PMC1",                      2, THREAD_SCOPE },
        { "IA32_PMC2",                      2, THREAD_SCOPE },
        { "IA32_PMC3",                      2, THREAD_SCOPE }
    },
    (SCrashdumpReg[])
    {   // Group 13
        { "MSR_LASTBRANCH_TOS",             1, THREAD_SCOPE },
        { "RAT_CR_LAST_INT_FROM_IP",        2, THREAD_SCOPE },
        { "RAT_CR_LAST_INT_TO_IP",          2, THREAD_SCOPE },
        { "THD_SPECIFIC_PATCHING_2",        2, THREAD_SCOPE },
        { "POST_COUNTER",                   1, THREAD_SCOPE },
        { "FLOW_DETAILS",                   1, THREAD_SCOPE },
        { "THD_SPECIFIC_PATCHING_0",        1, THREAD_SCOPE },
        { "THD_SPECIFIC_PATCHING_1",        1, THREAD_SCOPE },
        { "IA32_MCG_STATUS",                1, THREAD_SCOPE },
        { "PSMI_CTRL",                      1, THREAD_SCOPE },
        { "TARGET_SLEEP_STATE",             1, THREAD_SCOPE },
        { "PATCH_REV_ID",                   1, THREAD_SCOPE },
        { "MISC_FLAGS",                     1, THREAD_SCOPE },
        { "PSMI_BASE",                      1, THREAD_SCOPE },
        { "DR6",                            1, THREAD_SCOPE },
        { "THD_SP_SPARE_74D",               1, THREAD_SCOPE },
        { "THD_SP_SPARE_74E",               1, THREAD_SCOPE },
        { "INTER_THREAD_MISC",              1, THREAD_SCOPE },
        { "IA32_MISC_ENABLES",              2, THREAD_SCOPE }
    },
    (SCrashdumpReg[])
    {   // Group 14
        { "BAC_CR_CS_BASE",                 1, THREAD_SCOPE }
    },
    (SCrashdumpReg[])
    {   // Group 15
        { "CORE_CR_EPTP",                   2, THREAD_SCOPE }
    },
    (SCrashdumpReg[])
    {   // Group 16
        { "IA32_EFER",                      1, THREAD_SCOPE }
    },
    (SCrashdumpReg[])
    {   // Group 17
        { "MSR_MCG_CONTAIN",                1, CORE_SCOPE }
    },
    (SCrashdumpReg[])
    {   // Group 18
        { "IA32_MTRR_PHYSBASE0",            2, CORE_SCOPE },
        { "IA32_MTRR_PHYSMASK0",            2, CORE_SCOPE },
        { "IA32_MTRR_PHYSBASE1",            2, CORE_SCOPE },
        { "IA32_MTRR_PHYSMASK1",            2, CORE_SCOPE },
        { "IA32_MTRR_PHYSBASE2",            2, CORE_SCOPE },
        { "IA32_MTRR_PHYSMASK2",            2, CORE_SCOPE },
        { "IA32_MTRR_PHYSBASE3",            2, CORE_SCOPE },
        { "IA32_MTRR_PHYSMASK3",            2, CORE_SCOPE },
        { "IA32_MTRR_PHYSBASE4",            2, CORE_SCOPE },
        { "IA32_MTRR_PHYSMASK4",            2, CORE_SCOPE },
        { "IA32_MTRR_PHYSBASE5",            2, CORE_SCOPE },
        { "IA32_MTRR_PHYSMASK5",            2, CORE_SCOPE },
        { "IA32_MTRR_PHYSBASE6",            2, CORE_SCOPE },
        { "IA32_MTRR_PHYSMASK6",            2, CORE_SCOPE },
        { "IA32_MTRR_PHYSBASE7",            2, CORE_SCOPE },
        { "IA32_MTRR_PHYSMASK7",            2, CORE_SCOPE },
        { "IA32_MTRR_FIX64K_00000",         2, CORE_SCOPE },
        { "IA32_MTRR_FIX16K_80000",         2, CORE_SCOPE },
        { "IA32_MTRR_FIX16K_A0000",         2, CORE_SCOPE },
        { "IA32_MTRR_PHYSBASE8",            2, CORE_SCOPE },
        { "IA32_MTRR_PHYSMASK8",            2, CORE_SCOPE },
        { "IA32_MTRR_FIX4K_C0000",          2, CORE_SCOPE },
        { "IA32_MTRR_FIX4K_C8000",          2, CORE_SCOPE },
        { "IA32_MTRR_FIX4K_D0000",          2, CORE_SCOPE },
        { "IA32_MTRR_FIX4K_D8000",          2, CORE_SCOPE },
        { "IA32_MTRR_FIX4K_E0000",          2, CORE_SCOPE },
        { "IA32_MTRR_FIX4K_E8000",          2, CORE_SCOPE },
        { "IA32_MTRR_FIX4K_F0000",          2, CORE_SCOPE },
        { "IA32_MTRR_FIX4K_F8000",          2, CORE_SCOPE }
    },
    (SCrashdumpReg[])
    {   // Group 19
        { "IA32_MTRR_DEF_TYPE",             2, CORE_SCOPE },
        { "IA32_MTRR_PHYSBASE9",            2, CORE_SCOPE },
        { "IA32_MTRR_PHYSMASK9",            2, CORE_SCOPE }
    }
};

static SCrashdumpReg sCrashdumpGpThreadRegs[CD_REGS_GP_THREAD] =
{
    { "TSC_ADDR_H", 1, THREAD_SCOPE },
    { "TSC_ADDR_L", 1, THREAD_SCOPE },
    { "R15_H",      1, THREAD_SCOPE },
    { "R15_L",      1, THREAD_SCOPE },
    { "R14_H",      1, THREAD_SCOPE },
    { "R14_L",      1, THREAD_SCOPE },
    { "R13_H",      1, THREAD_SCOPE },
    { "R13_L",      1, THREAD_SCOPE },
    { "R12_H",      1, THREAD_SCOPE },
    { "R12_L",      1, THREAD_SCOPE },
    { "R11_H",      1, THREAD_SCOPE },
    { "R11_L",      1, THREAD_SCOPE },
    { "R10_H",      1, THREAD_SCOPE },
    { "R10_L",      1, THREAD_SCOPE },
    { "R9_H",       1, THREAD_SCOPE },
    { "R9_L",       1, THREAD_SCOPE },
    { "R8_H",       1, THREAD_SCOPE },
    { "R8_L",       1, THREAD_SCOPE },
    { "ESP_H",      1, THREAD_SCOPE },
    { "ESP_L",      1, THREAD_SCOPE },
    { "EBP_H",      1, THREAD_SCOPE },
    { "EBP_L",      1, THREAD_SCOPE },
    { "EDI_H",      1, THREAD_SCOPE },
    { "EDI_L",      1, THREAD_SCOPE },
    { "ESI_H",      1, THREAD_SCOPE },
    { "ESI_L",      1, THREAD_SCOPE },
    { "EDX_H",      1, THREAD_SCOPE },
    { "EDX_L",      1, THREAD_SCOPE },
    { "ECX_H",      1, THREAD_SCOPE },
    { "ECX_L",      1, THREAD_SCOPE },
    { "EBX_H",      1, THREAD_SCOPE },
    { "EBX_L",      1, THREAD_SCOPE },
    { "EAX_H",      1, THREAD_SCOPE },
    { "EAX_L",      1, THREAD_SCOPE },
    { "LIP_H",      1, THREAD_SCOPE },
    { "LIP_L",      1, THREAD_SCOPE }
};

static UINT8 u8CrashdumpCoreGroupSizes[] = {
    CD_CORE_GROUP_1_SIZE,
    CD_CORE_GROUP_2_SIZE,
    CD_CORE_GROUP_3_SIZE,
    CD_CORE_GROUP_4_SIZE,
    CD_CORE_GROUP_5_SIZE,
    CD_CORE_GROUP_6_SIZE,
    CD_CORE_GROUP_7_SIZE,
    CD_CORE_GROUP_8_SIZE,
    CD_CORE_GROUP_9_SIZE,
    CD_CORE_GROUP_10_SIZE,
    CD_CORE_GROUP_11_SIZE,
    CD_CORE_GROUP_12_SIZE,
    CD_CORE_GROUP_13_SIZE,
    CD_CORE_GROUP_14_SIZE,
    CD_CORE_GROUP_15_SIZE,
    CD_CORE_GROUP_16_SIZE,
    CD_CORE_GROUP_17_SIZE,
    CD_CORE_GROUP_18_SIZE,
    CD_CORE_GROUP_19_SIZE
};
#endif //BUILD_JSON) || BUILD_TXT

/******************************************************************************
*
*   crashdumpTxt
*
*   This function formats the Crashdump into the provided text file
*
******************************************************************************/
#ifdef BUILD_TXT
static void crashdumpTxt(UINT8 u8CPU, UINT32 u32NumReads, SCrashdump * sCrashdump, FILE * fp)
{
    UINT8 u8DwordNum;
    UINT32 u32CoreMask, u32CoreCount;
    UINT32 u32ExpectedNumReads;
    UINT32 u32CrashIndex = 0;
    UINT64 u64RegValue = 0;
    UINT32 i, u32RegNum;
    UINT8 u8GroupNum, u8CoreNum, u8ThreadNum;
    char buf[64];

    // get the system clock
    GetClockTime(buf, 63);
    fprintf(fp, "CPU #%d Crashdump Log: %s\n", u8CPU, buf);

    // Get the number of cores included in the crashdump
    u32CoreMask = ((sCrashdump->header.data[3] & 0xFFFF) << 16) | (sCrashdump->header.data[2] & 0xFFFF);
    u32CoreCount = __builtin_popcount(u32CoreMask);
    PRINT(PRINT_DBG, PRINT_INFO, "[DBG] - CPU #%d has %lx cores with errors \n", u8CPU, u32CoreCount);

    // Use the core count to calculate the expected number of reads and compare to the returned number of reads
    u32ExpectedNumReads = CD_DWORDS_UNCORE + (CD_DWORDS_CORE * u32CoreCount) + (CD_DWORDS_GP * u32CoreCount);

    // Write the crashdump header to the debug log
    fprintf(fp, "HEADER_SIZE: %08X\n", sCrashdump->header.size);
    for (i = 0; i < CD_DWORDS_HEADER; i++)
        fprintf(fp, "CRASHDUMP_HEADER%x: %08X\n", i, sCrashdump->header.data[i]);

    // Write the crashdump data to the debug log
    if (u32NumReads && u32ExpectedNumReads == u32NumReads) {
        PRINT(PRINT_DBG, PRINT_INFO, "[DBG] - CPU #%d Expected data size, logging full crashdump\n", u8CPU);
        // Write out the uncore register data
        PRINT(PRINT_DBG, PRINT_INFO, "[DBG] - CPU #%d logging uncore data\n", u8CPU);
        fprintf(fp, "\nUncore Registers:\n");
        for (u32RegNum = 0; u32RegNum < CD_REGS_UNCORE; u32RegNum++) {
            fprintf(fp, "%s: ", sCrashdumpUncoreRegs[u32RegNum].name);
            u64RegValue = 0;
            for (u8DwordNum = 0; u8DwordNum < sCrashdumpUncoreRegs[u32RegNum].dwords; u8DwordNum++) {
                u64RegValue |= ((sCrashdump->data[u32CrashIndex++]) << (32 * u8DwordNum));
            }
            fprintf(fp, "%016lX\n", u64RegValue);
        }
        // Write out the core register data
        PRINT(PRINT_DBG, PRINT_INFO, "[DBG] - CPU #%d logging core data\n", u8CPU);
        fprintf(fp, "\nCore Registers:\n");
        // The core data is returned in groups
        for (u8GroupNum = 0; u8GroupNum < CD_NUM_GROUPS_CORE; u8GroupNum++) {
            fprintf(fp, "\nGroup %d:", u8GroupNum);
            // Each group starts with Thread 0 followed by Thread 1 of the highest core
            for (u8CoreNum = CD_CORES_IN_MASK; u8CoreNum > 0; u8CoreNum--) {
                if (!(u32CoreMask & (1 << (u8CoreNum - 1))))
                    continue;
                fprintf(fp, "\nCore %d:\n", u8CoreNum - 1);
                for (u8ThreadNum = 0; u8ThreadNum < CD_THREADS_PER_CORE; u8ThreadNum++) {
                    for (u32RegNum = 0; u32RegNum < u8CrashdumpCoreGroupSizes[u8GroupNum]; u32RegNum++) {
                        // Thread scope registers are dumped for each thread, so print them with a thread number
                        if (sCrashdumpCoreRegs[u8GroupNum][u32RegNum].scope == THREAD_SCOPE) {
                                fprintf(fp, " thread%d.%s: ", u8ThreadNum, sCrashdumpCoreRegs[u8GroupNum][u32RegNum].name);
                                u64RegValue = 0;
                                for (u8DwordNum = 0; u8DwordNum < sCrashdumpCoreRegs[u8GroupNum][u32RegNum].dwords; u8DwordNum++) {
                                    u64RegValue |= ((sCrashdump->data[u32CrashIndex++]) << (32 * u8DwordNum));
                                }
                                fprintf(fp, "%016lX\n", u64RegValue);
                        // Core scope registers are only dumped for thread 0
                        } else if (sCrashdumpCoreRegs[u8GroupNum][u32RegNum].scope == CORE_SCOPE && u8ThreadNum == 0) {
                            fprintf(fp, " %s: ", sCrashdumpCoreRegs[u8GroupNum][u32RegNum].name);
                            u64RegValue = 0;
                            for (u8DwordNum = 0; u8DwordNum < sCrashdumpCoreRegs[u8GroupNum][u32RegNum].dwords; u8DwordNum++) {
                                u64RegValue |= ((sCrashdump->data[u32CrashIndex++]) << (32 * u8DwordNum));
                            }
                            fprintf(fp, "%016lX\n", u64RegValue);
                        }
                    }
                }
            }
        }

        // Write out the GP register data to the debug log
        PRINT(PRINT_DBG, PRINT_INFO, "[DBG] - CPU #%d logging GP data\n", u8CPU);
        fprintf(fp, "\nGP Registers:\n");
        for (u8CoreNum = CD_CORES_IN_MASK; u8CoreNum > 0; u8CoreNum--) {
            if (!(u32CoreMask & (1 << (u8CoreNum - 1))))
                continue;
            fprintf(fp, "\nCore %d:", u8CoreNum - 1);
            // Thread scope data is first
            for (u8ThreadNum = CD_THREADS_PER_CORE; u8ThreadNum > 0; u8ThreadNum--) {
                fprintf(fp, "\nThread %d:\n", u8ThreadNum - 1);
                for (u32RegNum = 0; u32RegNum < CD_REGS_GP_THREAD; u32RegNum++) {
                    fprintf(fp, " %s: ", sCrashdumpGpThreadRegs[u32RegNum].name);
                    u64RegValue = 0;
                    for (u8DwordNum = 0; u8DwordNum < sCrashdumpGpThreadRegs[u32RegNum].dwords; u8DwordNum++) {
                        u64RegValue |= ((sCrashdump->data[u32CrashIndex++]) << (32 * u8DwordNum));
                    }
                    fprintf(fp, "%016lX\n", u64RegValue);
                }
            }
        }
    } else if (u32NumReads) {
        PRINT(PRINT_DBG, PRINT_INFO, "Expected data size does not match, logging raw data\n");
        fprintf(fp, "Error in crashdump sequence (Expected: %d dwords; Read: %d dwords).\nRaw crashdump data read:\n", u32ExpectedNumReads, u32NumReads);
        for (i = 0; i < u32NumReads; i++)
            fprintf(fp, "%02d: %08X\n", i, sCrashdump->data[i]);
    } else {
        fprintf(fp, "\nNo crashdump data found\n");
    }
    fprintf(fp, "\n\n");
}
#endif //BUILD_TXT


/******************************************************************************
*
*   crashdumpRaw
*
*   This function writes the Crashdump into the provided raw file
*
******************************************************************************/
#ifdef BUILD_RAW
static void crashdumpRaw(UINT8 u8CPU, UINT32 u32NumReads, SCrashdump * sCrashdump, FILE * fpRaw)
{
#ifdef SPX_BMC_ACD
	UN_USED(u8CPU);
#endif
	fwrite(&sCrashdump->header, sizeof(SCrashdumpHeader), 1, fpRaw);
	if (u32NumReads)
	{
		fwrite(sCrashdump->data, (sizeof(UINT32) * u32NumReads), 1, fpRaw);
	}
}
#endif //BUILD_RAW


/******************************************************************************
*
*   crashdumpJson
*
*   This function formats the Crashdump into a JSON object
*
******************************************************************************/
#ifdef BUILD_JSON
static void crashdumpJson(UINT8 u8CPU, UINT32 u32NumReads, SCrashdump * sCrashdump, cJSON * pJsonChild)
{
    cJSON * socket;
    cJSON * thread_section;
    cJSON * crashdump_section;
    cJSON * gp_thread;
    char jsonItemName[CD_JSON_STRING_LEN];
    char jsonItemString[CD_JSON_STRING_LEN];
    int coreMaskNum;
    UINT8 u8CoreNum, u8GroupNum, u8ThreadNum, u8CoreStartNum, u8CoreEndNum;
    UINT32 u32CoreMask, u32CoreCount;
    UINT32 u32RegNum;
    UINT32 u32ExpectedNumReads;
    UINT32 i, u32CrashIndex = 0;

    // Get the number of cores included in the crashdump
    u32CoreMask = ((sCrashdump->header.data[3] & 0xFFFF) << 16) | (sCrashdump->header.data[2] & 0xFFFF);
    u32CoreCount = __builtin_popcount(u32CoreMask);
#ifndef SPX_BMC_ACD
    PRINT(PRINT_DBG, PRINT_INFO, "[DBG] - CPU #%d has %lx cores with errors \n", u8CPU, u32CoreCount);
#else
    PRINT(PRINT_DBG, PRINT_INFO, "[DBG] - CPU #%d has %d cores with errors \n", u8CPU, u32CoreCount);
#endif

    // Use the core count to calculate the expected number of reads and compare to the returned number of reads
    u32ExpectedNumReads = CD_DWORDS_UNCORE + (CD_DWORDS_CORE * u32CoreCount) + (CD_DWORDS_GP * u32CoreCount);

    // Add the socket number item to the Crashdump JSON structure
    snprintf(jsonItemName, CD_JSON_STRING_LEN, CD_JSON_SOCKET_NAME, u8CPU);
    cJSON_AddItemToObject(pJsonChild, jsonItemName, socket = cJSON_CreateObject());

    // Add the crashdump info to the Crashdump JSON structure
    cJSON_AddItemToObject(socket, CD_JSON_HEADER_NAME, crashdump_section = cJSON_CreateObject());
	if (sCrashdump->header.size != 0)
	{
	    for (i = 0; i < (sCrashdump->header.size - 1); i++) {
	        snprintf(jsonItemName, CD_JSON_STRING_LEN, CD_JSON_HEADER_ENTRY, i);
	        snprintf(jsonItemString, CD_JSON_STRING_LEN, "0x%08x", sCrashdump->header.data[i]);
	        cJSON_AddStringToObject(crashdump_section, jsonItemName, jsonItemString);
	    }
	}
    // Add the crashdump data size to the crashdump info section
    snprintf(jsonItemString, CD_JSON_STRING_LEN, "0x%x", u32NumReads);
    cJSON_AddStringToObject(crashdump_section, CD_JSON_DATA_SIZE_NAME, jsonItemString);

    // Check that we have the correct amount of data
    if (u32NumReads && u32ExpectedNumReads == u32NumReads) {
#ifdef PRINT_CRASHDUMP_RAW_DATA
        for (u32CrashIndex=0; u32CrashIndex<u32NumReads; u32CrashIndex++) {
            printf("data%d[%u] = 0x%08x\n", u8CPU, u32CrashIndex, sCrashdump->data[u32CrashIndex]);
        }
        u32CrashIndex = 0;
#endif //PRINT_CRASHDUMP_RAW_DATA

        // Add the uncore info to the Crashdump JSON structure
        PRINT(PRINT_DBG, PRINT_INFO, "[DBG] - CPU #%d logging uncore data to JSON\n", u8CPU);
        cJSON_AddItemToObject(socket, CD_JSON_UNCORE_NAME, crashdump_section = cJSON_CreateObject());
        for (u32RegNum = 0; u32RegNum < CD_REGS_UNCORE; u32RegNum++) {
            if (sCrashdumpUncoreRegs[u32RegNum].dwords == 1) {
                snprintf(jsonItemString, CD_JSON_STRING_LEN, "0x%08x", sCrashdump->data[u32CrashIndex++]);
            } else if (sCrashdumpUncoreRegs[u32RegNum].dwords == 2) {
                snprintf(jsonItemString, CD_JSON_STRING_LEN, "0x%08x%08x", sCrashdump->data[u32CrashIndex + 1], sCrashdump->data[u32CrashIndex]);
                u32CrashIndex += 2;
            }
            cJSON_AddStringToObject(crashdump_section, sCrashdumpUncoreRegs[u32RegNum].name, jsonItemString);
        }

        for (coreMaskNum = 0; coreMaskNum < 2; coreMaskNum++) {
            if (0 == coreMaskNum) {
                u8CoreStartNum=16;
                u8CoreEndNum = 0;
            } else {
                u8CoreStartNum=28;
                u8CoreEndNum = 16;
            }
            // Write out the core register data
            PRINT(PRINT_DBG, PRINT_INFO, "[DBG] - CPU #%d logging core data to JSON\n", u8CPU);
            // The core data is returned in groups
            for (u8GroupNum = 0; u8GroupNum < CD_NUM_GROUPS_CORE; u8GroupNum++) {
                for (u8ThreadNum = 0; u8ThreadNum < CD_THREADS_PER_CORE; u8ThreadNum++) {
                    snprintf(jsonItemName, CD_JSON_STRING_LEN, CD_JSON_THREAD_NAME, u8ThreadNum);
                    if ((thread_section = cJSON_GetObjectItemCaseSensitive(socket, jsonItemName)) == NULL) {
                        cJSON_AddItemToObject(socket, jsonItemName, thread_section = cJSON_CreateObject());
                    }
                    for (u8CoreNum = u8CoreStartNum; u8CoreNum > u8CoreEndNum; u8CoreNum--) {
                        if (!(u32CoreMask & (1 << (u8CoreNum - 1)))) {
                            continue;
                        }
                        // Add the core info to the Crashdump JSON structure for this group only if it doesn't already exist
                        snprintf(jsonItemName, CD_JSON_STRING_LEN, CD_JSON_CORE_NAME, (u8CoreNum - 1));
                        if ((crashdump_section = cJSON_GetObjectItemCaseSensitive(thread_section, jsonItemName)) == NULL) {
                            cJSON_AddItemToObject(thread_section, jsonItemName, crashdump_section = cJSON_CreateObject());
                        }

                        for (u32RegNum = 0; u32RegNum < u8CrashdumpCoreGroupSizes[u8GroupNum]; u32RegNum++) {
                            // Thread scope registers are dumped for each thread, so print them with a thread number
                            if (sCrashdumpCoreRegs[u8GroupNum][u32RegNum].scope == THREAD_SCOPE) {
                                if (sCrashdumpCoreRegs[u8GroupNum][u32RegNum].dwords == 1) {
                                    snprintf(jsonItemString, CD_JSON_STRING_LEN, "0x%08x", sCrashdump->data[u32CrashIndex++]);
                                } else if (sCrashdumpCoreRegs[u8GroupNum][u32RegNum].dwords == 2) {
                                    snprintf(jsonItemString, CD_JSON_STRING_LEN, "0x%08x%08x", sCrashdump->data[u32CrashIndex + 1], sCrashdump->data[u32CrashIndex]);
                                    u32CrashIndex += 2;
                                }
                                cJSON_AddStringToObject(crashdump_section, sCrashdumpCoreRegs[u8GroupNum][u32RegNum].name, jsonItemString);
                            // Core scope registers are only dumped for thread 0
                            } else if (sCrashdumpCoreRegs[u8GroupNum][u32RegNum].scope == CORE_SCOPE && u8ThreadNum == 0) {
                                if (sCrashdumpCoreRegs[u8GroupNum][u32RegNum].dwords == 1) {
                                    snprintf(jsonItemString, CD_JSON_STRING_LEN, "0x%08x", sCrashdump->data[u32CrashIndex++]);
                                } else if (sCrashdumpCoreRegs[u8GroupNum][u32RegNum].dwords == 2) {
                                    snprintf(jsonItemString, CD_JSON_STRING_LEN, "0x%08x%08x", sCrashdump->data[u32CrashIndex + 1], sCrashdump->data[u32CrashIndex]);
                                    u32CrashIndex += 2;
                                }
                                cJSON_AddStringToObject(crashdump_section, sCrashdumpCoreRegs[u8GroupNum][u32RegNum].name, jsonItemString);
                            }
                        }

                    }
                }
            }
        }

        // Write out the GP register data to the debug log
        PRINT(PRINT_DBG, PRINT_INFO, "[DBG] - CPU #%d logging GP data to JSON\n", u8CPU);
        for (u8CoreNum = CD_CORES_IN_MASK; u8CoreNum > 0; u8CoreNum--) {
            if (!(u32CoreMask & (1 << (u8CoreNum - 1)))) {
                continue;
            }
            // Add the GP reg info to the Crashdump JSON structure
            snprintf(jsonItemName, CD_JSON_STRING_LEN, CD_JSON_GP_NAME, (u8CoreNum - 1));
            cJSON_AddItemToObject(socket, jsonItemName, crashdump_section = cJSON_CreateObject());
            for (u8ThreadNum = CD_THREADS_PER_CORE; u8ThreadNum > 0; u8ThreadNum--) {
                snprintf(jsonItemName, CD_JSON_STRING_LEN, CD_JSON_THREAD_NAME, u8ThreadNum - 1);
                cJSON_AddItemToObject(crashdump_section, jsonItemName, gp_thread = cJSON_CreateObject());
                for (u32RegNum = 0; u32RegNum < CD_REGS_GP_THREAD; u32RegNum++) {
                    if (sCrashdumpGpThreadRegs[u32RegNum].dwords == 1) {
                        snprintf(jsonItemString, CD_JSON_STRING_LEN, "0x%08x", sCrashdump->data[u32CrashIndex++]);
                    } else if (sCrashdumpGpThreadRegs[u32RegNum].dwords == 2) {
                        snprintf(jsonItemString, CD_JSON_STRING_LEN, "0x%08x%08x", sCrashdump->data[u32CrashIndex + 1], sCrashdump->data[u32CrashIndex]);
                        u32CrashIndex += 2;
                    }
                    cJSON_AddStringToObject(gp_thread, sCrashdumpGpThreadRegs[u32RegNum].name, jsonItemString);
                }
            }
        }

    }
}
#endif //BUILD_JSON

#ifdef SPX_BMC_ACD
void LogTmpDumpData(char * log)
{
    FILE *fp;

    /* Open the log file */
    fp = fopen(TMPDUMPLOG, "a+");
    if (fp == NULL)
    {
        printf("LogTmpCrashDump: Failed to open %s file\n", TMPDUMPLOG);
        return;
    }

    if (log == NULL)
    {
        fclose(fp);
        return;
    }

    /* Append the log to the file */
    fwrite(log, strlen(log), 1, fp);

    /* close */
    fclose(fp);
    fp = NULL;

    return;
}
#endif

/******************************************************************************
*
*    logCrashdump
*
*    BMC performs the crashdump retrieve from the processor directly via
*    PECI interface for internal state of the cores and Cbos after a platform three (3) strike
*    failure. The Crash Dump from CPU will be empty (size 0) if no cores qualify to be dumped.
*    A core will not be dumped if the power state is such that it cannot be accessed. A core will
*    be dumped only if it has experienced a "3-strike" Machine Check Error. The PECI flow is
*    listed below to generate a Crash Dump, and decode it.
*
*    WrPkgConfig() -
*         0x80 0x0003 0x00010038
*         Open Crash Dump Sequence.
*
*    RdPkgConfig() -
*         0x80 0x1020 HEADER_SIZE
*         Data of 0x5 should be value for Crash Dump.
*
*    RdPkgConfig() -
*         0x80 0x0002 CRASHDUMP_HEADER0
*
*    RdPkgConfig() -
*         0x80 0x0002 CRASHDUMP_HEADER1
*
*    RdPkgConfig() -
*         0x80 0x0002 CRASHDUMP_HEADER2
*
*    RdPkgConfig() -
*         0x80 0x0002 CRASHDUMP_HEADER3
*
*    RdPkgConfig() -
*         0x80 0x0002 NUMBER_OF_READS
*         Data N is the number of additional RdPkgConfig() commands required to collect all the data
*         for the dump. This value depends on the number of cores flagged in the core mask, and the
*         content to be dumped.
*
*    RdPkgConfig() * N -
*         0x80 0x0002 DataN
*         Crash Dump data [1-N]. Use decoding table for decoding.
*
*    WrPkgConfig() -
*         0x80 0x0004 0x00010038 Close Crash Dump Sequence.
*
******************************************************************************/
ESTATUS logCrashdump(FILE * fpRaw, FILE * fp, cJSON * pJsonChild)
{
    EPECIStatus ePECIStatus = PECI_CC_SUCCESS;
    UINT8 u8CPU;
    SCrashdump sCrashdump;
    UINT32 u32NumReads = 0;
    UINT32 i;
    UINT32 u32work;
    ESTATUS eStatus = ST_OK;
    SRdPkgConfigReq sRdPkgConfigReq;
    SRdPkgConfigRes sRdPkgConfigRes;
    SWrPkgConfigReq sWrPkgConfigReq;
    SWrPkgConfigRes sWrPkgConfigRes;
    char tmplog[TMPLOGDATALEN] = {0};

#ifdef SPX_BMC_ACD
    UN_USED(fp);

    // Maintain last 100 lines from tmp dump log file
    snprintf(tmplog, sizeof(tmplog)-1, "tail -n 100 %s > %s; mv %s %s", TMPDUMPLOG, TMPDUMPLOG1, TMPDUMPLOG1, TMPDUMPLOG);
    system(tmplog);

    LogTmpDumpData("\n\nStart of new crashdump\n***********************\n");
#endif

    // Go through all CPUs
    for (u8CPU = CPU0_ID; u8CPU < MAX_CPU; u8CPU++) {
        if (!IsCpuPresent(u8CPU)) {
            continue;
        }

#ifdef SPX_BMC_ACD
        snprintf(tmplog, sizeof(tmplog)-1, "CPU %d\n", u8CPU);
        LogTmpDumpData(tmplog);
#endif

        // Clear the buffer for each CPU
        memset_s(&sCrashdump, sizeof(sCrashdump), 0);

        // Start the crashdump log
        PRINT(PRINT_DBG, PRINT_INFO, "Platform Debug - Crashdump log %d\n", u8CPU);

        // Open the Crashdump sequence
        PRINT(PRINT_DBG, PRINT_INFO, "[DBG] - CPU #%d Crashdump Sequence Opened\n", u8CPU);
#ifdef SPX_BMC_ACD
        LogTmpDumpData("Crashdump Open Sequence\n");
#endif
        memset(&sWrPkgConfigReq, 0 , sizeof(SWrPkgConfigReq));
        memset(&sWrPkgConfigRes, 0 , sizeof(SWrPkgConfigRes));
        sWrPkgConfigReq.sHeader.u8ClientAddr = PECI_BASE_ADDR + u8CPU;
        sWrPkgConfigReq.sHeader.u8WriteLength = sizeof(UINT32) + 6;
        sWrPkgConfigReq.sHeader.u8ReadLength = 0x01;
        sWrPkgConfigReq.u8CmdCode = 0xA5;
        sWrPkgConfigReq.u8HostID_Retry = 0x02;
        sWrPkgConfigReq.u8Index = MBX_INDEX_VCU;
        sWrPkgConfigReq.u16Parameter = VCU_OPEN_SEQ;
        u32work = CD_SEQ_DATA;
        memcpy (sWrPkgConfigReq.u8Data, &u32work, sizeof(UINT32));
        if (0 != PECI_WrPkgConfig (&sWrPkgConfigReq, &sWrPkgConfigRes))
        {
            sWrPkgConfigRes.u8CompletionCode = 0x00;
        }
#ifdef SPX_BMC_ACD
        snprintf(tmplog, sizeof(tmplog)-1, "\tCompletion Code: 0x%x\n", sWrPkgConfigRes.u8CompletionCode);
        LogTmpDumpData(tmplog);
#endif
        ePECIStatus = sWrPkgConfigRes.u8CompletionCode;
        if (ePECIStatus != PECI_CC_SUCCESS) {
            // Crashdump sequence failed, abort the sequence and go to the next CPU
            PRINT(PRINT_DBG, PRINT_ERROR, "[DBG] - CPU #%d Crashdump sequence failed, abort the sequence and go to next CPU\n", u8CPU);
#ifdef SPX_BMC_ACD
            LogTmpDumpData("Crashdump sequence failed, abort the sequence and go to next CPU\n");
#endif
            memset(&sWrPkgConfigReq, 0 , sizeof(SWrPkgConfigReq));
            memset(&sWrPkgConfigRes, 0 , sizeof(SWrPkgConfigRes));
            sWrPkgConfigReq.sHeader.u8ClientAddr = PECI_BASE_ADDR + u8CPU;
            sWrPkgConfigReq.sHeader.u8WriteLength = sizeof(UINT32) + 6;
            sWrPkgConfigReq.sHeader.u8ReadLength = 0x01;
            sWrPkgConfigReq.u8CmdCode = 0xA5;
            sWrPkgConfigReq.u8HostID_Retry = 0x02;
            sWrPkgConfigReq.u8Index = MBX_INDEX_VCU;
            sWrPkgConfigReq.u16Parameter = VCU_ABORT_SEQ;
            u32work = CD_SEQ_DATA;
            memcpy (sWrPkgConfigReq.u8Data, &u32work, sizeof(UINT32));
            if (0 != PECI_WrPkgConfig (&sWrPkgConfigReq, &sWrPkgConfigRes))
            {
                sWrPkgConfigRes.u8CompletionCode = 0x00;
            }
            PRINT(PRINT_DBG, PRINT_ERROR, "[DBG] - CPU #%d Crashdump Sequence Failed at line %d\n", u8CPU, __LINE__);
#ifdef SPX_BMC_ACD
            LogTmpDumpData("Crashdump Sequence Failed\n");
#endif
            eStatus = ST_HW_FAILURE;
            continue;
        }

        // Get the Header Data
        PRINT(PRINT_DBG, PRINT_INFO, "[DBG] - CPU #%d Get the Header Size\n", u8CPU);
#ifdef SPX_BMC_ACD
        LogTmpDumpData("Get the Header Size\n");
#endif
        memset(&sRdPkgConfigReq, 0 , sizeof(SRdPkgConfigReq));
        memset(&sRdPkgConfigRes, 0 , sizeof(SRdPkgConfigRes));
        sRdPkgConfigReq.sHeader.u8ClientAddr = PECI_BASE_ADDR + u8CPU;
        sRdPkgConfigReq.sHeader.u8WriteLength = 0x05;
        sRdPkgConfigReq.sHeader.u8ReadLength = sizeof(UINT32) + 1;
        sRdPkgConfigReq.u8CmdCode = 0xA1;
        sRdPkgConfigReq.u8HostID_Retry = 0x02;
        sRdPkgConfigReq.u8Index = MBX_INDEX_VCU;
        sRdPkgConfigReq.u16Parameter = CD_HEADER_PARAM;
        if (0 == PECI_RdPkgConfig (&sRdPkgConfigReq, &sRdPkgConfigRes))
        {
            memcpy(&sCrashdump.header.size, sRdPkgConfigRes.u8Data, sizeof(UINT32));
        }
        else
        {
            sRdPkgConfigRes.u8CompletionCode = 0x00;
        }
#ifdef SPX_BMC_ACD
        snprintf(tmplog, sizeof(tmplog)-1, "\tCompletion Code: 0x%x, Header Size: 0x%x\n", sRdPkgConfigRes.u8CompletionCode, sCrashdump.header.size);
        LogTmpDumpData(tmplog);
#endif
        ePECIStatus = sRdPkgConfigRes.u8CompletionCode;
        if (ePECIStatus != PECI_CC_SUCCESS) {
            // Crashdump sequence failed, abort the sequence and go to the next CPU
            PRINT(PRINT_DBG, PRINT_ERROR, "[DBG] - CPU #%d Get Header Size failed, sending ABORT and go to next CPU\n", u8CPU);
#ifdef SPX_BMC_ACD
            LogTmpDumpData("Get Header Size failed, sending ABORT and go to next CPU\n");
#endif
            memset(&sWrPkgConfigReq, 0 , sizeof(SWrPkgConfigReq));
            memset(&sWrPkgConfigRes, 0 , sizeof(SWrPkgConfigRes));
            sWrPkgConfigReq.sHeader.u8ClientAddr = PECI_BASE_ADDR + u8CPU;
            sWrPkgConfigReq.sHeader.u8WriteLength = sizeof(UINT32) + 6;
            sWrPkgConfigReq.sHeader.u8ReadLength = 0x01;
            sWrPkgConfigReq.u8CmdCode = 0xA5;
            sWrPkgConfigReq.u8HostID_Retry = 0x02;
            sWrPkgConfigReq.u8Index = MBX_INDEX_VCU;
            sWrPkgConfigReq.u16Parameter = VCU_ABORT_SEQ;
            u32work = CD_SEQ_DATA;
            memcpy (sWrPkgConfigReq.u8Data, &u32work, sizeof(UINT32));
            if (0 != PECI_WrPkgConfig (&sWrPkgConfigReq, &sWrPkgConfigRes))
            {
                sWrPkgConfigRes.u8CompletionCode = 0x00;
            }
            PRINT(PRINT_DBG, PRINT_ERROR, "[DBG] - CPU #%d Crashdump Sequence Failed at line %d\n", u8CPU, __LINE__);
#ifdef SPX_BMC_ACD
            LogTmpDumpData("Crashdump Sequence Failed\n");
#endif
            eStatus = ST_HW_FAILURE;
            continue;
        }
        for (i = 0; i < CD_DWORDS_HEADER; i++) {
            PRINT(PRINT_DBG, PRINT_INFO, "[DBG] - CPU #%d Get the Header Data %d\n", u8CPU, i);
#ifdef SPX_BMC_ACD
            snprintf(tmplog, sizeof(tmplog)-1, "Get the Header Data %d\n", i);
            LogTmpDumpData(tmplog);
#endif
            memset(&sRdPkgConfigReq, 0 , sizeof(SRdPkgConfigReq));
            memset(&sRdPkgConfigRes, 0 , sizeof(SRdPkgConfigRes));
            sRdPkgConfigReq.sHeader.u8ClientAddr = PECI_BASE_ADDR + u8CPU;
            sRdPkgConfigReq.sHeader.u8WriteLength = 0x05;
            sRdPkgConfigReq.sHeader.u8ReadLength = sizeof(UINT32) + 1;
            sRdPkgConfigReq.u8CmdCode = 0xA1;
            sRdPkgConfigReq.u8HostID_Retry = 0x02;
            sRdPkgConfigReq.u8Index = MBX_INDEX_VCU;
            sRdPkgConfigReq.u16Parameter = VCU_READ;
            if (0 == PECI_RdPkgConfig (&sRdPkgConfigReq, &sRdPkgConfigRes))
            {
                memcpy(&sCrashdump.header.data[i], sRdPkgConfigRes.u8Data, sizeof(UINT32));
#ifdef SPX_BMC_ACD
                snprintf(tmplog, sizeof(tmplog)-1, "\tData: 0x%x\n", sCrashdump.header.data[i]);
                LogTmpDumpData(tmplog);
#endif
            }
            else
            {
                sRdPkgConfigRes.u8CompletionCode = 0x00;
            }
#ifdef SPX_BMC_ACD
            snprintf(tmplog, sizeof(tmplog)-1, "\tCompletion Code: 0x%x\n", sRdPkgConfigRes.u8CompletionCode);
            LogTmpDumpData(tmplog);
#endif
            ePECIStatus = sRdPkgConfigRes.u8CompletionCode;
            if (ePECIStatus != PECI_CC_SUCCESS) {
                // Crashdump sequence failed, abort the sequence and break out of the header loop to go to the next CPU
                PRINT(PRINT_DBG, PRINT_ERROR, "[DBG] - CPU #%d Get Header Data %d failed, sending ABORT, break out of header loop and go to next CPU\n", u8CPU, i);
#ifdef SPX_BMC_ACD
                LogTmpDumpData("Get Header Data %d failed, sending ABORT, break out of header loop and go to next CPU\n");
#endif
                memset(&sWrPkgConfigReq, 0 , sizeof(SWrPkgConfigReq));
                memset(&sWrPkgConfigRes, 0 , sizeof(SWrPkgConfigRes));
                sWrPkgConfigReq.sHeader.u8ClientAddr = PECI_BASE_ADDR + u8CPU;
                sWrPkgConfigReq.sHeader.u8WriteLength = sizeof(UINT32) + 6;
                sWrPkgConfigReq.sHeader.u8ReadLength = 0x01;
                sWrPkgConfigReq.u8CmdCode = 0xA5;
                sWrPkgConfigReq.u8HostID_Retry = 0x02;
                sWrPkgConfigReq.u8Index = MBX_INDEX_VCU;
                sWrPkgConfigReq.u16Parameter = VCU_ABORT_SEQ;
                u32work = CD_SEQ_DATA;
                memcpy (sWrPkgConfigReq.u8Data, &u32work, sizeof(UINT32));
                if (0 != PECI_WrPkgConfig (&sWrPkgConfigReq, &sWrPkgConfigRes))
                {
                    sWrPkgConfigRes.u8CompletionCode = 0x00;
                }
                PRINT(PRINT_DBG, PRINT_ERROR, "[DBG] - CPU #%d Crashdump Sequence Failed at line %d\n", u8CPU, __LINE__);
#ifdef SPX_BMC_ACD
                LogTmpDumpData("Crashdump Sequence Failed\n");
#endif
                eStatus = ST_HW_FAILURE;
                break;
            }
        }
        // Check if the sequence failed. If so, go to the next CPU
        if (ePECIStatus != PECI_CC_SUCCESS)
            continue;

        // Get the number of remaining dword reads
        PRINT(PRINT_DBG, PRINT_INFO, "[DBG] - CPU #%d Get the Number of Reads\n", u8CPU);
#ifdef SPX_BMC_ACD
        LogTmpDumpData("Get the Number of Reads\n");
#endif
        memset(&sRdPkgConfigReq, 0 , sizeof(SRdPkgConfigReq));
        memset(&sRdPkgConfigRes, 0 , sizeof(SRdPkgConfigRes));
        sRdPkgConfigReq.sHeader.u8ClientAddr = PECI_BASE_ADDR + u8CPU;
        sRdPkgConfigReq.sHeader.u8WriteLength = 0x05;
        sRdPkgConfigReq.sHeader.u8ReadLength = sizeof(UINT32) + 1;
        sRdPkgConfigReq.u8CmdCode = 0xA1;
        sRdPkgConfigReq.u8HostID_Retry = 0x02;
        sRdPkgConfigReq.u8Index = MBX_INDEX_VCU;
        sRdPkgConfigReq.u16Parameter = VCU_READ;
        if (0 == PECI_RdPkgConfig (&sRdPkgConfigReq, &sRdPkgConfigRes))
        {
            memcpy(&u32NumReads, sRdPkgConfigRes.u8Data, sizeof(UINT32));
        }
        else
        {
            sRdPkgConfigRes.u8CompletionCode = 0x00;
        }
        ePECIStatus = sRdPkgConfigRes.u8CompletionCode;
        if (ePECIStatus != PECI_CC_SUCCESS) {
            // Crashdump sequence failed, abort the sequence and go to the next CPU
            PRINT(PRINT_DBG, PRINT_ERROR, "[DBG] - CPU #%d Get Number of Reads failed, sending ABORT and go to next CPU\n", u8CPU, i);
#ifdef SPX_BMC_ACD
            LogTmpDumpData("Get Number of Reads failed, sending ABORT and go to next CPU\n");
#endif
            memset(&sWrPkgConfigReq, 0 , sizeof(SWrPkgConfigReq));
            memset(&sWrPkgConfigRes, 0 , sizeof(SWrPkgConfigRes));
            sWrPkgConfigReq.sHeader.u8ClientAddr = PECI_BASE_ADDR + u8CPU;
            sWrPkgConfigReq.sHeader.u8WriteLength = sizeof(UINT32) + 6;
            sWrPkgConfigReq.sHeader.u8ReadLength = 0x01;
            sWrPkgConfigReq.u8CmdCode = 0xA5;
            sWrPkgConfigReq.u8HostID_Retry = 0x02;
            sWrPkgConfigReq.u8Index = MBX_INDEX_VCU;
            sWrPkgConfigReq.u16Parameter = VCU_ABORT_SEQ;
            u32work = CD_SEQ_DATA;
            memcpy (sWrPkgConfigReq.u8Data, &u32work, sizeof(UINT32));
            if (0 != PECI_WrPkgConfig (&sWrPkgConfigReq, &sWrPkgConfigRes))
            {
                sWrPkgConfigRes.u8CompletionCode = 0x00;
            }
            PRINT(PRINT_DBG, PRINT_ERROR, "[DBG] - CPU #%d Crashdump Sequence Failed at line %d\n", u8CPU, __LINE__);
#ifdef SPX_BMC_ACD
            LogTmpDumpData("Crashdump Sequence Failed\n");
#endif
            eStatus = ST_HW_FAILURE;
            continue;
        }
#ifdef SPX_BMC_ACD
        snprintf(tmplog, sizeof(tmplog)-1, "\tNumber of Reads: 0x%x\n", u32NumReads);
        LogTmpDumpData(tmplog);
#endif
		sCrashdump.header.u32NumReads = u32NumReads;

        // Get the raw data
        sCrashdump.data = (UINT32 *)calloc(u32NumReads, sizeof(UINT32));
        if (sCrashdump.data == NULL) {
            // calloc failed, abort the sequence and go to the next CPU
            memset(&sWrPkgConfigReq, 0 , sizeof(SWrPkgConfigReq));
            memset(&sWrPkgConfigRes, 0 , sizeof(SWrPkgConfigRes));
            sWrPkgConfigReq.sHeader.u8ClientAddr = PECI_BASE_ADDR + u8CPU;
            sWrPkgConfigReq.sHeader.u8WriteLength = sizeof(UINT32) + 6;
            sWrPkgConfigReq.sHeader.u8ReadLength = 0x01;
            sWrPkgConfigReq.u8CmdCode = 0xA5;
            sWrPkgConfigReq.u8HostID_Retry = 0x02;
            sWrPkgConfigReq.u8Index = MBX_INDEX_VCU;
            sWrPkgConfigReq.u16Parameter = VCU_ABORT_SEQ;
            u32work = CD_SEQ_DATA;
            memcpy (sWrPkgConfigReq.u8Data, &u32work, sizeof(UINT32));
            if (0 != PECI_WrPkgConfig (&sWrPkgConfigReq, &sWrPkgConfigRes))
            {
                sWrPkgConfigRes.u8CompletionCode = 0x00;
            }
            PRINT(PRINT_DBG, PRINT_ERROR, "[DBG] - CPU #%d Crashdump Sequence Failed at line %d\n", u8CPU, __LINE__);
            eStatus = ST_HW_FAILURE;
            continue;
        }
        for (i = 0; i < u32NumReads; i++) {
            PRINT(PRINT_DBG, PRINT_INFO, "[DBG] - CPU #%d Get crashdump data %d\n", u8CPU, i);
            memset(&sRdPkgConfigReq, 0 , sizeof(SRdPkgConfigReq));
            memset(&sRdPkgConfigRes, 0 , sizeof(SRdPkgConfigRes));
            sRdPkgConfigReq.sHeader.u8ClientAddr = PECI_BASE_ADDR + u8CPU;
            sRdPkgConfigReq.sHeader.u8WriteLength = 0x05;
            sRdPkgConfigReq.sHeader.u8ReadLength = sizeof(UINT32) + 1;
            sRdPkgConfigReq.u8CmdCode = 0xA1;
            sRdPkgConfigReq.u8HostID_Retry = 0x02;
            sRdPkgConfigReq.u8Index = MBX_INDEX_VCU;
            sRdPkgConfigReq.u16Parameter = VCU_READ;
            if (0 == PECI_RdPkgConfig (&sRdPkgConfigReq, &sRdPkgConfigRes))
            {
                memcpy(&sCrashdump.data[i], sRdPkgConfigRes.u8Data, sizeof(UINT32));
            }
            else
            {
                sRdPkgConfigRes.u8CompletionCode = 0x00;
            }
            ePECIStatus = sRdPkgConfigRes.u8CompletionCode;
            if (ePECIStatus != PECI_CC_SUCCESS) {
                // Crashdump sequence failed, note the number of dwords read and abort the sequence
                PRINT(PRINT_DBG, PRINT_ERROR, "[DBG] - CPU #%d Get data failed, note the Number of Reads, abort sequence and go to next CPU\n", u8CPU, i);
                u32NumReads = i;
                memset(&sWrPkgConfigReq, 0 , sizeof(SWrPkgConfigReq));
                memset(&sWrPkgConfigRes, 0 , sizeof(SWrPkgConfigRes));
                sWrPkgConfigReq.sHeader.u8ClientAddr = PECI_BASE_ADDR + u8CPU;
                sWrPkgConfigReq.sHeader.u8WriteLength = sizeof(UINT32) + 6;
                sWrPkgConfigReq.sHeader.u8ReadLength = 0x01;
                sWrPkgConfigReq.u8CmdCode = 0xA5;
                sWrPkgConfigReq.u8HostID_Retry = 0x02;
                sWrPkgConfigReq.u8Index = MBX_INDEX_VCU;
                sWrPkgConfigReq.u16Parameter = VCU_ABORT_SEQ;
                u32work = CD_SEQ_DATA;
                memcpy (sWrPkgConfigReq.u8Data, &u32work, sizeof(UINT32));
                if (0 != PECI_WrPkgConfig (&sWrPkgConfigReq, &sWrPkgConfigRes))
                {
                    sWrPkgConfigRes.u8CompletionCode = 0x00;
                }
                PRINT(PRINT_DBG, PRINT_ERROR, "[DBG] - CPU #%d Crashdump Sequence Failed at line %d\n", u8CPU, __LINE__);
                eStatus = ST_HW_FAILURE;
                break;
            }
        }

        // Close the Crashdump sequence
        PRINT(PRINT_DBG, PRINT_INFO, "[DBG] - CPU #%d Crashdump Sequence Closed\n", u8CPU);
#ifdef SPX_BMC_ACD
        LogTmpDumpData("Crashdump Close Sequence\n");
#endif
        memset(&sWrPkgConfigReq, 0 , sizeof(SWrPkgConfigReq));
        memset(&sWrPkgConfigRes, 0 , sizeof(SWrPkgConfigRes));
        sWrPkgConfigReq.sHeader.u8ClientAddr = PECI_BASE_ADDR + u8CPU;
        sWrPkgConfigReq.sHeader.u8WriteLength = sizeof(UINT32) + 6;
        sWrPkgConfigReq.sHeader.u8ReadLength = 0x01;
        sWrPkgConfigReq.u8CmdCode = 0xA5;
        sWrPkgConfigReq.u8HostID_Retry = 0x02;
        sWrPkgConfigReq.u8Index = MBX_INDEX_VCU;
        sWrPkgConfigReq.u16Parameter = VCU_CLOSE_SEQ;
        u32work = CD_SEQ_DATA;
        memcpy (sWrPkgConfigReq.u8Data, &u32work, sizeof(UINT32));
        if (0 != PECI_WrPkgConfig (&sWrPkgConfigReq, &sWrPkgConfigRes))
        {
            sWrPkgConfigRes.u8CompletionCode = 0x00;
        }
#ifdef SPX_BMC_ACD
        snprintf(tmplog, sizeof(tmplog)-1, "\tCompletion Code: 0x%x\n", sWrPkgConfigRes.u8CompletionCode);
        LogTmpDumpData(tmplog);
#endif

        // Log the Crashdump for this CPU
#ifdef BUILD_TXT
        if (fp != NULL) {
            crashdumpTxt(u8CPU, u32NumReads, &sCrashdump, fp);
        }
#endif //BUILD_TXT

#ifdef BUILD_RAW
		if (fpRaw != NULL) {
			crashdumpRaw(u8CPU, u32NumReads, &sCrashdump, fpRaw);
		}
#endif //BUILD_RAW

#ifdef BUILD_JSON
        if (pJsonChild != NULL) {
            crashdumpJson(u8CPU, u32NumReads, &sCrashdump, pJsonChild);
        }
#endif //_CRASHDUMP_H_

        free(sCrashdump.data);
    }

    return eStatus;
}
