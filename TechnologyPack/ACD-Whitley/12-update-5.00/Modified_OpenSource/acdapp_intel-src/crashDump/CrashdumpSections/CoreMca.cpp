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

#include "CoreMca.hpp"

extern "C" {
#ifndef SPX_BMC_ACD
#include <cjson/cJSON.h>
#endif
#include <stdio.h>
#include <string.h>
}

#include "crashdump.hpp"
#include "utils.hpp"

/******************************************************************************
 *
 *   coreMcaJsonCPX1
 *
 *   This function formats the Core MCA into a JSON object
 *
 ******************************************************************************/
static void coreMcaJsonCPX1(uint32_t u32CoreNum,
                            SCoreMcaRawData* sCoreMcaRawData, cJSON* pJsonChild)
{
    cJSON* core;
    cJSON* thread;
    cJSON* coreMca;
    char jsonItemName[CORE_MCA_JSON_STRING_LEN];
    char jsonItemString[CORE_MCA_JSON_STRING_LEN];
    uint32_t i;

    // Add the core number item to the Core MCA JSON structure
    cd_snprintf_s(jsonItemString, CORE_MCA_JSON_STRING_LEN,
                  CORE_MCA_JSON_CORE_NAME, u32CoreNum);
    cJSON_AddItemToObject(pJsonChild, jsonItemString,
                          core = cJSON_CreateObject());

    // Add the thread number item to the Core MCA JSON structure (always thread
    // 0 for v1)
    cd_snprintf_s(jsonItemString, CORE_MCA_JSON_STRING_LEN,
                  CORE_MCA_JSON_THREAD_NAME, 0);
    cJSON_AddItemToObject(core, jsonItemString, thread = cJSON_CreateObject());

    // Format the Core MCA data out to the .json debug file
    for (i = FIRST_CORE_MCA; i <= LAST_CORE_MCA; i++)
    {
        // Add the MCA number item to the Core MCA JSON structure
        cd_snprintf_s(jsonItemString, CORE_MCA_JSON_STRING_LEN,
                      CORE_MCA_JSON_MCA_NAME, i);
        cJSON_AddItemToObject(thread, jsonItemString,
                              coreMca = cJSON_CreateObject());

        // Fill in NULL for this MCE Bank if it's not valid
        if (sCoreMcaRawData[i].bInvalid)
        {
            cd_snprintf_s(jsonItemString, CORE_MCA_JSON_STRING_LEN,
                          CORE_MCA_UA_DF_CPX, sCoreMcaRawData[i].cc,
                          sCoreMcaRawData[i].ret);
            cd_snprintf_s(jsonItemName, CORE_MCA_JSON_STRING_LEN,
                          CORE_MCA_REG_NAME, i, coreMcaRegNames[CORE_CTL]);
            cJSON_AddStringToObject(coreMca, jsonItemName, jsonItemString);
            cd_snprintf_s(jsonItemName, CORE_MCA_JSON_STRING_LEN,
                          CORE_MCA_REG_NAME, i, coreMcaRegNames[CORE_STATUS]);
            cJSON_AddStringToObject(coreMca, jsonItemName, jsonItemString);
            cd_snprintf_s(jsonItemName, CORE_MCA_JSON_STRING_LEN,
                          CORE_MCA_REG_NAME, i, coreMcaRegNames[CORE_ADDR]);
            cJSON_AddStringToObject(coreMca, jsonItemName, jsonItemString);
            cd_snprintf_s(jsonItemName, CORE_MCA_JSON_STRING_LEN,
                          CORE_MCA_REG_NAME, i, coreMcaRegNames[CORE_MISC]);
            cJSON_AddStringToObject(coreMca, jsonItemName, jsonItemString);
            cd_snprintf_s(jsonItemName, CORE_MCA_JSON_STRING_LEN,
                          CORE_MCA_REG_NAME, i, coreMcaRegNames[CORE_CTL2]);
            cJSON_AddStringToObject(coreMca, jsonItemName, jsonItemString);
        }
        else if (PECI_CC_UA(sCoreMcaRawData[i].cc))
        {
            cd_snprintf_s(jsonItemString, CORE_MCA_JSON_STRING_LEN,
                          CORE_MCA_UA_CPX,
                          sCoreMcaRawData[i].uRegData.sReg.u64CoreMcaCtl,
                          sCoreMcaRawData[i].cc);
            cd_snprintf_s(jsonItemName, CORE_MCA_JSON_STRING_LEN,
                          CORE_MCA_REG_NAME, i, coreMcaRegNames[CORE_CTL]);
            cJSON_AddStringToObject(coreMca, jsonItemName, jsonItemString);
            cd_snprintf_s(jsonItemString, CORE_MCA_JSON_STRING_LEN,
                          CORE_MCA_UA_CPX,
                          sCoreMcaRawData[i].uRegData.sReg.u64CoreMcaStatus,
                          sCoreMcaRawData[i].cc);
            cd_snprintf_s(jsonItemName, CORE_MCA_JSON_STRING_LEN,
                          CORE_MCA_REG_NAME, i, coreMcaRegNames[CORE_STATUS]);
            cJSON_AddStringToObject(coreMca, jsonItemName, jsonItemString);
            cd_snprintf_s(jsonItemString, CORE_MCA_JSON_STRING_LEN,
                          CORE_MCA_UA_CPX,
                          sCoreMcaRawData[i].uRegData.sReg.u64CoreMcaAddr,
                          sCoreMcaRawData[i].cc);
            cd_snprintf_s(jsonItemName, CORE_MCA_JSON_STRING_LEN,
                          CORE_MCA_REG_NAME, i, coreMcaRegNames[CORE_ADDR]);
            cJSON_AddStringToObject(coreMca, jsonItemName, jsonItemString);
            cd_snprintf_s(jsonItemString, CORE_MCA_JSON_STRING_LEN,
                          CORE_MCA_UA_CPX,
                          sCoreMcaRawData[i].uRegData.sReg.u64CoreMcaMisc,
                          sCoreMcaRawData[i].cc);
            cd_snprintf_s(jsonItemName, CORE_MCA_JSON_STRING_LEN,
                          CORE_MCA_REG_NAME, i, coreMcaRegNames[CORE_MISC]);
            cJSON_AddStringToObject(coreMca, jsonItemName, jsonItemString);
            cd_snprintf_s(jsonItemString, CORE_MCA_JSON_STRING_LEN,
                          CORE_MCA_UA_CPX,
                          sCoreMcaRawData[i].uRegData.sReg.u64CoreMcaCtl2,
                          sCoreMcaRawData[i].cc);
            cd_snprintf_s(jsonItemName, CORE_MCA_JSON_STRING_LEN,
                          CORE_MCA_REG_NAME, i, coreMcaRegNames[CORE_CTL2]);
            cJSON_AddStringToObject(coreMca, jsonItemName, jsonItemString);
        }
        // Otherwise fill in the register data
        else
        {
            cd_snprintf_s(jsonItemName, CORE_MCA_JSON_STRING_LEN,
                          CORE_MCA_REG_NAME, i, coreMcaRegNames[CORE_CTL]);
            cd_snprintf_s(jsonItemString, CORE_MCA_JSON_STRING_LEN, "0x%llx",
                          sCoreMcaRawData[i].uRegData.sReg.u64CoreMcaCtl);
            cJSON_AddStringToObject(coreMca, jsonItemName, jsonItemString);
            cd_snprintf_s(jsonItemName, CORE_MCA_JSON_STRING_LEN,
                          CORE_MCA_REG_NAME, i, coreMcaRegNames[CORE_STATUS]);
            cd_snprintf_s(jsonItemString, CORE_MCA_JSON_STRING_LEN, "0x%llx",
                          sCoreMcaRawData[i].uRegData.sReg.u64CoreMcaStatus);
            cJSON_AddStringToObject(coreMca, jsonItemName, jsonItemString);
            cd_snprintf_s(jsonItemName, CORE_MCA_JSON_STRING_LEN,
                          CORE_MCA_REG_NAME, i, coreMcaRegNames[CORE_ADDR]);
            cd_snprintf_s(jsonItemString, CORE_MCA_JSON_STRING_LEN, "0x%llx",
                          sCoreMcaRawData[i].uRegData.sReg.u64CoreMcaAddr);
            cJSON_AddStringToObject(coreMca, jsonItemName, jsonItemString);
            cd_snprintf_s(jsonItemName, CORE_MCA_JSON_STRING_LEN,
                          CORE_MCA_REG_NAME, i, coreMcaRegNames[CORE_MISC]);
            cd_snprintf_s(jsonItemString, CORE_MCA_JSON_STRING_LEN, "0x%llx",
                          sCoreMcaRawData[i].uRegData.sReg.u64CoreMcaMisc);
            cJSON_AddStringToObject(coreMca, jsonItemName, jsonItemString);
            cd_snprintf_s(jsonItemName, CORE_MCA_JSON_STRING_LEN,
                          CORE_MCA_REG_NAME, i, coreMcaRegNames[CORE_CTL2]);
            cd_snprintf_s(jsonItemString, CORE_MCA_JSON_STRING_LEN, "0x%llx",
                          sCoreMcaRawData[i].uRegData.sReg.u64CoreMcaCtl2);
            cJSON_AddStringToObject(coreMca, jsonItemName, jsonItemString);
        }
    }
}

