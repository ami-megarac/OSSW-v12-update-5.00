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
#include "CoreMca.h"

extern void GetClockTime(char* buffer, int iSize);

/******************************************************************************
*
*   coreMcaTxt
*
*   This function formats the Core MCA log into the provided text file
*
******************************************************************************/
#ifdef BUILD_TXT
static void coreMcaTxt(UINT8 u8Cpu, UINT32 u32CoreNum, SCoreMcaRawData * sCoreMcaRawData, FILE * fp)
{
    char buf[64];
    UINT32 i;

    // get the system clock
    GetClockTime(buf, 63);
    fprintf(fp, "CPU #%d Core #%d MSR Log: %s\n", u8Cpu, u32CoreNum, buf);

    // Format the Core MCA data out to the .txt debug file
    for (i = FIRST_CORE_MCA; i <= LAST_CORE_MCA; i++) {
        // Note an error for this MCE Bank if it's not valid
        if (sCoreMcaRawData[i].bInvalid) {
            fprintf(fp, CORE_MCA_REG_NAME ": Error reading register.\n", i, coreMcaRegNames[CORE_CTL]);
            fprintf(fp, CORE_MCA_REG_NAME ": Error reading register.\n", i, coreMcaRegNames[CORE_STATUS]);
            fprintf(fp, CORE_MCA_REG_NAME ": Error reading register.\n", i, coreMcaRegNames[CORE_ADDR]);
            fprintf(fp, CORE_MCA_REG_NAME ": Error reading register.\n", i, coreMcaRegNames[CORE_MISC]);
            fprintf(fp, CORE_MCA_REG_NAME ": Error reading register.\n", i, coreMcaRegNames[CORE_CTL2]);
        // Otherwise print the register data
        } else {
            fprintf(fp, CORE_MCA_REG_NAME ": %08X%08X\n", i, coreMcaRegNames[CORE_CTL], sCoreMcaRawData[i].uRegData.u32Raw[1], sCoreMcaRawData[i].uRegData.u32Raw[0]);
            fprintf(fp, CORE_MCA_REG_NAME ": %08X%08X\n", i, coreMcaRegNames[CORE_STATUS], sCoreMcaRawData[i].uRegData.u32Raw[3], sCoreMcaRawData[i].uRegData.u32Raw[2]);
            fprintf(fp, CORE_MCA_REG_NAME ": %08X%08X\n", i, coreMcaRegNames[CORE_ADDR], sCoreMcaRawData[i].uRegData.u32Raw[5], sCoreMcaRawData[i].uRegData.u32Raw[4]);
            fprintf(fp, CORE_MCA_REG_NAME ": %08X%08X\n", i, coreMcaRegNames[CORE_MISC], sCoreMcaRawData[i].uRegData.u32Raw[7], sCoreMcaRawData[i].uRegData.u32Raw[6]);
            fprintf(fp, CORE_MCA_REG_NAME ": %08X%08X\n", i, coreMcaRegNames[CORE_CTL2], sCoreMcaRawData[i].uRegData.u32Raw[9], sCoreMcaRawData[i].uRegData.u32Raw[8]);
        }
        fprintf(fp, "\n");
    }
}
#endif //BUILD_TXT

#ifdef BUILD_RAW
static void coreMcaRaw(UINT8 u8Cpu, UINT32 u32CoreNum, SCoreMcaRawData * sCoreMcaRawData, FILE * fpRaw)
{
#ifdef SPX_BMC_ACD
    UN_USED(u8Cpu);
    UN_USED(u32CoreNum);
#endif
    fwrite(sCoreMcaRawData, (sizeof(SCoreMcaRawData) * (LAST_CORE_MCA + 1)), 1, fpRaw);
}
#endif //BUILD_RAW

