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

#ifndef _SQ_DUMP_H_
#define _SQ_DUMP_H_

// PECI sequence
#define SQ_SEQ_DATA 0x30004
#define SQ_START_PARAM 0x3002
#define SQ_ADDR_ARRAY 0x00000000
#define SQ_CTRL_ARRAY 0x80000000

// Other
#define SQ_DWORDS_PER_ENTRY 2

#define SQ_JSON_STRING_LEN 32
#define SQ_JSON_SOCKET_NAME "socket%d"
#define SQ_JSON_CORE_NAME "core%d"
#define SQ_JSON_ENTRY_NAME "entry%d"
#define SQ_JSON_DWORD_NAME "DW%d"
#define SQ_JSON_ADDR_ARRAY_NAME "mlc_sq_addr"
#define SQ_JSON_CTRL_ARRAY_NAME "mlc_sq_ctl"

/******************************************************************************
*
*   Structures
*
******************************************************************************/
typedef struct {
    UINT32 * pu32SqAddrArray;
    UINT32 u32SqAddrSize;
    UINT32 * pu32SqCtrlArray;
    UINT32 u32SqCtrlSize;
} SSqDump;

ESTATUS logSqDump(FILE * fpRaw, FILE * fp, cJSON * pJsonChild);
#endif //_SQ_DUMP_H_
