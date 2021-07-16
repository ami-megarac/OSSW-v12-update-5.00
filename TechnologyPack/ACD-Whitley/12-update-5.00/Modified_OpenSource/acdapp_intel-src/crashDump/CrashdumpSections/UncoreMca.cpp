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

#include "UncoreMca.hpp"

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
 *   coreMcaCboJson
 *
 *   This function formats the core MCA CBO registers into a JSON
 *   object
 *
 ******************************************************************************/
static void unCoreMcaCboJson(uint32_t u32CboNum,
                             SUncoreMcaRawData* psUncoreCboRawData,
                             cJSON* pJsonChild, int ret)
{
    char jsonItemString[UNCORE_MCA_JSON_STRING_LEN];
    char jsonNameString[UNCORE_MCA_JSON_STRING_LEN];

    // Add the uncore item to the Uncore MCA JSON structure only if it doesn't
    // already exist
    cJSON* uncore;
    cd_snprintf_s(jsonItemString, UNCORE_MCA_JSON_STRING_LEN,
                  UNCORE_MCA_JSON_SECTION_NAME);
    if ((uncore = cJSON_GetObjectItemCaseSensitive(pJsonChild,
                                                   jsonItemString)) == NULL)
    {
        cJSON_AddItemToObject(pJsonChild, jsonItemString,
                              uncore = cJSON_CreateObject());
    }

    // Format the Uncore Status CBO MCA data out to the .json debug file
    // Fill in NULL for this CBO MCA if it's not valid
    cJSON* uncoreMcaCbo;
    cd_snprintf_s(jsonNameString, UNCORE_MCA_JSON_STRING_LEN,
                  UNCORE_MCA_JSON_CBO_NAME, u32CboNum, uncoreMcaRegNames[0]);

    if ((uncoreMcaCbo =
             cJSON_GetObjectItemCaseSensitive(uncore, jsonNameString)) == NULL)
    {
        cd_snprintf_s(jsonItemString, UNCORE_MCA_JSON_STRING_LEN,
                      UNCORE_MCA_JSON_CBO_NAME, u32CboNum);
        cJSON_AddItemToObject(uncore, jsonItemString,
                              uncoreMcaCbo = cJSON_CreateObject());
    }
    if (psUncoreCboRawData->bInvalid)
    {
        for (int i = 0; i < UCM_NUM_MCA_QWORDS; i++)
        {
            cd_snprintf_s(jsonNameString, UNCORE_MCA_JSON_STRING_LEN,
                          UNCORE_MCA_CBO_REG_NAME, u32CboNum,
                          uncoreMcaRegNames[i]);
            cd_snprintf_s(jsonItemString, UNCORE_MCA_JSON_STRING_LEN,
                          UNCORE_MCA_UA_DF_CPX, psUncoreCboRawData->cc, ret);
            cJSON_AddStringToObject(uncoreMcaCbo, jsonNameString,
                                    jsonItemString);
        }
    }
    else if (PECI_CC_UA(psUncoreCboRawData->cc))
    {
        for (int i = 0; i < UCM_NUM_MCA_QWORDS; i++)
        {
            cd_snprintf_s(jsonNameString, UNCORE_MCA_JSON_STRING_LEN,
                          UNCORE_MCA_CBO_REG_NAME, u32CboNum,
                          uncoreMcaRegNames[i]);
            cd_snprintf_s(
                jsonItemString, UNCORE_MCA_JSON_STRING_LEN, UNCORE_MCA_UA_CPX,
                psUncoreCboRawData->uRegData.u64Raw[i], psUncoreCboRawData->cc);
            cJSON_AddStringToObject(uncoreMcaCbo, jsonNameString,
                                    jsonItemString);
        }
    }
    else // Otherwise fill in the register data
    {
        for (int i = 0; i < UCM_NUM_MCA_QWORDS; i++)
        {
            cd_snprintf_s(jsonNameString, UNCORE_MCA_JSON_STRING_LEN,
                          UNCORE_MCA_CBO_REG_NAME, u32CboNum,
                          uncoreMcaRegNames[i]);
            cd_snprintf_s(jsonItemString, UNCORE_MCA_JSON_STRING_LEN, "0x%llx",
                          psUncoreCboRawData->uRegData.u64Raw[i]);
            cJSON_AddStringToObject(uncoreMcaCbo, jsonNameString,
                                    jsonItemString);
        }
    }
}

/******************************************************************************
 *
 *   coreMcaCboJsonICX
 *
 *   This function formats the core MCA CBO registers into a JSON
 *   object
 *
 ******************************************************************************/