/******************************************************************************
*
*   coreMcaJson
*
*   This function formats the Core MCA into a JSON object
*
******************************************************************************/
#ifdef BUILD_JSON
static void coreMcaJson(UINT8 u8Cpu, UINT32 u32CoreNum, SCoreMcaRawData * sCoreMcaRawData, cJSON * pJsonChild)
{
    cJSON * socket;
    cJSON * core;
    cJSON * coreMca;
    char jsonItemString[CORE_MCA_JSON_STRING_LEN];
    UINT32 i;

    // Add the socket number item to the Core MCA JSON structure only if it doesn't already exist
    snprintf(jsonItemString, CORE_MCA_JSON_STRING_LEN, CORE_MCA_JSON_SOCKET_NAME, u8Cpu);
    if ((socket = cJSON_GetObjectItemCaseSensitive(pJsonChild, jsonItemString)) == NULL) {
        cJSON_AddItemToObject(pJsonChild, jsonItemString, socket = cJSON_CreateObject());
    }

    // Add the core number item to the Core MCA JSON structure
    snprintf(jsonItemString, CORE_MCA_JSON_STRING_LEN, CORE_MCA_JSON_CORE_NAME, u32CoreNum);
    cJSON_AddItemToObject(socket, jsonItemString, core = cJSON_CreateObject());

    // Format the Core MCA data out to the .json debug file
    for (i = FIRST_CORE_MCA; i <= LAST_CORE_MCA; i++) {
        // Add the MCA number item to the Core MCA JSON structure
        snprintf(jsonItemString, CORE_MCA_JSON_STRING_LEN, CORE_MCA_JSON_MCA_NAME, i);
        cJSON_AddItemToObject(core, jsonItemString, coreMca = cJSON_CreateObject());

        // Fill in NULL for this MCE Bank if it's not valid
        if (sCoreMcaRawData[i].bInvalid) {
            snprintf(jsonItemString, CORE_MCA_JSON_STRING_LEN, "%s0x%02x", CORE_MCA_FAILED, sCoreMcaRawData[i].uRegData.u32Raw[0]);
            cJSON_AddStringToObject(coreMca, coreMcaRegNames[CORE_CTL], jsonItemString);
            cJSON_AddStringToObject(coreMca, coreMcaRegNames[CORE_STATUS], jsonItemString);
            cJSON_AddStringToObject(coreMca, coreMcaRegNames[CORE_ADDR], jsonItemString);
            cJSON_AddStringToObject(coreMca, coreMcaRegNames[CORE_MISC], jsonItemString);
            cJSON_AddStringToObject(coreMca, coreMcaRegNames[CORE_CTL2], jsonItemString);
        // Otherwise fill in the register data
        } else {
            snprintf(jsonItemString, CORE_MCA_JSON_STRING_LEN, "0x%08x%08x", sCoreMcaRawData[i].uRegData.u32Raw[1], sCoreMcaRawData[i].uRegData.u32Raw[0]);
            cJSON_AddStringToObject(coreMca, coreMcaRegNames[CORE_CTL], jsonItemString);
            snprintf(jsonItemString, CORE_MCA_JSON_STRING_LEN, "0x%08x%08x", sCoreMcaRawData[i].uRegData.u32Raw[3], sCoreMcaRawData[i].uRegData.u32Raw[2]);
            cJSON_AddStringToObject(coreMca, coreMcaRegNames[CORE_STATUS], jsonItemString);
            snprintf(jsonItemString, CORE_MCA_JSON_STRING_LEN, "0x%08x%08x", sCoreMcaRawData[i].uRegData.u32Raw[5], sCoreMcaRawData[i].uRegData.u32Raw[4]);
            cJSON_AddStringToObject(coreMca, coreMcaRegNames[CORE_ADDR], jsonItemString);
            snprintf(jsonItemString, CORE_MCA_JSON_STRING_LEN, "0x%08x%08x", sCoreMcaRawData[i].uRegData.u32Raw[7], sCoreMcaRawData[i].uRegData.u32Raw[6]);
            cJSON_AddStringToObject(coreMca, coreMcaRegNames[CORE_MISC], jsonItemString);
            snprintf(jsonItemString, CORE_MCA_JSON_STRING_LEN, "0x%08x%08x", sCoreMcaRawData[i].uRegData.u32Raw[9], sCoreMcaRawData[i].uRegData.u32Raw[8]);
            cJSON_AddStringToObject(coreMca, coreMcaRegNames[CORE_CTL2], jsonItemString);
        }
    }
}
#endif //BUILD_JSON

