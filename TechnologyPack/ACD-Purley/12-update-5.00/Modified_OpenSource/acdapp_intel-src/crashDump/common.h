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

#ifndef _COMMON_H_
#define _COMMON_H_

#ifndef SPX_BMC_ACD
#include <Types.h>
#endif
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <stdbool.h>

#define BOOL int
#define UINT8 unsigned char
#define UINT16 unsigned short
#define UINT32 unsigned int
#define UINT64 long unsigned int

#ifdef SPX_BMC_ACD
#define TRUE true
#define FALSE false
#endif

#include "Commstat.h"
#include "CPU.h"
#include "peci_interface.h"
#ifdef SPX_BMC_ACD
#include "cJSON.h"
#endif

#define EPECIStatus int
#define PECI_CC_SUCCESS 0x40

#define PECI_BASE_ADDR              0x30
#define PKG_ID_CPU_ID               0x0000  // 0 - CPUID Info
#define PKG_ID_MAX_THREAD_ID        0x0003  // 3 - Max Thread ID
#define PKG_ID_MICROCODE_REV        0x0004  // 4 - CPU Microcode Update Revision
#define PKG_ID_MACHINE_CHECK_STATUS 0x0005  // 5 - Machine Check Status

#define MBX_INDEX_CPU_ID            0       // Package Identifier Read
#define MBX_INDEX_VCU               128     // VCU Index

#define VCU_SET_PARAM               0x0001
#define VCU_READ                    0x0002
#define VCU_OPEN_SEQ                0x0003
#define VCU_CLOSE_SEQ               0x0004
#define VCU_ABORT_SEQ               0x0005
#define VCU_VERSION                 0x0009

#define VCU_READ_LOCAL_MMIO_SEQ     0x6
#define VCU_UNCORE_MCA_SEQ          0x10000
#define VCU_UNCORE_CRASHDUMP_SEQ    0x30006

#define PRINT_DEBUG2 0
#define PRINT_DEBUG 0
#define PRINT_DBG 0
#define PRINT_INFO 0
#define PRINT_ERROR 1
#define PRINT_FWUPD_PSU 1

extern int prnt_dbg;

#define memset_s(dest, dest_size, ch) \
    memset((dest), (ch), (dest_size))

#define memcpy_s(dest, dest_size, src, count) \
    memcpy((dest), (src), (count))

#define memcmp_s(ptr1, ptr1_size, ptr2, num, ptr_diff) \
    memcmp((ptr1), (ptr2), (num))

static inline BOOL IsCpuPresent(UINT8 CpuNum)
{
#ifdef SPX_BMC_ACD
	UN_USED(CpuNum);
#endif
	return (true);
}

#ifdef SPX_BMC_ACD
//#define PRINT(dbg, info, format, ...)	printf(format, ##__VA_ARGS__)
static inline bool ShouldLog(int info_log_type)
{
    if ((info_log_type == PRINT_ERROR) || (info_log_type == PRINT_FWUPD_PSU))
        return true;

    if (prnt_dbg)
        return true;

    return false;
}

static inline void PRINT(int dbg, int info_log_type, const char *format, ...)
{
    UN_USED(dbg);

    va_list args;
    va_start(args, format);
    if (ShouldLog(info_log_type)) {
        vfprintf(stderr, format, args);
        fprintf(stderr, "\n");
    }
    va_end(args);
}
#else
static inline void PRINT(int dbg, int info, const char *format, ...)
{
}
#endif

#endif  // _COMMON_H_
