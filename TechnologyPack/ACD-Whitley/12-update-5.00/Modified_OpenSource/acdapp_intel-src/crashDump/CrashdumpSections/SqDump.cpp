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

#include "SqDump.hpp"

extern "C" {
#ifndef SPX_BMC_ACD
#include <cjson/cJSON.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
}

#include "crashdump.hpp"
#include "utils.hpp"

/******************************************************************************
 *
 *   sqDumpJson
 *
 *   This function formats the SQ Dump log into a JSON object
 *
 ******************************************************************************/
static void sqDumpJson(uint32_t u32CoreNum, SSqDump* psSqDump,
                       cJSON* pJsonChild)
{
    cJSON* core;
    cJSON* sq;
    char jsonItemName[SQ_JSON_STRING_LEN];
    char jsonItemString[SQ_JSON_STRING_LEN];
    uint32_t i;
    uint32_t index = 0;

    // Only add the section if there is data to include
    if (psSqDump->u32SqAddrSize == 0 && psSqDump->u32SqCtrlSize == 0)
    {
        return;
    }

    // Add the core number item to the SQ dump JSON structure only if it doesn't
    // already exist
    cd_snprintf_s(jsonItemName, SQ_JSON_STRING_LEN, SQ_JSON_CORE_NAME,
                  u32CoreNum);
    if ((core = cJSON_GetObjectItemCaseSensitive(pJsonChild, jsonItemName)) ==
        NULL)
    {
        cJSON_AddItemToObject(pJsonChild, jsonItemName,
                              core = cJSON_CreateObject());
    }

    // Add the the SQ dump item to the JSON structure only if it doesn't already
    // exist
    if ((sq = cJSON_GetObjectItemCaseSensitive(core, "SQ")) == NULL)
    {
        cJSON_AddItemToObject(core, "SQ", sq = cJSON_CreateObject());
    }

    // Add the Address Array data if it exists
    if (psSqDump->pu32SqAddrArray != NULL)
    {
        // Log the data as 64 bit entries
        for (i = 0; (i + 1) < psSqDump->u32SqAddrSize; i += 2)
        {
            cd_snprintf_s(jsonItemName, SQ_JSON_STRING_LEN, SQ_JSON_ENTRY_NAME,
                          index++);
            if ((psSqDump->puSqAddrRet[i] != PECI_CC_SUCCESS) ||
                (psSqDump->puSqAddrRet[i + 1] != PECI_CC_SUCCESS))
            {
                cd_snprintf_s(
                    jsonItemString, SQ_JSON_STRING_LEN, SQ_UA_DF_CPX,
                    psSqDump->pu8SqAddrCc[i], psSqDump->pu8SqAddrCc[i + 1],
                    psSqDump->puSqAddrRet[i], psSqDump->puSqAddrRet[i + 1]);
                cJSON_AddStringToObject(sq, jsonItemName, jsonItemString);
                break;
            }
            else if (PECI_CC_UA(psSqDump->pu8SqAddrCc[i]) ||
                     PECI_CC_UA(psSqDump->pu8SqAddrCc[i + 1]))
            {
                cd_snprintf_s(jsonItemString, SQ_JSON_STRING_LEN, SQ_UA_CPX,
                              psSqDump->pu32SqAddrArray[i],
                              psSqDump->pu32SqAddrArray[i + 1],
                              psSqDump->pu8SqAddrCc[i],
                              psSqDump->pu8SqAddrCc[i + 1]);
                cJSON_AddStringToObject(sq, jsonItemName, jsonItemString);
                break;
            }
            else
            {
                cd_snprintf_s(jsonItemString, SQ_JSON_STRING_LEN, "0x%x%08x",
                              psSqDump->pu32SqAddrArray[i],
                              psSqDump->pu32SqAddrArray[i + 1]);
                cJSON_AddStringToObject(sq, jsonItemName, jsonItemString);
            }
        }
    }
    // Add the Control Array data if it exists
    if (psSqDump->pu32SqCtrlArray != NULL)
    {
        // Log the data as 64 bit entries
        for (i = 0; (i + 1) < psSqDump->u32SqCtrlSize; i += 2)
        {
            cd_snprintf_s(jsonItemName, SQ_JSON_STRING_LEN, SQ_JSON_ENTRY_NAME,
                          index++);
            if ((psSqDump->puSqCtrlRet[i] != PECI_CC_SUCCESS) ||
                (psSqDump->puSqCtrlRet[i + 1] != PECI_CC_SUCCESS))
            {
                cd_snprintf_s(
                    jsonItemString, SQ_JSON_STRING_LEN, SQ_UA_DF_CPX,
                    psSqDump->pu8SqCtrlCc[i], psSqDump->pu8SqCtrlCc[i + 1],
                    psSqDump->puSqCtrlRet[i], psSqDump->puSqCtrlRet[i + 1]);
                cJSON_AddStringToObject(sq, jsonItemName, jsonItemString);
                break;
            }
            else if ((PECI_CC_UA(psSqDump->pu8SqCtrlCc[i])) ||
                     PECI_CC_UA(psSqDump->pu8SqCtrlCc[i + 1]))
            {
                cd_snprintf_s(jsonItemString, SQ_JSON_STRING_LEN, SQ_UA_CPX,
                              psSqDump->pu32SqCtrlArray[i],
                              psSqDump->pu32SqCtrlArray[i + 1],
                              psSqDump->pu8SqCtrlCc[i],
                              psSqDump->pu8SqCtrlCc[i + 1]);
                cJSON_AddStringToObject(sq, jsonItemName, jsonItemString);
                break;
            }
            else
            {
                cd_snprintf_s(jsonItemString, SQ_JSON_STRING_LEN, "0x%x%08x",
                              psSqDump->pu32SqCtrlArray[i],
                              psSqDump->pu32SqCtrlArray[i + 1]);
                cJSON_AddStringToObject(sq, jsonItemName, jsonItemString);
            }
        }
    }
}

