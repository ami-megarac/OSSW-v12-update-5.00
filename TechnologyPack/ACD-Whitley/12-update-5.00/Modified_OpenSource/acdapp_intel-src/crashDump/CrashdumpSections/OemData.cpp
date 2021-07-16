/******************************************************************************
 *
 * INTEL CONFIDENTIAL
 *
 * Copyright 2020 Intel Corporation.
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

#include "OemData.hpp"

extern "C" {
#ifndef SPX_BMC_ACD
#include <cjson/cJSON.h>
#endif
#include <stdio.h>
}
#include "crashdump.hpp"
#include "utils.hpp"

#ifdef CONFIG_SPX_FEATURE_OEMDATA_SECTION
int logOemDataCPX1(crashdump::CPUInfo& cpuInfo, cJSON* pJsonChild)
{
    (void)cpuInfo;
    cJSON_AddStringToObject(pJsonChild, "Test", "1");
    return 0;
}

int logOemDataICX1(crashdump::CPUInfo& cpuInfo, cJSON* pJsonChild)
{
    (void)cpuInfo;
    cJSON_AddStringToObject(pJsonChild, "Test", "1");
    return 0;
}

static const SOemDataVx sOemDataVx[] = {
    {crashdump::cpu::clx, logOemDataCPX1},
    {crashdump::cpu::cpx, logOemDataCPX1},
    {crashdump::cpu::skx, logOemDataCPX1},
    {crashdump::cpu::icx, logOemDataICX1},
    {crashdump::cpu::icx2, logOemDataICX1},
    {crashdump::cpu::icxd, logOemDataICX1},
};

/******************************************************************************
 *
 *   logOemData
 *
 *
 *
 ******************************************************************************/
int logOemData(crashdump::CPUInfo& cpuInfo, cJSON* pJsonChild)
{
    if (pJsonChild == NULL)
    {
        return 1;
    }
    for (uint32_t i = 0; i < (sizeof(sOemDataVx) / sizeof(SOemDataVx)); i++)
    {
        if (cpuInfo.model == sOemDataVx[i].cpuModel)
        {
            return sOemDataVx[i].logOemDataVx(cpuInfo, pJsonChild);
        }
    }
    return 1;
}
#endif