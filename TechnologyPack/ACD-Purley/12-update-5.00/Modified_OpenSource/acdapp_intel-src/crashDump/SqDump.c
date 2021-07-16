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
#include "SqDump.h"


#ifdef BUILD_RAW
static void sqDumpRaw(UINT8 u8Cpu, UINT32 u32CoreNum, SSqDump * psSqDump, FILE * fpRaw)
{
#ifdef SPX_BMC_ACD
    UN_USED(u8Cpu);
    UN_USED(u32CoreNum);
#endif
    fwrite(psSqDump, sizeof(SSqDump), 1, fpRaw);
}
#endif //BUILD_RAW

/******************************************************************************
*
*   sqDumpJson
*
*   This function formats the SQ Dump log into a JSON object
*
******************************************************************************/
#ifdef BUILD_JSON
static void sqDumpJson(UINT8 u8Cpu, UINT32 u32CoreNum, SSqDump * psSqDump, cJSON * pJsonChild)
{
    cJSON * socket;
    cJSON * core;
    cJSON * entry;
    cJSON * sqData;
    char jsonItemName[SQ_JSON_STRING_LEN];
    char jsonItemString[SQ_JSON_STRING_LEN];
    UINT32 i;
    UINT8 u8DwordNum;

    // Only add the section if there is data to include
    if (psSqDump->u32SqAddrSize == 0 && psSqDump->u32SqCtrlSize == 0) {
        return;
    }

    // Add the socket number item to the SQ dump JSON structure only if it doesn't already exist
    snprintf(jsonItemName, SQ_JSON_STRING_LEN, SQ_JSON_SOCKET_NAME, u8Cpu);
    if ((socket = cJSON_GetObjectItemCaseSensitive(pJsonChild, jsonItemName)) == NULL) {
        cJSON_AddItemToObject(pJsonChild, jsonItemName, socket = cJSON_CreateObject());
    }
    // Add the core number item to the SQ dump JSON structure only if it doesn't already exist
    snprintf(jsonItemName, SQ_JSON_STRING_LEN, SQ_JSON_CORE_NAME, u32CoreNum);
    if ((core = cJSON_GetObjectItemCaseSensitive(socket, jsonItemName)) == NULL) {
        cJSON_AddItemToObject(socket, jsonItemName, core = cJSON_CreateObject());
    }
    // Add the Address Array data if it exists
    if (psSqDump->pu32SqAddrArray != NULL) {
        // Log the data an entry at a time
        for (i = 0; (i + SQ_DWORDS_PER_ENTRY - 1) < psSqDump->u32SqAddrSize; i += SQ_DWORDS_PER_ENTRY) {
            // Add the entry number to the SQ dump JSON structure only if it doesn't already exist
            snprintf(jsonItemName, SQ_JSON_STRING_LEN, SQ_JSON_ENTRY_NAME, (i / SQ_DWORDS_PER_ENTRY));
            if ((entry = cJSON_GetObjectItemCaseSensitive(core, jsonItemName)) == NULL) {
                cJSON_AddItemToObject(core, jsonItemName, entry = cJSON_CreateObject());
            }
            // Add the address header for this SQ Entry
            cJSON_AddItemToObject(entry, SQ_JSON_ADDR_ARRAY_NAME, sqData = cJSON_CreateObject());
            // Add the address data for this SQ Entry
            for (u8DwordNum = 0; u8DwordNum < SQ_DWORDS_PER_ENTRY; u8DwordNum++) {
                // Add the DWORD number item to the SQ dump JSON structure
                snprintf(jsonItemName, SQ_JSON_STRING_LEN, SQ_JSON_DWORD_NAME, u8DwordNum);
                snprintf(jsonItemString, SQ_JSON_STRING_LEN, "0x%x", psSqDump->pu32SqAddrArray[i + u8DwordNum]);
                cJSON_AddStringToObject(sqData, jsonItemName, jsonItemString);
            }
        }
    }
    // Add the Control Array data if it exists
    if (psSqDump->pu32SqCtrlArray != NULL) {
        // Log the data an entry at a time
        for (i = 0; (i + SQ_DWORDS_PER_ENTRY - 1) < psSqDump->u32SqCtrlSize; i += SQ_DWORDS_PER_ENTRY) {
            // Add the entry number to the SQ dump JSON structure only if it doesn't already exist
            snprintf(jsonItemName, SQ_JSON_STRING_LEN, SQ_JSON_ENTRY_NAME, (i / SQ_DWORDS_PER_ENTRY));
            if ((entry = cJSON_GetObjectItemCaseSensitive(core, jsonItemName)) == NULL) {
                cJSON_AddItemToObject(core, jsonItemName, entry = cJSON_CreateObject());
            }
            // Add the control header for this SQ Entry
            cJSON_AddItemToObject(entry, SQ_JSON_CTRL_ARRAY_NAME, sqData = cJSON_CreateObject());
            // Add the control data for this SQ Entry
            for (u8DwordNum = 0; u8DwordNum < SQ_DWORDS_PER_ENTRY; u8DwordNum++) {
                // Add the DWORD number item to the SQ dump JSON structure
                snprintf(jsonItemName, SQ_JSON_STRING_LEN, SQ_JSON_DWORD_NAME, u8DwordNum);
                snprintf(jsonItemString, SQ_JSON_STRING_LEN, "0x%x", psSqDump->pu32SqCtrlArray[i + u8DwordNum]);
                cJSON_AddStringToObject(sqData, jsonItemName, jsonItemString);
            }
        }
    }
}
#endif //BUILD_JSON

