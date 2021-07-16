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
#include "../mock/libpeci_mock.hpp"
#include "../test_utils.hpp"
#include "CrashdumpSections/PowerManagement.hpp"

#include <safe_mem_lib.h>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace crashdump;

TEST_F(PeciTestFixture, logPowerManagement_null)
{
    cJSON* root = nullptr;
    CPUInfo cpuInfo = {};
    root = nullptr;
    int ret = logPowerManagement(cpuInfo, root);
    EXPECT_EQ(ret, 1);
}

TEST_F(PeciTestFixture, cpx_logPowerManagement)
{
    uint8_t Data[4] = {0xef, 0xbe, 0xad, 0xde};
    EXPECT_CALL(*PeciMock, peci_Lock(_, _))
        .Times(1)
        .WillOnce(DoAll(Return(PECI_CC_SUCCESS)));
    EXPECT_CALL(*PeciMock, peci_WrPkgConfig_seq(_, _, _, _, _, _, _))
        .Times(6)
        .WillRepeatedly(DoAll(SetArgPointee<6>(0x40), Return(PECI_CC_SUCCESS)));
    EXPECT_CALL(*PeciMock, peci_RdPkgConfig_seq(_, _, _, _, _, _, _))
        .Times(2)
        .WillRepeatedly(DoAll(SetArrayArgument<4>(Data, Data + 4),
                              SetArgPointee<6>(0x40), Return(PECI_CC_SUCCESS)));
    cJSON* root = nullptr;
    CPUInfo cpuInfo = {};
    root = cJSON_CreateObject();
    cpuInfo.coreMask = 0x1;
    cpuInfo.model = cpu::cpx;
    int ret = logPowerManagement(cpuInfo, root);
    // printJson(root);
    cJSON* expected = nullptr;
    expected = cJSON_CreateObject();
    expected = cJSON_GetObjectItemCaseSensitive(root->child, "c_state_reg");
    ASSERT_NE(expected, nullptr);
    EXPECT_STREQ(expected->valuestring, "0xdeadbeef");
    expected = cJSON_GetObjectItemCaseSensitive(root->child, "vid_ratio_reg");
    ASSERT_NE(expected, nullptr);
    EXPECT_STREQ(expected->valuestring, "0xdeadbeef");
    EXPECT_EQ(ret, 0);
    cJSON_Delete(root);
    expected = nullptr;
}

TEST_F(PeciTestFixture, icx_logPowerManagement)
{
    uint8_t Data[4] = {0xef, 0xbe, 0xad, 0xde};
    EXPECT_CALL(*PeciMock, peci_RdPkgConfig(_, _, _, _, _, _))
        .Times(167)
        .WillRepeatedly(DoAll(SetArrayArgument<4>(Data, Data + 4),
                              SetArgPointee<5>(0x40), Return(PECI_CC_SUCCESS)));
    cJSON* root = nullptr;
    CPUInfo cpuInfo = {};
    char* buffer = NULL;
    root = cJSON_CreateObject();
    cpuInfo.model = cpu::icx2;
    cpuInfo.coreMask = 0x1;
    buffer = readTestFile("../crashdump_input_icx.json");
    cJSON* inputFile = cJSON_Parse(buffer);
    cpuInfo.inputFile.bufferPtr = inputFile;
    int ret = logPowerManagement(cpuInfo, root);
    // printJson(root);
    cJSON* expected = nullptr;
    expected = cJSON_CreateObject();
    expected = cJSON_GetObjectItemCaseSensitive(root, "pkgs_master_fsm");
    ASSERT_NE(expected, nullptr);
    EXPECT_STREQ(expected->valuestring, "0xdeadbeef");
    EXPECT_EQ(ret, 0);
    cJSON_Delete(root);
    expected = nullptr;
}