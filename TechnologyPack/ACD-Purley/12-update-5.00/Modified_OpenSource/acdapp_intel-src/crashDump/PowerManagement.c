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
#include "PowerManagement.h"


#ifdef BUILD_RAW
static void powerManagementRaw(UINT8 u8CpuNum, UINT32 u32NumCores, SCpuPowerState * sCpuPowerStates, FILE * fpRaw)
{
#ifdef SPX_BMC_ACD
    UN_USED(u8CpuNum);
#endif
    fwrite(sCpuPowerStates, (sizeof(SCpuPowerState) * u32NumCores), 1, fpRaw);
}
#endif //BUILD_RAW

/******************************************************************************
*
*   powerManagementJson
*
*   This function formats the Power Management log into a JSON object
*
******************************************************************************/
#ifdef BUILD_JSON
static void powerManagementJson(UINT8 u8CpuNum, UINT32 u32NumCores, SCpuPowerState * sCpuPowerStates, cJSON * pJsonChild)
{
    cJSON * socket;
    cJSON * core;
    char jsonItemString[PM_JSON_STRING_LEN];
    UINT32 i;

    // Add the socket number item to the Power Management JSON structure
    snprintf(jsonItemString, PM_JSON_STRING_LEN, PM_JSON_SOCKET_NAME, u8CpuNum);
    cJSON_AddItemToObject(pJsonChild, jsonItemString, socket = cJSON_CreateObject());

    // Add the core number item to the Power Management JSON structure
    for (i = 0; i < u32NumCores; i++) {
        snprintf(jsonItemString, PM_JSON_STRING_LEN, PM_JSON_CORE_NAME, i);
        cJSON_AddItemToObject(socket, jsonItemString, core = cJSON_CreateObject());

        // Add the register data for this core to the Power Management JSON structure
        snprintf(jsonItemString, PM_JSON_STRING_LEN, "0x%x", sCpuPowerStates[i].u32CState);
        cJSON_AddStringToObject(core, PM_JSON_CSTATE_REG_NAME, jsonItemString);
        snprintf(jsonItemString, PM_JSON_STRING_LEN, "0x%x", sCpuPowerStates[i].u32VidRatio);
        cJSON_AddStringToObject(core, PM_JSON_VID_REG_NAME, jsonItemString);
    }
}
#endif //BUILD_JSON

