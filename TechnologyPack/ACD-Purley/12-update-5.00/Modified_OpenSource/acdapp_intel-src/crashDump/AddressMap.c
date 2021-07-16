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

#include "common.h"
#ifndef SPX_BMC_ACD
#include <cJSON.h>
#endif
#include "AddressMap.h"


#ifdef BUILD_JSON
static SSysMemRegion sSysMem[] =
{
    { "TSEG0", 1 },
    { "TSEG1", 1 },
    { "TSEG2", 1 },
    { "TSEG3", 1 },
    { "MESEG", 2 },
    { "MMCFG IIO", 2 },
    { "MMCFG PCIE", 1 },
    { "TOLM - TOHM", 2 },
};
#endif //BUILD_JSON

/******************************************************************************
*
*   addressMapJson
*
*   This function formats the Address Map log into a JSON object
*
******************************************************************************/
#ifdef BUILD_JSON
static void addressMapJson(SAddrMapRawData * sAddrMapRawData, cJSON * pJsonChild)
{
    cJSON * sysMem;
    cJSON * mmioMem;
    cJSON * socket;
    cJSON * entry;
    char jsonItemName[AM_JSON_STRING_LEN];
    char jsonItemString[AM_JSON_STRING_LEN];
    UINT32 i;
    UINT8 u8Cpu, u8Reg;
    UINT32 u32SysAddrIndex = 0;

    // Add the sys_mem object to the JSON structure
    cJSON_AddItemToObject(pJsonChild, AM_JSON_SYS_MEM_NAME, sysMem = cJSON_CreateObject());
    // Fill in each entry in the sys_mem object except for the final TOLM-TOHM Overall entry
    for (i = 0; i < (sizeof(sSysMem) / sizeof(SSysMemRegion)) - 1; i++) {
        snprintf(jsonItemName, AM_JSON_STRING_LEN, AM_JSON_ENTRY_NAME, i);
        cJSON_AddItemToObject(sysMem, jsonItemName, entry = cJSON_CreateObject());
        // Add the name for this region
        cJSON_AddStringToObject(entry, AM_JSON_REGION_NAME, sSysMem[i].regionName);
        // Add all registers for this region
        for (u8Reg = 0; u8Reg < sSysMem[i].u8NumRegs; u8Reg++) {
            snprintf(jsonItemString, AM_JSON_STRING_LEN, "%08x%08x", sAddrMapRawData->sysAddr[u32SysAddrIndex].uValue.u32[1], sAddrMapRawData->sysAddr[u32SysAddrIndex].uValue.u32[0]);
            cJSON_AddStringToObject(entry, sAddrMapRawData->sysAddr[u32SysAddrIndex++].regName, jsonItemString);
        }
    }
    // Fill in the TOLM-TOHM Overall entry
    cJSON_AddItemToObject(sysMem, AM_JSON_OVERALL_ENTRY_NAME, entry = cJSON_CreateObject());
    // Add the name for this region
    cJSON_AddStringToObject(entry, AM_JSON_REGION_NAME, sSysMem[i].regionName);
    // Add all registers for this region
    for (u8Reg = 0; u8Reg < sSysMem[i].u8NumRegs; u8Reg++) {
        snprintf(jsonItemString, AM_JSON_STRING_LEN, "%08x%08x", sAddrMapRawData->sysAddr[u32SysAddrIndex].uValue.u32[1], sAddrMapRawData->sysAddr[u32SysAddrIndex].uValue.u32[0]);
        cJSON_AddStringToObject(entry, sAddrMapRawData->sysAddr[u32SysAddrIndex++].regName, jsonItemString);
    }

    // Add the mmio_mem object to the JSON structure
    cJSON_AddItemToObject(pJsonChild, AM_JSON_MMIO_MEM_NAME, mmioMem = cJSON_CreateObject());
    // Fill in each entry in the mmio_mem object
    for (u8Cpu = 0; u8Cpu < MAX_CPU; u8Cpu++) {
        if (!IsCpuPresent(u8Cpu)) {
            continue;
        }
        // Add a socket object to the JSON structure
        snprintf(jsonItemName, AM_JSON_STRING_LEN, AM_JSON_SOCKET_NAME, u8Cpu);
        cJSON_AddItemToObject(mmioMem, jsonItemName, socket = cJSON_CreateObject());
        // Fill in the mmio entries for each socket
        for (i = 0; i < AM_NUM_MMIO; i++) {
            snprintf(jsonItemName, AM_JSON_STRING_LEN, AM_JSON_ENTRY_NAME, i);
            cJSON_AddItemToObject(socket, jsonItemName, entry = cJSON_CreateObject());
            // Add the register for this region
            snprintf(jsonItemString, AM_JSON_STRING_LEN, "%08x%08x", sAddrMapRawData->mmioAddr[u8Cpu][i].uValue.u32[1], sAddrMapRawData->mmioAddr[u8Cpu][i].uValue.u32[0]);
            cJSON_AddStringToObject(entry, sAddrMapRawData->mmioAddr[u8Cpu][i].regName, jsonItemString);
        }
    }
}
#endif //BUILD_JSON