/******************************************************************************
*
*   sqDump
*
*   This function gathers the specified type of SQ Dump
*
* Command Number        PECI Command        INDEX   Parameter   DATA                                    Comment
*       1               WrPkgConfig()       0x80    0x0003      0x30004                                 OPEN_SEQUENCE OpCode with SQ Dump Sequence ID
*       2               WrPkgConfig()       0x80    0x0001      [31]: 0=Addr Array, 1 = Control Array   SET_PARAMETER OpCode with configuration data
*                                                               [7:0] Core Number
*       3               RdPkgConfig()       0x80    0x3002      NUMBER_OF_READS (in Dwords)             START_SQ_DUMP Opcode
*       4 to (N+4)      RdPkgConfig() * N   0x80    0x0002      DataN                                   READ_DATA OpCode
*                       N = NUMBER_OF_READS
*       N+5             WrPkgConfig()       0x80    0x0004      0                                       Close SQ Dump Sequence
*
******************************************************************************/
static ESTATUS sqDump(UINT8 u8Cpu, UINT32 u32CoreNum, UINT32 u32SqType, UINT32 ** ppu32SqDumpRet, UINT32 * pu32SqDumpSize)
{
    UINT32 * pu32SqDump = NULL;
    UINT32 u32NumReads = 0;
    UINT32 i;
    UINT32 u32work;
    SRdPkgConfigReq sRdPkgConfigReq;
    SRdPkgConfigRes sRdPkgConfigRes;
    SWrPkgConfigReq sWrPkgConfigReq;
    SWrPkgConfigRes sWrPkgConfigRes;

    // Open the SQ dump sequence
    PRINT(PRINT_DBG, PRINT_INFO, "[DBG] - CPU #%d Core %u Type 0x%x SQ Dump Sequence Opened\n", u8Cpu, u32CoreNum, u32SqType);
    memset(&sWrPkgConfigReq, 0 , sizeof(SWrPkgConfigReq));
    memset(&sWrPkgConfigRes, 0 , sizeof(SWrPkgConfigRes));
    sWrPkgConfigReq.sHeader.u8ClientAddr = PECI_BASE_ADDR + u8Cpu;
    sWrPkgConfigReq.sHeader.u8WriteLength = sizeof(UINT32) + 6;
    sWrPkgConfigReq.sHeader.u8ReadLength = 0x01;
    sWrPkgConfigReq.u8CmdCode = 0xA5;
    sWrPkgConfigReq.u8HostID_Retry = 0x02;
    sWrPkgConfigReq.u8Index = MBX_INDEX_VCU;
    sWrPkgConfigReq.u16Parameter = VCU_OPEN_SEQ;
    u32work = SQ_SEQ_DATA;
    memcpy (sWrPkgConfigReq.u8Data, &u32work, sizeof(UINT32));
    if (0 != PECI_WrPkgConfig (&sWrPkgConfigReq, &sWrPkgConfigRes))
    {
        sWrPkgConfigRes.u8CompletionCode = 0x00;
    }
    if (sWrPkgConfigRes.u8CompletionCode != PECI_CC_SUCCESS) {
        // If SQ data is not available on a core, it will print this message, so treat it as INFO instead of ERROR.
        PRINT(PRINT_DBG, PRINT_INFO, "[DBG] - CPU #%d Core %u Type 0x%x SQ dump Sequence Failed CC=0x%02x\n", u8Cpu, u32CoreNum, u32SqType, sWrPkgConfigRes.u8CompletionCode);
        // SQ dump sequence failed, abort the sequence and go to the next CPU
        memset(&sWrPkgConfigReq, 0 , sizeof(SWrPkgConfigReq));
        memset(&sWrPkgConfigRes, 0 , sizeof(SWrPkgConfigRes));
        sWrPkgConfigReq.sHeader.u8ClientAddr = PECI_BASE_ADDR + u8Cpu;
        sWrPkgConfigReq.sHeader.u8WriteLength = sizeof(UINT32) + 6;
        sWrPkgConfigReq.sHeader.u8ReadLength = 0x01;
        sWrPkgConfigReq.u8CmdCode = 0xA5;
        sWrPkgConfigReq.u8HostID_Retry = 0x02;
        sWrPkgConfigReq.u8Index = MBX_INDEX_VCU;
        sWrPkgConfigReq.u16Parameter = VCU_ABORT_SEQ;
        u32work = SQ_SEQ_DATA;
        memcpy (sWrPkgConfigReq.u8Data, &u32work, sizeof(UINT32));
        if (0 != PECI_WrPkgConfig (&sWrPkgConfigReq, &sWrPkgConfigRes))
        {
            sWrPkgConfigRes.u8CompletionCode = 0x00;
        }
        return ST_HW_FAILURE;
    }

    // SET_PARAMETER OpCode with configuration data
    PRINT(PRINT_DBG, PRINT_INFO, "[DBG] - CPU #%d Core %u Type 0x%x Set SQ Dump OpCode with configuration data 0x%x\n", u8Cpu, u32CoreNum, u32SqType, (u32SqType | u32CoreNum));
    memset(&sWrPkgConfigReq, 0 , sizeof(SWrPkgConfigReq));
    memset(&sWrPkgConfigRes, 0 , sizeof(SWrPkgConfigRes));
    sWrPkgConfigReq.sHeader.u8ClientAddr = PECI_BASE_ADDR + u8Cpu;
    sWrPkgConfigReq.sHeader.u8WriteLength = sizeof(UINT32) + 6;
    sWrPkgConfigReq.sHeader.u8ReadLength = 0x01;
    sWrPkgConfigReq.u8CmdCode = 0xA5;
    sWrPkgConfigReq.u8HostID_Retry = 0x02;
    sWrPkgConfigReq.u8Index = MBX_INDEX_VCU;
    sWrPkgConfigReq.u16Parameter = VCU_SET_PARAM;
    u32work = (u32SqType | u32CoreNum);
    memcpy (sWrPkgConfigReq.u8Data, &u32work, sizeof(UINT32));
    if (0 != PECI_WrPkgConfig (&sWrPkgConfigReq, &sWrPkgConfigRes))
    {
        sWrPkgConfigRes.u8CompletionCode = 0x00;
    }
    if (sWrPkgConfigRes.u8CompletionCode != PECI_CC_SUCCESS) {
        PRINT(PRINT_DBG, PRINT_ERROR, "[DBG] - CPU #%d Core %u Type 0x%x SQ dump Sequence Failed CC=0x%02x\n", u8Cpu, u32CoreNum, u32SqType, sWrPkgConfigRes.u8CompletionCode);
        // SQ dump sequence failed, abort the sequence and go to the next CPU
        memset(&sWrPkgConfigReq, 0 , sizeof(SWrPkgConfigReq));
        memset(&sWrPkgConfigRes, 0 , sizeof(SWrPkgConfigRes));
        sWrPkgConfigReq.sHeader.u8ClientAddr = PECI_BASE_ADDR + u8Cpu;
        sWrPkgConfigReq.sHeader.u8WriteLength = sizeof(UINT32) + 6;
        sWrPkgConfigReq.sHeader.u8ReadLength = 0x01;
        sWrPkgConfigReq.u8CmdCode = 0xA5;
        sWrPkgConfigReq.u8HostID_Retry = 0x02;
        sWrPkgConfigReq.u8Index = MBX_INDEX_VCU;
        sWrPkgConfigReq.u16Parameter = VCU_ABORT_SEQ;
        u32work = SQ_SEQ_DATA;
        memcpy (sWrPkgConfigReq.u8Data, &u32work, sizeof(UINT32));
        if (0 != PECI_WrPkgConfig (&sWrPkgConfigReq, &sWrPkgConfigRes))
        {
            sWrPkgConfigRes.u8CompletionCode = 0x00;
        }
        return ST_HW_FAILURE;
    }

    // Start the SQ dump and get the number of dwords to read
    PRINT(PRINT_DBG, PRINT_INFO, "[DBG] - CPU #%d Core %u Type 0x%x Start the SQ dump and get the number of dwords to read\n", u8Cpu, u32CoreNum, u32SqType);
    memset(&sRdPkgConfigReq, 0 , sizeof(SRdPkgConfigReq));
    memset(&sRdPkgConfigRes, 0 , sizeof(SRdPkgConfigRes));
    sRdPkgConfigReq.sHeader.u8ClientAddr = PECI_BASE_ADDR + u8Cpu;
    sRdPkgConfigReq.sHeader.u8WriteLength = 0x05;
    sRdPkgConfigReq.sHeader.u8ReadLength = sizeof(UINT32) + 1;
    sRdPkgConfigReq.u8CmdCode = 0xA1;
    sRdPkgConfigReq.u8HostID_Retry = 0x02;
    sRdPkgConfigReq.u8Index = MBX_INDEX_VCU;
    sRdPkgConfigReq.u16Parameter = SQ_START_PARAM;
    if (0 == PECI_RdPkgConfig (&sRdPkgConfigReq, &sRdPkgConfigRes))
    {
        memcpy(&u32NumReads, sRdPkgConfigRes.u8Data, sizeof(UINT32));
    }
    else
    {
        sRdPkgConfigRes.u8CompletionCode = 0x00;
    }
    if (sRdPkgConfigRes.u8CompletionCode != PECI_CC_SUCCESS) {
        PRINT(PRINT_DBG, PRINT_ERROR, "[DBG] - CPU #%d Core %u Type 0x%x SQ dump Sequence Failed CC=0x%02x\n", u8Cpu, u32CoreNum, u32SqType, sRdPkgConfigRes.u8CompletionCode);
        // SQ dump sequence failed, abort the sequence and go to the next CPU
        memset(&sWrPkgConfigReq, 0 , sizeof(SWrPkgConfigReq));
        memset(&sWrPkgConfigRes, 0 , sizeof(SWrPkgConfigRes));
        sWrPkgConfigReq.sHeader.u8ClientAddr = PECI_BASE_ADDR + u8Cpu;
        sWrPkgConfigReq.sHeader.u8WriteLength = sizeof(UINT32) + 6;
        sWrPkgConfigReq.sHeader.u8ReadLength = 0x01;
        sWrPkgConfigReq.u8CmdCode = 0xA5;
        sWrPkgConfigReq.u8HostID_Retry = 0x02;
        sWrPkgConfigReq.u8Index = MBX_INDEX_VCU;
        sWrPkgConfigReq.u16Parameter = VCU_ABORT_SEQ;
        u32work = SQ_SEQ_DATA;
        memcpy (sWrPkgConfigReq.u8Data, &u32work, sizeof(UINT32));
        if (0 != PECI_WrPkgConfig (&sWrPkgConfigReq, &sWrPkgConfigRes))
        {
            sWrPkgConfigRes.u8CompletionCode = 0x00;
        }
        return ST_HW_FAILURE;
    }

    // Get the raw data
    pu32SqDump = (UINT32 *)calloc(u32NumReads, sizeof(UINT32));
    if (pu32SqDump == NULL) {
        // calloc failed, abort the sequence and go to the next CPU
        memset(&sWrPkgConfigReq, 0 , sizeof(SWrPkgConfigReq));
        memset(&sWrPkgConfigRes, 0 , sizeof(SWrPkgConfigRes));
        sWrPkgConfigReq.sHeader.u8ClientAddr = PECI_BASE_ADDR + u8Cpu;
        sWrPkgConfigReq.sHeader.u8WriteLength = sizeof(UINT32) + 6;
        sWrPkgConfigReq.sHeader.u8ReadLength = 0x01;
        sWrPkgConfigReq.u8CmdCode = 0xA5;
        sWrPkgConfigReq.u8HostID_Retry = 0x02;
        sWrPkgConfigReq.u8Index = MBX_INDEX_VCU;
        sWrPkgConfigReq.u16Parameter = VCU_ABORT_SEQ;
        u32work = SQ_SEQ_DATA;
        memcpy (sWrPkgConfigReq.u8Data, &u32work, sizeof(UINT32));
        if (0 != PECI_WrPkgConfig (&sWrPkgConfigReq, &sWrPkgConfigRes))
        {
            sWrPkgConfigRes.u8CompletionCode = 0x00;
        }
        PRINT(PRINT_DBG, PRINT_ERROR, "[DBG] - CPU #%d Core %u Type 0x%x SQ dump calloc Failed\n", u8Cpu, u32CoreNum, u32SqType);
        return ST_HW_FAILURE;
    }
    for (i = 0; i < u32NumReads; i++) {
        memset(&sRdPkgConfigReq, 0 , sizeof(SRdPkgConfigReq));
        memset(&sRdPkgConfigRes, 0 , sizeof(SRdPkgConfigRes));
        sRdPkgConfigReq.sHeader.u8ClientAddr = PECI_BASE_ADDR + u8Cpu;
        sRdPkgConfigReq.sHeader.u8WriteLength = 0x05;
        sRdPkgConfigReq.sHeader.u8ReadLength = sizeof(UINT32) + 1;
        sRdPkgConfigReq.u8CmdCode = 0xA1;
        sRdPkgConfigReq.u8HostID_Retry = 0x02;
        sRdPkgConfigReq.u8Index = MBX_INDEX_VCU;
        sRdPkgConfigReq.u16Parameter = VCU_READ;
        if (0 == PECI_RdPkgConfig (&sRdPkgConfigReq, &sRdPkgConfigRes))
        {
            memcpy(&pu32SqDump[i], sRdPkgConfigRes.u8Data, sizeof(UINT32));
        }
        else
        {
            sRdPkgConfigRes.u8CompletionCode = 0x00;
        }
        if (sRdPkgConfigRes.u8CompletionCode != PECI_CC_SUCCESS) {
            PRINT(PRINT_DBG, PRINT_ERROR, "[DBG] - CPU #%d Core %u Type 0x%x SQ dump Sequence Read #%d Failed CC=0x%02x\n", u8Cpu, u32CoreNum, u32SqType, i, sRdPkgConfigRes.u8CompletionCode);
            // SQ dump sequence failed, note the number of dwords read and abort the sequence
            u32NumReads = i;
            memset(&sWrPkgConfigReq, 0 , sizeof(SWrPkgConfigReq));
            memset(&sWrPkgConfigRes, 0 , sizeof(SWrPkgConfigRes));
            sWrPkgConfigReq.sHeader.u8ClientAddr = PECI_BASE_ADDR + u8Cpu;
            sWrPkgConfigReq.sHeader.u8WriteLength = sizeof(UINT32) + 6;
            sWrPkgConfigReq.sHeader.u8ReadLength = 0x01;
            sWrPkgConfigReq.u8CmdCode = 0xA5;
            sWrPkgConfigReq.u8HostID_Retry = 0x02;
            sWrPkgConfigReq.u8Index = MBX_INDEX_VCU;
            sWrPkgConfigReq.u16Parameter = VCU_ABORT_SEQ;
            u32work = SQ_SEQ_DATA;
            memcpy (sWrPkgConfigReq.u8Data, &u32work, sizeof(UINT32));
            if (0 != PECI_WrPkgConfig (&sWrPkgConfigReq, &sWrPkgConfigRes))
            {
                sWrPkgConfigRes.u8CompletionCode = 0x00;
            }
            break;
        }
    }

    // Close the SQ dump sequence
    PRINT(PRINT_DBG, PRINT_INFO, "[DBG] - CPU #%u Core %u Type0x%08x SQ dump Sequence Closed u32NumReads=%u\n", u8Cpu, u32CoreNum, u32SqType, u32NumReads);
    memset(&sWrPkgConfigReq, 0 , sizeof(SWrPkgConfigReq));
    memset(&sWrPkgConfigRes, 0 , sizeof(SWrPkgConfigRes));
    sWrPkgConfigReq.sHeader.u8ClientAddr = PECI_BASE_ADDR + u8Cpu;
    sWrPkgConfigReq.sHeader.u8WriteLength = sizeof(UINT32) + 6;
    sWrPkgConfigReq.sHeader.u8ReadLength = 0x01;
    sWrPkgConfigReq.u8CmdCode = 0xA5;
    sWrPkgConfigReq.u8HostID_Retry = 0x02;
    sWrPkgConfigReq.u8Index = MBX_INDEX_VCU;
    sWrPkgConfigReq.u16Parameter = VCU_CLOSE_SEQ;
    u32work = SQ_SEQ_DATA;
    memcpy (sWrPkgConfigReq.u8Data, &u32work, sizeof(UINT32));
    if (0 != PECI_WrPkgConfig (&sWrPkgConfigReq, &sWrPkgConfigRes))
    {
        sWrPkgConfigRes.u8CompletionCode = 0x00;
    }

    // Set the return data
    *ppu32SqDumpRet = pu32SqDump;
    *pu32SqDumpSize = u32NumReads;

    return ST_OK;
}

