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
#include "SysInfo.h"
#include "dbgout.h"

void stub_SysInfo_GetDevID(SGetDeviceIdResp *deviceId);
void stub_getMeDeviceId(SGetDeviceIdResp * devId);
void stub_getBiosId(UINT8 * biosId);

/******************************************************************************
*
*   fillCpuTypeJson
*
*   This function fills in the cpu_type JSON info
*
******************************************************************************/
void fillCpuTypeJson(SSysInfoRawData * sSysInfoRawData, char * cSectionName, cJSON * pJsonChild)
{
    cJSON * socket;
    char jsonItemName[SI_JSON_STRING_LEN];
    char jsonItemString[SI_JSON_STRING_LEN];
    UINT8 u8Cpu;
    UINT32 i;
    SCpuIdName sCpuIdNames[] =
    {
        { 0x00050655, "CLX" },
        { 0x00050654, "SKX" }
    };

    for (u8Cpu = 0; u8Cpu < sSysInfoRawData->u32NumCpus; u8Cpu++) {
        snprintf(jsonItemName, SI_JSON_STRING_LEN, SI_JSON_SOCKET_NAME, u8Cpu);
        if ((socket = cJSON_GetObjectItemCaseSensitive(pJsonChild, jsonItemName)) == NULL) {
            cJSON_AddItemToObject(pJsonChild, jsonItemName, socket = cJSON_CreateObject());
        }

        // Start with the raw CPUID
        snprintf(jsonItemString, SI_JSON_STRING_LEN, "%x", sSysInfoRawData->sCpuData[u8Cpu].cpuType);

        // the definition of CPUID:
        // 3:0 - Stepping
        // 7:4 - Model
        // 11:8 - Family
        // 13:12 - Processor Type
        // 19:16 - Extended Model
        // 27:20 - Extended Family
        // determine CPU Model name according to CPUId
        for (i = 0; i < sizeof(sCpuIdNames) / sizeof(SCpuIdName); i++) {
            if (sCpuIdNames[i].u32CpuId == sSysInfoRawData->sCpuData[u8Cpu].cpuType) {
                snprintf(jsonItemString, SI_JSON_STRING_LEN, "%s", sCpuIdNames[i].cpuModelName);
            }
        }
        cJSON_AddStringToObject(socket, cSectionName, jsonItemString);
    }
}

/******************************************************************************
*
*   fillCpuSteppingJson
*
*   This function fills in the cpu_stepping JSON info
*
******************************************************************************/
void fillCpuSteppingJson(SSysInfoRawData * sSysInfoRawData, char * cSectionName, cJSON * pJsonChild)
{
    cJSON * socket;
    char jsonItemName[SI_JSON_STRING_LEN];
    char jsonItemString[SI_JSON_STRING_LEN];
    UINT8 u8Cpu;

    for (u8Cpu = 0; u8Cpu < sSysInfoRawData->u32NumCpus; u8Cpu++) {
        snprintf(jsonItemName, SI_JSON_STRING_LEN, SI_JSON_SOCKET_NAME, u8Cpu);
        if ((socket = cJSON_GetObjectItemCaseSensitive(pJsonChild, jsonItemName)) == NULL) {
            cJSON_AddItemToObject(pJsonChild, jsonItemName, socket = cJSON_CreateObject());
        }

        snprintf(jsonItemString, SI_JSON_STRING_LEN, "%x", sSysInfoRawData->sCpuData[u8Cpu].cpuStepping);
        cJSON_AddStringToObject(socket, cSectionName, jsonItemString);
    }
}

