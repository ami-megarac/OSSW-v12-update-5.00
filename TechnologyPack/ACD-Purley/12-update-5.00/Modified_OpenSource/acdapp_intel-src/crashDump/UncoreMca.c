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
#include "UncoreMca.h"

extern void GetClockTime(char* buffer, int iSize);

/******************************************************************************
*
*   uncoreMcaTxt
*
*   This function formats the Uncore MCA log into the provided text file
*
******************************************************************************/
#ifdef BUILD_TXT
static void uncoreMcaTxt(UINT8 u8Cpu, SUncoreMcaRawData * sUncoreMcaRawData, FILE * fp)
{
    char buf[64];
    UINT32 i;

    // get the system clock
    GetClockTime(buf, 63);
    fprintf(fp, "CPU #%d Uncore MSR Log: %s\n", u8Cpu, buf);

    // Format the Uncore MCA data out to the .txt debug file
    for (i = FIRST_UNCORE_MCA; i <= LAST_UNCORE_MCA; i++) {
        // Note an error for this MCE Bank if it's not valid
        if (sUncoreMcaRawData[i].bInvalid) {
            fprintf(fp, UNCORE_MCA_REG_NAME ": Error reading register.\n", i, uncoreMcaRegNames[UNCORE_CTL]);
            fprintf(fp, UNCORE_MCA_REG_NAME ": Error reading register.\n", i, uncoreMcaRegNames[UNCORE_STATUS]);
            fprintf(fp, UNCORE_MCA_REG_NAME ": Error reading register.\n", i, uncoreMcaRegNames[UNCORE_ADDR]);
            fprintf(fp, UNCORE_MCA_REG_NAME ": Error reading register.\n", i, uncoreMcaRegNames[UNCORE_MISC]);
            fprintf(fp, UNCORE_MCA_REG_NAME ": Error reading register.\n", i, uncoreMcaRegNames[UNCORE_CTL2]);
        // Otherwise print the register data
        } else {
            fprintf(fp, UNCORE_MCA_REG_NAME ": %08X%08X\n", i, uncoreMcaRegNames[UNCORE_CTL], sUncoreMcaRawData[i].uRegData.u32Raw[1], sUncoreMcaRawData[i].uRegData.u32Raw[0]);
            fprintf(fp, UNCORE_MCA_REG_NAME ": %08X%08X\n", i, uncoreMcaRegNames[UNCORE_STATUS], sUncoreMcaRawData[i].uRegData.u32Raw[3], sUncoreMcaRawData[i].uRegData.u32Raw[2]);
            fprintf(fp, UNCORE_MCA_REG_NAME ": %08X%08X\n", i, uncoreMcaRegNames[UNCORE_ADDR], sUncoreMcaRawData[i].uRegData.u32Raw[5], sUncoreMcaRawData[i].uRegData.u32Raw[4]);
            fprintf(fp, UNCORE_MCA_REG_NAME ": %08X%08X\n", i, uncoreMcaRegNames[UNCORE_MISC], sUncoreMcaRawData[i].uRegData.u32Raw[7], sUncoreMcaRawData[i].uRegData.u32Raw[6]);
            fprintf(fp, UNCORE_MCA_REG_NAME ": %08X%08X\n", i, uncoreMcaRegNames[UNCORE_CTL2], sUncoreMcaRawData[i].uRegData.u32Raw[9], sUncoreMcaRawData[i].uRegData.u32Raw[8]);
        }
        fprintf(fp, "\n");
    }
}
#endif //BUILD_TXT


#ifdef BUILD_RAW
static void uncoreMcaRaw(UINT8 u8Cpu, SUncoreMcaRawData * sUncoreMcaRawData, FILE * fpRaw)
{
#ifdef SPX_BMC_ACD
    UN_USED(u8Cpu);
#endif
    fwrite(sUncoreMcaRawData, (sizeof(SUncoreMcaRawData) * (LAST_UNCORE_MCA + 1)), 1, fpRaw);
}
#endif //BUILD_RAW