/******************************************************************************
 *
 *   logCoreMcaCPX1
 *
 *   This function gathers the Core MCA register contents and adds them to the
 *   debug log.  The PECI flow is listed below to dump an MCE bank
 *
 *    WrPkgConfig() -
 *         0x80 0x0003 0x00010000
 *         Open Machine Check Dump Sequence
 *
 *    WrPkgConfig() -
 *         0x80 0x1000 [15:8] Core Number, [7:0] Bank Number
 *         Select the MCA bank
 *
 *    RdPkgConfig() -
 *         0x80 0x0002
 *         CTL[31:0]
 *
 *    RdPkgConfig() -
 *         0x80 0x0002
 *         CTL[63:32]
 *
 *    RdPkgConfig() -
 *         0x80 0x0002
 *         STATUS[31:0]
 *
 *    RdPkgConfig() -
 *         0x80 0x0002
 *         STATUS[63:32]
 *
 *    RdPkgConfig() -
 *         0x80 0x0002
 *         ADDR[31:0]
 *
 *    RdPkgConfig() -
 *         0x80 0x0002
 *         ADDR[63:32]
 *
 *    RdPkgConfig() -
 *         0x80 0x0002
 *         MISC[31:0]
 *
 *    RdPkgConfig() -
 *         0x80 0x0002
 *         MISC[63:32]
 *
 *    RdPkgConfig() -
 *         0x80 0x0002
 *         CTL2[31:0]
 *
 *    RdPkgConfig() -
 *         0x80 0x0002
 *         CTL2[63:32]
 *
 *    WrPkgConfig() -
 *         0x80 0x0004 0x00010000 Close Machine Check Dump Sequence.
 *
 ******************************************************************************/
