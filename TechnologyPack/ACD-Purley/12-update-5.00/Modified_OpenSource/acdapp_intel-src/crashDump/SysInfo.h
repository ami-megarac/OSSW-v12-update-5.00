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

#ifndef _SYSINFO_H_
#define _SYSINFO_H_

typedef struct {
    UINT8 u8FirmwareMajorVersion;
    UINT8 u8FirmwareMinorVersion;
    UINT8 u8B3AuxFwRevInfo;
    UINT8 u8B2AuxFwRevInfo;
    UINT8 u8B1AuxFwRevInfo;
    UINT8 u8B0AuxFwRevInfo;
} SGetDeviceIdResp;

#define SI_THREADS_PER_CORE 2

#define SI_JSON_STRING_LEN 48
#define SI_JSON_SOCKET_NAME "socket%d"

#define SI_IPMI_DEV_ID_NETFN 0x06
#define SI_IPMI_DEV_ID_CMD 0x01
#define SI_IPMI_DEV_ID_LUN 0

#define SI_BIOS_ID_LEN 64

#define SI_CPU_NAME_LEN 8

/******************************************************************************
*
*   Structures
*
******************************************************************************/
typedef struct {
    UINT32 u32CpuId;
    char cpuModelName[SI_CPU_NAME_LEN];
} SCpuIdName;

typedef struct {
    UINT32 u32NumCpus;
    struct {
        UINT32 cpuType;
        UINT32 cpuStepping;
        UINT32 packageId;
        UINT32 u32CoresPerCpu;
        UINT32 u32ThreadsPerCore;
        UINT32 u32UCodeVer;
        UINT32 u32VCodeVer;
        UINT32 u32McaErrSrcLog;
    } sCpuData[MAX_CPU];
    SGetDeviceIdResp bmcFwDeviceId;
    SGetDeviceIdResp meFwDeviceId;
    UINT8 u8BiosId[SI_BIOS_ID_LEN];
    UINT32 crystalRidgeFwVer;
    BOOL valid;
} SSysInfoRawData;

typedef struct {
    char sectionName[SI_JSON_STRING_LEN];
    void (*FillSysInfoJson)(SSysInfoRawData * sSysInfoRawData, char * cSectionName, cJSON * pJsonChild);
} SSysInfoSection;

ESTATUS logSysInfo(FILE * fpRaw, FILE * fp, cJSON * pJsonChild);
#endif //_SYSINFO_H_
