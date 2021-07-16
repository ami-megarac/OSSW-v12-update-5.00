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

extern "C" {
#ifndef SPX_BMC_ACD
#include <cjson/cJSON.h>
#endif
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
}

#ifndef SPX_BMC_ACD
#include "UncoreRegs.hpp"
#endif
#include "Uncore.hpp"
#include "crashdump.hpp"
#include "utils.hpp"

/******************************************************************************
 *
 *   uncoreStatusPciJson
 *
 *   This function formats the Uncore Status PCI registers into a JSON object
 *
 ******************************************************************************/
static void uncoreStatusPciJson(const char* regName,
                                SUncoreStatusRegRawData* sRegData,
                                cJSON* pJsonChild)
{
    char jsonItemString[US_JSON_STRING_LEN];
    // Format the Uncore Status register data out to the .json debug file
    if (sRegData->bInvalid)
    {
        cd_snprintf_s(jsonItemString, US_JSON_STRING_LEN, US_UA_DF_CPX,
                      sRegData->cc, sRegData->ret);
        cJSON_AddStringToObject(pJsonChild, regName, jsonItemString);
        return;
    }
    else if (PECI_CC_UA(sRegData->cc))
    {
        cd_snprintf_s(jsonItemString, US_JSON_STRING_LEN, US_UA_CPX,
                      sRegData->uValue.u64, sRegData->cc);
        cJSON_AddStringToObject(pJsonChild, regName, jsonItemString);
        return;
    }
    else
    {
        cd_snprintf_s(jsonItemString, US_JSON_STRING_LEN, "0x%llx",
                      sRegData->uValue.u64);
        cJSON_AddStringToObject(pJsonChild, regName, jsonItemString);
    }
}

/******************************************************************************
 *
 *   uncoreStatusMmioRead
 *
 *   This function gathers the Uncore Status PCI MMIO registers
 *
 ******************************************************************************/
static int uncoreStatusMmioRead(
    crashdump::CPUInfo& cpuInfo, uint32_t u32Param, uint8_t u8NumDwords,
    SUncoreStatusRegRawData* sUncoreStatusMmioRawData, int peci_fd)
{
    uint8_t cc = 0;
    int ret = 0;

    // Open the MMIO dump sequence
    ret = peci_WrPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU, VCU_OPEN_SEQ,
                               VCU_READ_LOCAL_MMIO_SEQ, sizeof(uint32_t),
                               peci_fd, &cc);
    sUncoreStatusMmioRawData->cc = cc;
    sUncoreStatusMmioRawData->ret = ret;
    if (ret != PECI_CC_SUCCESS)
    {
        // MMIO sequence failed, abort the sequence
        peci_WrPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU, VCU_ABORT_SEQ,
                             VCU_READ_LOCAL_MMIO_SEQ, sizeof(uint32_t), peci_fd,
                             &cc);
        sUncoreStatusMmioRawData->bInvalid = true;
        return ret;
    }

    // Set MMIO address
    ret = peci_WrPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU, US_MMIO_PARAM,
                               u32Param, sizeof(uint32_t), peci_fd, &cc);
    sUncoreStatusMmioRawData->ret = ret;
    sUncoreStatusMmioRawData->cc = cc;
    if (ret != PECI_CC_SUCCESS)
    {
        // MMIO sequence failed, abort the sequence
        peci_WrPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU, VCU_ABORT_SEQ,
                             VCU_READ_LOCAL_MMIO_SEQ, sizeof(uint32_t), peci_fd,
                             &cc);
        sUncoreStatusMmioRawData->bInvalid = true;
        return ret;
    }

    // Get the MMIO data
    for (uint8_t u8Dword = 0; u8Dword < u8NumDwords; u8Dword++)
    {
        ret = peci_RdPkgConfig_seq(
            cpuInfo.clientAddr, MBX_INDEX_VCU, VCU_READ, sizeof(uint32_t),
            (uint8_t*)&sUncoreStatusMmioRawData->uValue.u32[u8Dword], peci_fd,
            &cc);
        sUncoreStatusMmioRawData->cc = cc;
        sUncoreStatusMmioRawData->ret = ret;
        if (ret != PECI_CC_SUCCESS)
        {
            // MMIO sequence failed, abort the sequence
            peci_WrPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU,
                                 VCU_ABORT_SEQ, VCU_READ_LOCAL_MMIO_SEQ,
                                 sizeof(uint32_t), peci_fd, &cc);
            sUncoreStatusMmioRawData->bInvalid = true;
            return ret;
        }
    }
    // Close the MMIO sequence
    peci_WrPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU, VCU_CLOSE_SEQ,
                         VCU_READ_LOCAL_MMIO_SEQ, sizeof(uint32_t), peci_fd,
                         &cc);

    return ret;
}

/******************************************************************************
 *
 *   uncoreStatusPciCPX
 *
 *   This function gathers the Uncore Status PCI registers using input file
 *
 ******************************************************************************/