int logCoreMcaCPX1(crashdump::CPUInfo& cpuInfo, cJSON* pJsonChild)
{
    int ret = 0;
    int peci_fd = -1;
    uint8_t cc = 0;

    ret = peci_Lock(&peci_fd, PECI_WAIT_FOREVER);
    if (ret != PECI_CC_SUCCESS)
    {
        return ret;
    }

    // Go through each enabled core and read from thread 0
    for (uint32_t u32CoreNum = 0; (cpuInfo.coreMask >> u32CoreNum) != 0;
         u32CoreNum++)
    {
        if (!CHECK_BIT(cpuInfo.coreMask, u32CoreNum))
        {
            continue;
        }
        SCoreMcaRawData sCoreMcaRawData[LAST_CORE_MCA + 1]{};

        // Read the Core MCA registers from the CPU
        for (uint32_t j = FIRST_CORE_MCA; j <= LAST_CORE_MCA; j++)
        {
            // Open the MCA Bank dump sequence
            ret = peci_WrPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU,
                                       VCU_OPEN_SEQ, VCU_CORE_MCA_SEQ,
                                       sizeof(uint32_t), peci_fd, &cc);
            sCoreMcaRawData[j].cc = cc;
            sCoreMcaRawData[j].ret = ret;
            if (ret != PECI_CC_SUCCESS)
            {
                // MCE Bank sequence failed, abort the sequence and go to the
                // next Bank
                peci_WrPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU,
                                     VCU_ABORT_SEQ, VCU_CORE_MCA_SEQ,
                                     sizeof(uint32_t), peci_fd, &cc);
                continue;
            }

            // Set MCE Bank number
            ret = peci_WrPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU,
                                       CM_BANK_PARAM, (u32CoreNum << 8) | j,
                                       sizeof(uint32_t), peci_fd, &cc);
            sCoreMcaRawData[j].cc = cc;
            sCoreMcaRawData[j].ret = ret;
            if (ret != PECI_CC_SUCCESS)
            {
                // MCE Bank sequence failed, abort the sequence and go to the
                // next Bank
                peci_WrPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU,
                                     VCU_ABORT_SEQ, VCU_CORE_MCA_SEQ,
                                     sizeof(uint32_t), peci_fd, &cc);
                continue;
            }

            // Get the MCE Bank Registers
            for (uint8_t u8Dword = 0; u8Dword < CM_NUM_MCA_DWORDS; u8Dword++)
            {
                ret = peci_RdPkgConfig_seq(
                    cpuInfo.clientAddr, MBX_INDEX_VCU, VCU_READ,
                    sizeof(uint32_t),
                    (uint8_t*)&sCoreMcaRawData[j].uRegData.u32Raw[u8Dword],
                    peci_fd, &cc);
                sCoreMcaRawData[j].cc = cc;
                sCoreMcaRawData[j].ret = ret;
                if (ret != PECI_CC_SUCCESS)
                {
                    // MCE Bank sequence failed, abort the sequence and go to
                    // the next Bank
                    peci_WrPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU,
                                         VCU_ABORT_SEQ, VCU_CORE_MCA_SEQ,
                                         sizeof(uint32_t), peci_fd, &cc);
                    sCoreMcaRawData[j].bInvalid = true;
                    break;
                }
            }

            // Close the MCE Bank sequence
            peci_WrPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU,
                                 VCU_CLOSE_SEQ, VCU_CORE_MCA_SEQ,
                                 sizeof(uint32_t), peci_fd, &cc);
        }
        // Log the Core MCA for this CPU
        coreMcaJsonCPX1(u32CoreNum, sCoreMcaRawData, pJsonChild);
    }

    peci_Unlock(peci_fd);
    return ret;
}