static void unCoreMcaCboJsonICX(EUncoreRegNames mca_bank, uint32_t u32CboNum,
                                SUncoreMcaRawData* psUncoreCboRawData,
                                cJSON* pJsonChild, int ret)
{
    char jsonItemString[UNCORE_MCA_JSON_STRING_LEN];
    char jsonNameString[UNCORE_MCA_JSON_STRING_LEN];

    // Add the uncore item to the Uncore MCA JSON structure only if it doesn't
    // already exist
    cJSON* uncore;
    cd_snprintf_s(jsonItemString, UNCORE_MCA_JSON_STRING_LEN,
                  UNCORE_MCA_JSON_SECTION_NAME);
    if ((uncore = cJSON_GetObjectItemCaseSensitive(pJsonChild,
                                                   jsonItemString)) == NULL)
    {
        cJSON_AddItemToObject(pJsonChild, jsonItemString,
                              uncore = cJSON_CreateObject());
    }

    // Format the Uncore Status CBO MCA data out to the .json debug file
    // Fill in NULL for this CBO MCA if it's not valid
    cJSON* uncoreMcaCbo;
    cd_snprintf_s(jsonNameString, UNCORE_MCA_JSON_STRING_LEN,
                  UNCORE_MCA_JSON_CBO_NAME, u32CboNum, uncoreMcaRegNames[0]);

    if ((uncoreMcaCbo =
             cJSON_GetObjectItemCaseSensitive(uncore, jsonNameString)) == NULL)
    {
        cd_snprintf_s(jsonItemString, UNCORE_MCA_JSON_STRING_LEN,
                      UNCORE_MCA_JSON_CBO_NAME, u32CboNum);
        cJSON_AddItemToObject(uncore, jsonItemString,
                              uncoreMcaCbo = cJSON_CreateObject());
    }
    if (psUncoreCboRawData->bInvalid)
    {
        cd_snprintf_s(jsonNameString, UNCORE_MCA_JSON_STRING_LEN,
                      UNCORE_MCA_CBO_REG_NAME, u32CboNum,
                      uncoreMcaRegNames[mca_bank]);
        cd_snprintf_s(jsonItemString, UNCORE_MCA_JSON_STRING_LEN,
                      UNCORE_MCA_UA_DF, psUncoreCboRawData->cc, ret);
        cJSON_AddStringToObject(uncoreMcaCbo, jsonNameString, jsonItemString);
    }
    else if (PECI_CC_UA(psUncoreCboRawData->cc))
    {

        cd_snprintf_s(jsonNameString, UNCORE_MCA_JSON_STRING_LEN,
                      UNCORE_MCA_CBO_REG_NAME, u32CboNum,
                      uncoreMcaRegNames[mca_bank]);
        cd_snprintf_s(jsonItemString, UNCORE_MCA_JSON_STRING_LEN, UNCORE_MCA_UA,
                      psUncoreCboRawData->cc);
        cJSON_AddStringToObject(uncoreMcaCbo, jsonNameString, jsonItemString);
    }
    else // Otherwise fill in the register data
    {

        cd_snprintf_s(jsonNameString, UNCORE_MCA_JSON_STRING_LEN,
                      UNCORE_MCA_CBO_REG_NAME, u32CboNum,
                      uncoreMcaRegNames[mca_bank]);
        cd_snprintf_s(jsonItemString, UNCORE_MCA_JSON_STRING_LEN, "0x%llx",
                      psUncoreCboRawData->uRegData.u64Raw[mca_bank]);
        cJSON_AddStringToObject(uncoreMcaCbo, jsonNameString, jsonItemString);
    }
}

/******************************************************************************
 *
 *   uncoreCboMcaReadCPX1
 *
 *   This function gathers the Uncore Cbo MCA registers
 *
 ******************************************************************************/
static int uncoreCboMcaReadCPX1(crashdump::CPUInfo& cpuInfo, uint32_t u32Param,
                                SUncoreMcaRawData* sUncoreStatusMcaRawData,
                                int peci_fd)
{
    uint8_t cc = 0;
    int ret = 0;

    // Open the MCA Bank dump sequence
    ret = peci_WrPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU, VCU_OPEN_SEQ,
                               VCU_UNCORE_MCA_SEQ, sizeof(uint32_t), peci_fd,
                               &cc);
    sUncoreStatusMcaRawData->cc = cc;
    sUncoreStatusMcaRawData->ret = ret;
    if (ret != PECI_CC_SUCCESS)
    {
        // MCA Bank sequence failed, abort the sequence
        peci_WrPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU, VCU_ABORT_SEQ,
                             VCU_UNCORE_MCA_SEQ, sizeof(uint32_t), peci_fd,
                             &cc);
        return ret;
    }

    // Set MCA Bank number
    ret =
        peci_WrPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU, UCM_BANK_PARAM,
                             u32Param, sizeof(uint32_t), peci_fd, &cc);
    sUncoreStatusMcaRawData->cc = cc;
    sUncoreStatusMcaRawData->ret = ret;
    if (ret != PECI_CC_SUCCESS)
    {
        // MCA Bank sequence failed, abort the sequence
        sUncoreStatusMcaRawData->cc = cc;
        peci_WrPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU, VCU_ABORT_SEQ,
                             VCU_UNCORE_MCA_SEQ, sizeof(uint32_t), peci_fd,
                             &cc);
        return ret;
    }

    // Get the MCA Bank Registers
    for (uint8_t u8Dword = 0; u8Dword < UCM_NUM_MCA_DWORDS; u8Dword++)
    {
        ret = peci_RdPkgConfig_seq(
            cpuInfo.clientAddr, MBX_INDEX_VCU, VCU_READ, sizeof(uint32_t),
            (uint8_t*)&sUncoreStatusMcaRawData->uRegData.u32Raw[u8Dword],
            peci_fd, &cc);
        sUncoreStatusMcaRawData->cc = cc;
        sUncoreStatusMcaRawData->ret = ret;
        if (ret != PECI_CC_SUCCESS)
        {
            // MCA Bank sequence failed, abort the sequence
            sUncoreStatusMcaRawData->cc = cc;
            peci_WrPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU,
                                 VCU_ABORT_SEQ, VCU_UNCORE_MCA_SEQ,
                                 sizeof(uint32_t), peci_fd, &cc);
            return ret;
        }
    }

    // Close the MCA Bank sequence
    peci_WrPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU, VCU_CLOSE_SEQ,
                         VCU_UNCORE_MCA_SEQ, sizeof(uint32_t), peci_fd, &cc);

    return ret;
}