/******************************************************************************
*
*   fillPackageIdJson
*
*   This function fills in the package_id JSON info
*
******************************************************************************/
void fillPackageIdJson(SSysInfoRawData * sSysInfoRawData, char * cSectionName, cJSON * pJsonChild)
{
    cJSON * socket;
    char jsonItemName[SI_JSON_STRING_LEN];
    // char jsonItemString[SI_JSON_STRING_LEN];
    UINT8 u8Cpu;

    for (u8Cpu = 0; u8Cpu < sSysInfoRawData->u32NumCpus; u8Cpu++) {
        snprintf(jsonItemName, SI_JSON_STRING_LEN, SI_JSON_SOCKET_NAME, u8Cpu);
        if ((socket = cJSON_GetObjectItemCaseSensitive(pJsonChild, jsonItemName)) == NULL) {
            cJSON_AddItemToObject(pJsonChild, jsonItemName, socket = cJSON_CreateObject());
        }

        // Fill in package_id as "N/A" for now
        cJSON_AddStringToObject(socket, cSectionName, "N/A");
        // snprintf(jsonItemString, SI_JSON_STRING_LEN, "%x", sSysInfoRawData->sCpuData[u8Cpu].packageId);
        // cJSON_AddStringToObject(socket, cSectionName, jsonItemString);
    }
}

/******************************************************************************
*
*   fillNumCpusJson
*
*   This function fills in the num_of_cpu JSON info
*
******************************************************************************/
void fillNumCpusJson(SSysInfoRawData * sSysInfoRawData, char * cSectionName, cJSON * pJsonChild)
{
    char jsonItemString[SI_JSON_STRING_LEN];

    snprintf(jsonItemString, SI_JSON_STRING_LEN, "0x%x", sSysInfoRawData->u32NumCpus);
    cJSON_AddStringToObject(pJsonChild, cSectionName , jsonItemString);
}

/******************************************************************************
*
*   fillCoresPerCpuJson
*
*   This function fills in the cores_per_cpu JSON info
*
******************************************************************************/
void fillCoresPerCpuJson(SSysInfoRawData * sSysInfoRawData, char * cSectionName, cJSON * pJsonChild)
{
    cJSON * socket;
    char jsonItemName[SI_JSON_STRING_LEN];
    char jsonItemString[SI_JSON_STRING_LEN];
    UINT8 u8Cpu;

    for (u8Cpu = 0; u8Cpu < sSysInfoRawData->u32NumCpus; u8Cpu++) {
        snprintf(jsonItemName, SI_JSON_STRING_LEN, SI_JSON_SOCKET_NAME, u8Cpu);
        if ((socket = cJSON_GetObjectItemCaseSensitive(pJsonChild, jsonItemName)) == NULL) {
            cJSON_AddItemToObject(pJsonChild, jsonItemName, socket = cJSON_CreateObject());
        }

        snprintf(jsonItemString, SI_JSON_STRING_LEN, "0x%x", sSysInfoRawData->sCpuData[u8Cpu].u32CoresPerCpu);
        cJSON_AddStringToObject(socket, cSectionName , jsonItemString);
    }
}

/******************************************************************************
*
*   fillThreadsPerCoreJson
*
*   This function fills in the threads_per_core JSON info
*
******************************************************************************/
void fillThreadsPerCoreJson(SSysInfoRawData * sSysInfoRawData, char * cSectionName, cJSON * pJsonChild)
{
    cJSON * socket;
    char jsonItemName[SI_JSON_STRING_LEN];
    char jsonItemString[SI_JSON_STRING_LEN];
    UINT8 u8Cpu;

    for (u8Cpu = 0; u8Cpu < sSysInfoRawData->u32NumCpus; u8Cpu++) {
        snprintf(jsonItemName, SI_JSON_STRING_LEN, SI_JSON_SOCKET_NAME, u8Cpu);
        if ((socket = cJSON_GetObjectItemCaseSensitive(pJsonChild, jsonItemName)) == NULL) {
            cJSON_AddItemToObject(pJsonChild, jsonItemName, socket = cJSON_CreateObject());
        }

        snprintf(jsonItemString, SI_JSON_STRING_LEN, "0x%x", sSysInfoRawData->sCpuData[u8Cpu].u32ThreadsPerCore);
        cJSON_AddStringToObject(socket, cSectionName , jsonItemString);
    }
}

