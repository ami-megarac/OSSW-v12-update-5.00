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

#include <stdint.h>

#include <cstddef>

extern "C" {
#ifndef SPX_BMC_ACD
#include <cjson/cJSON.h>
#else
#include "cJSON.h"
#endif
}

#include "crashdump.hpp"

#define DEFAULT_INPUT_FILE "/conf/crashdump/input/crashdump_input_%s.json"
#define OVERRIDE_INPUT_FILE "/tmp/crashdump/input/crashdump_input_%s.json"
#define DEFAULT_TELEMETRY_FILE                                                 \
    "/conf/crashdump/input/telemetry_input_%s.json"
#define OVERRIDE_TELEMETRY_FILE "/tmp/crashdump/input/telemetry_input_%s.json"

#define PECI_CC_SKIP_CORE(cc)                                                  \
    (((cc) == PECI_DEV_CC_CATASTROPHIC_MCA_ERROR ||                            \
      (cc) == PECI_DEV_CC_NEED_RETRY || (cc) == PECI_DEV_CC_OUT_OF_RESOURCE || \
      (cc) == PECI_DEV_CC_UNAVAIL_RESOURCE ||                                  \
      (cc) == PECI_DEV_CC_INVALID_REQ || (cc) == PECI_DEV_CC_MCA_ERROR)        \
         ? true                                                                \
         : false)

#define PECI_CC_SKIP_SOCKET(cc)                                                \
    (((cc) == PECI_DEV_CC_PARITY_ERROR_ON_GPSB_OR_PMSB ||                      \
      (cc) == PECI_DEV_CC_PARITY_ERROR_ON_GPSB_OR_PMSB_IERR ||                 \
      (cc) == PECI_DEV_CC_PARITY_ERROR_ON_GPSB_OR_PMSB_MCA)                    \
         ? true                                                                \
         : false)

#define PECI_CC_UA(cc)                                                         \
    (((cc) != PECI_DEV_CC_SUCCESS && (cc) != PECI_DEV_CC_FATAL_MCA_DETECTED)   \
         ? true                                                                \
         : false)

#define FREE(ptr)                                                              \
    do                                                                         \
    {                                                                          \
        free(ptr);                                                             \
        ptr = NULL;                                                            \
    } while (0)

#define SET_BIT(val, pos) ((val) |= ((uint64_t)1 << ((uint64_t)pos)))
#define CLEAR_BIT(val, pos) ((val) &= ~((uint64_t)1 << ((uint64_t)pos)))
#define CHECK_BIT(val, pos) ((val) & ((uint64_t)1 << ((uint64_t)pos)))
#define CPU_STR_LEN 5
#define NAME_STR_LEN 255
#define DEFAULT_VALUE -1

#define UT_REG_NAME_LEN 32
#define UT_REG_DWORD 4
#define UT_REG_QWORD 8

enum inputField
{
    FLAG_ENABLE,
    FLAG_DISABLE,
    FLAG_NOT_PRESENT
};

typedef union
{
    uint64_t u64;
    uint32_t u32[2];
} SRegValue;

typedef struct
{
    SRegValue uValue;
    uint8_t cc;
    int ret;
} SRegRawData;

typedef struct
{
    char regName[UT_REG_NAME_LEN];
    uint8_t u8Seg;
    uint8_t u8Bus;
    uint8_t u8Dev;
    uint8_t u8Func;
    uint16_t u16Reg;
    uint8_t u8Size;
} SRegPci;

static const SRegPci sPciReg[] = {
    // Register, Seg, Bus, Dev, Func, Offset, Size
    {"ierrloggingreg", 0, 30, 0, 0, 0xA4, 4},
    {"mcerrloggingreg", 0, 30, 0, 0, 0xA8, 4},
    {"firstierrtsc", 0, 31, 30, 4, 0xFCF8, 8},
    {"firstmcerrtsc", 0, 31, 30, 4, 0xF4F0, 8},
    {"mca_err_src_log", 0, 31, 30, 2, 0xEC, 4},
};

int cd_snprintf_s(char* str, size_t len, const char* format, ...);

void setFields(uint32_t* value, uint32_t msb, uint32_t lsb, uint32_t inputVal);
uint32_t getFields(uint32_t value, uint32_t msb, uint32_t lsb);
uint32_t bitField(uint32_t offset, uint32_t size, uint32_t val);

// crashdump_input_xxx.json #define(s) and c-helper functions
#define RECORD_ENABLE "_record_enable"
#define ENABLE 0

void updateRecordEnable(cJSON* root, bool enable);
cJSON* getCrashDataSection(cJSON* root, char* section, bool* enable);
cJSON* getCrashDataSectionRegList(cJSON* root, char* section, char* regType,
                                  bool* enable);
int getCrashDataSectionVersion(cJSON* root, char* section);
cJSON* selectAndReadInputFile(crashdump::cpu::Model cpuModel, char** filename,
                              bool isTelemetry);
int getPciRegister(crashdump::CPUInfo& cpuInfo, SRegRawData* sRegData,
                   uint8_t u8index);
struct timespec calculateDelay(struct timespec* crashdumpStart,
                               uint32_t delayTimeFromInputFileInSec);
uint32_t getDelayFromInputFile(crashdump::CPUInfo& cpuInfo, char* sectionName);
uint64_t tsToNanosecond(timespec* ts);
bool getTorSkipFromInputFile(crashdump::CPUInfo& cpuInfo, char* sectionName);
inputField getFlagValueFromInputFile(crashdump::CPUInfo& cpuInfo,
                                     char* sectionName, char* flagName);