/******************************************************************************
 *
 *   uncoreMcaJsonCPX1
 *
 *   This function formats the Uncore MCA into a JSON object
 *
 ******************************************************************************/
static void uncoreMcaJsonCPX1(SUncoreMcaRawData* sUncoreMcaRawData,
                              cJSON* pJsonChild, uint32_t bank)
{
    cJSON* uncore;
    cJSON* uncoreMca;
    char jsonItemName[UNCORE_MCA_JSON_STRING_LEN];
    char jsonItemString[UNCORE_MCA_JSON_STRING_LEN];

    // Add the uncore item to the Uncore MCA JSON structure only if it doesn't
    // already exist
    cd_snprintf_s(jsonItemString, UNCORE_MCA_JSON_STRING_LEN,
                  UNCORE_MCA_JSON_SECTION_NAME);
    if ((uncore = cJSON_GetObjectItemCaseSensitive(pJsonChild,
                                                   jsonItemString)) == NULL)
    {
        cJSON_AddItemToObject(pJsonChild, jsonItemString,
                              uncore = cJSON_CreateObject());
    }

    // Format the Uncore MCA data out to the .json debug file
    // Add the MCA number item to the Uncore MCA JSON structure
    cd_snprintf_s(jsonItemString, UNCORE_MCA_JSON_STRING_LEN,
                  UNCORE_MCA_JSON_MCA_NAME, bank);
    cJSON_AddItemToObject(uncore, jsonItemString,
                          uncoreMca = cJSON_CreateObject());

    // Fill in NULL for this MCE Bank if it's not valid
    if (sUncoreMcaRawData->bInvalid)
    {
        cd_snprintf_s(jsonItemName, UNCORE_MCA_JSON_STRING_LEN,
                      UNCORE_MCA_REG_NAME, bank, uncoreMcaRegNames[UNCORE_CTL]);
        cd_snprintf_s(jsonItemString, UNCORE_MCA_JSON_STRING_LEN,
                      UNCORE_MCA_UA_DF_CPX, sUncoreMcaRawData->cc,
                      sUncoreMcaRawData->ret);
        cJSON_AddStringToObject(uncoreMca, jsonItemName, jsonItemString);
        cd_snprintf_s(jsonItemName, UNCORE_MCA_JSON_STRING_LEN,
                      UNCORE_MCA_REG_NAME, bank,
                      uncoreMcaRegNames[UNCORE_STATUS]);
        cJSON_AddStringToObject(uncoreMca, jsonItemName, jsonItemString);
        cd_snprintf_s(jsonItemName, UNCORE_MCA_JSON_STRING_LEN,
                      UNCORE_MCA_REG_NAME, bank,
                      uncoreMcaRegNames[UNCORE_ADDR]);
        cJSON_AddStringToObject(uncoreMca, jsonItemName, jsonItemString);
        cd_snprintf_s(jsonItemName, UNCORE_MCA_JSON_STRING_LEN,
                      UNCORE_MCA_REG_NAME, bank,
                      uncoreMcaRegNames[UNCORE_MISC]);
        cJSON_AddStringToObject(uncoreMca, jsonItemName, jsonItemString);
        cd_snprintf_s(jsonItemName, UNCORE_MCA_JSON_STRING_LEN,
                      UNCORE_MCA_REG_NAME, bank,
                      uncoreMcaRegNames[UNCORE_CTL2]);
        cJSON_AddStringToObject(uncoreMca, jsonItemName, jsonItemString);
        return;
    }
    else if (PECI_CC_UA(sUncoreMcaRawData->cc))
    {
        cd_snprintf_s(jsonItemName, UNCORE_MCA_JSON_STRING_LEN,
                      UNCORE_MCA_REG_NAME, bank, uncoreMcaRegNames[UNCORE_CTL]);
        cd_snprintf_s(jsonItemString, UNCORE_MCA_JSON_STRING_LEN,
                      UNCORE_MCA_UA_CPX,
                      sUncoreMcaRawData->uRegData.sReg.u64UncoreMcaCtl,
                      sUncoreMcaRawData->cc);
        cJSON_AddStringToObject(uncoreMca, jsonItemName, jsonItemString);
        cd_snprintf_s(jsonItemName, UNCORE_MCA_JSON_STRING_LEN,
                      UNCORE_MCA_REG_NAME, bank,
                      uncoreMcaRegNames[UNCORE_STATUS]);
        cd_snprintf_s(jsonItemString, UNCORE_MCA_JSON_STRING_LEN,
                      UNCORE_MCA_UA_CPX,
                      sUncoreMcaRawData->uRegData.sReg.u64UncoreMcaStatus,
                      sUncoreMcaRawData->cc);
        cJSON_AddStringToObject(uncoreMca, jsonItemName, jsonItemString);
        cd_snprintf_s(jsonItemName, UNCORE_MCA_JSON_STRING_LEN,
                      UNCORE_MCA_REG_NAME, bank,
                      uncoreMcaRegNames[UNCORE_ADDR]);
        cd_snprintf_s(jsonItemString, UNCORE_MCA_JSON_STRING_LEN,
                      UNCORE_MCA_UA_CPX,
                      sUncoreMcaRawData->uRegData.sReg.u64UncoreMcaAddr,
                      sUncoreMcaRawData->cc);
        cJSON_AddStringToObject(uncoreMca, jsonItemName, jsonItemString);
        cd_snprintf_s(jsonItemName, UNCORE_MCA_JSON_STRING_LEN,
                      UNCORE_MCA_REG_NAME, bank,
                      uncoreMcaRegNames[UNCORE_MISC]);
        cd_snprintf_s(jsonItemString, UNCORE_MCA_JSON_STRING_LEN,
                      UNCORE_MCA_UA_CPX,
                      sUncoreMcaRawData->uRegData.sReg.u64UncoreMcaMisc,
                      sUncoreMcaRawData->cc);
        cJSON_AddStringToObject(uncoreMca, jsonItemName, jsonItemString);
        cd_snprintf_s(jsonItemName, UNCORE_MCA_JSON_STRING_LEN,
                      UNCORE_MCA_REG_NAME, bank,
                      uncoreMcaRegNames[UNCORE_CTL2]);
        cd_snprintf_s(jsonItemString, UNCORE_MCA_JSON_STRING_LEN,
                      UNCORE_MCA_UA_CPX,
                      sUncoreMcaRawData->uRegData.sReg.u64UncoreMcaCtl2,
                      sUncoreMcaRawData->cc);
        cJSON_AddStringToObject(uncoreMca, jsonItemName, jsonItemString);
        return;
    }
    // Otherwise fill in the register data
    else
    {
        cd_snprintf_s(jsonItemName, UNCORE_MCA_JSON_STRING_LEN,
                      UNCORE_MCA_REG_NAME, bank, uncoreMcaRegNames[UNCORE_CTL]);
        cd_snprintf_s(jsonItemString, UNCORE_MCA_JSON_STRING_LEN, "0x%llx",
                      sUncoreMcaRawData->uRegData.sReg.u64UncoreMcaCtl);
        cJSON_AddStringToObject(uncoreMca, jsonItemName, jsonItemString);
        cd_snprintf_s(jsonItemName, UNCORE_MCA_JSON_STRING_LEN,
                      UNCORE_MCA_REG_NAME, bank,
                      uncoreMcaRegNames[UNCORE_STATUS]);
        cd_snprintf_s(jsonItemString, UNCORE_MCA_JSON_STRING_LEN, "0x%llx",
                      sUncoreMcaRawData->uRegData.sReg.u64UncoreMcaStatus);
        cJSON_AddStringToObject(uncoreMca, jsonItemName, jsonItemString);
        cd_snprintf_s(jsonItemName, UNCORE_MCA_JSON_STRING_LEN,
                      UNCORE_MCA_REG_NAME, bank,
                      uncoreMcaRegNames[UNCORE_ADDR]);
        cd_snprintf_s(jsonItemString, UNCORE_MCA_JSON_STRING_LEN, "0x%llx",
                      sUncoreMcaRawData->uRegData.sReg.u64UncoreMcaAddr);
        cJSON_AddStringToObject(uncoreMca, jsonItemName, jsonItemString);
        cd_snprintf_s(jsonItemName, UNCORE_MCA_JSON_STRING_LEN,
                      UNCORE_MCA_REG_NAME, bank,
                      uncoreMcaRegNames[UNCORE_MISC]);
        cd_snprintf_s(jsonItemString, UNCORE_MCA_JSON_STRING_LEN, "0x%llx",
                      sUncoreMcaRawData->uRegData.sReg.u64UncoreMcaMisc);
        cJSON_AddStringToObject(uncoreMca, jsonItemName, jsonItemString);
        cd_snprintf_s(jsonItemName, UNCORE_MCA_JSON_STRING_LEN,
                      UNCORE_MCA_REG_NAME, bank,
                      uncoreMcaRegNames[UNCORE_CTL2]);
        cd_snprintf_s(jsonItemString, UNCORE_MCA_JSON_STRING_LEN, "0x%llx",
                      sUncoreMcaRawData->uRegData.sReg.u64UncoreMcaCtl2);
        cJSON_AddStringToObject(uncoreMca, jsonItemName, jsonItemString);
    }
}

