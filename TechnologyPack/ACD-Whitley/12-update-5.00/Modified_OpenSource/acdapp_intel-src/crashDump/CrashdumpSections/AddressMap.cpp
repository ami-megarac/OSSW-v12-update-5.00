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

#include "AddressMap.hpp"

extern "C" {
#ifndef SPX_BMC_ACD
#include <cjson/cJSON.h>
#endif
#include <stdio.h>
}

#include "crashdump.hpp"
#include "utils.hpp"

/******************************************************************************
 *
 *   addressMapJson
 *
 *   This function formats the Address Map entry into a JSON object
 *
 ******************************************************************************/
static void addressMapJson(const SAddrMapEntry* sAddrMapEntry,
                           uint64_t* u64Data, cJSON* pJsonChild, int ret,
                           uint8_t cc)
{
    char jsonItemString[AM_JSON_STRING_LEN];
    if (u64Data == NULL)
    {
        cd_snprintf_s(jsonItemString, AM_JSON_STRING_LEN, AM_UA_DF_CPX, cc,
                      ret);
    }
    else if (PECI_CC_UA(cc))
    {
        cd_snprintf_s(jsonItemString, AM_JSON_STRING_LEN, AM_UA_CPX, *u64Data,
                      cc);
    }
    else
    {
        cd_snprintf_s(jsonItemString, AM_JSON_STRING_LEN, AM_UINT64_FMT,
                      *u64Data);
    }
    cJSON_AddStringToObject(pJsonChild, sAddrMapEntry->regName, jsonItemString);
}

/******************************************************************************
 *
 *   logAddressMapEntries
 *
 *   This function gathers the Address Map log from the specified PECI client
 *   and adds it to the debug log
 *
 ******************************************************************************/
int logAddressMapEntries(crashdump::CPUInfo& cpuInfo,
                         const SAddrMapEntry* sAddrMapEntries,
                         size_t numAddrMapEntries, cJSON* pJsonChild)
{
    int ret = 0;
    int peci_fd = -1;
    uint8_t cc = 0;

    ret = peci_Lock(&peci_fd, PECI_WAIT_FOREVER);
    if (ret != PECI_CC_SUCCESS)
    {
        return ret;
    }

    // Get the Address Map register values
    for (size_t i = 0; i < numAddrMapEntries; i++)
    {
        UAddrMapRegValue uValue = {};
        uint64_t* pValue = &uValue.u64;
        uint8_t ccVal = PECI_DEV_CC_SUCCESS;
        if (sAddrMapEntries[i].u8Size == 8)
        {
            for (uint8_t u8Dword = 0; u8Dword < 2; u8Dword++)
            {
                ret = peci_RdPCIConfigLocal_seq(
                    cpuInfo.clientAddr, sAddrMapEntries[i].u8Bus,
                    sAddrMapEntries[i].u8Dev, sAddrMapEntries[i].u8Func,
                    sAddrMapEntries[i].u16Reg + (u8Dword * 4), sizeof(uint32_t),
                    (uint8_t*)&uValue.u32[u8Dword], peci_fd, &cc);
                if (PECI_CC_UA(cc))
                {
                    ccVal = cc;
                }
                if (ret != PECI_CC_SUCCESS)
                {
                    pValue = NULL;
                    break;
                }
            }
        }
        else
        {
            ret = peci_RdPCIConfigLocal_seq(
                cpuInfo.clientAddr, sAddrMapEntries[i].u8Bus,
                sAddrMapEntries[i].u8Dev, sAddrMapEntries[i].u8Func,
                sAddrMapEntries[i].u16Reg, sAddrMapEntries[i].u8Size,
                (uint8_t*)&uValue.u64, peci_fd, &cc);
            if (ret != PECI_CC_SUCCESS)
            {
                pValue = NULL;
            }
        }
        if (PECI_CC_UA(ccVal))
        {
            cc = ccVal;
        }
        addressMapJson(&sAddrMapEntries[i], pValue, pJsonChild, ret, cc);
    }

    peci_Unlock(peci_fd);
    return ret;
}

