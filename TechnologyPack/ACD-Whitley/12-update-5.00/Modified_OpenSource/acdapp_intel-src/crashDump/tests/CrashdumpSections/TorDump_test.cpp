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
#include "CrashdumpSections/TorDump.hpp"

#include <safe_mem_lib.h>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace crashdump;

TEST_F(PeciTestFixture, logTorDump_null)
{
    cJSON* root = nullptr;
    CPUInfo cpuInfo = {};
    root = nullptr;
    int ret = logTorDump(cpuInfo, root);
    EXPECT_EQ(ret, 1);
}

TEST_F(PeciTestFixture, cpx_logTorDump)
{
    uint8_t Data[4] = {0x48, 0x00, 0x00, 0x00};
    EXPECT_CALL(*PeciMock, peci_Lock(_, _))
        .Times(1)
        .WillOnce(DoAll(Return(PECI_CC_SUCCESS)));
    EXPECT_CALL(*PeciMock, peci_WrPkgConfig_seq(_, _, _, _, _, _, _))
        .Times(3)
        .WillRepeatedly(DoAll(SetArgPointee<6>(0x40), Return(PECI_CC_SUCCESS)));
    EXPECT_CALL(*PeciMock, peci_RdPkgConfig_seq(_, _, _, _, _, _, _))
        .Times(74)
        .WillRepeatedly(DoAll(SetArrayArgument<4>(Data, Data + 4),
                              SetArgPointee<6>(0x40), Return(PECI_CC_SUCCESS)));
    cJSON* root = nullptr;
    CPUInfo cpuInfo = {};
    cpuInfo.model = cpu::cpx;
    root = cJSON_CreateObject();
    int ret = logTorDump(cpuInfo, root);
    // printJson(root);
    cJSON* expected = nullptr;
    expected = cJSON_CreateObject();
    expected =
        cJSON_GetObjectItemCaseSensitive(root->child->child, "subindex0");
    ASSERT_NE(expected, nullptr);
    EXPECT_STREQ(expected->valuestring, "0x48");
    EXPECT_EQ(ret, 0);
    cJSON_Delete(root);
    expected = nullptr;
}

TEST_F(PeciTestFixture, icx_logTorDump)
{
    uint8_t u8CrashdumpDisabled = 0;
    uint16_t u16CrashdumpNumAgents = 2;
    // uint64_t u64UniqueId = 0xef;
    // uint64_t u64PayloadExp = 0xFF;
    uint8_t u64UniqueId[8] = {0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t PayloadExp[8] = {0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t Data[8] = {0xef, 0xbe, 0xad, 0xde, 0xef, 0xbe, 0xad, 0xde};
    // uint8_t Data[8] =  {0xef, 0xbe, 0xad, 0xde, 0xef, 0xbe, 0xad, 0xde};
    EXPECT_CALL(*PeciMock, peci_CrashDump_Discovery(_, _, _, _, _, _, _, _))
        .WillOnce(DoAll(SetArgPointee<6>(u8CrashdumpDisabled),
                        SetArgPointee<7>(0x40), Return(PECI_CC_SUCCESS)))
        .WillOnce(DoAll(SetArgPointee<6>(u16CrashdumpNumAgents),
                        SetArgPointee<7>(0x40), Return(PECI_CC_SUCCESS)))
        .WillOnce(DoAll(SetArrayArgument<6>(u64UniqueId, u64UniqueId + 8),
                        SetArgPointee<7>(0x40), Return(PECI_CC_SUCCESS)))
        .WillOnce(DoAll(SetArrayArgument<6>(PayloadExp, PayloadExp + 8),
                        SetArgPointee<7>(0x40), Return(PECI_CC_SUCCESS)));

    EXPECT_CALL(*PeciMock, peci_CrashDump_GetFrame(_, _, _, _, _, _, _))
        .Times(256)
        .WillRepeatedly(DoAll(SetArrayArgument<5>(Data, Data + 8),
                              SetArgPointee<6>(0x40), Return(PECI_CC_SUCCESS)));

    cJSON* root = nullptr;
    CPUInfo cpuInfo = {};
    cpuInfo.model = cpu::icx2;
    cpuInfo.chaCount = 0x1;
    root = cJSON_CreateObject();
    int ret = logTorDump(cpuInfo, root);
    // printJson(root);
    cJSON* expected = nullptr;
    expected = cJSON_CreateObject();
    expected =
        cJSON_GetObjectItemCaseSensitive(root->child->child, "subindex0");
    ASSERT_NE(expected, nullptr);
    EXPECT_STREQ(expected->valuestring, "0xdeadbeefdeadbeef");
    EXPECT_EQ(ret, 0);
    cJSON_Delete(root);
    expected = nullptr;
}