/******************************************************************************
*
*   fillUCodeVersionJson
*
*   This function fills in the ucode_patch_ver JSON info
*
******************************************************************************/
void fillUCodeVersionJson(SSysInfoRawData * sSysInfoRawData, char * cSectionName, cJSON * pJsonChild)
{
    cJSON * socket;
    char jsonItemName[SI_JSON_STRING_LEN];
    char jsonItemString[SI_JSON_STRING_LEN];
    UINT8 u8Cpu;

    for (u8Cpu = 0; u8Cpu < sSysInfoRawData->u32NumCpus; u8Cpu++) {
        snprintf(jsonItemName, SI_JSON_STRING_LEN, SI_JSON_SOCKET_NAME, u8Cpu);
        if ((socket = cJSON_GetObjectItemCaseSensitive(pJsonChild, jsonItemName)) == NULL) {
            cJSON_AddItemToObject(pJsonChild, jsonItemName, socket = cJSON_CreateObject());
        }

        snprintf(jsonItemString, SI_JSON_STRING_LEN, "0x%x", sSysInfoRawData->sCpuData[u8Cpu].u32UCodeVer);
        cJSON_AddStringToObject(socket, cSectionName, jsonItemString);
    }
}

/******************************************************************************
*
*   fillVCodeVersionJson
*
*   This function fills in the vcode_patch_ver JSON info
*
******************************************************************************/
void fillVCodeVersionJson(SSysInfoRawData * sSysInfoRawData, char * cSectionName, cJSON * pJsonChild)
{
    cJSON * socket;
    char jsonItemName[SI_JSON_STRING_LEN];
    char jsonItemString[SI_JSON_STRING_LEN];
    UINT8 u8Cpu;

    for (u8Cpu = 0; u8Cpu < sSysInfoRawData->u32NumCpus; u8Cpu++) {
        snprintf(jsonItemName, SI_JSON_STRING_LEN, SI_JSON_SOCKET_NAME, u8Cpu);
        if ((socket = cJSON_GetObjectItemCaseSensitive(pJsonChild, jsonItemName)) == NULL) {
            cJSON_AddItemToObject(pJsonChild, jsonItemName, socket = cJSON_CreateObject());
        }

        snprintf(jsonItemString, SI_JSON_STRING_LEN, "0x%x", sSysInfoRawData->sCpuData[u8Cpu].u32VCodeVer);
        cJSON_AddStringToObject(socket, cSectionName, jsonItemString);
    }
}

/******************************************************************************
*
*   fillBmcVersionJson
*
*   This function fills in the bmc_fw_ver JSON info
*
******************************************************************************/
void fillBmcVersionJson(SSysInfoRawData * sSysInfoRawData, char * cSectionName, cJSON * pJsonChild)
{
    char jsonItemString[SI_JSON_STRING_LEN];

    snprintf(jsonItemString, SI_JSON_STRING_LEN, "%d.%02x.%02x%02x%02x%02x",
                                                sSysInfoRawData->bmcFwDeviceId.u8FirmwareMajorVersion & 0x7f,
                                                sSysInfoRawData->bmcFwDeviceId.u8FirmwareMinorVersion,
                                                sSysInfoRawData->bmcFwDeviceId.u8B3AuxFwRevInfo,
                                                sSysInfoRawData->bmcFwDeviceId.u8B2AuxFwRevInfo,
                                                sSysInfoRawData->bmcFwDeviceId.u8B1AuxFwRevInfo,
                                                sSysInfoRawData->bmcFwDeviceId.u8B0AuxFwRevInfo);
    cJSON_AddStringToObject(pJsonChild, cSectionName, jsonItemString);
}

/******************************************************************************
*
*   fillMeVersionJson
*
*   This function fills in the me_fw_ver JSON info
*
******************************************************************************/
void fillMeVersionJson(SSysInfoRawData * sSysInfoRawData, char * cSectionName, cJSON * pJsonChild)
{
    char jsonItemString[SI_JSON_STRING_LEN];

    // As descripted in NM-SPEC-550710-r2-6 Section 2.2
    // Major Version - Byte4[6:0]
    // Minor.Milestone - Byte5
    // BCD Encode X.Y: X maps to minor version, Y maps to milestone version
    // Build Version Number: 100*A + 10*B + C
    // BCD Encode A.B:  Byte14
    // BCD Encode C:    Byte15[7:4]
    snprintf(jsonItemString, SI_JSON_STRING_LEN, "%02d.%02x.%02x.%02x%x",
                                                sSysInfoRawData->meFwDeviceId.u8FirmwareMajorVersion & 0x7f,
                                                (sSysInfoRawData->meFwDeviceId.u8FirmwareMinorVersion & 0xf0) >> 4,
                                                sSysInfoRawData->meFwDeviceId.u8FirmwareMinorVersion & 0x0f,
                                                sSysInfoRawData->meFwDeviceId.u8B1AuxFwRevInfo,
                                                sSysInfoRawData->meFwDeviceId.u8B2AuxFwRevInfo >> 4);
    cJSON_AddStringToObject(pJsonChild, cSectionName, jsonItemString);
}