/******************************************************************************
*
*   logCoreMca
*
*   This function gathers the Core MCA register contents and adds them to the
*   debug log.  The PECI flow is listed below to dump an MCE bank
*
*    WrPkgConfig() -
*         0x80 0x0003 0x00010000
*         Open Machine Check Dump Sequence
*
*    WrPkgConfig() -
*         0x80 0x1000 [15:8] Core Number, [7:0] Bank Number
*         Select the MCA bank
*
*    RdPkgConfig() -
*         0x80 0x0002
*         CTL[31:0]
*
*    RdPkgConfig() -
*         0x80 0x0002
*         CTL[63:32]
*
*    RdPkgConfig() -
*         0x80 0x0002
*         STATUS[31:0]
*
*    RdPkgConfig() -
*         0x80 0x0002
*         STATUS[63:32]
*
*    RdPkgConfig() -
*         0x80 0x0002
*         ADDR[31:0]
*
*    RdPkgConfig() -
*         0x80 0x0002
*         ADDR[63:32]
*
*    RdPkgConfig() -
*         0x80 0x0002
*         MISC[31:0]
*
*    RdPkgConfig() -
*         0x80 0x0002
*         MISC[63:32]
*
*    RdPkgConfig() -
*         0x80 0x0002
*         CTL2[31:0]
*
*    RdPkgConfig() -
*         0x80 0x0002
*         CTL2[63:32]
*
*    WrPkgConfig() -
*         0x80 0x0004 0x00010000 Close Machine Check Dump Sequence.
*
******************************************************************************/
ESTATUS logCoreMca(FILE * fpRaw, FILE * fp, cJSON * pJsonChild)
{
    EPECIStatus ePECIStatus;
    UINT8 u8Cpu, u8Dword;
    UINT32 j;
    UINT32 u32work;
    ESTATUS eStatus = ST_OK;
    UINT32 u32MaxThreadID = 0;
    UINT32 u32CoreNum;
    SCoreMcaRawData sCoreMcaRawData[LAST_CORE_MCA + 1];
    SRdPkgConfigReq sRdPkgConfigReq;
    SRdPkgConfigRes sRdPkgConfigRes;
    SWrPkgConfigReq sWrPkgConfigReq;
    SWrPkgConfigRes sWrPkgConfigRes;

#ifdef SPX_BMC_ACD
    UN_USED(fp);
#endif

    for (u8Cpu = CPU0_ID; u8Cpu < MAX_CPU; u8Cpu++) {
        if (FALSE == IsCpuPresent(u8Cpu)) {
            continue;
        }

        PRINT(PRINT_DBG, PRINT_INFO, "Platform Debug - Core MCA log %d\n", u8Cpu);

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
            memcpy(&u32MaxThreadID, sRdPkgConfigRes.u8Data, sizeof(UINT32));
        }
        else
        {
            sRdPkgConfigRes.u8CompletionCode = 0x00;
        }
        ePECIStatus = sRdPkgConfigRes.u8CompletionCode;
        if (PECI_CC_SUCCESS == ePECIStatus) {
#ifndef SPX_BMC_ACD
            PRINT(PRINT_DBG, PRINT_INFO, "[DBG] - CPU #%d has %ld threads\n", u8Cpu, u32MaxThreadID);
#else
            PRINT(PRINT_DBG, PRINT_INFO, "[DBG] - CPU #%d has %d threads\n", u8Cpu, u32MaxThreadID);
#endif
#ifdef BUILD_RAW
			fwrite(&u32MaxThreadID, sizeof(u32MaxThreadID), 1, fpRaw);
#endif //BUILD_RAW
        } else {
            PRINT(PRINT_DBG, PRINT_ERROR, "[DBG] - Failed to get number of threads for CPU #%d\n", u8Cpu);
            continue;
        }

        // Only read from thread 0 per core starting from core #0
        for (u32CoreNum = 0; u32CoreNum <= u32MaxThreadID / 2; u32CoreNum++) {
#ifndef SPX_BMC_ACD
            PRINT(PRINT_DBG, PRINT_INFO, "Platform Debug - Core MCA log %d Core %ld\n", u8Cpu, u32CoreNum);
#else
            PRINT(PRINT_DBG, PRINT_INFO, "Platform Debug - Core MCA log %d Core %d\n", u8Cpu, u32CoreNum);
#endif

            // Clear the buffer for each Core
            memset_s(sCoreMcaRawData, sizeof(sCoreMcaRawData), 0);

            // Read the Core MCA registers from the CPU
            for (j = FIRST_CORE_MCA; j <= LAST_CORE_MCA; j++) {
                // Open the MCA Bank dump sequence
#ifndef SPX_BMC_ACD
                PRINT(PRINT_DBG, PRINT_INFO, "[DBG] - CPU #%d Core %ld MCE Bank %ld Sequence Opened\n", u8Cpu, u32CoreNum, j);
#else
                PRINT(PRINT_DBG, PRINT_INFO, "[DBG] - CPU #%d Core %d MCE Bank %d Sequence Opened\n", u8Cpu, u32CoreNum, j);
#endif
                memset(&sWrPkgConfigReq, 0 , sizeof(SWrPkgConfigReq));
                memset(&sWrPkgConfigRes, 0 , sizeof(SWrPkgConfigRes));
                sWrPkgConfigReq.sHeader.u8ClientAddr = PECI_BASE_ADDR + u8Cpu;
                sWrPkgConfigReq.sHeader.u8WriteLength = sizeof(UINT32) + 6;
                sWrPkgConfigReq.sHeader.u8ReadLength = 0x01;
                sWrPkgConfigReq.u8CmdCode = 0xA5;
                sWrPkgConfigReq.u8HostID_Retry = 0x02;
                sWrPkgConfigReq.u8Index = MBX_INDEX_VCU;
                sWrPkgConfigReq.u16Parameter = VCU_OPEN_SEQ;
                u32work = CM_SEQ_DATA;
                memcpy (sWrPkgConfigReq.u8Data, &u32work, sizeof(UINT32));
                if (0 != PECI_WrPkgConfig (&sWrPkgConfigReq, &sWrPkgConfigRes))
                {
                    sWrPkgConfigRes.u8CompletionCode = 0x00;
                }
                ePECIStatus = sWrPkgConfigRes.u8CompletionCode;
                if (ePECIStatus != PECI_CC_SUCCESS) {
                    // MCE Bank sequence failed, abort the sequence and go to the next Bank
                    memset(&sWrPkgConfigReq, 0 , sizeof(SWrPkgConfigReq));
                    memset(&sWrPkgConfigRes, 0 , sizeof(SWrPkgConfigRes));
                    sWrPkgConfigReq.sHeader.u8ClientAddr = PECI_BASE_ADDR + u8Cpu;
                    sWrPkgConfigReq.sHeader.u8WriteLength = sizeof(UINT32) + 6;
                    sWrPkgConfigReq.sHeader.u8ReadLength = 0x01;
                    sWrPkgConfigReq.u8CmdCode = 0xA5;
                    sWrPkgConfigReq.u8HostID_Retry = 0x02;
                    sWrPkgConfigReq.u8Index = MBX_INDEX_VCU;
                    sWrPkgConfigReq.u16Parameter = VCU_ABORT_SEQ;
                    u32work = CM_SEQ_DATA;
                    memcpy (sWrPkgConfigReq.u8Data, &u32work, sizeof(UINT32));
                    if (0 != PECI_WrPkgConfig (&sWrPkgConfigReq, &sWrPkgConfigRes))
                    {
                        sWrPkgConfigRes.u8CompletionCode = 0x00;
                    }
#ifndef SPX_BMC_ACD
                    PRINT(PRINT_DBG, PRINT_ERROR, "[DBG] - CPU #%d Core %ld MCE Bank %ld Sequence Failed\n", u8Cpu, u32CoreNum, j);
#else
                    PRINT(PRINT_DBG, PRINT_ERROR, "[DBG] - CPU #%d Core %d MCE Bank %d Sequence Failed\n", u8Cpu, u32CoreNum, j);
#endif
                    eStatus = ST_HW_FAILURE;
                    continue;
                }

                // Set MCE Bank number
#ifndef SPX_BMC_ACD
                PRINT(PRINT_DBG, PRINT_INFO, "[DBG] - CPU #%d Core %ld Set MCE Bank %ld\n", u8Cpu, u32CoreNum, j);
#else
                PRINT(PRINT_DBG, PRINT_INFO, "[DBG] - CPU #%d Core %d Set MCE Bank %d\n", u8Cpu, u32CoreNum, j);
#endif
                memset(&sWrPkgConfigReq, 0 , sizeof(SWrPkgConfigReq));
                memset(&sWrPkgConfigRes, 0 , sizeof(SWrPkgConfigRes));
                sWrPkgConfigReq.sHeader.u8ClientAddr = PECI_BASE_ADDR + u8Cpu;
                sWrPkgConfigReq.sHeader.u8WriteLength = sizeof(UINT32) + 6;
                sWrPkgConfigReq.sHeader.u8ReadLength = 0x01;
                sWrPkgConfigReq.u8CmdCode = 0xA5;
                sWrPkgConfigReq.u8HostID_Retry = 0x02;
                sWrPkgConfigReq.u8Index = MBX_INDEX_VCU;
                sWrPkgConfigReq.u16Parameter = CM_BANK_PARAM;
                u32work = (u32CoreNum << 8) | j;
                memcpy (sWrPkgConfigReq.u8Data, &u32work, sizeof(UINT32));
                if (0 != PECI_WrPkgConfig (&sWrPkgConfigReq, &sWrPkgConfigRes))
                {
                    sWrPkgConfigRes.u8CompletionCode = 0x00;
                }
                ePECIStatus = sWrPkgConfigRes.u8CompletionCode;
                if (ePECIStatus != PECI_CC_SUCCESS) {
                    // MCE Bank sequence failed, abort the sequence and go to the next Bank
                    memset(&sWrPkgConfigReq, 0 , sizeof(SWrPkgConfigReq));
                    memset(&sWrPkgConfigRes, 0 , sizeof(SWrPkgConfigRes));
                    sWrPkgConfigReq.sHeader.u8ClientAddr = PECI_BASE_ADDR + u8Cpu;
                    sWrPkgConfigReq.sHeader.u8WriteLength = sizeof(UINT32) + 6;
                    sWrPkgConfigReq.sHeader.u8ReadLength = 0x01;
                    sWrPkgConfigReq.u8CmdCode = 0xA5;
                    sWrPkgConfigReq.u8HostID_Retry = 0x02;
                    sWrPkgConfigReq.u8Index = MBX_INDEX_VCU;
                    sWrPkgConfigReq.u16Parameter = VCU_ABORT_SEQ;
                    u32work = CM_SEQ_DATA;
                    memcpy (sWrPkgConfigReq.u8Data, &u32work, sizeof(UINT32));
                    if (0 != PECI_WrPkgConfig (&sWrPkgConfigReq, &sWrPkgConfigRes))
                    {
                        sWrPkgConfigRes.u8CompletionCode = 0x00;
                    }
#ifndef SPX_BMC_ACD
                    PRINT(PRINT_DBG, PRINT_ERROR, "[DBG] - CPU #%d Core %ld MCE Bank %ld Sequence Failed\n", u8Cpu, u32CoreNum, j);
#else
                    PRINT(PRINT_DBG, PRINT_ERROR, "[DBG] - CPU #%d Core %d MCE Bank %d Sequence Failed\n", u8Cpu, u32CoreNum, j);
#endif
                    eStatus = ST_HW_FAILURE;
                    continue;
                }

                // Get the MCE Bank Registers
                for (u8Dword = 0; u8Dword < CM_NUM_MCA_DWORDS; u8Dword++) {
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
                        memcpy(&sCoreMcaRawData[j].uRegData.u32Raw[u8Dword], sRdPkgConfigRes.u8Data, sizeof(UINT32));
                    }
                    else
                    {
                        sRdPkgConfigRes.u8CompletionCode = 0x00;
                    }
                    ePECIStatus = sRdPkgConfigRes.u8CompletionCode;
                    if (ePECIStatus != PECI_CC_SUCCESS) {
                        // MCE Bank sequence failed, abort the sequence and go to the next Bank
                        memset(&sWrPkgConfigReq, 0 , sizeof(SWrPkgConfigReq));
                        memset(&sWrPkgConfigRes, 0 , sizeof(SWrPkgConfigRes));
                        sWrPkgConfigReq.sHeader.u8ClientAddr = PECI_BASE_ADDR + u8Cpu;
                        sWrPkgConfigReq.sHeader.u8WriteLength = sizeof(UINT32) + 6;
                        sWrPkgConfigReq.sHeader.u8ReadLength = 0x01;
                        sWrPkgConfigReq.u8CmdCode = 0xA5;
                        sWrPkgConfigReq.u8HostID_Retry = 0x02;
                        sWrPkgConfigReq.u8Index = MBX_INDEX_VCU;
                        sWrPkgConfigReq.u16Parameter = VCU_ABORT_SEQ;
                        u32work = CM_SEQ_DATA;
                        memcpy (sWrPkgConfigReq.u8Data, &u32work, sizeof(UINT32));
                        if (0 != PECI_WrPkgConfig (&sWrPkgConfigReq, &sWrPkgConfigRes))
                        {
                            sWrPkgConfigRes.u8CompletionCode = 0x00;
                        }
#ifndef SPX_BMC_ACD
                        PRINT(PRINT_DBG, PRINT_ERROR, "[DBG] - CPU #%d Core %ld MCE Bank %ld Sequence Failed\n", u8Cpu, u32CoreNum, j);
#else
                        PRINT(PRINT_DBG, PRINT_ERROR, "[DBG] - CPU #%d Core %d MCE Bank %d Sequence Failed\n", u8Cpu, u32CoreNum, j);
#endif
                        sCoreMcaRawData[j].bInvalid = TRUE;
                        eStatus = ST_HW_FAILURE;
                        break;
                    }
                }

                // Close the MCE Bank sequence
#ifndef SPX_BMC_ACD
                PRINT(PRINT_DBG, PRINT_INFO, "[DBG] - CPU #%d Core %ld MCE Bank %ld Sequence Closed\n", u8Cpu, u32CoreNum, j);
#else
                PRINT(PRINT_DBG, PRINT_INFO, "[DBG] - CPU #%d Core %d MCE Bank %d Sequence Closed\n", u8Cpu, u32CoreNum, j);
#endif
                memset(&sWrPkgConfigReq, 0 , sizeof(SWrPkgConfigReq));
                memset(&sWrPkgConfigRes, 0 , sizeof(SWrPkgConfigRes));
                sWrPkgConfigReq.sHeader.u8ClientAddr = PECI_BASE_ADDR + u8Cpu;
                sWrPkgConfigReq.sHeader.u8WriteLength = sizeof(UINT32) + 6;
                sWrPkgConfigReq.sHeader.u8ReadLength = 0x01;
                sWrPkgConfigReq.u8CmdCode = 0xA5;
                sWrPkgConfigReq.u8HostID_Retry = 0x02;
                sWrPkgConfigReq.u8Index = MBX_INDEX_VCU;
                sWrPkgConfigReq.u16Parameter = VCU_CLOSE_SEQ;
                u32work = CM_SEQ_DATA;
                memcpy (sWrPkgConfigReq.u8Data, &u32work, sizeof(UINT32));
                if (0 != PECI_WrPkgConfig (&sWrPkgConfigReq, &sWrPkgConfigRes))
                {
                    sWrPkgConfigRes.u8CompletionCode = 0x00;
                }
            }

            // Log the Core MCA for this CPU
#ifdef BUILD_TXT
            if (fp != NULL) {
                coreMcaTxt(u8Cpu, u32CoreNum, sCoreMcaRawData, fp);
            }
#endif //BUILD_TXT

#ifdef BUILD_RAW
            if (fpRaw != NULL) {
                coreMcaRaw(u8Cpu, u32CoreNum, sCoreMcaRawData, fpRaw);
            }
#endif //BUILD_RAW

#ifdef BUILD_JSON
            if (pJsonChild != NULL) {
                coreMcaJson(u8Cpu, u32CoreNum, sCoreMcaRawData, pJsonChild);
            }
#endif //BUILD_JSON
        }
    }

    return eStatus;
}
