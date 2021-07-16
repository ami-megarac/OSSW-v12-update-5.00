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

#include "utils.hpp"

#include <stdlib.h>
#include <unistd.h>

#ifndef SPX_BMC_ACD
#include <boost/container/flat_map.hpp>
#include <sdbusplus/bus.hpp>
#include <sdbusplus/message.hpp>
#include <variant>
#else
#include <cstdio>
#include <cstdarg>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef SPX_BMC_ACD
#include <safe_mem_lib.h>
#include <safe_str_lib.h>
#endif

//#include <safeclib_header/safe_str_lib.h>
#include <string.h>

#ifdef __cplusplus
}
#endif

#include <cstdarg>
#include <cstring>

int cd_snprintf_s(char* str, size_t len, const char* format, ...)
{
    int ret;
    va_list args;
    va_start(args, format);
    ret = vsnprintf(str, len, format, args);
    va_end(args);
    return ret;
}

static uint32_t setMask(uint32_t msb, uint32_t lsb)
{
    uint32_t range = (msb - lsb) + 1;
    uint32_t mask = ((1u << range) - 1);
    return mask;
}

void setFields(uint32_t* value, uint32_t msb, uint32_t lsb, uint32_t setVal)
{
    uint32_t mask = setMask(msb, lsb);
    setVal = setVal << lsb;
    *value &= ~(mask << lsb);
    *value |= setVal;
}

uint32_t getFields(uint32_t value, uint32_t msb, uint32_t lsb)
{
    uint32_t mask = setMask(msb, lsb);
    value = value >> lsb;
    return (mask & value);
}

uint32_t bitField(uint32_t offset, uint32_t size, uint32_t val)
{
    uint32_t mask = (1u << size) - 1;
    return (val & mask) << offset;
}

cJSON* readInputFile(const char* filename)
{
    char* buffer = NULL;
    cJSON* jsonBuf = NULL;
    long int length = 0;
    FILE* fp = fopen(filename, "r");
    size_t result = 0;

    if (fp == NULL)
    {
        return NULL;
    }

    fseek(fp, 0, SEEK_END);
    length = ftell(fp);
    if (length == -1L)
    {
        fclose(fp);
        return NULL;
    }
    fseek(fp, 0, SEEK_SET);
    buffer = (char*)calloc(length, sizeof(char));
    if (buffer)
    {
        result = fread(buffer, 1, length, fp);
        if ((int)result != length)
        {
            fprintf(stderr, "fread read %zu bytes, but length is %ld", result,
                    length);
            fclose(fp);
            FREE(buffer);
            return NULL;
        }
    }

    fclose(fp);
    // Convert and return cJSON object from buffer
    jsonBuf = cJSON_Parse(buffer);
    FREE(buffer);
    return jsonBuf;
}

cJSON* getCrashDataSection(cJSON* root, char* section, bool* enable)
{
    *enable = false;
    cJSON* child = cJSON_GetObjectItemCaseSensitive(
        cJSON_GetObjectItemCaseSensitive(root, "crash_data"), section);

    if (child != NULL)
    {
        cJSON* recordEnable = cJSON_GetObjectItem(child, RECORD_ENABLE);
        if (recordEnable == NULL)
        {
            *enable = true;
        }
        else
        {
            *enable = cJSON_IsTrue(recordEnable);
        }
    }

    return child;
}

cJSON* getCrashDataSectionRegList(cJSON* root, char* section, char* regType,
                                  bool* enable)
{
    cJSON* child = getCrashDataSection(root, section, enable);

    if (child != NULL)
    {
        return cJSON_GetObjectItemCaseSensitive(
            cJSON_GetObjectItemCaseSensitive(child, regType), "reg_list");
    }

    return child;
}