/******************************************************************************
*
*   fillBiosIdJson
*
*   This function fills in the bios_id JSON info
*
******************************************************************************/
void fillBiosIdJson(SSysInfoRawData * sSysInfoRawData, char * cSectionName, cJSON * pJsonChild)
{
    char jsonItemString[SI_BIOS_ID_LEN] = {0};
    int ret = 0;

    ret = snprintf(jsonItemString, sizeof(jsonItemString), "%s", sSysInfoRawData->u8BiosId);
    if(ret < 0 || ret >= (signed)sizeof(jsonItemString))
    {
	    TCRIT("Buffer Overflow\n");
	    return;
    }
    cJSON_AddStringToObject(pJsonChild, cSectionName, jsonItemString);
}

/******************************************************************************
*
*   fillCrystalRidgeVersionJson
*
*   This function fills in the crystalridge_fw_ver JSON info
*
******************************************************************************/
void fillCrystalRidgeVersionJson(SSysInfoRawData * sSysInfoRawData, char * cSectionName, cJSON * pJsonChild)
{
#ifdef SPX_BMC_ACD
    UN_USED(sSysInfoRawData);
#endif
    //Fill in as N/A for now
    cJSON_AddStringToObject(pJsonChild, cSectionName, "N/A");
}

/******************************************************************************
*
*   fillMcaErrSrcLogJson
*
*   This function fills in the mca_err_src_log JSON info
*
******************************************************************************/
void fillMcaErrSrcLogJson(SSysInfoRawData * sSysInfoRawData, char * cSectionName, cJSON * pJsonChild)
{
    cJSON * socket;
    char jsonItemName[SI_JSON_STRING_LEN];
    char jsonItemString[SI_JSON_STRING_LEN];
    UINT8 u8Cpu;

    for (u8Cpu = 0; u8Cpu < sSysInfoRawData->u32NumCpus; u8Cpu++) {
        snprintf(jsonItemName, SI_JSON_STRING_LEN, SI_JSON_SOCKET_NAME, u8Cpu);
        if ((socket = cJSON_GetObjectItemCaseSensitive(pJsonChild, jsonItemName)) == NULL) {
            cJSON_AddItemToObject(pJsonChild, jsonItemName, socket = cJSON_CreateObject());
        }

        snprintf(jsonItemString, SI_JSON_STRING_LEN, "0x%x", sSysInfoRawData->sCpuData[u8Cpu].u32McaErrSrcLog);
        cJSON_AddStringToObject(socket, cSectionName, jsonItemString);
    }
}

/******************************************************************************
*
*   fillValidJson
*
*   This function fills in the valid JSON info
*
******************************************************************************/
void fillValidJson(SSysInfoRawData * sSysInfoRawData, char * cSectionName, cJSON * pJsonChild)
{
    char jsonItemString[SI_JSON_STRING_LEN];

    snprintf(jsonItemString, SI_JSON_STRING_LEN, "0x%x", sSysInfoRawData->valid);
    cJSON_AddStringToObject(pJsonChild, cSectionName, jsonItemString);
}

/******************************************************************************
*
*   getBiosId
*
*   This function gets the BIOS ID
*
******************************************************************************/
static ESTATUS getBiosId(UINT8 * biosId)
{
	stub_getBiosId(biosId);
    return ST_OK;
}

/******************************************************************************
*
*   getMeDeviceId
*
*   This function gets the Device ID from the Intel ME
*
******************************************************************************/
static ESTATUS getMeDeviceId(SGetDeviceIdResp * devId)
{
	stub_getMeDeviceId(devId);
    return ST_OK;
}

