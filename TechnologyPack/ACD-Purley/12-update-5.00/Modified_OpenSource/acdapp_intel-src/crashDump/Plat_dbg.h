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

#ifndef _PLAT_DBG_H_
#define _PLAT_DBG_H_

#ifndef SPX_BMC_ACD
#include <cJSON.h>
#else
#include "cJSON.h"
#endif

/******************************************************************************
*
*  Defines for Platform Debug
*
******************************************************************************/
#define CRASHDUMP_DIRECTORY "/conf/crashdump"
#ifdef SPX_BMC_ACD
#define CRASHDUMP_JSON_DIRECTORY "/var/crashdump/json"
#define CRASHDUMP_PATH_FILE	"/etc/crashdump/crashdumppath"
#endif
#define RAM_CPUDUMP_RAW_FILE ("CPUDump.raw")        // create the dump file in the RAM disk first
#define RAM_CPUDUMP_FILE ("CPUDump.txt")        // create the dump file in the RAM disk first
#ifndef SPX_BMC_ACD
#define RAM_CPUJSON_FILE ("bmc_crashdump.json")      // create the json file in the RAM disk first
#define RAM_SYSINFO_RAW_FILE "/conf/crashdump/sysInfo.raw"
#define ARCHIVE_FILE_1 "bmc_crashdump.1.gz"
#define ARCHIVE_FILE_2 "bmc_crashdump.2.gz"
#else
#define RAM_CPUJSON_FILE "sysdebug.json"      // create the json file in the RAM disk first
#define RAM_SYSINFO_RAW_FILE ("sysInfo.raw")
#endif

#define PLAT_DBG_ARG_LEN 16
#if 0
#define PLAT_DBG_ARG_FLAGS 0x0E
#define DBG_LOG_RETRIES_ENABLE_MASK 0x01
#endif

#define DBG_STATUS_ITEM_NAME "status"
#define DBG_FAILED_STATUS "N/A"

#define DBG_ABORTED_ITEM_NAME "_status"
#define DBG_ABORTED_STATUS "Aborted"

#define DBG_TIME_ITEM_NAME "_time"

#ifndef SPX_BMC_ACD
#define SECTION_NAME_LEN 32
#else
#define DEBUG_SECTION_NAME_LEN 32
#define DIR_NAME_LEN 128
#define MAX_DIR_LEN  64
#endif

/******************************************************************************
*
*  Enum Definitions
*
******************************************************************************/
typedef enum _EPlatDBGCmd {
    PLAT_DBG_NONE,

    PLAT_DBG_ON_IERR,
    PLAT_DBG_ON_ERR2,
    PLAT_DBG_ON_SMI,

    PLAT_DBG_CMD

} EPlatDBGCmd;

/******************************************************************************
*
*   Structures
*
******************************************************************************/
typedef struct _SPlatDBG {
    EPlatDBGCmd ePlatDBGCmd;
    BOOL(*PreFunc)
    (void);
    BOOL(*PostFunc)
    (void);
    UINT8 u8Arg[PLAT_DBG_ARG_LEN];
} SPlatDBG;

typedef struct {
#ifndef SPX_BMC_ACD
        char sectionName[SECTION_NAME_LEN];
#else
        char sectionName[DEBUG_SECTION_NAME_LEN];
#endif
    ESTATUS (*GetSectionLog)(FILE * fpRaw, FILE * fp, cJSON * pJsonChild);
} SDebugLogSection;

#endif //_PLAT_DBG_H_