#ifdef BUILD_RAW
static void addressMapRaw(SAddrMapRawData * sAddrMapRawData, FILE * fpRaw)
{
    fwrite(sAddrMapRawData, sizeof(SAddrMapRawData), 1, fpRaw);
}
#endif //BUILD_RAW

/******************************************************************************
*
*   logAddressMap
*
*   This function gathers the Address Map log and adds it to the debug log
*
******************************************************************************/
ESTATUS logAddressMap(FILE * fpRaw, FILE * fp, cJSON * pJsonChild)
{
    ESTATUS eStatus = ST_OK;
    UINT32 i;
    UINT8 u8Cpu, u8Dword;
#ifdef SPX_BMC_ACD
    UN_USED(fp);
#endif

    SAddrMapRawData sAddrMapRawData =
    {
        {   // System Memory
            // Register,        Bus,            Dev,            Func,               Offset,         Size
#ifndef SPX_BMC_ACD
            { "TSEG0",          AM_LOCAL_BUS_0, AM_DEV_MMAP_5,  AM_FUNC_MMAP_0,     TSEG,           AM_REG_QWORD },
            { "TSEG1",          AM_LOCAL_BUS_1, AM_DEV_MMAP_5,  AM_FUNC_MMAP_0,     TSEG,           AM_REG_QWORD },
            { "TSEG2",          AM_LOCAL_BUS_2, AM_DEV_MMAP_5,  AM_FUNC_MMAP_0,     TSEG,           AM_REG_QWORD },
            { "TSEG3",          AM_LOCAL_BUS_3, AM_DEV_MMAP_5,  AM_FUNC_MMAP_0,     TSEG,           AM_REG_QWORD },
            { "MESEG_BASE",     AM_LOCAL_BUS_1, AM_DEV_CHA_29,  AM_FUNC_SAD_ALL_0,  MESEG_BASE,     AM_REG_QWORD },
            { "MESEG_LIMIT",    AM_LOCAL_BUS_1, AM_DEV_CHA_29,  AM_FUNC_SAD_ALL_0,  MESEG_LIMIT,    AM_REG_QWORD },
            { "MMCFG_BASE",     AM_LOCAL_BUS_0, AM_DEV_MMAP_5,  AM_FUNC_MMAP_0,     MMCFG_BASE,     AM_REG_QWORD },
            { "MMCFG_LIMIT",    AM_LOCAL_BUS_0, AM_DEV_MMAP_5,  AM_FUNC_MMAP_0,     MMCFG_LIMIT,    AM_REG_QWORD },
            { "MMCFG_Rule",     AM_LOCAL_BUS_1, AM_DEV_CHA_29,  AM_FUNC_UTIL_ALL_1, MMCFG_Rule,     AM_REG_QWORD },
            { "TOLM",           AM_LOCAL_BUS_0, AM_DEV_MMAP_5,  AM_FUNC_MMAP_0,     TOLM,           AM_REG_DWORD },
            { "TOHM",           AM_LOCAL_BUS_0, AM_DEV_MMAP_5,  AM_FUNC_MMAP_0,     TOHM,           AM_REG_QWORD },
#else
            { "TSEG0",          AM_LOCAL_BUS_0, AM_DEV_MMAP_5,  AM_FUNC_MMAP_0,     TSEG,           AM_REG_QWORD,	{0} },
            { "TSEG1",          AM_LOCAL_BUS_1, AM_DEV_MMAP_5,  AM_FUNC_MMAP_0,     TSEG,           AM_REG_QWORD,	{0} },
            { "TSEG2",          AM_LOCAL_BUS_2, AM_DEV_MMAP_5,  AM_FUNC_MMAP_0,     TSEG,           AM_REG_QWORD,	{0} },
            { "TSEG3",          AM_LOCAL_BUS_3, AM_DEV_MMAP_5,  AM_FUNC_MMAP_0,     TSEG,           AM_REG_QWORD,	{0} },
            { "MESEG_BASE",     AM_LOCAL_BUS_1, AM_DEV_CHA_29,  AM_FUNC_SAD_ALL_0,  MESEG_BASE,     AM_REG_QWORD,	{0} },
            { "MESEG_LIMIT",    AM_LOCAL_BUS_1, AM_DEV_CHA_29,  AM_FUNC_SAD_ALL_0,  MESEG_LIMIT,    AM_REG_QWORD,	{0} },
            { "MMCFG_BASE",     AM_LOCAL_BUS_0, AM_DEV_MMAP_5,  AM_FUNC_MMAP_0,     MMCFG_BASE,     AM_REG_QWORD,	{0} },
            { "MMCFG_LIMIT",    AM_LOCAL_BUS_0, AM_DEV_MMAP_5,  AM_FUNC_MMAP_0,     MMCFG_LIMIT,    AM_REG_QWORD,	{0} },
            { "MMCFG_Rule",     AM_LOCAL_BUS_1, AM_DEV_CHA_29,  AM_FUNC_UTIL_ALL_1, MMCFG_Rule,     AM_REG_QWORD,	{0} },
            { "TOLM",           AM_LOCAL_BUS_0, AM_DEV_MMAP_5,  AM_FUNC_MMAP_0,     TOLM,           AM_REG_DWORD,	{0} },
            { "TOHM",           AM_LOCAL_BUS_0, AM_DEV_MMAP_5,  AM_FUNC_MMAP_0,     TOHM,           AM_REG_QWORD,	{0} },
#endif
        },
		{
		},
    };
    unsigned int work, bus, device, func, reg;
    SRdPCIConfigLocalReq sRdPCIConfigLocalReq;
    SRdPCIConfigLocalRes sRdPCIConfigLocalRes;

    PRINT(PRINT_DBG, PRINT_INFO, "Platform Debug - Address Map\n");

    // Get the System Memory Address Map
    for (i = 0; i < (sizeof(sAddrMapRawData.sysAddr) / sizeof(SAddrMapReg)); i++) {
        for (u8Dword = 0; u8Dword < sAddrMapRawData.sysAddr[i].u8Dwords; u8Dword++) {
            memset(&sRdPCIConfigLocalReq, 0 , sizeof(SRdPCIConfigLocalReq));
            memset(&sRdPCIConfigLocalRes, 0 , sizeof(SRdPCIConfigLocalRes));
            sRdPCIConfigLocalReq.sHeader.u8ClientAddr = PECI_BASE_ADDR;
            sRdPCIConfigLocalReq.sHeader.u8WriteLength = 0x05;
            sRdPCIConfigLocalReq.sHeader.u8ReadLength = sizeof(UINT32) + 1;
            sRdPCIConfigLocalReq.u8CmdCode = 0xE1;
            sRdPCIConfigLocalReq.u8HostID_Retry = 0x02;
            bus = sAddrMapRawData.sysAddr[i].u8Bus;
            device = sAddrMapRawData.sysAddr[i].u8Dev;
            func = sAddrMapRawData.sysAddr[i].u8Func;
            reg = sAddrMapRawData.sysAddr[i].u16Reg + (u8Dword * 4);
            work = 0x00;
            work |= (bus << 20) & 0x00F00000;
            work |= (device << 15) & 0x000f8000;
            work |= (func << 12) & 0x00007000;
            work |= reg & 0x00000FFF;
            sRdPCIConfigLocalReq.u8PCIConfigAddr[0] = work & 0x000000FF;
            sRdPCIConfigLocalReq.u8PCIConfigAddr[1] = (work >> 8) & 0x000000FF;
            sRdPCIConfigLocalReq.u8PCIConfigAddr[2] = (work >> 16) & 0x000000FF;
            if (0 == PECI_RdPCIConfigLocal (&sRdPCIConfigLocalReq, &sRdPCIConfigLocalRes))
            {
                memcpy(&sAddrMapRawData.sysAddr[i].uValue.u32[u8Dword], sRdPCIConfigLocalRes.u8Data, sizeof(UINT32));
            }
            else
            {
                sRdPCIConfigLocalRes.u8CompletionCode = 0x00;
            }
            if (sRdPCIConfigLocalRes.u8CompletionCode != PECI_CC_SUCCESS) {
                PRINT(PRINT_DBG, PRINT_ERROR, "%s (BDF %x:%x:%x reg %x) Local PCI Failed\n", sAddrMapRawData.sysAddr[i].regName, sAddrMapRawData.sysAddr[i].u8Bus, sAddrMapRawData.sysAddr[i].u8Dev, sAddrMapRawData.sysAddr[i].u8Func, sAddrMapRawData.sysAddr[i].u16Reg + (u8Dword * 4));
                eStatus = ST_HW_FAILURE;
            }
        }
    }

    // Get the MMIO Memory Address Map
    for (u8Cpu = 0; u8Cpu < MAX_CPU; u8Cpu++) {
        if (!IsCpuPresent(u8Cpu)) {
            continue;
        }
        for (i = 0; i < AM_NUM_MMIO; i++) {
            for (u8Dword = 0; u8Dword < AM_REG_QWORD; u8Dword++) {
                // Fill in the register name
                snprintf(sAddrMapRawData.mmioAddr[u8Cpu][i].regName, AM_REG_NAME_LEN, AM_MMIO_REG_NAME_BASE"%d", i);
                memset(&sRdPCIConfigLocalReq, 0 , sizeof(SRdPCIConfigLocalReq));
                memset(&sRdPCIConfigLocalRes, 0 , sizeof(SRdPCIConfigLocalRes));
                sRdPCIConfigLocalReq.sHeader.u8ClientAddr = PECI_BASE_ADDR + u8Cpu;
                sRdPCIConfigLocalReq.sHeader.u8WriteLength = 0x05;
                sRdPCIConfigLocalReq.sHeader.u8ReadLength = sizeof(UINT32) + 1;
                sRdPCIConfigLocalReq.u8CmdCode = 0xE1;
                sRdPCIConfigLocalReq.u8HostID_Retry = 0x02;
                bus = AM_LOCAL_BUS_1;
                device = AM_DEV_CHA_29;
                func = AM_FUNC_UTIL_ALL_1;
                reg = MMIO_RULE_CFG_BASE + (i * 8) + (u8Dword * 4);
                work = 0x00;
                work |= (bus << 20) & 0x00F00000;
                work |= (device << 15) & 0x000f8000;
                work |= (func << 12) & 0x00007000;
                work |= reg & 0x00000FFF;
                sRdPCIConfigLocalReq.u8PCIConfigAddr[0] = work & 0x000000FF;
                sRdPCIConfigLocalReq.u8PCIConfigAddr[1] = (work >> 8) & 0x000000FF;
                sRdPCIConfigLocalReq.u8PCIConfigAddr[2] = (work >> 16) & 0x000000FF;
                if (0 == PECI_RdPCIConfigLocal (&sRdPCIConfigLocalReq, &sRdPCIConfigLocalRes))
                {
                    memcpy(&sAddrMapRawData.mmioAddr[u8Cpu][i].uValue.u32[u8Dword], sRdPCIConfigLocalRes.u8Data, sizeof(UINT32));
                }
                else
                {
                    sRdPCIConfigLocalRes.u8CompletionCode = 0x00;
                }
                if (sRdPCIConfigLocalRes.u8CompletionCode != PECI_CC_SUCCESS) {
#ifndef SPX_BMC_ACD
                    PRINT(PRINT_DBG, PRINT_ERROR, "CPU #%d %s (BDF %x:%x:%x reg %lx) Local PCI Failed\n", u8Cpu, sAddrMapRawData.mmioAddr[u8Cpu][i].regName, AM_LOCAL_BUS_1, AM_DEV_CHA_29, AM_FUNC_UTIL_ALL_1, MMIO_RULE_CFG_BASE + (i * 8) + (u8Dword * 4));
#else
                    PRINT(PRINT_DBG, PRINT_ERROR, "CPU #%d %s (BDF %x:%x:%x reg %x) Local PCI Failed\n", u8Cpu, sAddrMapRawData.mmioAddr[u8Cpu][i].regName, AM_LOCAL_BUS_1, AM_DEV_CHA_29, AM_FUNC_UTIL_ALL_1, MMIO_RULE_CFG_BASE + (i * 8) + (u8Dword * 4));
#endif						
                    eStatus = ST_HW_FAILURE;
                }
            }
        }
    }

    // Log the Address Map
#ifdef BUILD_RAW
    if (fpRaw != NULL) {
        addressMapRaw(&sAddrMapRawData, fpRaw);
    }
#endif //BUILD_RAW

#ifdef BUILD_JSON
    if (pJsonChild != NULL) {
        addressMapJson(&sAddrMapRawData, pJsonChild);
    }
#endif //BUILD_JSON

    return eStatus;
}