int uncoreStatusPciCPX(crashdump::CPUInfo& cpuInfo, cJSON* pJsonChild)
{
    uint8_t cc = 0;
    int ret = 0;
    int retval = 0;

    cJSON* regList = NULL;
    cJSON* itRegs = NULL;
    cJSON* itParams = NULL;
    bool enable = false;

    regList = getCrashDataSectionRegList(cpuInfo.inputFile.bufferPtr, "uncore",
                                         "pci", &enable);

    if (regList == NULL)
    {
        cJSON_AddStringToObject(pJsonChild, FILE_PCI_KEY, FILE_PCI_ERR);
        return 1;
    }

    if (!enable)
    {
        cJSON_AddFalseToObject(pJsonChild, RECORD_ENABLE);
        return 0;
    }

    SUncoreStatusRegPciIcxCpx pciReg = {};

    cJSON_ArrayForEach(itRegs, regList)
    {
        int position = 0;
        cJSON_ArrayForEach(itParams, itRegs)
        {
            switch (position)
            {
                case US_PCI_REG_NAME:
                    pciReg.regName = itParams->valuestring;
                    break;
                case US_PCI_BUS:
                    pciReg.u8Bus = itParams->valueint;
                    break;
                case US_PCI_DEVICE:
                    pciReg.u8Dev = itParams->valueint;
                    break;
                case US_PCI_FUNCTION:
                    pciReg.u8Func = itParams->valueint;
                    break;
                case US_PCI_OFFSET:
                    pciReg.u16Reg = strtoull(itParams->valuestring, NULL, 16);
                    break;
                case US_PCI_SIZE:
                    pciReg.u8Size = itParams->valueint;
                    break;
                default:
                    break;
            }
            position++;
        }

        SUncoreStatusRegRawData sRegData = {};

        switch (pciReg.u8Size)
        {
            case US_REG_BYTE:
            case US_REG_WORD:
            case US_REG_DWORD:
                ret = peci_RdPCIConfigLocal(
                    cpuInfo.clientAddr, pciReg.u8Bus, pciReg.u8Dev,
                    pciReg.u8Func, pciReg.u16Reg, pciReg.u8Size,
                    (uint8_t*)&sRegData.uValue.u64, &cc);
                sRegData.cc = cc;
                if (ret != PECI_CC_SUCCESS)
                {
                    sRegData.bInvalid = true;
                    sRegData.ret = ret;
                    retval = ret;
                }
                break;
            case US_REG_QWORD:
                for (uint8_t u8Dword = 0; u8Dword < 2; u8Dword++)
                {
                    ret = peci_RdPCIConfigLocal(
                        cpuInfo.clientAddr, pciReg.u8Bus, pciReg.u8Dev,
                        pciReg.u8Func, pciReg.u16Reg + (u8Dword * 4),
                        sizeof(uint32_t),
                        (uint8_t*)&sRegData.uValue.u32[u8Dword], &cc);
                    sRegData.cc = cc;
                    if (ret != PECI_CC_SUCCESS)
                    {
                        sRegData.bInvalid = true;
                        sRegData.ret = ret;
                        retval = ret;
                    }
                }
                break;
	    default:
		break;
        }
        // Log this Uncore Status PCI Register
        uncoreStatusPciJson(pciReg.regName, &sRegData, pJsonChild);
    }

    return retval;
}

/******************************************************************************
 *
 *   uncoreStatusPciMmioCPX
 *
 *   This function gathers the Uncore Status PCI MMIO registers using input
 *   file
 *
 ******************************************************************************/
int uncoreStatusPciMmioCPX(crashdump::CPUInfo& cpuInfo, cJSON* pJsonChild)
{
    int peci_fd = -1;
    int ret = 0;

    cJSON* itRegs = NULL;
    cJSON* itParams = NULL;
    cJSON* regList = NULL;
    bool enable = false;

    regList = getCrashDataSectionRegList(cpuInfo.inputFile.bufferPtr, "uncore",
                                         "mmio", &enable);

    if (regList == NULL)
    {
        cJSON_AddStringToObject(pJsonChild, FILE_MMIO_KEY, FILE_MMIO_ERR);
        return 1;
    }

    if (!enable)
    {
        cJSON_AddFalseToObject(pJsonChild, RECORD_ENABLE);
        return 0;
    }

    ret = peci_Lock(&peci_fd, PECI_WAIT_FOREVER);
    if (ret != PECI_CC_SUCCESS)
    {
        return ret;
    }

    SUncoreStatusRegPciMmioCPX mmioReg = {};

    cJSON_ArrayForEach(itRegs, regList)
    {
        int position = 0;
        cJSON_ArrayForEach(itParams, itRegs)
        {
            switch (position)
            {
                case US_MMIO_REG_NAME:
                    mmioReg.regName = itParams->valuestring;
                    break;
                case US_MMIO_BAR_ID:
                    mmioReg.uMmioReg.fields.bar = itParams->valueint;
                    break;
                case US_MMIO_BUS:
                    mmioReg.uMmioReg.fields.bus = itParams->valueint;
                    break;
                case US_MMIO_DEVICE:
                    mmioReg.uMmioReg.fields.dev = itParams->valueint;
                    break;
                case US_MMIO_FUNCTION:
                    mmioReg.uMmioReg.fields.func = itParams->valueint;
                    break;
                case US_MMIO_OFFSET:
                    mmioReg.uMmioReg.fields.reg =
                        strtoull(itParams->valuestring, NULL, 16);
                    break;
                case US_MMIO_SIZE:
                    mmioReg.uMmioReg.fields.lenCode = itParams->valueint;
                    break;
                default:
                    break;
            }
            position++;
        }
        SUncoreStatusRegRawData sRegData = {};
        uint32_t u32MmioParam = mmioReg.uMmioReg.raw;
        uint8_t u8NumDwords =
            mmioReg.uMmioReg.fields.lenCode == US_MMIO_QWORD ? 2 : 1;
        // Get the MMIO data
        ret = uncoreStatusMmioRead(cpuInfo, u32MmioParam, u8NumDwords,
                                   &sRegData, peci_fd);
        // Log this Uncore Status PCI Register
        uncoreStatusPciJson(mmioReg.regName, &sRegData, pJsonChild);
    }

    peci_Unlock(peci_fd);
    return ret;
}