/******************************************************************************
 *
 *   uncoreMcaCboCPX1
 *
 *   This function gathers the Uncore CBO MCA registers
 *
 ******************************************************************************/
static int uncoreMcaCboCPX1(crashdump::CPUInfo& cpuInfo, cJSON* pJsonChild)
{
    int peci_fd = -1;
    int ret = 0;
    ret = peci_Lock(&peci_fd, PECI_WAIT_FOREVER);
    if (ret != PECI_CC_SUCCESS)
    {
        return ret;
    }

    // Go through each CBO in this CPU
    for (size_t i = 0; i < cpuInfo.chaCount; i++)
    {
        SUncoreMcaRawData sMcaData = {};
        // Build the MCA parameter for this CBO
        uint32_t u32CboParam = ((i / US_NUM_CBO_BANKS) << 24) | US_MCA_UNMERGE |
                               ((i % US_NUM_CBO_BANKS) + US_BASE_CBO_BANK);

        // Get the CBO MCA data
        ret = uncoreCboMcaReadCPX1(cpuInfo, u32CboParam, &sMcaData, peci_fd);
        if (ret != 0)
        {
            sMcaData.bInvalid = true;
        }

        // Log the MCA for this CBO
        unCoreMcaCboJson(i, &sMcaData, pJsonChild, ret);
    }

    peci_Unlock(peci_fd);
    return ret;
}

