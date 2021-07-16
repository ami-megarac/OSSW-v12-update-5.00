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
#include <cjson/cJSON.h>
#endif
#include <stdbool.h>
#include <stdint.h>

#define SI_THREADS_PER_CORE 2

#define SI_JSON_STRING_LEN 48
#define SI_JSON_SOCKET_NAME "cpu%d"

#define SI_IPMI_DEV_ID_NETFN 0x06
#define SI_IPMI_DEV_ID_CMD 0x01
#define SI_IPMI_DEV_ID_LUN 0

#define SI_BIOS_ID_LEN 64
#define SI_BMC_VER_LEN 64

#define SI_CPU_NAME_LEN 8
#define SI_CRASHDUMP_VER_LEN 8
#define SI_CRASHDUMP_VER "BMC_1.0.6"

#define SI_PECI_PPIN_IDX 19
#define SI_PECI_PPIN_LOWER 0x01
#define SI_PECI_PPIN_UPPER 0x02
#define MD_UINT64 "0x%" PRIx64 ""
#define MD_UA "UA:0x%x"
#define MD_DF "DF:0x%x"
#define MD_UA_DF "UA:0x%x,DF:0x%x"
#define MD_NA "N/A"
#define MD_STARTUP "STARTUP"
#define MD_EVENT "EVENT"
#define MD_OVERWRITTEN "OVERWRITTEN"
#define MD_INVALID "INVALID"

#define RESET_DETECTED_DEFAULT "NONE"

/******************************************************************************
 *
 *   Structures
 *
 ******************************************************************************/
typedef struct
{
    char sectionName[SI_JSON_STRING_LEN];
    int (*FillSysInfoJson)(crashdump::CPUInfo& cpuInfo, char* cSectionName,
                           cJSON* pJsonChild);
} SSysInfoSection;

typedef struct
{
    char sectionName[SI_JSON_STRING_LEN];
    int (*FillSysInfoJson)(char* cSectionName, cJSON* pJsonChild);
} SSysInfoCommonSection;

typedef struct
{
    char sectionName[SI_JSON_STRING_LEN];
    int (*FillSysInfoJson)(crashdump::CPUInfo& cpuInfo, char* cSectionName,
                           cJSON* pJsonChild,
                           crashdump::InputFileInfo* inputFileInfo);
} SSysInfoInputFileSection;

typedef struct
{
    crashdump::cpu::Model cpuModel;
    int (*getPpinVx)(crashdump::CPUInfo& cpuInfo, char* cSectionName,
                     cJSON* pJsonChild);
} SPpinVx;

int logSysInfo(crashdump::CPUInfo& cpuInfo, cJSON* pJsonChild);
int logSysInfoCommon(cJSON* pJsonChild);
int logSysInfoInputfile(crashdump::CPUInfo& cpuInfo, cJSON* pJsonChild,
                        crashdump::InputFileInfo* inputFileInfo);
int logResetDetected(cJSON* metadata, int cpuNum, int sectionName);