/******************************************************************************
 *
 *   coreMcaJsonICX1
 *
 *   This function formats the Core MCA into a JSON object
 *
 ******************************************************************************/
static void coreMcaJsonICX1(uint32_t u32CoreNum, uint32_t u32ThreadNum,
                            const SCoreMcaReg* sCoreMcaReg,
                            uint64_t u64CoreMcaData, cJSON* pJsonChild,
                            uint8_t cc, int ret, bool isValid)
{
    char jsonItemString[CORE_MCA_JSON_STRING_LEN];

    // Add the core number item to the Core MCA JSON structure only if it
    // doesn't already exist
    cJSON* core;
    cd_snprintf_s(jsonItemString, CORE_MCA_JSON_STRING_LEN,
                  CORE_MCA_JSON_CORE_NAME, u32CoreNum);
    if ((core = cJSON_GetObjectItemCaseSensitive(pJsonChild, jsonItemString)) ==
        NULL)
    {
        cJSON_AddItemToObject(pJsonChild, jsonItemString,
                              core = cJSON_CreateObject());
    }

    // Add the thread number item to the Core MCA JSON structure only if it
    // doesn't already exist
    cJSON* thread;
    cd_snprintf_s(jsonItemString, CORE_MCA_JSON_STRING_LEN,
                  CORE_MCA_JSON_THREAD_NAME, u32ThreadNum);
    if ((thread = cJSON_GetObjectItemCaseSensitive(core, jsonItemString)) ==
        NULL)
    {
        cJSON_AddItemToObject(core, jsonItemString,
                              thread = cJSON_CreateObject());
    }

    // Add the MCA number item to the Core MCA JSON structure only if it
    // doesn't already exist
    cJSON* coreMca;
    if ((coreMca = cJSON_GetObjectItemCaseSensitive(
             thread, sCoreMcaReg->bankName)) == NULL)
    {
        cJSON_AddItemToObject(thread, sCoreMcaReg->bankName,
                              coreMca = cJSON_CreateObject());
    }

    if (!isValid)
    {
        cd_snprintf_s(jsonItemString, CORE_MCA_JSON_STRING_LEN, CORE_MCA_NA,
                      cc);
        cJSON_AddStringToObject(coreMca, sCoreMcaReg->regName, jsonItemString);
        return;
    }

    // Add the MCA register data to the Core MCA JSON structure
    if (ret != PECI_CC_SUCCESS)
    {
        cd_snprintf_s(jsonItemString, CORE_MCA_JSON_STRING_LEN, CORE_MCA_UA_DF,
                      cc, ret);
    }
    else if (PECI_CC_UA(cc))
    {
        cd_snprintf_s(jsonItemString, CORE_MCA_JSON_STRING_LEN, CORE_MCA_UA,
                      cc);
    }
    else
    {
        // Add the MCA register data to the Core MCA JSON structure
        cd_snprintf_s(jsonItemString, CORE_MCA_JSON_STRING_LEN,
                      CORE_MCA_UINT64_FMT, u64CoreMcaData);
    }

    // Remove initial N/A entry
    cJSON* tmp = NULL;
    tmp = cJSON_GetObjectItemCaseSensitive(coreMca, sCoreMcaReg->regName);
    if (tmp != NULL)
    {
        cJSON_DeleteItemFromObjectCaseSensitive(coreMca, tmp->string);
    }
    cJSON_AddStringToObject(coreMca, sCoreMcaReg->regName, jsonItemString);
}