cJSON* getCrashDataSectionObjectOneLevel(cJSON* root, char* section,
                                         const char* firstLevel, bool* enable)
{
    cJSON* child = getCrashDataSection(root, section, enable);

    if (child != NULL)
    {
        return cJSON_GetObjectItemCaseSensitive(child, firstLevel);
    }

    return child;
}

int getCrashDataSectionVersion(cJSON* root, char* section)
{
    uint64_t version = 0;
    bool enable = false;
    cJSON* child = getCrashDataSection(root, section, &enable);

    if (child != NULL)
    {
        cJSON* jsonVer = cJSON_GetObjectItem(child, "_version");
        if ((jsonVer != NULL) && cJSON_IsString(jsonVer))
        {
            version = strtoull(jsonVer->valuestring, NULL, 16);
        }
    }

    return version;
}

cJSON* selectAndReadInputFile(crashdump::cpu::Model cpuModel, char** filename,
                              bool isTelemetry)
{
    char cpuStr[CPU_STR_LEN] = {0};
    char nameStr[NAME_STR_LEN] = {0};

    switch (cpuModel)
    {
        case crashdump::cpu::skx:
        case crashdump::cpu::clx:
            strncpy(cpuStr, "skx", sizeof(cpuStr));
            break;
        case crashdump::cpu::cpx:
            strncpy(cpuStr, "cpx", sizeof(cpuStr));
            break;
        case crashdump::cpu::icx:
        case crashdump::cpu::icx2:
        case crashdump::cpu::icxd:
            strncpy(cpuStr, "icx", sizeof(cpuStr));
            break;
        default:
            CRASHDUMP_PRINT(ERR, stderr,
                            "Error selecting input file (CPUID 0x%x).\n",
                            cpuModel);
            return NULL;
    }

    char* override_file;

    isTelemetry ? override_file = OVERRIDE_TELEMETRY_FILE
                : override_file = OVERRIDE_INPUT_FILE;
    cd_snprintf_s(nameStr, NAME_STR_LEN, override_file, cpuStr);

    if (access(nameStr, F_OK) != -1)
    {
        CRASHDUMP_PRINT(INFO, stderr, "Using override file - %s\n", nameStr);
    }
    else
    {
        const char* default_file;
        isTelemetry ? default_file = DEFAULT_TELEMETRY_FILE
                    : default_file = DEFAULT_INPUT_FILE;
        cd_snprintf_s(nameStr, NAME_STR_LEN, default_file, cpuStr);
    }
    *filename = (char*)malloc(sizeof(nameStr));
    if (*filename == NULL)
    {
        return NULL;
    }
    strncpy(*filename, nameStr, NAME_STR_LEN);

    return readInputFile(nameStr);
}

void updateRecordEnable(cJSON* root, bool enable)
{
    cJSON* logSection = NULL;
    logSection = cJSON_GetObjectItemCaseSensitive(root, RECORD_ENABLE);
    if (logSection == NULL)
    {
        cJSON_AddBoolToObject(root, RECORD_ENABLE, enable);
    }
}