/******************************************************************************
 *
 *   uncoreStatusMcaRead
 *
 *   This function gathers the Uncore Status MCA registers
 *
 ******************************************************************************/
static int uncoreStatusMcaRead(crashdump::CPUInfo& cpuInfo, uint32_t u32Param,
                               SUncoreStatusMcaRawData* sUncoreStatusMcaRawData,
                               int peci_fd)
{
    uint8_t cc = 0;
    int ret = 0;

    // Open the MCA Bank dump sequence
    ret = peci_WrPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU, VCU_OPEN_SEQ,
                               VCU_UNCORE_MCA_SEQ, sizeof(uint32_t), peci_fd,
                               &cc);
    sUncoreStatusMcaRawData->ret = ret;
    sUncoreStatusMcaRawData->cc = cc;
    if (ret != PECI_CC_SUCCESS)
    {
        // MCA Bank sequence failed, abort the sequence
        peci_WrPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU, VCU_ABORT_SEQ,
                             VCU_UNCORE_MCA_SEQ, sizeof(uint32_t), peci_fd,
                             &cc);

        return ret;
    }

    // Set MCA Bank number
    ret = peci_WrPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU, US_MCA_PARAM,
                               u32Param, sizeof(uint32_t), peci_fd, &cc);
    sUncoreStatusMcaRawData->ret = ret;
    sUncoreStatusMcaRawData->cc = cc;
    if (ret != PECI_CC_SUCCESS)
    {
        // MCA Bank sequence failed, abort the sequence
        peci_WrPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU, VCU_ABORT_SEQ,
                             VCU_UNCORE_MCA_SEQ, sizeof(uint32_t), peci_fd,
                             &cc);

        return ret;
    }

    // Get the MCA Bank Registers
    for (uint8_t u8Dword = 0; u8Dword < US_NUM_MCA_DWORDS; u8Dword++)
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
 *   uncoreStatusIioJson
 *
 *   This function formats the Uncore Status IIO MCA registers into a JSON
 *   object
 *
 ******************************************************************************/
static void uncoreStatusIioJson(const char* regName,
                                SUncoreStatusMcaRawData* sMcaData,
                                cJSON* pJsonChild)
{
    char jsonItemString[US_JSON_STRING_LEN];
    char jsonNameString[US_JSON_STRING_LEN];
    uint32_t i;

    // Format the Uncore Status IIO MCA data out to the .json debug file
    // Fill in NULL for this IIO MCA if it's not valid
    if (sMcaData->bInvalid)
    {
        for (i = 0; i < US_NUM_MCA_QWORDS; i++)
        {
            cd_snprintf_s(jsonNameString, US_JSON_STRING_LEN, regName,
                          uncoreStatusMcaRegNames[i]);
            cd_snprintf_s(jsonItemString, US_JSON_STRING_LEN, US_UA_DF_CPX,
                          sMcaData->cc, sMcaData->ret);
            cJSON_AddStringToObject(pJsonChild, jsonNameString, jsonItemString);
        }
        return;
    }
    else if (PECI_CC_UA(sMcaData->cc))
    {
        for (i = 0; i < US_NUM_MCA_QWORDS; i++)
        {
            cd_snprintf_s(jsonNameString, US_JSON_STRING_LEN, regName,
                          uncoreStatusMcaRegNames[i]);
            cd_snprintf_s(jsonItemString, US_JSON_STRING_LEN, US_UA_CPX,
                          sMcaData->uRegData.u64Raw[i], sMcaData->cc);
            cJSON_AddStringToObject(pJsonChild, jsonNameString, jsonItemString);
        }
        return;
    }
    else
    {
        // Otherwise fill in the register data
        for (i = 0; i < US_NUM_MCA_QWORDS; i++)
        {
            cd_snprintf_s(jsonNameString, US_JSON_STRING_LEN, regName,
                          uncoreStatusMcaRegNames[i]);
            cd_snprintf_s(jsonItemString, US_JSON_STRING_LEN, "0x%llx",
                          sMcaData->uRegData.u64Raw[i]);
            cJSON_AddStringToObject(pJsonChild, jsonNameString, jsonItemString);
        }
    }
}

/******************************************************************************
 *
 *   uncoreStatusIio
 *
 *   This function gathers the Uncore Status IIO MCA registers
 *
 ******************************************************************************/
