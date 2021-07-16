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

#ifndef _POWER_MANAGEMENT_H_
#define _POWER_MANAGEMENT_H_

// PECI sequence
#define PM_SEQ_DATA 0x1002a
#define PM_CSTATE_PARAM 0x4660B4
#define PM_VID_PARAM 0x488004
#define PM_READ_PARAM 0x1019
#define PM_CORE_OFFSET 24

#define PM_JSON_STRING_LEN 32
#define PM_JSON_SOCKET_NAME "socket%d"
#define PM_JSON_CORE_NAME "core%d"
#define PM_JSON_CSTATE_REG_NAME "c_state_reg"
#define PM_JSON_VID_REG_NAME "vid_ratio_reg"

/******************************************************************************
*
*   Structures
*
******************************************************************************/
typedef struct {
    UINT32 u32CState;
    UINT32 u32VidRatio;
} SCpuPowerState;

ESTATUS logPowerManagement(FILE * fpRaw, FILE * fp, cJSON * pJsonChild);
#endif //_POWER_MANAGEMENT_H_