/******************************************************************************
 *
 *   logUncoreMcaCPX1
 *
 *   This function gathers the Uncore MCA register contents and adds them to the
 *   debug log. The PECI flow is listed below to dump an MCE bank
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
int logUncoreMcaCPX1(crashdump::CPUInfo& cpuInfo, cJSON* pJsonChild)
{
    int ret = 0;
    int peci_fd = -1;
    uint8_t cc = 0;

    ret = peci_Lock(&peci_fd, PECI_WAIT_FOREVER);
    if (ret != PECI_CC_SUCCESS)
    {
        return ret;
    }

    // Read the Uncore MCA registers from the CPU
    for (uint32_t j = FIRST_UNCORE_MCA; j <= LAST_UNCORE_MCA; j++)
    {
        if ((CHECK_BIT(cpuInfo.capidRead.capid2, CAPID2_DISABLE0) &&
             (j == UNCORE_UPI0)) ||
            (CHECK_BIT(cpuInfo.capidRead.capid2, CAPID2_DISABLE1) &&
             (j == UNCORE_UPI1)) ||
            (CHECK_BIT(cpuInfo.capidRead.capid2, CAPID2_DISABLE2) &&
             (j == UNCORE_UPI2)) ||
            (CHECK_BIT(cpuInfo.capidRead.capid2, CAPID2_DISABLE3) &&
             (j == UNCORE_UPI3)) ||
            (CHECK_BIT(cpuInfo.capidRead.capid2, CAPID2_DISABLE4) &&
             (j == UNCORE_UPI4)) ||
            (CHECK_BIT(cpuInfo.capidRead.capid2, CAPID2_DISABLE5) &&
             (j == UNCORE_UPI5)))
        {
            continue;
        }
        SUncoreMcaRawData sUncoreMcaRawData = {};
        // Open the MCA Bank dump sequence
        ret = peci_WrPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU,
                                   VCU_OPEN_SEQ, VCU_UNCORE_MCA_SEQ,
                                   sizeof(uint32_t), peci_fd, &cc);
        sUncoreMcaRawData.cc = cc;
        sUncoreMcaRawData.ret = ret;
        if (ret != PECI_CC_SUCCESS)
        {
            // MCE Bank sequence failed, abort the sequence and go to the next
            // Bank
            peci_WrPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU,
                                 VCU_ABORT_SEQ, VCU_UNCORE_MCA_SEQ,
                                 sizeof(uint32_t), peci_fd, &cc);
            sUncoreMcaRawData.bInvalid = true;
            uncoreMcaJsonCPX1(&sUncoreMcaRawData, pJsonChild, j);
            continue;
        }

        // Set MCE Bank number
        ret = peci_WrPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU,
                                   UCM_BANK_PARAM, j, sizeof(uint32_t), peci_fd,
                                   &cc);
        sUncoreMcaRawData.cc = cc;
        sUncoreMcaRawData.ret = ret;
        if (ret != PECI_CC_SUCCESS)
        {
            // MCE Bank sequence failed, abort the sequence and go to the next
            // Bank
            peci_WrPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU,
                                 VCU_ABORT_SEQ, VCU_UNCORE_MCA_SEQ,
                                 sizeof(uint32_t), peci_fd, &cc);
            sUncoreMcaRawData.bInvalid = true;
            uncoreMcaJsonCPX1(&sUncoreMcaRawData, pJsonChild, j);
            continue;
        }

        // Get the MCE Bank Registers
        for (uint8_t u8Dword = 0; u8Dword < UCM_NUM_MCA_DWORDS; u8Dword++)
        {
            ret = peci_RdPkgConfig_seq(
                cpuInfo.clientAddr, MBX_INDEX_VCU, VCU_READ, sizeof(uint32_t),
                (uint8_t*)&sUncoreMcaRawData.uRegData.u32Raw[u8Dword], peci_fd,
                &cc);
            sUncoreMcaRawData.cc = cc;
            sUncoreMcaRawData.ret = ret;
            if (ret != PECI_CC_SUCCESS)
            {
                // MCE Bank sequence failed, abort the sequence and go to the
                // next Bank
                peci_WrPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU,
                                     VCU_ABORT_SEQ, VCU_UNCORE_MCA_SEQ,
                                     sizeof(uint32_t), peci_fd, &cc);
                sUncoreMcaRawData.bInvalid = true;
                break;
            }
        }

        // Close the MCE Bank sequence
        peci_WrPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU, VCU_CLOSE_SEQ,
                             VCU_UNCORE_MCA_SEQ, sizeof(uint32_t), peci_fd,
                             &cc);

        // Log this MCE Bank to the Uncore MCA for this CPU
        uncoreMcaJsonCPX1(&sUncoreMcaRawData, pJsonChild, j);
    }

    peci_Unlock(peci_fd);
    ret |= uncoreMcaCboCPX1(cpuInfo, pJsonChild);
    return ret;
}

static const SUncoreMcaReg sUncoreMcaRegs[] = {
    // Bank, Register, Address, Instance ID

    // PCU
    {"MC4", "mc4_ctl", 0x410, 0},
    {"MC4", "mc4_status", 0x411, 0},
    {"MC4", "mc4_addr", 0x412, 0},
    {"MC4", "mc4_misc", 0x413, 0},
    {"MC4", "mc4_ctl2", 0x284, 0},

    // Intel UPI0
    {"MC5", "mc5_ctl", 0x414, 0},
    {"MC5", "mc5_status", 0x415, 0},
    {"MC5", "mc5_addr", 0x416, 0},
    {"MC5", "mc5_misc", 0x417, 0},
    {"MC5", "mc5_ctl2", 0x285, 0},

    // UBOX
    {"MC6", "mc6_ctl", 0x418, 0},
    {"MC6", "mc6_status", 0x419, 0},
    {"MC6", "mc6_addr", 0x41a, 0},
    {"MC6", "mc6_misc", 0x41b, 0},
    {"MC6", "mc6_ctl2", 0x286, 0},

    // Intel UPI1
    {"MC7", "mc7_ctl", 0x41c, 1},
    {"MC7", "mc7_status", 0x41d, 1},
    {"MC7", "mc7_addr", 0x41e, 1},
    {"MC7", "mc7_misc", 0x41f, 1},
    {"MC7", "mc7_ctl2", 0x287, 1},

    // Intel UPI2
    {"MC8", "mc8_ctl", 0x420, 2},
    {"MC8", "mc8_status", 0x421, 2},
    {"MC8", "mc8_addr", 0x422, 2},
    {"MC8", "mc8_misc", 0x423, 2},
    {"MC8", "mc8_ctl2", 0x288, 2},

    // CHA0
    {"MC9", "mc9_status", 0x425, 0},
    {"MC9", "mc9_ctl2", 0x289, 0},

    // CHA1
    {"MC10", "mc10_status", 0x429, 0},
    {"MC10", "mc10_ctl2", 0x28a, 0},

    // CHA2
    {"MC11", "mc11_status", 0x42d, 0},
    {"MC11", "mc11_ctl2", 0x28b, 0},

    // M2M0
    {"MC12", "mc12_ctl", 0x430, 0},
    {"MC12", "mc12_status", 0x431, 0},
    {"MC12", "mc12_addr", 0x432, 0},
    {"MC12", "mc12_misc", 0x433, 0},
    {"MC12", "mc12_ctl2", 0x28c, 0},

    // IMC0_CH0
    {"MC13", "mc13_ctl", 0x434, 0},
    {"MC13", "mc13_status", 0x435, 0},
    {"MC13", "mc13_addr", 0x436, 0},
    {"MC13", "mc13_misc", 0x437, 0},
    {"MC13", "mc13_ctl2", 0x28d, 0},

    // IMC0_CH1
    {"MC14", "mc14_ctl", 0x438, 0},
    {"MC14", "mc14_status", 0x439, 0},
    {"MC14", "mc14_addr", 0x43a, 0},
    {"MC14", "mc14_misc", 0x43b, 0},
    {"MC14", "mc14_ctl2", 0x28e, 0},

    // M2M1
    {"MC16", "mc16_ctl", 0x440, 1},
    {"MC16", "mc16_status", 0x441, 1},
    {"MC16", "mc16_addr", 0x442, 1},
    {"MC16", "mc16_misc", 0x443, 1},
    {"MC16", "mc16_ctl2", 0x290, 1},

    // IMC1_CH0
    {"MC17", "mc17_ctl", 0x444, 1},
    {"MC17", "mc17_status", 0x445, 1},
    {"MC17", "mc17_addr", 0x446, 1},
    {"MC17", "mc17_misc", 0x447, 1},
    {"MC17", "mc17_ctl2", 0x291, 1},

    // IMC1_CH1
    {"MC18", "mc18_ctl", 0x448, 1},
    {"MC18", "mc18_status", 0x449, 1},
    {"MC18", "mc18_addr", 0x44a, 1},
    {"MC18", "mc18_misc", 0x44b, 1},
    {"MC18", "mc18_ctl2", 0x292, 1},

    // M2M2
    {"MC20", "mc20_ctl", 0x450, 2},
    {"MC20", "mc20_status", 0x451, 2},
    {"MC20", "mc20_addr", 0x452, 2},
    {"MC20", "mc20_misc", 0x453, 2},
    {"MC20", "mc20_ctl2", 0x294, 2},

    // IMC2_CH0
    {"MC21", "mc21_ctl", 0x454, 2},
    {"MC21", "mc21_status", 0x455, 2},
    {"MC21", "mc21_addr", 0x456, 2},
    {"MC21", "mc21_misc", 0x457, 2},
    {"MC21", "mc21_ctl2", 0x295, 2},

    // IMC2_CH1
    {"MC22", "mc22_ctl", 0x458, 2},
    {"MC22", "mc22_status", 0x459, 2},
    {"MC22", "mc22_addr", 0x45a, 2},
    {"MC22", "mc22_misc", 0x45b, 2},
    {"MC22", "mc22_ctl2", 0x296, 2},

    // M2M3
    {"MC24", "mc24_ctl", 0x460, 3},
    {"MC24", "mc24_status", 0x461, 3},
    {"MC24", "mc24_addr", 0x462, 3},
    {"MC24", "mc24_misc", 0x463, 3},
    {"MC24", "mc24_ctl2", 0x298, 3},

    // IMC3_CH0
    {"MC25", "mc25_ctl", 0x464, 3},
    {"MC25", "mc25_status", 0x465, 3},
    {"MC25", "mc25_addr", 0x466, 3},
    {"MC25", "mc25_misc", 0x467, 3},
    {"MC25", "mc25_ctl2", 0x299, 3},

    // IMC3_CH1
    {"MC26", "mc26_ctl", 0x468, 3},
    {"MC26", "mc26_status", 0x469, 3},
    {"MC26", "mc26_addr", 0x46a, 3},
    {"MC26", "mc26_misc", 0x46b, 3},
    {"MC26", "mc26_ctl2", 0x29a, 3},

};

/******************************************************************************
 *
 *   uncoreMcaJsonICX1
 *
 *   This function formats the Uncore MCA into a JSON object
 *
 ******************************************************************************/