static const SCoreMcaReg sCoreMcaRegs[] = {
    // Bank, Register Name, Address

    // MC0
    {"MC0", "mc0_status", 0x401},
    {"MC0", "mc0_ctl", 0x400},
    {"MC0", "mc0_addr", 0x402},
    {"MC0", "mc0_misc", 0x403},
    {"MC0", "mc0_ctl2", 0x280},

    // MC1
    {"MC1", "mc1_status", 0x405},
    {"MC1", "mc1_ctl", 0x404},
    {"MC1", "mc1_addr", 0x406},
    {"MC1", "mc1_misc", 0x407},
    {"MC1", "mc1_ctl2", 0x281},

    // MC2
    {"MC2", "mc2_status", 0x409},
    {"MC2", "mc2_ctl", 0x408},
    {"MC2", "mc2_addr", 0x40a},
    {"MC2", "mc2_misc", 0x40b},
    {"MC2", "mc2_ctl2", 0x282},

    // MC3
    {"MC3", "mc3_status", 0x40d},
    {"MC3", "mc3_ctl", 0x40c},
    {"MC3", "mc3_addr", 0x40e},
    {"MC3", "mc3_misc", 0x40f},
    {"MC3", "mc3_ctl2", 0x283},
};

/******************************************************************************
 *
 *   logCoreMcaICX1
 *
 *   This function gathers the Core MCA register contents and adds them to the
 *   debug log.
 *
 ******************************************************************************/
