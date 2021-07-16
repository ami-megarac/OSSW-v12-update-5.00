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
#include "CrashdumpSections/Uncore.hpp"

#include <safe_mem_lib.h>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace crashdump;

TEST_F(PeciTestFixture, logUncoreStatus_null)
{
    cJSON* root = nullptr;
    CPUInfo cpuInfo = {};
    root = nullptr;
    int ret = logUncoreStatus(cpuInfo, root);
    EXPECT_EQ(ret, 1);
}

TEST_F(PeciTestFixture, cpx_logUncoreStatus)
{
    uint8_t Data[4] = {0x07, 0x00, 0x00, 0x0};
    uint8_t Data2[4] = {0xef, 0xbe, 0xad, 0xde};
    EXPECT_CALL(*PeciMock, peci_Lock(_, _))
        .Times(3655)
        .WillRepeatedly(DoAll(Return(PECI_CC_SUCCESS)));
    EXPECT_CALL(*PeciMock, peci_WrPkgConfig_seq(_, _, _, _, _, _, _))
        .Times(4038)
        .WillRepeatedly(DoAll(SetArgPointee<6>(0x40), Return(PECI_CC_SUCCESS)));
    EXPECT_CALL(*PeciMock, peci_RdPkgConfig_seq(_, _, _, _, _, _, _))
        .Times(1398)
        .WillRepeatedly(DoAll(SetArrayArgument<4>(Data, Data + 4),
                              SetArgPointee<6>(0x40), Return(PECI_CC_SUCCESS)));
    EXPECT_CALL(*PeciMock, peci_RdPCIConfigLocal_seq(_, _, _, _, _, _, _, _, _))
        .Times(3652)
        .WillRepeatedly(DoAll(SetArrayArgument<6>(Data2, Data2 + 4),
                              SetArgPointee<8>(0x40), Return(PECI_CC_SUCCESS)));
    cJSON* root = nullptr;
    CPUInfo cpuInfo = {};
    char* buffer = NULL;
    cpuInfo.model = cpu::cpx;
    buffer = readTestFile("../crashdump_input_icx.json");
    cJSON* inputFile = cJSON_Parse(buffer);
    cpuInfo.inputFile.bufferPtr = inputFile;
    root = cJSON_CreateObject();
    int ret = logUncoreStatus(cpuInfo, root);
    // printJson(root);
    cJSON* expected = cJSON_CreateObject();
    expected = cJSON_GetObjectItemCaseSensitive(root, "uncore_crashdump_dw0");
    ASSERT_NE(expected, nullptr);
    EXPECT_STREQ(expected->valuestring, "0x7");
    expected = cJSON_GetObjectItemCaseSensitive(root, "B00_D00_F0_0x0");
    ASSERT_NE(expected, nullptr);
    EXPECT_STREQ(expected->valuestring, "0xdeadbeef");
    expected = cJSON_GetObjectItemCaseSensitive(root, "B00_D00_F0_0x90");
    ASSERT_NE(expected, nullptr);
    EXPECT_STREQ(expected->valuestring, "0xdeadbeefdeadbeef");
    expected = cJSON_GetObjectItemCaseSensitive(root, "iio_cstack_mc_ctl");
    ASSERT_NE(expected, nullptr);
    EXPECT_STREQ(expected->valuestring, "0x700000007");
    expected = cJSON_GetObjectItemCaseSensitive(root, "B30_D29_F0_0x24ED8");
    ASSERT_NE(expected, nullptr);
    EXPECT_STREQ(expected->valuestring, "0x7");
    EXPECT_EQ(ret, 0);
    cJSON_Delete(root);
    expected = nullptr;
}