static void uncoreMcaJsonICX1(const SUncoreMcaReg* sUncoreMcaReg,
                              uint64_t u64UncoreMcaData, cJSON* pJsonChild,
                              uint8_t cc, int ret)
{
    char jsonItemString[UNCORE_MCA_JSON_STRING_LEN];

    // Add the uncore item to the Uncore MCA JSON structure only if it doesn't
    // already exist
    cJSON* uncore;
    cd_snprintf_s(jsonItemString, UNCORE_MCA_JSON_STRING_LEN,
                  UNCORE_MCA_JSON_SECTION_NAME);
    if ((uncore = cJSON_GetObjectItemCaseSensitive(pJsonChild,
                                                   jsonItemString)) == NULL)
    {
        cJSON_AddItemToObject(pJsonChild, jsonItemString,
                              uncore = cJSON_CreateObject());
    }

    // Add the MCA number item to the Uncore MCA JSON structure only if it
    // doesn't already exist
    cJSON* uncoreMca;
    if ((uncoreMca = cJSON_GetObjectItemCaseSensitive(
             uncore, sUncoreMcaReg->bankName)) == NULL)
    {
        cJSON_AddItemToObject(uncore, sUncoreMcaReg->bankName,
                              uncoreMca = cJSON_CreateObject());
    }
    if (ret != PECI_CC_SUCCESS)
    {
        cd_snprintf_s(jsonItemString, UNCORE_MCA_JSON_STRING_LEN,
                      UNCORE_MCA_UA_DF, cc, ret);
    }
    else if (PECI_CC_UA(cc))
    {
        cd_snprintf_s(jsonItemString, UNCORE_MCA_JSON_STRING_LEN, UNCORE_MCA_UA,
                      cc);
    }
    else
    {
        cd_snprintf_s(jsonItemString, UNCORE_MCA_JSON_STRING_LEN, "0x%llx",
                      u64UncoreMcaData);
    }

    // Add the MCA register data to the Uncore MCA JSON structure
    cJSON_AddStringToObject(uncoreMca, sUncoreMcaReg->regName, jsonItemString);
}