/******************************************************************************
*
*   powerManagementReadReg
*
*   This function executes the PECI sequence to read the provided power management
*   register.
*
*   NOTE: As of now, this sequence requires DEBUG_ENABLE, so it may not work in
*   production systems.
*
******************************************************************************/
static ESTATUS powerManagementReadReg(UINT8 u8Cpu, UINT32 u32CoreNum, UINT32 u32RegParam, UINT32 * u32RegData)
{
    UINT32 u32work;
    SRdPkgConfigReq sRdPkgConfigReq;
    SRdPkgConfigRes sRdPkgConfigRes;
    SWrPkgConfigReq sWrPkgConfigReq;
    SWrPkgConfigRes sWrPkgConfigRes;

    // Open the register read sequence
#ifndef SPX_BMC_ACD
    PRINT(PRINT_DBG, PRINT_INFO, "[INFO] - CPU #%d Core %ld Reg Read Sequence Opened\n", u8Cpu, u32CoreNum);
#else
    PRINT(PRINT_DBG, PRINT_INFO, "[INFO] - CPU #%d Core %d Reg Read Sequence Opened\n", u8Cpu, u32CoreNum);
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
    u32work = PM_SEQ_DATA;
    memcpy (sWrPkgConfigReq.u8Data, &u32work, sizeof(UINT32));
    if (0 != PECI_WrPkgConfig (&sWrPkgConfigReq, &sWrPkgConfigRes))
    {
        sWrPkgConfigRes.u8CompletionCode = 0x00;
    }
    if (sWrPkgConfigRes.u8CompletionCode != PECI_CC_SUCCESS) {
        // Reg read sequence failed, abort the sequence
        memset(&sWrPkgConfigReq, 0 , sizeof(SWrPkgConfigReq));
        memset(&sWrPkgConfigRes, 0 , sizeof(SWrPkgConfigRes));
        sWrPkgConfigReq.sHeader.u8ClientAddr = PECI_BASE_ADDR + u8Cpu;
        sWrPkgConfigReq.sHeader.u8WriteLength = sizeof(UINT32) + 6;
        sWrPkgConfigReq.sHeader.u8ReadLength = 0x01;
        sWrPkgConfigReq.u8CmdCode = 0xA5;
        sWrPkgConfigReq.u8HostID_Retry = 0x02;
        sWrPkgConfigReq.u8Index = MBX_INDEX_VCU;
        sWrPkgConfigReq.u16Parameter = VCU_ABORT_SEQ;
        u32work = PM_SEQ_DATA;
        memcpy (sWrPkgConfigReq.u8Data, &u32work, sizeof(UINT32));
        if (0 != PECI_WrPkgConfig (&sWrPkgConfigReq, &sWrPkgConfigRes))
        {
            sWrPkgConfigRes.u8CompletionCode = 0x00;
        }
#ifndef SPX_BMC_ACD
        PRINT(PRINT_DBG, PRINT_ERROR, "[ERROR] - CPU #%d Core %ld Reg Read Sequence Failed\n", u8Cpu, u32CoreNum);
#else
        PRINT(PRINT_DBG, PRINT_ERROR, "[ERROR] - CPU #%d Core %d Reg Read Sequence Failed\n", u8Cpu, u32CoreNum);
#endif
        return ST_HW_FAILURE;
    }

    // Set register number
#ifndef SPX_BMC_ACD
    PRINT(PRINT_DBG, PRINT_INFO, "[INFO] - CPU #%d Core %ld Set Reg\n", u8Cpu, u32CoreNum);
#else
    PRINT(PRINT_DBG, PRINT_INFO, "[INFO] - CPU #%d Core %d Set Reg\n", u8Cpu, u32CoreNum);
#endif
    memset(&sWrPkgConfigReq, 0 , sizeof(SWrPkgConfigReq));
    memset(&sWrPkgConfigRes, 0 , sizeof(SWrPkgConfigRes));
    sWrPkgConfigReq.sHeader.u8ClientAddr = PECI_BASE_ADDR + u8Cpu;
    sWrPkgConfigReq.sHeader.u8WriteLength = sizeof(UINT32) + 6;
    sWrPkgConfigReq.sHeader.u8ReadLength = 0x01;
    sWrPkgConfigReq.u8CmdCode = 0xA5;
    sWrPkgConfigReq.u8HostID_Retry = 0x02;
    sWrPkgConfigReq.u8Index = MBX_INDEX_VCU;
    sWrPkgConfigReq.u16Parameter = VCU_SET_PARAM;
    u32work = (u32CoreNum << PM_CORE_OFFSET) | u32RegParam;
    memcpy (sWrPkgConfigReq.u8Data, &u32work, sizeof(UINT32));
    if (0 != PECI_WrPkgConfig (&sWrPkgConfigReq, &sWrPkgConfigRes))
    {
        sWrPkgConfigRes.u8CompletionCode = 0x00;
    }
    if (sWrPkgConfigRes.u8CompletionCode != PECI_CC_SUCCESS) {
        // Reg read sequence failed, abort the sequence
        memset(&sWrPkgConfigReq, 0 , sizeof(SWrPkgConfigReq));
        memset(&sWrPkgConfigRes, 0 , sizeof(SWrPkgConfigRes));
        sWrPkgConfigReq.sHeader.u8ClientAddr = PECI_BASE_ADDR + u8Cpu;
        sWrPkgConfigReq.sHeader.u8WriteLength = sizeof(UINT32) + 6;
        sWrPkgConfigReq.sHeader.u8ReadLength = 0x01;
        sWrPkgConfigReq.u8CmdCode = 0xA5;
        sWrPkgConfigReq.u8HostID_Retry = 0x02;
        sWrPkgConfigReq.u8Index = MBX_INDEX_VCU;
        sWrPkgConfigReq.u16Parameter = VCU_ABORT_SEQ;
        u32work = PM_SEQ_DATA;
        memcpy (sWrPkgConfigReq.u8Data, &u32work, sizeof(UINT32));
        if (0 != PECI_WrPkgConfig (&sWrPkgConfigReq, &sWrPkgConfigRes))
        {
            sWrPkgConfigRes.u8CompletionCode = 0x00;
        }
#ifndef SPX_BMC_ACD
        PRINT(PRINT_DBG, PRINT_ERROR, "[ERROR] - CPU #%d Core %ld Reg Read Sequence Failed\n", u8Cpu, u32CoreNum);
#else
        PRINT(PRINT_DBG, PRINT_ERROR, "[ERROR] - CPU #%d Core %d Reg Read Sequence Failed\n", u8Cpu, u32CoreNum);
#endif
        return ST_HW_FAILURE;
    }

    // Get the register data
    memset(&sRdPkgConfigReq, 0 , sizeof(SRdPkgConfigReq));
    memset(&sRdPkgConfigRes, 0 , sizeof(SRdPkgConfigRes));
    sRdPkgConfigReq.sHeader.u8ClientAddr = PECI_BASE_ADDR + u8Cpu;
    sRdPkgConfigReq.sHeader.u8WriteLength = 0x05;
    sRdPkgConfigReq.sHeader.u8ReadLength = sizeof(UINT32) + 1;
    sRdPkgConfigReq.u8CmdCode = 0xA1;
    sRdPkgConfigReq.u8HostID_Retry = 0x02;
    sRdPkgConfigReq.u8Index = MBX_INDEX_VCU;
    sRdPkgConfigReq.u16Parameter = PM_READ_PARAM;
    if (0 == PECI_RdPkgConfig (&sRdPkgConfigReq, &sRdPkgConfigRes))
    {
        memcpy(u32RegData, sRdPkgConfigRes.u8Data, sizeof(UINT32));
    }
    else
    {
        sRdPkgConfigRes.u8CompletionCode = 0x00;
    }
    if (sRdPkgConfigRes.u8CompletionCode != PECI_CC_SUCCESS) {
        // Reg read sequence failed, abort the sequence
        memset(&sWrPkgConfigReq, 0 , sizeof(SWrPkgConfigReq));
        memset(&sWrPkgConfigRes, 0 , sizeof(SWrPkgConfigRes));
        sWrPkgConfigReq.sHeader.u8ClientAddr = PECI_BASE_ADDR + u8Cpu;
        sWrPkgConfigReq.sHeader.u8WriteLength = sizeof(UINT32) + 6;
        sWrPkgConfigReq.sHeader.u8ReadLength = 0x01;
        sWrPkgConfigReq.u8CmdCode = 0xA5;
        sWrPkgConfigReq.u8HostID_Retry = 0x02;
        sWrPkgConfigReq.u8Index = MBX_INDEX_VCU;
        sWrPkgConfigReq.u16Parameter = VCU_ABORT_SEQ;
        u32work = PM_SEQ_DATA;
        memcpy (sWrPkgConfigReq.u8Data, &u32work, sizeof(UINT32));
        if (0 != PECI_WrPkgConfig (&sWrPkgConfigReq, &sWrPkgConfigRes))
        {
            sWrPkgConfigRes.u8CompletionCode = 0x00;
        }
#ifndef SPX_BMC_ACD
        PRINT(PRINT_DBG, PRINT_ERROR, "[ERROR] - CPU #%d Core %ld Reg Read Sequence Failed\n", u8Cpu, u32CoreNum);
#else
        PRINT(PRINT_DBG, PRINT_ERROR, "[ERROR] - CPU #%d Core %d Reg Read Sequence Failed\n", u8Cpu, u32CoreNum);
#endif
        return ST_HW_FAILURE;
    }

    // Close the register read sequence
#ifndef SPX_BMC_ACD
    PRINT(PRINT_DBG, PRINT_INFO, "[INFO] - CPU #%d Core %ld Reg Read Sequence Closed\n", u8Cpu, u32CoreNum);
#else
    PRINT(PRINT_DBG, PRINT_INFO, "[INFO] - CPU #%d Core %d Reg Read Sequence Closed\n", u8Cpu, u32CoreNum);
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
    u32work = PM_SEQ_DATA;
    memcpy (sWrPkgConfigReq.u8Data, &u32work, sizeof(UINT32));
    if (0 != PECI_WrPkgConfig (&sWrPkgConfigReq, &sWrPkgConfigRes))
    {
        sWrPkgConfigRes.u8CompletionCode = 0x00;
    }

    return ST_OK;
}

