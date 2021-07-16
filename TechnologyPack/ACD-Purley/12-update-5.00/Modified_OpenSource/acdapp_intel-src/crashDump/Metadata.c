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

#include <stdio.h>
#include <time.h>
#define UINT32 unsigned int
#include "Commstat.h"
#ifndef SPX_BMC_ACD
#include <cJSON.h>
#endif
#include "Metadata.h"

/******************************************************************************
*
*   metadataJson
*
*   This function fills in the JSON metadata object
*
******************************************************************************/
static void metadataJson(cJSON * pJsonChild)
{
    char logTime[64];
    time_t curtime;
    struct tm* loctime;
    char jsonItemString[MD_JSON_STRING_LEN];

    // Add the Crashdump Version
    cJSON_AddStringToObject(pJsonChild, MD_REV_NAME, "0.6");

    // Add the Crashdump Reason
    snprintf(jsonItemString, MD_JSON_STRING_LEN, "0x%x", MD_REASON_ERROR);
    cJSON_AddStringToObject(pJsonChild, MD_REASON_NAME, jsonItemString);

    // Add the timestamp
    curtime = time(NULL);
    loctime = localtime(&curtime);
    if (NULL != loctime) {
        strftime(logTime, sizeof(logTime), "%Y%m%d_%H%M%S", loctime);
    }
    cJSON_AddStringToObject(pJsonChild, MD_TIMESTAMP_NAME, logTime);
}

/******************************************************************************
*
*   logMetadata
*
*   This function logs the metadata
*
******************************************************************************/
ESTATUS logMetadata(FILE * fpRaw, FILE * fp, cJSON * pJsonChild)
{
    if (pJsonChild != NULL) {
        metadataJson(pJsonChild);
    }
#ifdef SPX_BMC_ACD
    UN_USED(fpRaw);
    UN_USED(fp);
#endif
    return ST_OK;
}