/******************************************************************************
*
*   uncoreMcaJson
*
*   This function formats the Uncore MCA into a JSON object
*
******************************************************************************/
#ifdef BUILD_JSON
static void uncoreMcaJson(UINT8 u8Cpu, SUncoreMcaRawData * sUncoreMcaRawData, cJSON * pJsonChild)
{
    cJSON * socket;
    cJSON * uncoreMca;
    char jsonItemString[UNCORE_MCA_JSON_STRING_LEN];
    UINT32 i;

    // Add the socket number item to the Uncore MCA JSON structure
    snprintf(jsonItemString, UNCORE_MCA_JSON_STRING_LEN, UNCORE_MCA_JSON_SOCKET_NAME, u8Cpu);
    cJSON_AddItemToObject(pJsonChild, jsonItemString, socket = cJSON_CreateObject());

    // Format the Uncore MCA data out to the .json debug file
    for (i = FIRST_UNCORE_MCA; i <= LAST_UNCORE_MCA; i++) {
        // Add the MCA number item to the Uncore MCA JSON structure
        snprintf(jsonItemString, UNCORE_MCA_JSON_STRING_LEN, UNCORE_MCA_JSON_MCA_NAME, i);
        cJSON_AddItemToObject(socket, jsonItemString, uncoreMca = cJSON_CreateObject());

        // Fill in NULL for this MCE Bank if it's not valid
        if (sUncoreMcaRawData[i].bInvalid) {
            snprintf(jsonItemString, UNCORE_MCA_JSON_STRING_LEN, "%s0x%02x", UNCORE_MCA_FAILED, sUncoreMcaRawData[i].uRegData.u32Raw[0]);
            cJSON_AddStringToObject(uncoreMca, uncoreMcaRegNames[UNCORE_CTL], jsonItemString);
            cJSON_AddStringToObject(uncoreMca, uncoreMcaRegNames[UNCORE_STATUS], jsonItemString);
            cJSON_AddStringToObject(uncoreMca, uncoreMcaRegNames[UNCORE_ADDR], jsonItemString);
            cJSON_AddStringToObject(uncoreMca, uncoreMcaRegNames[UNCORE_MISC], jsonItemString);
            cJSON_AddStringToObject(uncoreMca, uncoreMcaRegNames[UNCORE_CTL2], jsonItemString);
        // Otherwise fill in the register data
        } else {
            snprintf(jsonItemString, UNCORE_MCA_JSON_STRING_LEN, "0x%08x%08x", sUncoreMcaRawData[i].uRegData.u32Raw[1], sUncoreMcaRawData[i].uRegData.u32Raw[0]);
            cJSON_AddStringToObject(uncoreMca, uncoreMcaRegNames[UNCORE_CTL], jsonItemString);
            snprintf(jsonItemString, UNCORE_MCA_JSON_STRING_LEN, "0x%08x%08x", sUncoreMcaRawData[i].uRegData.u32Raw[3], sUncoreMcaRawData[i].uRegData.u32Raw[2]);
            cJSON_AddStringToObject(uncoreMca, uncoreMcaRegNames[UNCORE_STATUS], jsonItemString);
            snprintf(jsonItemString, UNCORE_MCA_JSON_STRING_LEN, "0x%08x%08x", sUncoreMcaRawData[i].uRegData.u32Raw[5], sUncoreMcaRawData[i].uRegData.u32Raw[4]);
            cJSON_AddStringToObject(uncoreMca, uncoreMcaRegNames[UNCORE_ADDR], jsonItemString);
            snprintf(jsonItemString, UNCORE_MCA_JSON_STRING_LEN, "0x%08x%08x", sUncoreMcaRawData[i].uRegData.u32Raw[7], sUncoreMcaRawData[i].uRegData.u32Raw[6]);
            cJSON_AddStringToObject(uncoreMca, uncoreMcaRegNames[UNCORE_MISC], jsonItemString);
            snprintf(jsonItemString, UNCORE_MCA_JSON_STRING_LEN, "0x%08x%08x", sUncoreMcaRawData[i].uRegData.u32Raw[9], sUncoreMcaRawData[i].uRegData.u32Raw[8]);
            cJSON_AddStringToObject(uncoreMca, uncoreMcaRegNames[UNCORE_CTL2], jsonItemString);
        }
    }
}
#endif //BUILD_JSON