/******************************************************************************
 *
 *   sqDump
 *
 *   This function gathers the specified type of SQ Dump
 *
 ******************************************************************************/
static int sqDump(crashdump::CPUInfo& cpuInfo, uint32_t u32CoreNum,
                  uint32_t u32SqType, uint32_t** ppu32SqDumpRet,
                  uint32_t* pu32SqDumpSize, int peci_fd, uint8_t** ppu8Cc,
                  int** ppuRet)
{
    uint8_t cc = 0;
    int ret = 0;

    // Open the SQ dump sequence
    ret = peci_WrPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU, VCU_OPEN_SEQ,
                               VCU_SQ_DUMP_SEQ, sizeof(uint32_t), peci_fd, &cc);
    if (ret != PECI_CC_SUCCESS)
    {
        // SQ dump sequence failed, abort the sequence and go to the next CPU
        peci_WrPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU, VCU_ABORT_SEQ,
                             VCU_SQ_DUMP_SEQ, sizeof(uint32_t), peci_fd, &cc);
        return ret;
    }

    // Set SQ dump parameter 0
    ret = peci_WrPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU, VCU_SET_PARAM,
                               (u32SqType | u32CoreNum), sizeof(uint32_t),
                               peci_fd, &cc);
    if (ret != PECI_CC_SUCCESS)
    {
        // SQ dump sequence failed, abort the sequence and go to the next CPU
        peci_WrPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU, VCU_ABORT_SEQ,
                             VCU_SQ_DUMP_SEQ, sizeof(uint32_t), peci_fd, &cc);
        return ret;
    }

    // Get the number of dwords to read
    uint32_t u32NumReads = 0;
    ret = peci_RdPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU,
                               SQ_START_PARAM, sizeof(uint32_t),
                               (uint8_t*)&u32NumReads, peci_fd, &cc);
    if (ret != PECI_CC_SUCCESS || (PECI_CC_UA(cc)))
    {
        // SQ dump sequence failed, abort the sequence and go to the next CPU
        peci_WrPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU, VCU_ABORT_SEQ,
                             VCU_SQ_DUMP_SEQ, sizeof(uint32_t), peci_fd, &cc);
        return ret;
    }
    // Get the raw data
    uint32_t* pu32SqDump = NULL;
    uint8_t* pu8Cc = NULL;
    int* puRet = NULL;
    pu32SqDump = (uint32_t*)calloc(u32NumReads, sizeof(uint32_t));
    pu8Cc = (uint8_t*)calloc(u32NumReads, sizeof(uint8_t));
    puRet = (int*)calloc(u32NumReads, sizeof(int));
    if (pu32SqDump == NULL || pu8Cc == NULL || puRet == NULL)
    {
        // calloc failed, abort the sequence and go to the next CPU
        peci_WrPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU, VCU_ABORT_SEQ,
                             VCU_SQ_DUMP_SEQ, sizeof(uint32_t), peci_fd, &cc);
        return SIZE_FAILURE;
    }
    for (uint32_t i = 0; i < u32NumReads; i++)
    {
        ret = peci_RdPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU, VCU_READ,
                                   sizeof(uint32_t), (uint8_t*)&pu32SqDump[i],
                                   peci_fd, &pu8Cc[i]);
        puRet[i] = ret;
        if (ret != PECI_CC_SUCCESS)
        {
            // SQ dump sequence failed, note the number of dwords read and abort
            // the sequence
            u32NumReads = i;
            peci_WrPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU,
                                 VCU_ABORT_SEQ, VCU_SQ_DUMP_SEQ,
                                 sizeof(uint32_t), peci_fd, &cc);
            break;
        }
    }

    // Close the SQ dump sequence
    peci_WrPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU, VCU_CLOSE_SEQ,
                         VCU_SQ_DUMP_SEQ, sizeof(uint32_t), peci_fd, &cc);

    // Set the return data
    *ppu32SqDumpRet = pu32SqDump;
    *pu32SqDumpSize = u32NumReads;
    *ppu8Cc = pu8Cc;
    *ppuRet = puRet;
    return ret;
}