static int uncoreStatusIio(crashdump::CPUInfo& cpuInfo, cJSON* pJsonChild)
{
    int peci_fd = -1;
    int ret = 0;
    ret = peci_Lock(&peci_fd, PECI_WAIT_FOREVER);
    if (ret != PECI_CC_SUCCESS)
    {
        return ret;
    }

    // Go through each IIO in this CPU
    for (uint32_t i = 0;
         i < (sizeof(sUncoreStatusIio) / sizeof(SUncoreStatusRegIio)); i++)
    {
        SUncoreStatusMcaRawData sMcaData = {};
        // Build the MCA parameter for this IIO
        uint32_t u32IioParam = (sUncoreStatusIio[i].u8IioNum << 24 |
                                US_MCA_UNMERGE | US_BASE_IIO_BANK);

        // Get the IIO MCA data
        ;
        ret = uncoreStatusMcaRead(cpuInfo, u32IioParam, &sMcaData, peci_fd);
        if (ret != PECI_CC_SUCCESS)
        {
            sMcaData.bInvalid = true;
        }

        // Log the MCA for this IIO
        uncoreStatusIioJson(sUncoreStatusIio[i].regName, &sMcaData, pJsonChild);
    }

    peci_Unlock(peci_fd);
    return ret;
}

/******************************************************************************
 *
 *   uncoreStatusCrashdumpJson
 *
 *   This function formats the Uncore Status Crashdump into a JSON object
 *
 ******************************************************************************/
static void uncoreStatusCrashdumpJson(uint32_t u32NumReads,
                                      uint32_t* pu32UncoreCrashdump,
                                      uint8_t* pu8UncoreCc, int* puUncoreRet,
                                      cJSON* pJsonChild)
{
    char jsonItemString[US_JSON_STRING_LEN];
    char jsonNameString[US_JSON_STRING_LEN];

    // Add the Uncore Crashdump dump info to the Uncore Status dump JSON
    // structure
    for (uint32_t i = 0; i < u32NumReads; i++)
    {
        cd_snprintf_s(jsonNameString, US_JSON_STRING_LEN,
                      US_UNCORE_CRASH_DW_NAME, i);
        if (puUncoreRet[i] != PECI_CC_SUCCESS)
        {
            cd_snprintf_s(jsonItemString, US_JSON_STRING_LEN, US_UA_DF_CPX,
                          pu8UncoreCc[i], puUncoreRet[i]);
            cJSON_AddStringToObject(pJsonChild, jsonNameString, jsonItemString);
            return;
        }
        else if (PECI_CC_UA(pu8UncoreCc[i]))
        {
            cd_snprintf_s(jsonItemString, US_JSON_STRING_LEN, US_UA_CPX,
                          pu32UncoreCrashdump[i], pu8UncoreCc[i]);
            cJSON_AddStringToObject(pJsonChild, jsonNameString, jsonItemString);
        }
        else
        {
            cd_snprintf_s(jsonItemString, US_JSON_STRING_LEN, "0x%lx",
                          pu32UncoreCrashdump[i]);
        }
        cJSON_AddStringToObject(pJsonChild, jsonNameString, jsonItemString);
    }
}

/******************************************************************************
 *
 *   uncoreStatusCrashdump
 *
 *   This function gathers the Uncore Status Crashdump
 *
 ******************************************************************************/
