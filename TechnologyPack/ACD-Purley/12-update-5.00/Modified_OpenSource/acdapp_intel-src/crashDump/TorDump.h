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

#ifndef _TORDUMP_H_
#define _TORDUMP_H_

// PECI sequence
#define TD_SEQ_DATA 0x30002
#define TD_START_PARAM 0x3001
#define TD_PARAM_ZERO 0x80000000

// Other
#define TD_TORS_PER_CHA 24
#define TD_DWORDS_PER_TOR 3

#define TD_JSON_STRING_LEN 32
#define TD_JSON_SOCKET_NAME "socket%d"
#define TD_JSON_CHA_NAME "CHA%d"
#define TD_JSON_TOR_NAME "TOR%d"
#define TD_JSON_DWORD_NAME "DW%d"

ESTATUS logTorDump(FILE * fpRaw, FILE * fp, cJSON * pJsonChild);
#endif //_TORDUMP_H_