/******************************************************************************
 *
 *   logUnCoreMcaCboICX1
 *
 *   This function gathers the UnCore MCA CBO registers
 *
 ******************************************************************************/
static int logUnCoreMcaCboICX1(crashdump::CPUInfo& cpuInfo, cJSON* pJsonChild)
{
    uint8_t cc = 0;
    int ret = 0;

    // Go through each CBO in this CPU
    for (size_t i = 0; i < cpuInfo.chaCount; i++)
    {
        SUncoreMcaRawData sMcaData = {};
        sMcaData.bInvalid = false;
        ret = peci_RdIAMSR(cpuInfo.clientAddr, i, UCM_CBO_MCA_PARAM_CTL,
                           &sMcaData.uRegData.sReg.u64UncoreMcaCtl, &cc);
        sMcaData.ret = ret;
        sMcaData.cc = cc;
        if (ret != PECI_CC_SUCCESS)
        {
            sMcaData.bInvalid = true;
        }
        unCoreMcaCboJsonICX(UNCORE_CTL, i, &sMcaData, pJsonChild, ret);
        ret = peci_RdIAMSR(cpuInfo.clientAddr, i, UCM_CBO_MCA_PARAM_STATUS,
                           &sMcaData.uRegData.sReg.u64UncoreMcaStatus, &cc);
        sMcaData.ret = ret;
        sMcaData.cc = cc;
        if (ret != PECI_CC_SUCCESS)
        {
            sMcaData.bInvalid = true;
        }
        unCoreMcaCboJsonICX(UNCORE_STATUS, i, &sMcaData, pJsonChild, ret);
        ret = peci_RdIAMSR(cpuInfo.clientAddr, i, UCM_CBO_MCA_PARAM_ADDR,
                           &sMcaData.uRegData.sReg.u64UncoreMcaAddr, &cc);
        sMcaData.ret = ret;
        sMcaData.cc = cc;
        if (ret != PECI_CC_SUCCESS)
        {
            sMcaData.bInvalid = true;
        }
        unCoreMcaCboJsonICX(UNCORE_ADDR, i, &sMcaData, pJsonChild, ret);
        ret = peci_RdIAMSR(cpuInfo.clientAddr, i, UCM_CBO_MCA_PARAM_MISC,
                           &sMcaData.uRegData.sReg.u64UncoreMcaMisc, &cc);
        sMcaData.ret = ret;
        sMcaData.cc = cc;
        if (ret != PECI_CC_SUCCESS)
        {
            sMcaData.bInvalid = true;
        }
        unCoreMcaCboJsonICX(UNCORE_MISC, i, &sMcaData, pJsonChild, ret);
        ret = peci_RdIAMSR(cpuInfo.clientAddr, i, UCM_CBO_MCA_PARAM_MISC2,
                           &sMcaData.uRegData.sReg.u64UncoreMcaMisc2, &cc);
        sMcaData.ret = ret;
        sMcaData.cc = cc;
        if (ret != PECI_CC_SUCCESS)
        {
            sMcaData.bInvalid = true;
        }
        unCoreMcaCboJsonICX(UNCORE_CTL2, i, &sMcaData, pJsonChild, ret);
    }

    return ret;
}