static int uncoreStatusCrashdump(crashdump::CPUInfo& cpuInfo, cJSON* pJsonChild)
{
    int peci_fd = -1;
    uint8_t cc = 0;
    int ret = 0;
    int retval = 0;

    ret = peci_Lock(&peci_fd, PECI_WAIT_FOREVER);
    if (ret != PECI_CC_SUCCESS)
    {
        return ret;
    }

    // Start the Uncore Crashdump dump log

    // Open the Uncore Crashdump dump sequence
    ret = peci_WrPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU, VCU_OPEN_SEQ,
                               VCU_UNCORE_CRASHDUMP_SEQ, sizeof(uint32_t),
                               peci_fd, &cc);
    if (ret != PECI_CC_SUCCESS)
    {
        // Uncore Crashdump dump sequence failed, abort the sequence
        peci_WrPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU, VCU_ABORT_SEQ,
                             VCU_UNCORE_CRASHDUMP_SEQ, sizeof(uint32_t),
                             peci_fd, &cc);
        peci_Unlock(peci_fd);
        return ret;
    }

    // Set Uncore Crashdump dump parameter
    ret = peci_WrPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU, VCU_SET_PARAM,
                               US_UCRASH_PARAM, sizeof(uint32_t), peci_fd, &cc);
    if (ret != PECI_CC_SUCCESS)
    {
        // Uncore Crashdump dump sequence failed, abort the sequence
        peci_WrPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU, VCU_ABORT_SEQ,
                             VCU_UNCORE_CRASHDUMP_SEQ, sizeof(uint32_t),
                             peci_fd, &cc);
        peci_Unlock(peci_fd);
        return ret;
    }

    // Get the number of dwords to read
    uint32_t u32NumReads = 0;
    ret = peci_RdPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU,
                               US_UCRASH_START, sizeof(uint32_t),
                               (uint8_t*)&u32NumReads, peci_fd, &cc);
    if (ret != PECI_CC_SUCCESS || (PECI_CC_UA(cc)))
    {
        // Uncore Crashdump dump sequence failed, abort the sequence
        peci_WrPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU, VCU_ABORT_SEQ,
                             VCU_UNCORE_CRASHDUMP_SEQ, sizeof(uint32_t),
                             peci_fd, &cc);
        peci_Unlock(peci_fd);
        return ret;
    }

    // Get the API version number
    uint32_t u32ApiVersion = 0;
    ret = peci_RdPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU, VCU_READ,
                               sizeof(uint32_t), (uint8_t*)&u32ApiVersion,
                               peci_fd, &cc);
    if (ret != PECI_CC_SUCCESS)
    {
        // Uncore Crashdump dump sequence failed, abort the sequence
        peci_WrPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU, VCU_ABORT_SEQ,
                             VCU_UNCORE_CRASHDUMP_SEQ, sizeof(uint32_t),
                             peci_fd, &cc);
        peci_Unlock(peci_fd);
        return ret;
    }
    // API version is included in the number of reads, so decrement by one
    u32NumReads--;

    // Get the raw data
    uint32_t* pu32UncoreCrashdump =
        (uint32_t*)calloc(u32NumReads, sizeof(uint32_t));
    uint8_t* pu32UncoreCc = (uint8_t*)calloc(u32NumReads, sizeof(uint8_t));
    int* pu32UncoreRet = (int*)calloc(u32NumReads, sizeof(int));
    if (pu32UncoreCrashdump == NULL || pu32UncoreCc == NULL ||
        pu32UncoreRet == NULL)
    {
        if (pu32UncoreCrashdump != NULL)
        {
            FREE(pu32UncoreCrashdump);
        }
        if (pu32UncoreRet != NULL)
        {
            FREE(pu32UncoreRet);
        }
        if (pu32UncoreCc != NULL)
        {
            FREE(pu32UncoreCc);
        }
        // calloc failed, abort the sequence
        peci_WrPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU, VCU_ABORT_SEQ,
                             VCU_UNCORE_CRASHDUMP_SEQ, sizeof(uint32_t),
                             peci_fd, &cc);
        peci_Unlock(peci_fd);
        return SIZE_FAILURE;
    }
    for (uint32_t i = 0; i < u32NumReads; i++)
    {
        ret = peci_RdPkgConfig_seq(
            cpuInfo.clientAddr, MBX_INDEX_VCU, VCU_READ, sizeof(uint32_t),
            (uint8_t*)&pu32UncoreCrashdump[i], peci_fd, &cc);
        pu32UncoreRet[i] = ret;
        pu32UncoreCc[i] = cc;
        if (ret != PECI_CC_SUCCESS)
        {
            // Uncore Crashdump dump sequence failed, note the number of dwords
            // read and abort the sequence
            u32NumReads = i;
            peci_WrPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU,
                                 VCU_ABORT_SEQ, VCU_UNCORE_CRASHDUMP_SEQ,
                                 sizeof(uint32_t), peci_fd, &cc);
            retval = ret;
            break;
        }
    }

    // Close the Uncore Crashdump dump sequence
    peci_WrPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU, VCU_CLOSE_SEQ,
                         VCU_UNCORE_CRASHDUMP_SEQ, sizeof(uint32_t), peci_fd,
                         &cc);

    // Log the Uncore Crashdump
    uncoreStatusCrashdumpJson(u32NumReads, pu32UncoreCrashdump, pu32UncoreCc,
                              pu32UncoreRet, pJsonChild);
    if (pu32UncoreCrashdump != NULL)
    {
        FREE(pu32UncoreCrashdump);
    }
    if (pu32UncoreCc != NULL)
    {
        FREE(pu32UncoreCc);
    }
    if (pu32UncoreRet != NULL)
    {
        FREE(pu32UncoreRet);
    }

    peci_Unlock(peci_fd);
    return retval;
}

static UncoreStatusRead UncoreStatusTypesCPX1[] = {
    uncoreStatusCrashdump,
    uncoreStatusPciCPX,
    uncoreStatusPciMmioCPX,
    uncoreStatusIio,
};

/******************************************************************************
 *
 *   logUncoreStatusCPX1
 *
 *   This function gathers the Uncore Status register contents and adds them to
 *   the debug log.
 *
 ******************************************************************************/
int logUncoreStatusCPX1(crashdump::CPUInfo& cpuInfo, cJSON* pJsonChild)
{
    int ret = 0;

    for (uint32_t i = 0;
         i < (sizeof(UncoreStatusTypesCPX1) / sizeof(UncoreStatusTypesCPX1[0]));
         i++)
    {
        if (UncoreStatusTypesCPX1[i](cpuInfo, pJsonChild) != 0)
        {
            ret = 1;
        }
    }

    return ret;
}

/******************************************************************************
 *
 *   uncoreStatusJsonICX
 *
 *   This function formats the Uncore Status PCI registers into a JSON object
 *
 ******************************************************************************/