static const SAddrMapEntry sAddrMapEntriesCPX1[] = {
    // Register, Bus, Dev, Fun, Offset, Size
    {"MMCFG_BASE", 0, 5, 0, 0x90, 8},
    {"MMCFG_LIMIT", 0, 5, 0, 0x98, 8},
    {"TSEG", 0, 5, 0, 0xA8, 8},
    {"TOLM", 0, 5, 0, 0xD0, 4},
    {"TOHM_0", 0, 5, 0, 0xD4, 4},
    {"TOHM_1", 0, 5, 0, 0xD8, 4},
    {"NCMEM_BASE_0", 0, 5, 0, 0xE0, 4},
    {"NCMEM_BASE_1", 0, 5, 0, 0xE4, 4},
    {"NCMEM_LIMIT_0", 0, 5, 0, 0xE8, 4},
    {"NCMEM_LIMIT_1", 0, 5, 0, 0xEC, 4},
    {"MENCMEM_BASE_0", 0, 5, 0, 0xF0, 4},
    {"MENCMEM_BASE_1", 0, 5, 0, 0xF4, 4},
    {"MENCMEM_LIMIT_0", 0, 5, 0, 0xF8, 4},
    {"MENCMEM_LIMIT_1", 0, 5, 0, 0xFC, 4},
    {"PCIE0_MMIOH_NON_INTERLEAVE", 0, 5, 0, 0x340, 8},
    {"PCIE0_MMIOH_INTERLEAVE", 0, 5, 0, 0x348, 8},
    {"PCIE1_MMIOH_NON_INTERLEAVE", 1, 5, 0, 0x340, 8},
    {"PCIE1_MMIOH_INTERLEAVE", 1, 5, 0, 0x348, 8},
    {"PAM0123", 1, 29, 0, 0x40, 4},
    {"PAM456", 1, 29, 0, 0x44, 4},
    {"MESEG_BASE", 1, 29, 0, 0x50, 8},
    {"MESEG_LIMIT", 1, 29, 0, 0x58, 8},
    {"DRAM_RULE_CFG_0", 1, 29, 0, 0x60, 4},
    {"INTERLEAVE_LIST_CFG_0", 1, 29, 0, 0x64, 4},
    {"DRAM_RULE_CFG_1", 1, 29, 0, 0x68, 4},
    {"INTERLEAVE_LIST_CFG_1", 1, 29, 0, 0x6C, 4},
    {"DRAM_RULE_CFG_2", 1, 29, 0, 0x70, 4},
    {"INTERLEAVE_LIST_CFG_2", 1, 29, 0, 0x74, 4},
    {"DRAM_RULE_CFG_3", 1, 29, 0, 0x78, 4},
    {"INTERLEAVE_LIST_CFG_3", 1, 29, 0, 0x7C, 4},
    {"DRAM_RULE_CFG_4", 1, 29, 0, 0x80, 4},
    {"INTERLEAVE_LIST_CFG_4", 1, 29, 0, 0x84, 4},
    {"DRAM_RULE_CFG_5", 1, 29, 0, 0x88, 4},
    {"INTERLEAVE_LIST_CFG_5", 1, 29, 0, 0x8C, 4},
    {"DRAM_RULE_CFG_6", 1, 29, 0, 0x90, 4},
    {"INTERLEAVE_LIST_CFG_6", 1, 29, 0, 0x94, 4},
    {"DRAM_RULE_CFG_7", 1, 29, 0, 0x98, 4},
    {"INTERLEAVE_LIST_CFG_7", 1, 29, 0, 0x9C, 4},
    {"DRAM_RULE_CFG_8", 1, 29, 0, 0xA0, 4},
    {"INTERLEAVE_LIST_CFG_8", 1, 29, 0, 0xA4, 4},
    {"DRAM_RULE_CFG_9", 1, 29, 0, 0xA8, 4},
    {"INTERLEAVE_LIST_CFG_9", 1, 29, 0, 0xAC, 4},
    {"DRAM_RULE_CFG_10", 1, 29, 0, 0xB0, 4},
    {"INTERLEAVE_LIST_CFG_10", 1, 29, 0, 0xB4, 4},
    {"DRAM_RULE_CFG_11", 1, 29, 0, 0xB8, 4},
    {"INTERLEAVE_LIST_CFG_11", 1, 29, 0, 0xBC, 4},
    {"DRAM_RULE_CFG_12", 1, 29, 0, 0xC0, 4},
    {"INTERLEAVE_LIST_CFG_12", 1, 29, 0, 0xC4, 4},
    {"DRAM_RULE_CFG_13", 1, 29, 0, 0xC8, 4},
    {"INTERLEAVE_LIST_CFG_13", 1, 29, 0, 0xCC, 4},
    {"DRAM_RULE_CFG_14", 1, 29, 0, 0xD0, 4},
    {"INTERLEAVE_LIST_CFG_14", 1, 29, 0, 0xD4, 4},
    {"DRAM_RULE_CFG_15", 1, 29, 0, 0xD8, 4},
    {"INTERLEAVE_LIST_CFG_15", 1, 29, 0, 0xDC, 4},
    {"DRAM_RULE_CFG_16", 1, 29, 0, 0xE0, 4},
    {"INTERLEAVE_LIST_CFG_16", 1, 29, 0, 0xE4, 4},
    {"DRAM_RULE_CFG_17", 1, 29, 0, 0xE8, 4},
    {"INTERLEAVE_LIST_CFG_17", 1, 29, 0, 0xEC, 4},
    {"DRAM_RULE_CFG_18", 1, 29, 0, 0xF0, 4},
    {"INTERLEAVE_LIST_CFG_18", 1, 29, 0, 0xF4, 4},
    {"DRAM_RULE_CFG_19", 1, 29, 0, 0xF8, 4},
    {"INTERLEAVE_LIST_CFG_19", 1, 29, 0, 0xFC, 4},
    {"DRAM_RULE_CFG_20", 1, 29, 0, 0x100, 4},
    {"INTERLEAVE_LIST_CFG_20", 1, 29, 0, 0x104, 4},
    {"DRAM_RULE_CFG_21", 1, 29, 0, 0x108, 4},
    {"INTERLEAVE_LIST_CFG_21", 1, 29, 0, 0x10C, 4},
    {"DRAM_RULE_CFG_22", 1, 29, 0, 0x110, 4},
    {"INTERLEAVE_LIST_CFG_22", 1, 29, 0, 0x114, 4},
    {"DRAM_RULE_CFG_23", 1, 29, 0, 0x118, 4},
    {"INTERLEAVE_LIST_CFG_23", 1, 29, 0, 0x11C, 4},
    {"IOPORT_TARGET_LIST_CFG_0", 1, 29, 0, 0x2B0, 4},
    {"IOPORT_TARGET_LIST_CFG_1", 1, 29, 0, 0x2B4, 4},
    {"IOPORT_TARGET_LIST_CFG_2", 1, 29, 0, 0x2B8, 4},
    {"IOPORT_TARGET_LIST_CFG_3", 1, 29, 0, 0x2BC, 4},
    {"MMIO_RULE_CFG_0", 1, 29, 1, 0x40, 8},
    {"MMIO_RULE_CFG_1", 1, 29, 1, 0x48, 8},
    {"MMIO_RULE_CFG_2", 1, 29, 1, 0x50, 8},
    {"MMIO_RULE_CFG_3", 1, 29, 1, 0x58, 8},
    {"MMIO_RULE_CFG_4", 1, 29, 1, 0x60, 8},
    {"MMIO_RULE_CFG_5", 1, 29, 1, 0x68, 8},
    {"MMIO_RULE_CFG_6", 1, 29, 1, 0x70, 8},
    {"MMIO_RULE_CFG_7", 1, 29, 1, 0x78, 8},
    {"MMIO_RULE_CFG_8", 1, 29, 1, 0x80, 8},
    {"MMIO_RULE_CFG_9", 1, 29, 1, 0x88, 8},
    {"MMIO_RULE_CFG_10", 1, 29, 1, 0x90, 8},
    {"MMIO_RULE_CFG_11", 1, 29, 1, 0x98, 8},
    {"MMIO_RULE_CFG_12", 1, 29, 1, 0xA0, 8},
    {"MMIO_RULE_CFG_13", 1, 29, 1, 0xA8, 8},
    {"MMIO_RULE_CFG_14", 1, 29, 1, 0xB0, 8},
    {"MMIO_RULE_CFG_15", 1, 29, 1, 0xB8, 8},
    {"MMCFG_RULE", 1, 29, 1, 0xC0, 8},
    {"MMCFG_LOCAL_RULE_ADDRESS_CFG_0", 1, 29, 1, 0xC8, 4},
    {"MMCFG_LOCAL_RULE_ADDRESS_CFG_1", 1, 29, 1, 0xCC, 4},
    {"LT_CONTROL", 1, 29, 1, 0xD0, 4},
    {"IOAPIC_TARGET_LIST_CFG_0", 1, 29, 1, 0xD4, 4},
    {"IOAPIC_TARGET_LIST_CFG_1", 1, 29, 1, 0xD8, 4},
    {"IOAPIC_TARGET_LIST_CFG_2", 1, 29, 1, 0xDC, 4},
    {"IOAPIC_TARGET_LIST_CFG_3", 1, 29, 1, 0xE0, 4},
    {"MMCFG_LOCAL_RULE", 1, 29, 1, 0xE4, 4},
    {"MMIO_TARGET_LIST_CFG_0", 1, 29, 1, 0xE8, 4},
    {"MMCFG_TARGET_LIST", 1, 29, 1, 0xEC, 4},
    {"SAD_TARGET", 1, 29, 1, 0xF0, 4},
    {"SAD_CONTROL", 1, 29, 1, 0xF4, 4},
    {"MMIO_TARGET_LIST_CFG_1", 1, 29, 1, 0xF8, 4},
    {"MMIOH_INTERLEAVE_RULE", 1, 29, 1, 0x100, 8},
    {"MMIOH_NONINTERLEAVE_RULE", 1, 29, 1, 0x108, 8},
    {"MMIO_TARGET_INTERLEAVE_LIST_CFG_0", 1, 29, 1, 0x204, 4},
    {"MMIO_TARGET_INTERLEAVE_LIST_CFG_1", 1, 29, 1, 0x208, 4},
    {"MMIO_TARGET_INTERLEAVE_LIST_CFG_2", 1, 29, 1, 0x20C, 4},
    {"MMIO_TARGET_INTERLEAVE_LIST_CFG_3", 1, 29, 1, 0x210, 4},
    {"PCIE2_MMIOH_NON_INTERLEAVE", 2, 5, 0, 0x340, 8},
    {"PCIE2_MMIOH_INTERLEAVE", 2, 5, 0, 0x348, 8},
    // Process a total of 112 registers
};