/******************************************************************************
*
*   logSqDump
*
*   This function gathers the SQ Dump and adds it to the debug log.
*   The PECI flow is listed below to generate a SQ Dump.
*
*    WrPkgConfig() -
*         0x80 0x0003 0x00030004
*         Open SQ Dump Sequence
*
*    WrPkgConfig() -
*         0x80 0x0001 0x80000000
*         Set Parameter 0
*
*    RdPkgConfig() -
*         0x80 0x3002
*         Start SQ dump sequence
*
*    RdPkgConfig() -
*         0x80 0x0002
*         Returns the number of additional RdPkgConfig() commands required to collect all the data
*         for the dump.
*
*    RdPkgConfig() * N -
*         0x80 0x0002
*         SQ Dump data [1-N]
*
*    WrPkgConfig() -
*         0x80 0x0004 0x00030004 Close SQ Dump Sequence.
*
******************************************************************************/
ESTATUS logSqDump(FILE * fpRaw, FILE * fp, cJSON * pJsonChild)
{
    UINT8 u8Cpu;
    SSqDump sSqDump;
    UINT32 i, u32NumCores;
    BOOL bSqDataLogged = FALSE;
    SRdPkgConfigReq sRdPkgConfigReq;
    SRdPkgConfigRes sRdPkgConfigRes;

#ifdef SPX_BMC_ACD
    UN_USED(fp);
#endif

    // Go through all CPUs
    for (u8Cpu = CPU0_ID; u8Cpu < MAX_CPU; u8Cpu++) {
        if (!IsCpuPresent(u8Cpu)) {
            u32NumCores= 0;
#ifdef BUILD_RAW
            fwrite(&u32NumCores, sizeof(u32NumCores), 1, fpRaw);
#endif //BUILD_RAW
            continue;
        }
        // Start the SQ dump log
        PRINT(PRINT_DBG, PRINT_INFO, "Platform Debug - SQ dump log %d\n", u8Cpu);

        // Get the maximum Thread ID of the processor
        memset(&sRdPkgConfigReq, 0 , sizeof(SRdPkgConfigReq));
        memset(&sRdPkgConfigRes, 0 , sizeof(SRdPkgConfigRes));
        sRdPkgConfigReq.sHeader.u8ClientAddr = PECI_BASE_ADDR + u8Cpu;
        sRdPkgConfigReq.sHeader.u8WriteLength = 0x05;
        sRdPkgConfigReq.sHeader.u8ReadLength = sizeof(UINT32) + 1;
        sRdPkgConfigReq.u8CmdCode = 0xA1;
        sRdPkgConfigReq.u8HostID_Retry = 0x02;
        sRdPkgConfigReq.u8Index = MBX_INDEX_CPU_ID;
        sRdPkgConfigReq.u16Parameter = PKG_ID_MAX_THREAD_ID;
        if (0 == PECI_RdPkgConfig (&sRdPkgConfigReq, &sRdPkgConfigRes))
        {
            memcpy(&u32NumCores, sRdPkgConfigRes.u8Data, sizeof(UINT32));
        }
        else
        {
            sRdPkgConfigRes.u8CompletionCode = 0x00;
        }
        if (sRdPkgConfigRes.u8CompletionCode == PECI_CC_SUCCESS) {
            // Convert max thread ID to number of cores
            u32NumCores = (u32NumCores / 2) + 1;
            PRINT(PRINT_DBG, PRINT_INFO, "[DBG] - CPU #%d has %u cores\n", u8Cpu, u32NumCores);
        } else {
            PRINT(PRINT_DBG, PRINT_ERROR, "[DBG] - Failed to get number of cores for CPU #%d\n", u8Cpu);
            u32NumCores= 0;
#ifdef BUILD_RAW
            fwrite(&u32NumCores, sizeof(u32NumCores), 1, fpRaw);
#endif //BUILD_RAW
            continue;
        }

#ifdef BUILD_RAW
        fwrite(&u32NumCores, sizeof(u32NumCores), 1, fpRaw);
#endif //BUILD_RAW

        // Go through each core in this CPU
        for (i = 0; i < u32NumCores; i++) {
            // Clear the buffer for each core
            memset_s(&sSqDump, sizeof(sSqDump), 0);

            // Get the SQ Dump Address Array data
            if (sqDump(u8Cpu, i, SQ_ADDR_ARRAY, &sSqDump.pu32SqAddrArray, &sSqDump.u32SqAddrSize) == ST_OK) {
                // Set the flag indicating that we found SQ data in one of the cores, so we should return success
                bSqDataLogged = TRUE;
                PRINT(PRINT_DBG, PRINT_INFO, "[DBG] - CPU #%d Core %u SQ Address data found\n", u8Cpu, i);
            }
            // Get the SQ Dump Control Array data
            if (sqDump(u8Cpu, i, SQ_CTRL_ARRAY, &sSqDump.pu32SqCtrlArray, &sSqDump.u32SqCtrlSize) == ST_OK) {
                // Set the flag indicating that we found SQ data in one of the cores, so we should return success
                bSqDataLogged = TRUE;
                PRINT(PRINT_DBG, PRINT_INFO, "[DBG] - CPU #%d Core %u SQ Control data found\n", u8Cpu, i);
            }

            // Log the SQ dump for this Core
#ifdef BUILD_RAW
            if (fpRaw != NULL) {
                sqDumpRaw(u8Cpu, i, &sSqDump, fpRaw);
            }
#endif //BUILD_RAW
#ifdef BUILD_JSON
            if (pJsonChild != NULL) {
                sqDumpJson(u8Cpu, i, &sSqDump, pJsonChild);
            }
#endif //BUILD_JSON

            // Free any allocated memory
            if (sSqDump.pu32SqAddrArray) {
                free(sSqDump.pu32SqAddrArray);
            }
            if (sSqDump.pu32SqCtrlArray) {
                free(sSqDump.pu32SqCtrlArray);
            }
        }
    }

    // Cores that have no SQ data will return a PECI failure, so we have to scan all the cores.
    // If any core has data, we should return success.  If no cores have data, return failure.
    if (bSqDataLogged) {
        return ST_OK;
    } else {
        PRINT(PRINT_DBG, PRINT_ERROR, "[DBG] - No SQ data found\n");
        return ST_HW_FAILURE;
    }
}