#if defined(BUILD_JSON) || defined(BUILD_TXT)
static SSysInfoSection sSysInfoTable[] =
{
    { "num_of_cpu", fillNumCpusJson },
    { "cpu_type", fillCpuTypeJson },
    { "cpu_stepping", fillCpuSteppingJson },
    { "package_id", fillPackageIdJson },
    { "cores_per_cpu", fillCoresPerCpuJson },
    { "threads_per_core", fillThreadsPerCoreJson },
    { "ucode_patch_ver", fillUCodeVersionJson },
    { "vcode_ver", fillVCodeVersionJson },
    { "bmc_fw_ver", fillBmcVersionJson },
    { "me_fw_ver", fillMeVersionJson },
    { "bios_id", fillBiosIdJson },
    { "crystalridge_fw_ver", fillCrystalRidgeVersionJson },
    { "mca_err_src_log", fillMcaErrSrcLogJson },
    { "valid", fillValidJson }
};
#endif //BUILD_JSON || BUILD_TXT

/******************************************************************************
*
*   sysInfoTxt
*
*   This function formats the system information into the provided text file
*
******************************************************************************/
#ifdef BUILD_TXT
static void sysInfoTxt(SSysInfoRawData * sSysInfoRawData, FILE * fp)
{
    UINT8 u8Cpu;
    for (u8Cpu = 0; u8Cpu < sSysInfoRawData->u32NumCpus; u8Cpu++) {
        // Only include the MCA_ERR_SRC_LOG for now
        fprintf(fp, "\nCPU_%x_MCA_ERR_SRC_LOG: %016x", u8Cpu, sSysInfoRawData->sCpuData[u8Cpu].u32McaErrSrcLog);
    }
    fprintf(fp, "\n\n");
}
#endif //BUILD_TXT

/******************************************************************************
*
*   sysInfoJson
*
*   This function formats the system information into a JSON object
*
******************************************************************************/
#ifdef BUILD_JSON
static void sysInfoJson(SSysInfoRawData * sSysInfoRawData, cJSON * pJsonChild)
{
    UINT32 i;
    for (i = 0; i < (sizeof(sSysInfoTable) / sizeof(SSysInfoSection)); i++) {
        sSysInfoTable[i].FillSysInfoJson(sSysInfoRawData, sSysInfoTable[i].sectionName, pJsonChild);
    }
}
#endif //BUILD_JSON

#ifdef BUILD_RAW
static void sysInfoRaw(SSysInfoRawData * sSysInfoRawData, FILE * fpRaw)
{
    fwrite(sSysInfoRawData, sizeof(SSysInfoRawData), 1, fpRaw);
}
#endif //BUILD_RAW