int getPciRegister(crashdump::CPUInfo& cpuInfo, SRegRawData* sRegData,
                   uint8_t u8index)
{
    int peci_fd = -1;
    int ret = 0;
    uint16_t u16Offset = 0;
    uint8_t u8Size = 0;
    ret = peci_Lock(&peci_fd, PECI_WAIT_FOREVER);
    if (ret != PECI_CC_SUCCESS)
    {
        sRegData->ret = ret;
        return ACD_FAILURE;
    }
    switch (sPciReg[u8index].u8Size)
    {
        case UT_REG_DWORD:
            ret = peci_RdEndPointConfigPciLocal_seq(
                cpuInfo.clientAddr, sPciReg[u8index].u8Seg,
                sPciReg[u8index].u8Bus, sPciReg[u8index].u8Dev,
                sPciReg[u8index].u8Func, sPciReg[u8index].u16Reg,
                sPciReg[u8index].u8Size, (uint8_t*)&sRegData->uValue.u64,
                peci_fd, &sRegData->cc);
            sRegData->ret = ret;
            if (ret != PECI_CC_SUCCESS)
            {
                peci_Unlock(peci_fd);
                return ACD_FAILURE;
            }
            sRegData->uValue.u64 &= 0xFFFFFFFF;
            break;
        case UT_REG_QWORD:
            for (uint8_t u8Dword = 0; u8Dword < 2; u8Dword++)
            {
                u16Offset = ((sPciReg[u8index].u16Reg) >> (u8Dword * 8)) & 0xFF;
                u8Size = sPciReg[u8index].u8Size / 2;
                ret = peci_RdEndPointConfigPciLocal_seq(
                    cpuInfo.clientAddr, sPciReg[u8index].u8Seg,
                    sPciReg[u8index].u8Bus, sPciReg[u8index].u8Dev,
                    sPciReg[u8index].u8Func, u16Offset, u8Size,
                    (uint8_t*)&sRegData->uValue.u32[u8Dword], peci_fd,
                    &sRegData->cc);
                sRegData->ret = ret;
                if (ret != PECI_CC_SUCCESS)
                {
                    peci_Unlock(peci_fd);
                    return ACD_FAILURE;
                }
            }
            break;
        default:
            ret = ACD_FAILURE;
    }
    peci_Unlock(peci_fd);
    return ACD_SUCCESS;
}

struct timespec calculateDelay(struct timespec* crashdumpStart,
                               uint32_t delayTimeFromInputFileInSec)
{
    struct timespec current = {0, 0};
    clock_gettime(CLOCK_MONOTONIC, &current);

    uint64_t runTimeInNs =
        tsToNanosecond(&current) - tsToNanosecond(crashdumpStart);

    uint64_t delayTimeFromInputFileInNs =
        delayTimeFromInputFileInSec * (uint64_t)1e9;

    struct timespec delay = {0, 0};
    if (runTimeInNs < delayTimeFromInputFileInNs)
    {
        delay.tv_sec = (delayTimeFromInputFileInNs - runTimeInNs) / 1e9;
        delay.tv_nsec =
            (delayTimeFromInputFileInNs - runTimeInNs) % (uint64_t)1e9;

        return delay;
    }

    return delay;
}

uint32_t getDelayFromInputFile(crashdump::CPUInfo& cpuInfo, char* sectionName)
{
    bool enable = false;
    cJSON* inputDelayField = getCrashDataSectionObjectOneLevel(
        cpuInfo.inputFile.bufferPtr, sectionName, "_max_wait_sec", &enable);

    if (inputDelayField != NULL && enable)
    {
        return (uint32_t)inputDelayField->valueint;
    }

    return 0;
}

bool getTorSkipFromInputFile(crashdump::CPUInfo& cpuInfo, char* sectionName)
{
    bool enable = false;
    cJSON* skipIfFailRead = getCrashDataSectionObjectOneLevel(
        cpuInfo.inputFile.bufferPtr, sectionName, "_skip_cha_on_fail", &enable);

    if (skipIfFailRead != NULL && enable)
    {
        return cJSON_IsTrue(skipIfFailRead);
    }

    return false;
}

uint64_t tsToNanosecond(timespec* ts)
{
    return (ts->tv_sec * (uint64_t)1e9 + ts->tv_nsec);
}

inputField getFlagValueFromInputFile(crashdump::CPUInfo& cpuInfo,
                                     char* sectionName, char* flagName)
{
    bool enable;
    cJSON* flagField = getCrashDataSectionObjectOneLevel(
        cpuInfo.inputFile.bufferPtr, sectionName, flagName, &enable);

    if (flagField != NULL && enable)
    {
        return (cJSON_IsTrue(flagField) ? inputField::FLAG_ENABLE
                                        : inputField::FLAG_DISABLE);
    }

    return inputField::FLAG_NOT_PRESENT;
}