/******************************************************************************
*
*   logPowerManagement
*
*   This function gathers the Power Management log and adds it to the debug log
*   The PECI flow is listed below to dump the core state registers
*
*    WrPkgConfig() -
*         0x80 0x0003 0x0001002a
*         Open register read sequence
*
*    WrPkgConfig() -
*         0x80 0x1 [29:24] Core Number, [23:0] Register
*         Select the core and register
*
*    RdPkgConfig() -
*         0x80 0x1019
*         Read register data
*
*    WrPkgConfig() -
*         0x80 0x0004 0x0001002a
*         Close register read sequence.
*
******************************************************************************/
ESTATUS logPowerManagement(FILE * fpRaw, FILE * fp, cJSON * pJsonChild)
{
    ESTATUS eStatus = ST_OK;
    UINT8 u8Cpu;
    UINT32 i, u32NumCores;
    SCpuPowerState * sCpuPowerStates = NULL;
    SRdPkgConfigReq sRdPkgConfigReq;
    SRdPkgConfigRes sRdPkgConfigRes;

#ifdef SPX_BMC_ACD
    UN_USED(fp);
#endif

    for (u8Cpu = CPU0_ID; u8Cpu < MAX_CPU; u8Cpu++) {
        if (FALSE == IsCpuPresent(u8Cpu)) {
			u32NumCores = 0;
#ifdef BUILD_RAW
			fwrite(&u32NumCores, sizeof(u32NumCores), 1, fpRaw);
#endif //BUILD_RAW
            continue;
        }

        PRINT(PRINT_DBG, PRINT_INFO, "Platform Debug - Power Management %d\n", u8Cpu);

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
#ifndef SPX_BMC_ACD
            PRINT(PRINT_DBG, PRINT_INFO, "[INFO] - CPU #%d has %ld cores\n", u8Cpu, u32NumCores);
#else
            PRINT(PRINT_DBG, PRINT_INFO, "[INFO] - CPU #%d has %d cores\n", u8Cpu, u32NumCores);
#endif
        } else {
            PRINT(PRINT_DBG, PRINT_ERROR, "[ERROR] - Failed to get number of cores for CPU #%d\n", u8Cpu);
			u32NumCores = 0;
#ifdef BUILD_RAW
			fwrite(&u32NumCores, sizeof(u32NumCores), 1, fpRaw);
#endif //BUILD_RAW
            continue;
        }

        // Allocate memory for each core power state
        sCpuPowerStates = calloc(u32NumCores, sizeof(SCpuPowerState));
        if (sCpuPowerStates == NULL) {
            // calloc failed, try the next CPU
            PRINT(PRINT_DBG, PRINT_ERROR, "[DBG] - Failed to calloc memory for CPU #%d\n", u8Cpu);
            eStatus = ST_NO_RESOURCE;
			u32NumCores = 0;
#ifdef BUILD_RAW
			fwrite(&u32NumCores, sizeof(u32NumCores), 1, fpRaw);
#endif //BUILD_RAW
            continue;
        }

        // Get the power state for each core in the CPU
        for (i = 0; i < u32NumCores; i++) {
            // First get the C-State register for this core
            eStatus = powerManagementReadReg(u8Cpu, i, PM_CSTATE_PARAM, &sCpuPowerStates[i].u32CState);
            // Then get the VID Ratio register for this core
            eStatus = powerManagementReadReg(u8Cpu, i, PM_VID_PARAM, &sCpuPowerStates[i].u32VidRatio);
            // If we hit a failure, assume that register reads are not allowed and bail
            if (eStatus != ST_OK) {
                break;
            }
        }
        // If we hit a failure, assume that register reads are not allowed and try the next CPU
        if (eStatus != ST_OK) {
            free(sCpuPowerStates);
			u32NumCores = 0;
#ifdef BUILD_RAW
			fwrite(&u32NumCores, sizeof(u32NumCores), 1, fpRaw);
#endif //BUILD_RAW
            continue;
        }

#ifdef BUILD_RAW
        if (fpRaw != NULL) {
            powerManagementRaw(u8Cpu, u32NumCores, sCpuPowerStates, fpRaw);
        }
#endif //BUILD_RAW

        // Log the Power Management for this CPU
#ifdef BUILD_JSON
        if (pJsonChild != NULL) {
            powerManagementJson(u8Cpu, u32NumCores, sCpuPowerStates, pJsonChild);
        }
#endif //BUILD_JSON

        free(sCpuPowerStates);
    }

    return eStatus;
}