static const SAddrMapEntry sAddrMapEntriesICX1[] = {
    // Register, Bus, Dev, Fun, Offset, Size
    {"MMCFG_BASE", 0, 0, 0, 0x90, 8},
    {"MMCFG_LIMIT", 0, 0, 0, 0x98, 8},
    {"TSEG", 0, 0, 0, 0xA8, 8},
    {"TOCM", 0, 0, 0, 0xC0, 1},
    {"TOHM", 0, 0, 0, 0xC8, 8},
    {"TOLM", 0, 0, 0, 0xD0, 4},
    {"TOMMIOL", 0, 0, 0, 0xD8, 4},
    {"NCMEM_BASE", 0, 0, 0, 0xE0, 8},
    {"NCMEM_LIMIT", 0, 0, 0, 0xE8, 8},
    {"MMIO_BASE", 30, 0, 1, 0xD0, 4},
    {"SCF_BAR", 30, 0, 1, 0xD4, 4},
    {"MEM0_BAR", 30, 0, 1, 0xD8, 4},
    {"MEM1_BAR", 30, 0, 1, 0xDC, 4},
    {"MEM2_BAR", 30, 0, 1, 0xE0, 4},
    {"MEM3_BAR", 30, 0, 1, 0xE4, 4},
    {"MEM4_BAR", 30, 0, 1, 0xE8, 4},
    {"MEM5_BAR", 30, 0, 1, 0xEC, 4},
    {"MEM6_BAR", 30, 0, 1, 0xF0, 4},
    {"MEM7_BAR", 30, 0, 1, 0xF4, 4},
    {"SBREG_BAR", 30, 0, 1, 0xF8, 4},
    {"PCU_BAR", 30, 0, 1, 0xFC, 4},
    {"PAM0123_CFG", 31, 29, 0, 0x80, 4},
    {"PAM456_CFG", 31, 29, 0, 0x84, 4},
    {"MESEG_BASE_CFG", 31, 29, 0, 0x90, 8},
    {"MESEG_LIMIT_CFG", 31, 29, 0, 0x98, 8},
    {"SMMSEG_BASE_CFG", 31, 29, 0, 0xA0, 8},
    {"SMMSEG_LIMIT_CFG", 31, 29, 0, 0xA8, 8},
    {"DRAM_RULE_CFG_0", 31, 29, 0, 0x108, 4},
    {"INTERLEAVE_LIST_CFG_0", 31, 29, 0, 0x10C, 4},
    {"DRAM_RULE_CFG_1", 31, 29, 0, 0x110, 4},
    {"INTERLEAVE_LIST_CFG_1", 31, 29, 0, 0x114, 4},
    {"DRAM_RULE_CFG_2", 31, 29, 0, 0x118, 4},
    {"INTERLEAVE_LIST_CFG_2", 31, 29, 0, 0x11C, 4},
    {"DRAM_RULE_CFG_3", 31, 29, 0, 0x120, 4},
    {"INTERLEAVE_LIST_CFG_3", 31, 29, 0, 0x124, 4},
    {"DRAM_RULE_CFG_4", 31, 29, 0, 0x128, 4},
    {"INTERLEAVE_LIST_CFG_4", 31, 29, 0, 0x12C, 4},
    {"DRAM_RULE_CFG_5", 31, 29, 0, 0x130, 4},
    {"INTERLEAVE_LIST_CFG_5", 31, 29, 0, 0x134, 4},
    {"DRAM_RULE_CFG_6", 31, 29, 0, 0x138, 4},
    {"INTERLEAVE_LIST_CFG_6", 31, 29, 0, 0x13C, 4},
    {"DRAM_RULE_CFG_7", 31, 29, 0, 0x140, 4},
    {"INTERLEAVE_LIST_CFG_7", 31, 29, 0, 0x144, 4},
    {"DRAM_RULE_CFG_8", 31, 29, 0, 0x148, 4},
    {"INTERLEAVE_LIST_CFG_8", 31, 29, 0, 0x14C, 4},
    {"DRAM_RULE_CFG_9", 31, 29, 0, 0x150, 4},
    {"INTERLEAVE_LIST_CFG_9", 31, 29, 0, 0x154, 4},
    {"DRAM_RULE_CFG_10", 31, 29, 0, 0x158, 4},
    {"INTERLEAVE_LIST_CFG_10", 31, 29, 0, 0x15C, 4},
    {"DRAM_RULE_CFG_11", 31, 29, 0, 0x160, 4},
    {"INTERLEAVE_LIST_CFG_11", 31, 29, 0, 0x164, 4},
    {"DRAM_RULE_CFG_12", 31, 29, 0, 0x168, 4},
    {"INTERLEAVE_LIST_CFG_12", 31, 29, 0, 0x16C, 4},
    {"DRAM_RULE_CFG_13", 31, 29, 0, 0x170, 4},
    {"INTERLEAVE_LIST_CFG_13", 31, 29, 0, 0x174, 4},
    {"DRAM_RULE_CFG_14", 31, 29, 0, 0x178, 4},
    {"INTERLEAVE_LIST_CFG_14", 31, 29, 0, 0x17C, 4},
    {"DRAM_RULE_CFG_15", 31, 29, 0, 0x180, 4},
    {"INTERLEAVE_LIST_CFG_15", 31, 29, 0, 0x184, 4},
    {"IOPORT_TARGET_LIST_CFG_0", 31, 29, 0, 0x2C0, 4},
    {"IOPORT_TARGET_LIST_CFG_1", 31, 29, 0, 0x2C4, 4},
    {"IOPORT_TARGET_LIST_CFG_2", 31, 29, 0, 0x2C8, 4},
    {"IOPORT_TARGET_LIST_CFG_3", 31, 29, 0, 0x2CC, 4},
    {"MMCFG_RULE_CFG", 31, 29, 1, 0xC0, 8},
    {"MMCFG_LOCAL_RULE_ADDRESS_CFG_0", 31, 29, 1, 0xC8, 4},
    {"MMCFG_LOCAL_RULE_ADDRESS_CFG_1", 31, 29, 1, 0xCC, 4},
    {"MMCFG_LOCAL_RULE_CFG", 31, 29, 1, 0xE4, 4},
    {"MMCFG_TARGET_LIST_CFG", 31, 29, 1, 0xEC, 4},
    {"LT_CONTROL_CFG", 31, 29, 1, 0xF0, 4},
    {"SAD_CONTROL_CFG", 31, 29, 1, 0xF4, 4},
    {"SAD_TARGET_CFG", 31, 29, 1, 0xF8, 4},
    {"MMIO_TARGET_LIST_CFG_0", 31, 29, 1, 0x1A0, 4},
    {"MMIO_TARGET_LIST_CFG_1", 31, 29, 1, 0x1A4, 4},
    {"MMIO_RULE_CFG_0", 31, 29, 1, 0x108, 8},
    {"MMIO_RULE_CFG_1", 31, 29, 1, 0x110, 8},
    {"MMIO_RULE_CFG_2", 31, 29, 1, 0x118, 8},
    {"MMIO_RULE_CFG_3", 31, 29, 1, 0x120, 8},
    {"MMIO_RULE_CFG_4", 31, 29, 1, 0x128, 8},
    {"MMIO_RULE_CFG_5", 31, 29, 1, 0x130, 8},
    {"MMIO_RULE_CFG_6", 31, 29, 1, 0x138, 8},
    {"MMIO_RULE_CFG_7", 31, 29, 1, 0x140, 8},
    {"MMIO_RULE_CFG_8", 31, 29, 1, 0x148, 8},
    {"MMIO_RULE_CFG_9", 31, 29, 1, 0x150, 8},
    {"MMIO_RULE_CFG_10", 31, 29, 1, 0x158, 8},
    {"MMIO_RULE_CFG_11", 31, 29, 1, 0x160, 8},
    {"MMIO_RULE_CFG_12", 31, 29, 1, 0x168, 8},
    {"MMIO_RULE_CFG_13", 31, 29, 1, 0x170, 8},
    {"MMIO_UBOX_RULE_CFG", 31, 29, 1, 0x180, 8},
    {"MMIOH_RULE_CFG_0", 31, 29, 1, 0x1D8, 8},
    {"MMIOH_RULE_CFG_1", 31, 29, 1, 0x1E0, 8},
    {"MMIOH_RULE_CFG_2", 31, 29, 1, 0x1E8, 8},
    {"MMIOH_RULE_CFG_3", 31, 29, 1, 0x1F0, 8},
    {"MMIOH_RULE_CFG_4", 31, 29, 1, 0x1F8, 8},
    {"MMIOH_RULE_CFG_5", 31, 29, 1, 0x200, 8},
    {"MMIOH_RULE_CFG_6", 31, 29, 1, 0x208, 8},
    {"MMIOH_RULE_CFG_7", 31, 29, 1, 0x210, 8},
    {"MMIOH_RULE_CFG_8", 31, 29, 1, 0x218, 8},
    {"MMIOH_RULE_CFG_9", 31, 29, 1, 0x220, 8},
    {"MMIOH_RULE_CFG_10", 31, 29, 1, 0x228, 8},
    {"MMIOH_RULE_CFG_11", 31, 29, 1, 0x230, 8},
    {"MMIOH_RULE_CFG_12", 31, 29, 1, 0x238, 8},
    {"IOAPIC_TARGET_LIST_CFG_0", 31, 29, 1, 0xD0, 4},
    {"IOAPIC_TARGET_LIST_CFG_1", 31, 29, 1, 0xD4, 4},
    {"IOAPIC_TARGET_LIST_CFG_2", 31, 29, 1, 0xD8, 4},
    {"IOAPIC_TARGET_LIST_CFG_3", 31, 29, 1, 0xDC, 4},
    // Process a total of 105 registers
};