int logCoreMcaICX1(crashdump::CPUInfo& cpuInfo, cJSON* pJsonChild)
{
    int ret = 0;

    // Go through each enabled core
    for (uint32_t u32CoreNum = 0; (cpuInfo.coreMask >> u32CoreNum) != 0;
         u32CoreNum++)
    {
        if (!CHECK_BIT(cpuInfo.coreMask, u32CoreNum))
        {
            continue;
        }

        if (CHECK_BIT(cpuInfo.crashedCoreMask, u32CoreNum))
        {
            continue;
        }

        // Go through each thread on this core
        for (uint32_t u32ThreadNum = 0;
             u32ThreadNum < CORE_MCA_THREADS_PER_CORE; u32ThreadNum++)
        {
            uint64_t u64CoreMcaData = 0;
            uint8_t cc = 0;

            // Fill json with N/A first
            for (uint32_t i = 0;
                 i < (sizeof(sCoreMcaRegs) / sizeof(SCoreMcaReg)); i++)
            {
                coreMcaJsonICX1(u32CoreNum, u32ThreadNum, &sCoreMcaRegs[i],
                                u64CoreMcaData, pJsonChild, cc, ret,
                                !CORE_MCA_VALID);
            }

            // Go through each core register on this thread and log it
            for (uint32_t i = 0;
                 i < (sizeof(sCoreMcaRegs) / sizeof(SCoreMcaReg)); i++)
            {
                u64CoreMcaData = 0;
                cc = 0;
                ret = peci_RdIAMSR(cpuInfo.clientAddr,
                                   (u32CoreNum * 2) + u32ThreadNum,
                                   sCoreMcaRegs[i].addr, &u64CoreMcaData, &cc);

                if (PECI_CC_SKIP_CORE(cc))
                {
                    coreMcaJsonICX1(u32CoreNum, u32ThreadNum, &sCoreMcaRegs[i],
                                    u64CoreMcaData, pJsonChild, cc, ret,
                                    CORE_MCA_VALID);
                    goto nextCore;
                }

                if (PECI_CC_SKIP_SOCKET(cc))
                {
                    coreMcaJsonICX1(u32CoreNum, u32ThreadNum, &sCoreMcaRegs[i],
                                    u64CoreMcaData, pJsonChild, cc, ret,
                                    CORE_MCA_VALID);
                    return ret;
                }

                // Log the MCA register
                coreMcaJsonICX1(u32CoreNum, u32ThreadNum, &sCoreMcaRegs[i],
                                u64CoreMcaData, pJsonChild, cc, ret,
                                CORE_MCA_VALID);
            }
        }
    nextCore:;
    }
    return ret;
}

static const SCoreMcaLogVx sCoreMcaLogVx[] = {
    {crashdump::cpu::clx, logCoreMcaCPX1},
    {crashdump::cpu::cpx, logCoreMcaCPX1},
    {crashdump::cpu::skx, logCoreMcaCPX1},
    {crashdump::cpu::icx, logCoreMcaICX1},
    {crashdump::cpu::icx2, logCoreMcaICX1},
    {crashdump::cpu::icxd, logCoreMcaICX1},
};

/******************************************************************************
 *
 *   logCoreMca
 *
 *   This function gathers the Core MCA register contents and adds them to the
 *   debug log.
 *
 ******************************************************************************/
int logCoreMca(crashdump::CPUInfo& cpuInfo, cJSON* pJsonChild)
{
    if (pJsonChild == NULL)
    {
        return 1;
    }

    for (uint32_t i = 0; i < (sizeof(sCoreMcaLogVx) / sizeof(SCoreMcaLogVx));
         i++)
    {
        if (cpuInfo.model == sCoreMcaLogVx[i].cpuModel)
        {
            return sCoreMcaLogVx[i].logCoreMcaVx(cpuInfo, pJsonChild);
        }
    }

    CRASHDUMP_PRINT(ERR, stderr, "Cannot find version for %s\n", __FUNCTION__);
    return 1;
}