/******************************************************************************
 *
 *   logUncoreMcaICX1
 *
 *   This function gathers the Uncore MCA register contents and adds them to the
 *   debug log.
 *
 ******************************************************************************/
int logUncoreMcaICX1(crashdump::CPUInfo& cpuInfo, cJSON* pJsonChild)
{
    int ret = 0;
    uint8_t cc = 0;

    // Go through each uncore register on this cpu and log it
    for (uint32_t i = 0; i < (sizeof(sUncoreMcaRegs) / sizeof(SUncoreMcaReg));
         i++)
    {
        uint64_t u64UncoreMcaData;
        ret = peci_RdIAMSR(cpuInfo.clientAddr, sUncoreMcaRegs[i].instance_id,
                           sUncoreMcaRegs[i].addr, &u64UncoreMcaData, &cc);
        // Log the MCA register
        uncoreMcaJsonICX1(&sUncoreMcaRegs[i], u64UncoreMcaData, pJsonChild, cc,
                          ret);
    }

    logUnCoreMcaCboICX1(cpuInfo, pJsonChild);

    return ret;
}

static const SUncoreMcaLogVx sUncoreMcaLogVx[] = {
    {crashdump::cpu::clx, logUncoreMcaCPX1},
    {crashdump::cpu::cpx, logUncoreMcaCPX1},
    {crashdump::cpu::skx, logUncoreMcaCPX1},
    {crashdump::cpu::icx, logUncoreMcaICX1},
    {crashdump::cpu::icx2, logUncoreMcaICX1},
    {crashdump::cpu::icxd, logUncoreMcaICX1},
};

/******************************************************************************
 *
 *   logUncoreMca
 *
 *   This function gathers the Uncore MCA register contents and adds them to the
 *   debug log.
 *
 ******************************************************************************/
int logUncoreMca(crashdump::CPUInfo& cpuInfo, cJSON* pJsonChild)
{
    if (pJsonChild == NULL)
    {
        return 1;
    }

    for (uint32_t i = 0;
         i < (sizeof(sUncoreMcaLogVx) / sizeof(SUncoreMcaLogVx)); i++)
    {
        if (cpuInfo.model == sUncoreMcaLogVx[i].cpuModel)
        {
            return sUncoreMcaLogVx[i].logUncoreMcaVx(cpuInfo, pJsonChild);
        }
    }

    CRASHDUMP_PRINT(ERR, stderr, "Cannot find version for %s\n", __FUNCTION__);
    return 1;
}