/******************************************************************************
 *
 *   logAddressMapCPX1
 *
 *   This function logs the CPX1 Address Map
 *
 ******************************************************************************/
int logAddressMapCPX1(crashdump::CPUInfo& cpuInfo, cJSON* pJsonChild)
{
    return logAddressMapEntries(
        cpuInfo, sAddrMapEntriesCPX1,
        sizeof(sAddrMapEntriesCPX1) / sizeof(SAddrMapEntry), pJsonChild);
}

/******************************************************************************
 *
 *   addressMapJsonICX
 *
 *   This function formats the Address Map PCI registers into a JSON object
 *
 ******************************************************************************/
static void addressMapJsonICX(const char* regName,
                              SAddressMapRegRawData* sRegData,
                              cJSON* pJsonChild, uint8_t cc, int ret)
{
    char jsonItemString[AM_JSON_STRING_LEN];

    if (sRegData->bInvalid)
    {
        cd_snprintf_s(jsonItemString, AM_JSON_STRING_LEN, AM_UA_DF, cc, ret);
    }
    else if (PECI_CC_UA(cc))
    {
        cd_snprintf_s(jsonItemString, AM_JSON_STRING_LEN, AM_UA, cc);
    }
    else
    {
        cd_snprintf_s(jsonItemString, AM_JSON_STRING_LEN, AM_UINT64_FMT,
                      sRegData->uValue.u64, cc);
    }

    cJSON_AddStringToObject(pJsonChild, regName, jsonItemString);
}