/******************************************************************************
 *
 *   logSqDumpCPX1
 *
 *   This function gathers the SQ Dump and adds it to the debug log.
 *   The PECI flow is listed below to generate a SQ Dump.
 *
 *    WrPkgConfig() -
 *         0x80 0x0003 0x00030004
 *         Open SQ Dump Sequence
 *
 *    WrPkgConfig() -
 *         0x80 0x0001 0x80000000
 *         Set Parameter 0
 *
 *    RdPkgConfig() -
 *         0x80 0x3002
 *         Returns the number of additional RdPkgConfig() commands required to
 *collect all the data for the dump.
 *
 *    RdPkgConfig() * N -
 *         0x80 0x0002
 *         SQ Dump data [1-N]
 *
 *    WrPkgConfig() -
 *         0x80 0x0004 0x00030004 Close SQ Dump Sequence.
 *
 ******************************************************************************/
int logSqDumpCPX1(crashdump::CPUInfo& cpuInfo, cJSON* pJsonChild)
{
    int peci_fd = -1;
    int ret = 0;
    int retval = 0;

    ret = peci_Lock(&peci_fd, PECI_WAIT_FOREVER);
    if (ret != PECI_CC_SUCCESS)
    {
        return ret;
    }

    // Start the SQ dump log
    // Go through each enabled core and read from thread 0
    for (uint32_t u32CoreNum = 0; (cpuInfo.coreMask >> u32CoreNum) != 0;
         u32CoreNum++)
    {
        if (!CHECK_BIT(cpuInfo.coreMask, u32CoreNum))
        {
            continue;
        }
        SSqDump sSqDump = {};

        // Get the SQ Dump Address Array data
        ret = sqDump(cpuInfo, u32CoreNum, SQ_ADDR_ARRAY,
                     &sSqDump.pu32SqAddrArray, &sSqDump.u32SqAddrSize, peci_fd,
                     &sSqDump.pu8SqAddrCc, &sSqDump.puSqAddrRet);
        if (ret != 0)
        {
            retval = ret;
        }

        // Get the SQ Dump Control Array data
        ret = sqDump(cpuInfo, u32CoreNum, SQ_CTRL_ARRAY,
                     &sSqDump.pu32SqCtrlArray, &sSqDump.u32SqCtrlSize, peci_fd,
                     &sSqDump.pu8SqCtrlCc, &sSqDump.puSqCtrlRet);
        if (ret != 0)
        {
            retval = ret;
        }

        // Log the SQ dump for this Core
        if (retval == 0)
        {
            sqDumpJson(u32CoreNum, &sSqDump, pJsonChild);
        }
        // Free any allocated memory
        if (sSqDump.pu32SqAddrArray)
        {
            free(sSqDump.pu32SqAddrArray);
        }
        if (sSqDump.pu8SqAddrCc)
        {
            free(sSqDump.pu8SqAddrCc);
        }
        if (sSqDump.puSqAddrRet)
        {
            free(sSqDump.puSqAddrRet);
        }
        if (sSqDump.pu32SqCtrlArray)
        {
            free(sSqDump.pu32SqCtrlArray);
        }
        if (sSqDump.pu8SqCtrlCc)
        {
            free(sSqDump.pu8SqCtrlCc);
        }
        if (sSqDump.puSqCtrlRet)
        {
            free(sSqDump.puSqCtrlRet);
        }
    }
    peci_Unlock(peci_fd);

    // Cores that have no SQ data will return a PECI failure, so we have to scan
    // all the cores. If any core has data, we should return success. If no
    // cores have data, return failure.
    if (retval != PECI_CC_SUCCESS)
    {
        return retval;
    }
    return ret;
}

/******************************************************************************
 *
 *   logSqDumpICX1
 *
 *   The ICX1 SQ dump is part of the core info section, so this does nothing
 *
 ******************************************************************************/
int logSqDumpICX1(crashdump::CPUInfo& cpuInfo, cJSON* pJsonChild)
{
    (void)cpuInfo;
    (void)pJsonChild;
    return 0;
}

static const SSqDumpVx sSqDumpVx[] = {
    {crashdump::cpu::clx, logSqDumpCPX1},
    {crashdump::cpu::cpx, logSqDumpCPX1},
    {crashdump::cpu::skx, logSqDumpCPX1},
    {crashdump::cpu::icx, logSqDumpICX1},
    {crashdump::cpu::icx2, logSqDumpICX1},
    {crashdump::cpu::icxd, logSqDumpICX1},
};

/******************************************************************************
 *
 *    logSqDump
 *
 *    BMC performs the SQ dump retrieve from the processor directly via
 *    PECI interface after a platform three (3) strike failure.
 *
 ******************************************************************************/
int logSqDump(crashdump::CPUInfo& cpuInfo, cJSON* pJsonChild)
{
    if (pJsonChild == NULL)
    {
        return 1;
    }

    for (size_t i = 0; i < (sizeof(sSqDumpVx) / sizeof(SSqDumpVx)); i++)
    {
        if (cpuInfo.model == sSqDumpVx[i].cpuModel)
        {
            return sSqDumpVx[i].logSqDumpVx(cpuInfo, pJsonChild);
        }
    }

    CRASHDUMP_PRINT(ERR, stderr, "Cannot find version for %s\n", __FUNCTION__);
    return 1;
}