/******************************************************************************
*
*   logSysInfo
*
*   This function gathers various bits of system information and compiles them
*   into a single group
*
******************************************************************************/
ESTATUS logSysInfo(FILE * fpRaw, FILE * fp, cJSON * pJsonChild)
{
    SSysInfoRawData sSysInfoRawData = {0};
    UINT32 u32PeciData = 0;
    UINT8 u8Cpu = 0;
    ESTATUS eStatus = ST_OK;
    SRdPkgConfigReq sRdPkgConfigReq;
    SRdPkgConfigRes sRdPkgConfigRes;

#ifdef SPX_BMC_ACD
    UN_USED(fp);
#endif

    PRINT(PRINT_DBG, PRINT_INFO, "Platform Debug - Sys Info log\n");

    // Get the CPUID info
    for (u8Cpu = 0; u8Cpu < MAX_CPU; u8Cpu++) {
        if (FALSE == IsCpuPresent(u8Cpu)) {
            continue;
        }

        memset(&sRdPkgConfigReq, 0 , sizeof(SRdPkgConfigReq));
        memset(&sRdPkgConfigRes, 0 , sizeof(SRdPkgConfigRes));
        sRdPkgConfigReq.sHeader.u8ClientAddr = PECI_BASE_ADDR + u8Cpu;
        sRdPkgConfigReq.sHeader.u8WriteLength = 0x05;
        sRdPkgConfigReq.sHeader.u8ReadLength = sizeof(UINT32) + 1;
        sRdPkgConfigReq.u8CmdCode = 0xA1;
        sRdPkgConfigReq.u8HostID_Retry = 0x02;
        sRdPkgConfigReq.u8Index = MBX_INDEX_CPU_ID;
        sRdPkgConfigReq.u16Parameter = PKG_ID_CPU_ID;
        if (0 == PECI_RdPkgConfig (&sRdPkgConfigReq, &sRdPkgConfigRes))
        {
            memcpy(&u32PeciData, sRdPkgConfigRes.u8Data, sizeof(UINT32));
        }
        else
        {
            sRdPkgConfigRes.u8CompletionCode = 0x00;
        }
        if (sRdPkgConfigRes.u8CompletionCode != PECI_CC_SUCCESS) {
            PRINT(PRINT_DBG, PRINT_ERROR, "[DBG] - CPU #%d CPUID Failed\n", u8Cpu);
            eStatus = ST_HW_FAILURE;
        }
        sSysInfoRawData.sCpuData[u8Cpu].cpuType = u32PeciData;
        sSysInfoRawData.sCpuData[u8Cpu].cpuStepping = u32PeciData & 0x0f;
        sSysInfoRawData.u32NumCpus++;

        // Get the core and thread count
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
            memcpy(&u32PeciData, sRdPkgConfigRes.u8Data, sizeof(UINT32));
        }
        else
        {
            sRdPkgConfigRes.u8CompletionCode = 0x00;
        }
        if (sRdPkgConfigRes.u8CompletionCode != PECI_CC_SUCCESS) {
            PRINT(PRINT_DBG, PRINT_ERROR, "[DBG] - CPU #%d Max Thread ID Failed\n", u8Cpu);
            eStatus = ST_HW_FAILURE;
        }
        sSysInfoRawData.sCpuData[u8Cpu].u32CoresPerCpu = (u32PeciData + 1) / SI_THREADS_PER_CORE;
        sSysInfoRawData.sCpuData[u8Cpu].u32ThreadsPerCore = SI_THREADS_PER_CORE;

        // Get the UCode Version
        memset(&sRdPkgConfigReq, 0 , sizeof(SRdPkgConfigReq));
        memset(&sRdPkgConfigRes, 0 , sizeof(SRdPkgConfigRes));
        sRdPkgConfigReq.sHeader.u8ClientAddr = PECI_BASE_ADDR + u8Cpu;
        sRdPkgConfigReq.sHeader.u8WriteLength = 0x05;
        sRdPkgConfigReq.sHeader.u8ReadLength = sizeof(UINT32) + 1;
        sRdPkgConfigReq.u8CmdCode = 0xA1;
        sRdPkgConfigReq.u8HostID_Retry = 0x02;
        sRdPkgConfigReq.u8Index = MBX_INDEX_CPU_ID;
        sRdPkgConfigReq.u16Parameter = PKG_ID_MICROCODE_REV;
        if (0 == PECI_RdPkgConfig (&sRdPkgConfigReq, &sRdPkgConfigRes))
        {
            memcpy(&u32PeciData, sRdPkgConfigRes.u8Data, sizeof(UINT32));
        }
        else
        {
            sRdPkgConfigRes.u8CompletionCode = 0x00;
        }
        if (sRdPkgConfigRes.u8CompletionCode != PECI_CC_SUCCESS) {
            PRINT(PRINT_DBG, PRINT_ERROR, "[DBG] - CPU #%d Ucode Version Failed\n", u8Cpu);
            eStatus = ST_HW_FAILURE;
        }
        sSysInfoRawData.sCpuData[u8Cpu].u32UCodeVer = u32PeciData;

        // Get the VCode Version
        memset(&sRdPkgConfigReq, 0 , sizeof(SRdPkgConfigReq));
        memset(&sRdPkgConfigRes, 0 , sizeof(SRdPkgConfigRes));
        sRdPkgConfigReq.sHeader.u8ClientAddr = PECI_BASE_ADDR + u8Cpu;
        sRdPkgConfigReq.sHeader.u8WriteLength = 0x05;
        sRdPkgConfigReq.sHeader.u8ReadLength = sizeof(UINT32) + 1;
        sRdPkgConfigReq.u8CmdCode = 0xA1;
        sRdPkgConfigReq.u8HostID_Retry = 0x02;
        sRdPkgConfigReq.u8Index = MBX_INDEX_VCU;
        sRdPkgConfigReq.u16Parameter = VCU_VERSION;
        if (0 == PECI_RdPkgConfig (&sRdPkgConfigReq, &sRdPkgConfigRes))
        {
            memcpy(&u32PeciData, sRdPkgConfigRes.u8Data, sizeof(UINT32));
        }
        else
        {
            sRdPkgConfigRes.u8CompletionCode = 0x00;
        }
        if (sRdPkgConfigRes.u8CompletionCode != PECI_CC_SUCCESS) {
            PRINT(PRINT_DBG, PRINT_ERROR, "[DBG] - CPU #%d VCU Version Failed\n", u8Cpu);
            eStatus = ST_HW_FAILURE;
        }
        sSysInfoRawData.sCpuData[u8Cpu].u32VCodeVer = u32PeciData;

        // Get the MCA_ERR_SRC_LOG
        memset(&sRdPkgConfigReq, 0 , sizeof(SRdPkgConfigReq));
        memset(&sRdPkgConfigRes, 0 , sizeof(SRdPkgConfigRes));
        sRdPkgConfigReq.sHeader.u8ClientAddr = PECI_BASE_ADDR + u8Cpu;
        sRdPkgConfigReq.sHeader.u8WriteLength = 0x05;
        sRdPkgConfigReq.sHeader.u8ReadLength = sizeof(UINT32) + 1;
        sRdPkgConfigReq.u8CmdCode = 0xA1;
        sRdPkgConfigReq.u8HostID_Retry = 0x02;
        sRdPkgConfigReq.u8Index = MBX_INDEX_CPU_ID;
        sRdPkgConfigReq.u16Parameter = PKG_ID_MACHINE_CHECK_STATUS;
        if (0 == PECI_RdPkgConfig (&sRdPkgConfigReq, &sRdPkgConfigRes))
        {
            memcpy(&u32PeciData, sRdPkgConfigRes.u8Data, sizeof(UINT32));
        }
        else
        {
            sRdPkgConfigRes.u8CompletionCode = 0x00;
        }
        if (sRdPkgConfigRes.u8CompletionCode != PECI_CC_SUCCESS) {
            PRINT(PRINT_DBG, PRINT_ERROR, "[DBG] - CPU #%d MCA_ERR_SRC_LOG Failed\n", u8Cpu);
            eStatus = ST_HW_FAILURE;
        }
        sSysInfoRawData.sCpuData[u8Cpu].u32McaErrSrcLog = u32PeciData;
    }

    // Get the BIOS ID
    if (getBiosId(sSysInfoRawData.u8BiosId) != ST_OK) {
        eStatus = ST_IPMB_ERROR;
    }

    // Get the ME Device ID
    if (getMeDeviceId(&sSysInfoRawData.meFwDeviceId) != ST_OK) {
        eStatus = ST_IPMB_ERROR;
    }

    // Get the BMC Device ID
    stub_SysInfo_GetDevID(&sSysInfoRawData.bmcFwDeviceId);

    // Determine the data validity (always TRUE for now)
    sSysInfoRawData.valid = TRUE;

    // Log the System Info
#ifdef BUILD_TXT
    if (fp != NULL) {
        sysInfoTxt(&sSysInfoRawData, fp);
    }
#endif //BUILD_TXT

#ifdef BUILD_RAW
    if (fpRaw != NULL) { //((fpRaw != NULL) && (fp == NULL) && (pJsonChild == NULL)) {
        sysInfoRaw(&sSysInfoRawData, fpRaw);
    }
#endif //BUILD_RAW

#ifdef BUILD_JSON
    if (pJsonChild != NULL) {
        sysInfoJson(&sSysInfoRawData, pJsonChild);
    }
#endif //BUILD_JSON

    return eStatus;
}