static void uncoreStatusJsonICX(const char* regName,
                                SUncoreStatusRegRawData* sRegData,
                                cJSON* pJsonChild, uint8_t cc, int ret)
{
    char jsonItemString[US_JSON_STRING_LEN];

    if (sRegData->bInvalid)
    {
        cd_snprintf_s(jsonItemString, US_JSON_STRING_LEN, UNCORE_UA_DF, cc,
                      ret);
        cJSON_AddStringToObject(pJsonChild, regName, jsonItemString);
        return;
    }
    else if (PECI_CC_UA(cc))
    {
        cd_snprintf_s(jsonItemString, US_JSON_STRING_LEN, US_UA, cc);
        cJSON_AddStringToObject(pJsonChild, regName, jsonItemString);
        return;
    }
    else
    {
        cd_snprintf_s(jsonItemString, US_JSON_STRING_LEN, US_UINT64_FMT,
                      sRegData->uValue.u64, cc);
    }

    cJSON_AddStringToObject(pJsonChild, regName, jsonItemString);
}

/******************************************************************************
 *
 *   bus30ToPostEnumeratedBus()
 *
 *   This function is dedicated to converting bus 30 to post enumerated
 *   bus number for MMIO read.
 *
 ******************************************************************************/
static int bus30ToPostEnumeratedBus(uint32_t addr, uint8_t* postEnumBus)
{
    uint32_t cpubusno_valid = 0;
    uint32_t cpubusno2 = 0;
    uint8_t cc = 0;
    int ret = 0;

    // Use PCS Service 76, Parameter 5 to check valid post enumerated bus#
    ret = peci_RdPkgConfig(addr, 76, 5, sizeof(uint32_t),
                           (uint8_t*)&cpubusno_valid, &cc);
    if ((ret != PECI_CC_SUCCESS) || (PECI_CC_UA(cc)))
    {
        CRASHDUMP_PRINT(ERR, stderr,
                        "Unable to read cpubusno_valid - cc: 0x%x ret: 0x%x\n",
                        cc, ret);
        // Need to return 1 for all failures
        return 1;
    }

    // Bit 11 is for checking bus 30 contains valid post enumerated bus#
    if (0 == CHECK_BIT(cpubusno_valid, 11))
    {
        CRASHDUMP_PRINT(ERR, stderr,
                        "Bus 30 does not contain valid post enumerated bus"
                        "number! (0x%x)\n",
                        cpubusno_valid);
        return 1;
    }

    // Use PCS Service 76, Parameter 4 to get raw post enumerated buses value
    ret = peci_RdPkgConfig(addr, 76, 4, sizeof(uint32_t), (uint8_t*)&cpubusno2,
                           &cc);
    if ((ret != PECI_CC_SUCCESS) || (PECI_CC_UA(cc)))
    {
        CRASHDUMP_PRINT(ERR, stderr,
                        "Unable to read cpubusno2 - cc: 0x%x\n ret: 0x%x\n", cc,
                        ret);
        // Need to return 1 for all failures
        return 1;
    }

    // CPUBUSNO2[23:16] for Bus 30
    *postEnumBus = ((cpubusno2 >> 16) & 0xff);

    return 0;
}

/******************************************************************************
 *
 *   uncoreStatusRdIAMSRICX
 *
 *   This function gathers the Uncore Status MSR using input file
 *
 ******************************************************************************/
int uncoreStatusRdIAMSRICX(crashdump::CPUInfo& cpuInfo, cJSON* pJsonChild)
{
    char jsonNameString[US_REG_NAME_LEN];
    int ret = 0;
    uint8_t cc = 0;

    cJSON* regList = NULL;
    cJSON* itRegs = NULL;
    cJSON* itParams = NULL;
    bool enable = false;

    regList = getCrashDataSectionRegList(cpuInfo.inputFile.bufferPtr, "uncore",
                                         "rdiamsr", &enable);

    if (regList == NULL)
    {
        cJSON_AddStringToObject(pJsonChild, FILE_RDIAMSR_KEY, FILE_RDIAMSR_ERR);
        return 1;
    }

    if (!enable)
    {
        cJSON_AddFalseToObject(pJsonChild, RECORD_ENABLE);
        return 0;
    }

    SUncoreStatusMsrRegICX reg{};

    cJSON_ArrayForEach(itRegs, regList)
    {
        int position = 0;
        cJSON_ArrayForEach(itParams, itRegs)
        {
            switch (position)
            {
                case US_RDIAMSR_REG_NAME:
                    reg.regName = itParams->valuestring;
                    break;
                case US_RDIAMSR_ADDR:
                    reg.addr = strtoull(itParams->valuestring, NULL, 16);
                    break;
                case US_RDIAMSR_THREAD_ID:
                    reg.threadID = itParams->valueint;
                    break;
                case US_RDIAMSR_SIZE:
                    reg.u8Size = itParams->valueint;
                    break;
                default:
                    break;
            }
            position++;
        }
        SUncoreStatusRegRawData sRegData{};
        ret = peci_RdIAMSR(cpuInfo.clientAddr, reg.threadID, reg.addr,
                           &sRegData.uValue.u64, &cc);
        cd_snprintf_s(jsonNameString, US_JSON_STRING_LEN, reg.regName);
        if (reg.u8Size == US_REG_DWORD)
        {
            sRegData.uValue.u64 = sRegData.uValue.u64 & 0xFFFFFFFF;
        }
        uncoreStatusJsonICX(jsonNameString, &sRegData, pJsonChild, cc, ret);
    }
    return 0;
}

/******************************************************************************
 *
 *   uncoreStatusPciICX
 *
 *   This function gathers the ICX Uncore Status PCI registers by using
 *   crashdump_input_icx.json
 *
 ******************************************************************************/