/******************************************************************************
*
*   logUncoreMca
*
*   This function gathers the Uncore MCA register contents and adds them to the
*   debug log. The PECI flow is listed below to dump an MCE bank
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
ESTATUS logUncoreMca(FILE * fpRaw, FILE * fp, cJSON * pJsonChild)
{
    EPECIStatus ePECIStatus;
    UINT8 u8Cpu, u8Dword;
    UINT32 j;
    ESTATUS eStatus = ST_OK;
    SUncoreMcaRawData sUncoreMcaRawData[LAST_UNCORE_MCA + 1];
    UINT32 u32work;
    SRdPkgConfigReq sRdPkgConfigReq;
    SRdPkgConfigRes sRdPkgConfigRes;
    SWrPkgConfigReq sWrPkgConfigReq;
    SWrPkgConfigRes sWrPkgConfigRes;

#ifdef SPX_BMC_ACD
    UN_USED(fp);
#endif

    for (u8Cpu = CPU0_ID; u8Cpu < MAX_CPU; u8Cpu++) {
        // Clear the buffer for each CPU
        memset_s(sUncoreMcaRawData, sizeof(sUncoreMcaRawData), 0);

        if (FALSE == IsCpuPresent(u8Cpu)) {
            continue;
        }

        PRINT(PRINT_DBG, PRINT_INFO, "Platform Debug - Uncore MCA log %d\n", u8Cpu);

        // Read the Uncore MCA registers from the CPU
        for (j = FIRST_UNCORE_MCA; j <= LAST_UNCORE_MCA; j++) {
            // Open the MCA Bank dump sequence
#ifndef SPX_BMC_ACD
            PRINT(PRINT_DBG, PRINT_INFO, "[DBG] - CPU #%d MCE Bank %ld Sequence Opened\n", u8Cpu, j);
#else
            PRINT(PRINT_DBG, PRINT_INFO, "[DBG] - CPU #%d MCE Bank %d Sequence Opened\n", u8Cpu, j);
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
            u32work = UCM_SEQ_DATA;
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
                u32work = UCM_SEQ_DATA;
                memcpy (sWrPkgConfigReq.u8Data, &u32work, sizeof(UINT32));
                if (0 != PECI_WrPkgConfig (&sWrPkgConfigReq, &sWrPkgConfigRes))
                {
                    sWrPkgConfigRes.u8CompletionCode = 0x00;
                }
#ifndef SPX_BMC_ACD
                PRINT(PRINT_DBG, PRINT_ERROR, "[DBG] - CPU #%d MCE Bank %ld Sequence Failed\n", u8Cpu, j);
#else
                PRINT(PRINT_DBG, PRINT_ERROR, "[DBG] - CPU #%d MCE Bank %d Sequence Failed\n", u8Cpu, j);
#endif
                eStatus = ST_HW_FAILURE;
                continue;
            }

            // Set MCE Bank number
#ifndef SPX_BMC_ACD
            PRINT(PRINT_DBG, PRINT_INFO, "[DBG] - CPU #%d Set MCE Bank %ld\n", u8Cpu, j);
#else
            PRINT(PRINT_DBG, PRINT_INFO, "[DBG] - CPU #%d Set MCE Bank %d\n", u8Cpu, j);
#endif
            memset(&sWrPkgConfigReq, 0 , sizeof(SWrPkgConfigReq));
            memset(&sWrPkgConfigRes, 0 , sizeof(SWrPkgConfigRes));
            sWrPkgConfigReq.sHeader.u8ClientAddr = PECI_BASE_ADDR + u8Cpu;
            sWrPkgConfigReq.sHeader.u8WriteLength = sizeof(UINT32) + 6;
            sWrPkgConfigReq.sHeader.u8ReadLength = 0x01;
            sWrPkgConfigReq.u8CmdCode = 0xA5;
            sWrPkgConfigReq.u8HostID_Retry = 0x02;
            sWrPkgConfigReq.u8Index = MBX_INDEX_VCU;
            sWrPkgConfigReq.u16Parameter = UCM_BANK_PARAM;
            u32work = j;
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
                u32work = UCM_SEQ_DATA;
                memcpy (sWrPkgConfigReq.u8Data, &u32work, sizeof(UINT32));
                if (0 != PECI_WrPkgConfig (&sWrPkgConfigReq, &sWrPkgConfigRes))
                {
                    sWrPkgConfigRes.u8CompletionCode = 0x00;
                }
#ifndef SPX_BMC_ACD
                PRINT(PRINT_DBG, PRINT_ERROR, "[DBG] - CPU #%d MCE Bank %ld Sequence Failed\n", u8Cpu, j);
#else
                PRINT(PRINT_DBG, PRINT_ERROR, "[DBG] - CPU #%d MCE Bank %d Sequence Failed\n", u8Cpu, j);
#endif
                eStatus = ST_HW_FAILURE;
                continue;
            }

            // Get the MCE Bank Registers
            for (u8Dword = 0; u8Dword < UCM_NUM_MCA_DWORDS; u8Dword++) {
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
                    memcpy(&sUncoreMcaRawData[j].uRegData.u32Raw[u8Dword], sRdPkgConfigRes.u8Data, sizeof(UINT32));
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
                    u32work = UCM_SEQ_DATA;
                    memcpy (sWrPkgConfigReq.u8Data, &u32work, sizeof(UINT32));
                    if (0 != PECI_WrPkgConfig (&sWrPkgConfigReq, &sWrPkgConfigRes))
                    {
                        sWrPkgConfigRes.u8CompletionCode = 0x00;
                    }
#ifndef SPX_BMC_ACD
                    PRINT(PRINT_DBG, PRINT_ERROR, "[DBG] - CPU #%d MCE Bank %ld Sequence Failed\n", u8Cpu, j);
#else
                    PRINT(PRINT_DBG, PRINT_ERROR, "[DBG] - CPU #%d MCE Bank %d Sequence Failed\n", u8Cpu, j);
#endif
                    sUncoreMcaRawData[j].uRegData.u32Raw[0] = (UINT32)ePECIStatus;
                    sUncoreMcaRawData[j].bInvalid = TRUE;
                    eStatus = ST_HW_FAILURE;
                    break;
                }
            }

            // Close the MCE Bank sequence
#ifndef SPX_BMC_ACD
            PRINT(PRINT_DBG, PRINT_INFO, "[DBG] - CPU #%d MCE Bank %ld Sequence Closed\n", u8Cpu, j);
#else
            PRINT(PRINT_DBG, PRINT_INFO, "[DBG] - CPU #%d MCE Bank %d Sequence Closed\n", u8Cpu, j);
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
            u32work = UCM_SEQ_DATA;
            memcpy (sWrPkgConfigReq.u8Data, &u32work, sizeof(UINT32));
            if (0 != PECI_WrPkgConfig (&sWrPkgConfigReq, &sWrPkgConfigRes))
            {
                sWrPkgConfigRes.u8CompletionCode = 0x00;
            }

        }
        // Log this MCE Bank to the Uncore MCA for this CPU
#ifdef BUILD_TXT
        if (fp != NULL) {
            uncoreMcaTxt(u8Cpu, sUncoreMcaRawData, fp);
        }
#endif //BUILD_TXT

#ifdef BUILD_RAW
        if (fpRaw != NULL) {
            uncoreMcaRaw(u8Cpu, sUncoreMcaRawData, fpRaw);
        }
#endif //BUILD_RAW

#ifdef BUILD_JSON
        if (pJsonChild != NULL) {
            uncoreMcaJson(u8Cpu, sUncoreMcaRawData, pJsonChild);
        }
#endif //BUILD_JSON
    }

    return eStatus;
}
