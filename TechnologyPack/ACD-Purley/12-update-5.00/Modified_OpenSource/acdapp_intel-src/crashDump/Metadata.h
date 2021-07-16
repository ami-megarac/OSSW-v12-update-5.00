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

#ifndef _METADATA_H_
#define _METADATA_H_

#ifdef SPX_BMC_ACD
#include "common.h"
#endif

#define MD_JSON_STRING_LEN 32

#define MD_REV_NAME "crashdump_version"
#define MD_REASON_NAME "crashdump_reason"
#define MD_TIMESTAMP_NAME "timestamp"

#define MD_REASON_ERROR (1 << 2)

/******************************************************************************
*
*   Structures
*
******************************************************************************/

ESTATUS logMetadata(FILE * fpRaw, FILE * fp, cJSON * pJsonChild);
#endif //_METADATA_H_