int uncoreStatusPciICX(crashdump::CPUInfo& cpuInfo, cJSON* pJsonChild)
{
    int peci_fd = -1;
    int ret = 0;

    cJSON* regList = NULL;
    cJSON* itRegs = NULL;
    cJSON* itParams = NULL;
    bool enable = false;

    regList = getCrashDataSectionRegList(cpuInfo.inputFile.bufferPtr, "uncore",
                                         "pci", &enable);

    if (regList == NULL)
    {
        cJSON_AddStringToObject(pJsonChild, FILE_PCI_KEY, FILE_PCI_ERR);
        return 1;
    }

    if (!enable)
    {
        cJSON_AddFalseToObject(pJsonChild, RECORD_ENABLE);
        return 0;
    }

    ret = peci_Lock(&peci_fd, PECI_WAIT_FOREVER);
    if (ret != PECI_CC_SUCCESS)
    {
        return ret;
    }

    SUncoreStatusRegPciIcxCpx pciReg = {};

    cJSON_ArrayForEach(itRegs, regList)
    {
        int position = 0;
        cJSON_ArrayForEach(itParams, itRegs)
        {
            switch (position)
            {
                case US_PCI_REG_NAME:
                    pciReg.regName = itParams->valuestring;
                    break;
                case US_PCI_BUS:
                    pciReg.u8Bus = itParams->valueint;
                    break;
                case US_PCI_DEVICE:
                    pciReg.u8Dev = itParams->valueint;
                    break;
                case US_PCI_FUNCTION:
                    pciReg.u8Func = itParams->valueint;
                    break;
                case US_PCI_OFFSET:
                    pciReg.u16Reg = strtoull(itParams->valuestring, NULL, 16);
                    break;
                case US_PCI_SIZE:
                    pciReg.u8Size = itParams->valueint;
                    break;
                default:
                    break;
            }
            position++;
        }

        SUncoreStatusRegRawData sRegData = {};
        uint8_t cc = 0;
        uint8_t bus = 0;

        // ICX EDS Reference Section: PCI Configuration Space Registers
        // Note that registers located in Bus 30 and 31
        // have been translated to Bus 13 and 14 respectively for PECI access.
        if (pciReg.u8Bus == 30)
        {
            bus = 13;
        }
        else if (pciReg.u8Bus == 31)
        {
            bus = 14;
        }
        else
        {
            bus = pciReg.u8Bus;
        }

        switch (pciReg.u8Size)
        {
            case US_REG_BYTE:
            case US_REG_WORD:
            case US_REG_DWORD:

                ret = peci_RdEndPointConfigPciLocal_seq(
                    cpuInfo.clientAddr, US_PCI_SEG, bus, pciReg.u8Dev,
                    pciReg.u8Func, pciReg.u16Reg, pciReg.u8Size,
                    (uint8_t*)&sRegData.uValue.u64, peci_fd, &cc);
                if (ret != PECI_CC_SUCCESS)
                {
                    sRegData.bInvalid = true;
                }
                break;
            case US_REG_QWORD:
                for (uint8_t u8Dword = 0; u8Dword < 2; u8Dword++)
                {
                    ret = peci_RdEndPointConfigPciLocal_seq(
                        cpuInfo.clientAddr, US_PCI_SEG, bus, pciReg.u8Dev,
                        pciReg.u8Func, pciReg.u16Reg + (u8Dword * 4),
                        sizeof(uint32_t),
                        (uint8_t*)&sRegData.uValue.u32[u8Dword], peci_fd, &cc);
                    if (ret != PECI_CC_SUCCESS)
                    {
                        sRegData.bInvalid = true;
                        break;
                    }
                }
                break;
            default:
                sRegData.bInvalid = true;
                ret = SIZE_FAILURE;
        }
        uncoreStatusJsonICX(pciReg.regName, &sRegData, pJsonChild, cc, ret);
    }

    peci_Unlock(peci_fd);
    return ret;
}

/******************************************************************************
 *
 *   uncoreStatusPciMmioICX
 *
 *   This function gathers the Uncore Status PCI MMIO registers by using
 *   crashdump_input_icx.json
 *
 ******************************************************************************/