/******************************************************************************
 *
 *   logAddressMapEntriesICX1
 *
 *   This function gathers the Address Map PCI registers
 *
 ******************************************************************************/
int logAddressMapEntriesICX1(crashdump::CPUInfo& cpuInfo, cJSON* pJsonChild)
{
    int peci_fd = -1;
    int ret = 0;

    ret = peci_Lock(&peci_fd, PECI_WAIT_FOREVER);
    if (ret != PECI_CC_SUCCESS)
    {
        return ret;
    }

    for (uint32_t i = 0;
         i < (sizeof(sAddrMapEntriesICX1) / sizeof(SAddrMapEntry)); i++)
    {
        SAddressMapRegRawData sRegData = {};
        uint8_t cc = 0;
        uint8_t bus = 0;
        uint8_t ccVal = PECI_DEV_CC_SUCCESS;

        // ICX EDS Reference Section: PCI Configuration Space Registers
        // Note that registers located in Bus 30 and 31
        // have been translated to Bus 13 and 14 respectively for PECI access.
        if (sAddrMapEntriesICX1[i].u8Bus == 30)
        {
            bus = 13;
        }
        else if (sAddrMapEntriesICX1[i].u8Bus == 31)
        {
            bus = 14;
        }
        else
        {
            bus = sAddrMapEntriesICX1[i].u8Bus;
        }

        switch (sAddrMapEntriesICX1[i].u8Size)
        {
            case AM_REG_BYTE:
            case AM_REG_WORD:
            case AM_REG_DWORD:
                ret = peci_RdEndPointConfigPciLocal_seq(
                    cpuInfo.clientAddr, AM_PCI_SEG, bus,
                    sAddrMapEntriesICX1[i].u8Dev, sAddrMapEntriesICX1[i].u8Func,
                    sAddrMapEntriesICX1[i].u16Reg,
                    sAddrMapEntriesICX1[i].u8Size,
                    (uint8_t*)&sRegData.uValue.u64, peci_fd, &cc);
                if (ret != PECI_CC_SUCCESS)
                {
                    sRegData.bInvalid = true;
                }
                break;
            case AM_REG_QWORD:
                for (uint8_t u8Dword = 0; u8Dword < 2; u8Dword++)
                {
                    ret = peci_RdEndPointConfigPciLocal_seq(
                        cpuInfo.clientAddr, AM_PCI_SEG, bus,
                        sAddrMapEntriesICX1[i].u8Dev,
                        sAddrMapEntriesICX1[i].u8Func,
                        sAddrMapEntriesICX1[i].u16Reg + (u8Dword * 4),
                        sizeof(uint32_t),
                        (uint8_t*)&sRegData.uValue.u32[u8Dword], peci_fd, &cc);
                    if (PECI_CC_UA(cc))
                    {
                        ccVal = cc;
                    }
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
        if (PECI_CC_UA(ccVal))
        {
            cc = ccVal;
        }
        addressMapJsonICX(sAddrMapEntriesICX1[i].regName, &sRegData, pJsonChild,
                          cc, ret);
    }

    peci_Unlock(peci_fd);
    return ret;
}

/******************************************************************************
 *
 *   logAddressMapICX1
 *
 *   This function logs the ICX1 Address Map
 *
 ******************************************************************************/
int logAddressMapICX1(crashdump::CPUInfo& cpuInfo, cJSON* pJsonChild)
{
    return logAddressMapEntriesICX1(cpuInfo, pJsonChild);
}

static const SAddrMapVx sAddrMapVx[] = {
    {crashdump::cpu::clx, logAddressMapCPX1},
    {crashdump::cpu::cpx, logAddressMapCPX1},
    {crashdump::cpu::skx, logAddressMapCPX1},
    {crashdump::cpu::icx, logAddressMapICX1},
    {crashdump::cpu::icx2, logAddressMapICX1},
    {crashdump::cpu::icxd, logAddressMapICX1},
};

/******************************************************************************
 *
 *   logAddressMap
 *
 *   This function gathers the Address Map log and adds it to the debug log
 *
 ******************************************************************************/
int logAddressMap(crashdump::CPUInfo& cpuInfo, cJSON* pJsonChild)
{
    if (pJsonChild == NULL)
    {
        return 1;
    }

    for (uint32_t i = 0; i < (sizeof(sAddrMapVx) / sizeof(SAddrMapVx)); i++)
    {
        if (cpuInfo.model == sAddrMapVx[i].cpuModel)
        {
            return sAddrMapVx[i].logAddrMapVx(cpuInfo, pJsonChild);
        }
    }

    CRASHDUMP_PRINT(ERR, stderr, "Cannot find version for %s\n", __FUNCTION__);
    return 1;
}