int uncoreStatusPciMmioICX(crashdump::CPUInfo& cpuInfo, cJSON* pJsonChild)
{
    char jsonNameString[US_REG_NAME_LEN];
    int peci_fd = -1;
    int ret = 0;
    uint8_t cc = 0;
    uint8_t postEnumBus = 0;

    cJSON* itRegs = NULL;
    cJSON* itParams = NULL;
    cJSON* regList = NULL;
    bool enable = false;

    regList = getCrashDataSectionRegList(cpuInfo.inputFile.bufferPtr, "uncore",
                                         "mmio", &enable);

    if (regList == NULL)
    {
        cJSON_AddStringToObject(pJsonChild, FILE_MMIO_KEY, FILE_MMIO_ERR);
        return 1;
    }

    if (!enable)
    {
        cJSON_AddFalseToObject(pJsonChild, RECORD_ENABLE);
        return 0;
    }

    ret = peci_Lock(&peci_fd, PECI_WAIT_FOREVER);
    if (ret != PECI_CC_SUCCESS)
    {
        return ret;
    }

    if (0 != bus30ToPostEnumeratedBus(cpuInfo.clientAddr, &postEnumBus))
    {
        peci_Unlock(peci_fd);
        return 1;
    }

    cd_snprintf_s(jsonNameString, US_JSON_STRING_LEN, "B%d", postEnumBus, cc);
    cJSON_AddStringToObject(pJsonChild, "_post_enumerated_B30", jsonNameString);

    SUncoreStatusRegPciMmioICX mmioReg = {};

    cJSON_ArrayForEach(itRegs, regList)
    {
        int position = 0;
        cJSON_ArrayForEach(itParams, itRegs)
        {
            switch (position)
            {
                case US_MMIO_REG_NAME:
                    mmioReg.regName = itParams->valuestring;
                    break;
                case US_MMIO_BAR_ID:
                    mmioReg.u8Bar = itParams->valueint;
                    break;
                case US_MMIO_BUS:
                    mmioReg.u8Bus = itParams->valueint;
                    break;
                case US_MMIO_DEVICE:
                    mmioReg.u8Dev = itParams->valueint;
                    break;
                case US_MMIO_FUNCTION:
                    mmioReg.u8Func = itParams->valueint;
                    break;
                case US_MMIO_OFFSET:
                    mmioReg.u64Offset =
                        strtoull(itParams->valuestring, NULL, 16);
                    break;
                case US_MMIO_ADDRTYPE:
                    mmioReg.u8AddrType = itParams->valueint;
                    break;
                case US_MMIO_SIZE:
                    mmioReg.u8Size = itParams->valueint;
                    break;
                default:
                    break;
            }
            position++;
        }

        SUncoreStatusRegRawData sRegData = {};
        uint8_t readLen = 0;

        switch (mmioReg.u8Size)
        {
            case US_REG_BYTE:
            case US_REG_WORD:
            case US_REG_DWORD:
                readLen = US_REG_DWORD;
                break;
            case US_REG_QWORD:
                readLen = US_REG_QWORD;
                break;
            default:
                sRegData.bInvalid = true;
                ret = SIZE_FAILURE;
        }

        ret = peci_RdEndPointConfigMmio_seq(
            cpuInfo.clientAddr, US_MMIO_SEG, postEnumBus, mmioReg.u8Dev,
            mmioReg.u8Func, mmioReg.u8Bar, mmioReg.u8AddrType,
            mmioReg.u64Offset, readLen, (uint8_t*)&sRegData.uValue.u64, peci_fd,
            &cc);
        if (ret != PECI_CC_SUCCESS)
        {
            sRegData.bInvalid = true;
        }

        uncoreStatusJsonICX(mmioReg.regName, &sRegData, pJsonChild, cc, ret);
    }

    peci_Unlock(peci_fd);
    return ret;
}

static UncoreStatusRead UncoreStatusTypesICX1[] = {
    uncoreStatusPciICX,
    uncoreStatusPciMmioICX,
    uncoreStatusRdIAMSRICX,
};

/******************************************************************************
 *
 *   logUncoreStatusICX1
 *
 *   This function gathers the Uncore Status register contents and adds them to
 *   the debug log.
 *
 ******************************************************************************/
int logUncoreStatusICX1(crashdump::CPUInfo& cpuInfo, cJSON* pJsonChild)
{
    int ret = 0;

    for (uint32_t i = 0;
         i < (sizeof(UncoreStatusTypesICX1) / sizeof(UncoreStatusTypesICX1[0]));
         i++)
    {
        if (UncoreStatusTypesICX1[i](cpuInfo, pJsonChild) != 0)
        {
            ret = 1;
        }
    }

    return ret;
}

static const SUncoreStatusLogVx sUncoreStatusLogVx[] = {
    {crashdump::cpu::clx, logUncoreStatusCPX1},
    {crashdump::cpu::cpx, logUncoreStatusCPX1},
    {crashdump::cpu::skx, logUncoreStatusCPX1},
    {crashdump::cpu::icx, logUncoreStatusICX1},
    {crashdump::cpu::icx2, logUncoreStatusICX1},
    {crashdump::cpu::icxd, logUncoreStatusICX1},
};

/******************************************************************************
 *
 *   logUncoreStatus
 *
 *   This function gathers the Uncore Status register contents and adds them to
 *   the debug log.
 *
 ******************************************************************************/
int logUncoreStatus(crashdump::CPUInfo& cpuInfo, cJSON* pJsonChild)
{
    if (pJsonChild == NULL)
    {
        return 1;
    }

    for (uint32_t i = 0;
         i < (sizeof(sUncoreStatusLogVx) / sizeof(SUncoreStatusLogVx)); i++)
    {
        if (cpuInfo.model == sUncoreStatusLogVx[i].cpuModel)
        {
            revision::revision_uncore = getCrashDataSectionVersion(
                cpuInfo.inputFile.bufferPtr, "uncore");
            logCrashdumpVersion(pJsonChild, cpuInfo,
                                record_type::uncoreStatusLog);
            return sUncoreStatusLogVx[i].logUncoreStatusVx(cpuInfo, pJsonChild);
        }
    }

    CRASHDUMP_PRINT(ERR, stderr, "Cannot find version for %s\n", __FUNCTION__);
    return 1;
}