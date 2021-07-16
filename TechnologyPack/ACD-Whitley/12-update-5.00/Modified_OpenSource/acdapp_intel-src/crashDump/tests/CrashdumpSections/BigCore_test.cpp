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
#include "CrashdumpSections/BigCore.hpp"

#include <safe_mem_lib.h>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace crashdump;

TEST_F(PeciTestFixture, logCrashdump_null)
{
    cJSON* root = nullptr;
    CPUInfo cpuInfo = {};
    root = nullptr;
    int ret = logCrashdump(cpuInfo, root);
    EXPECT_EQ(ret, 1);
}

TEST_F(PeciTestFixture, logCrashdump_unknown_cpu)
{
    cJSON* root = nullptr;
    CPUInfo cpuInfo = {};
    root = cJSON_CreateObject();
    cpuInfo.model = (cpu::Model)10; // Unknown Processor
    int ret = logCrashdump(cpuInfo, root);
    EXPECT_EQ(ret, 1);
}

TEST_F(PeciTestFixture, cpx_logCrashdump_peci_lock_failure)
{
    //#ifdef MOCK
    EXPECT_CALL(*PeciMock, peci_Lock(_, _)).WillOnce(Return(PECI_CC_TIMEOUT));
    //#endif // MOCK
    cJSON* root = nullptr;
    CPUInfo cpuInfo = {};
    root = cJSON_CreateObject();
    cpuInfo.model = cpu::cpx;
    int ret = logCrashdump(cpuInfo, root);
    EXPECT_EQ(ret, 6);
}

TEST_F(PeciTestFixture, cpx_logCrashdump)
{
    uint8_t Header0[4] = {0x1, 0x0, 0x6, 0x0};
    uint8_t Header1[4] = {0xc2, 0x0, 0x0, 0x0};
    uint8_t Header2[4] = {0x1, 0x0, 0x0, 0x0};
    uint8_t Header3[4] = {0x0, 0x0, 0x0, 0x0};
    uint8_t DataSize[4] = {0x44, 0x1, 0x0, 0x0};
    uint8_t DataR[4] = {0xef, 0xbe, 0xad, 0xde};
    EXPECT_CALL(*PeciMock, peci_Lock(_, _)).WillOnce(Return(PECI_CC_SUCCESS));
    EXPECT_CALL(*PeciMock, peci_WrPkgConfig_seq(_, _, _, _, _, _, _))
        .Times(AnyNumber())
        .WillRepeatedly(DoAll(SetArgPointee<6>(0x40), Return(PECI_CC_SUCCESS)));
    EXPECT_CALL(*PeciMock, peci_RdPkgConfig_seq(_, MBX_INDEX_VCU,
                                                CD_HEADER_PARAM, _, _, _, _))
        .Times(1)
        .WillOnce(DoAll(SetArgPointee<4>(0x5), SetArgPointee<6>(0x40),
                        Return(PECI_CC_SUCCESS)));
    EXPECT_CALL(*PeciMock,
                peci_RdPkgConfig_seq(_, MBX_INDEX_VCU, VCU_READ, _, _, _, _))
        .Times(329)
        .WillOnce(DoAll(SetArrayArgument<4>(Header0, Header0 + 4),
                        SetArgPointee<6>(0x40), Return(PECI_CC_SUCCESS)))
        .WillOnce(DoAll(SetArrayArgument<4>(Header1, Header1 + 4),
                        SetArgPointee<6>(0x40), Return(PECI_CC_SUCCESS)))
        .WillOnce(DoAll(SetArrayArgument<4>(Header2, Header2 + 4),
                        SetArgPointee<6>(0x40), Return(PECI_CC_SUCCESS)))
        .WillOnce(DoAll(SetArrayArgument<4>(Header3, Header3 + 4),
                        SetArgPointee<6>(0x40), Return(PECI_CC_SUCCESS)))
        .WillOnce(DoAll(SetArrayArgument<4>(DataSize, DataSize + 4),
                        SetArgPointee<6>(0x40), Return(PECI_CC_SUCCESS)))
        .WillRepeatedly(DoAll(SetArrayArgument<4>(DataR, DataR + 4),
                              SetArgPointee<6>(0x40), Return(PECI_CC_SUCCESS)));
    cJSON* root = nullptr;
    cJSON* expected = nullptr;
    CPUInfo cpuInfo = {};
    root = cJSON_CreateObject();
    cpuInfo.model = cpu::cpx;
    int ret = logCrashdump(cpuInfo, root);
    // printJson(root);
    expected = cJSON_CreateObject();
    expected = cJSON_GetObjectItemCaseSensitive(root, "header0");
    ASSERT_NE(expected, nullptr);
    EXPECT_STREQ(expected->valuestring, "0x60001");
    expected = cJSON_GetObjectItemCaseSensitive(
        root->child->next->next->next->next->next->child,
        "IA32_X2APIC_CUR_COUNT");
    ASSERT_NE(expected, nullptr);
    EXPECT_STREQ(expected->valuestring, "0xdeadbeef");
    expected = cJSON_GetObjectItemCaseSensitive(
        root->child->next->next->next->next->next->child, "IA32_FIXED_CTR0");
    ASSERT_NE(expected, nullptr);
    EXPECT_STREQ(expected->valuestring, "0xdeadbeefdeadbeef");
    EXPECT_EQ(ret, 0);
    cJSON_Delete(root);
}

TEST_F(PeciTestFixture, icx_logCrashdump)
{ // 0x401e002
    uint8_t u8CrashdumpDisabled = ICX_A0_CRASHDUMP_ENABLED;
    uint16_t u16CrashdumpNumAgents = 2;
    uint64_t u64UniqueId = 0xabcdef;
    uint64_t u64PayloadExp = 0x2b0;
    uint8_t VerSizeRaw[8] = {0x02, 0xe0, 0x01, 0x04, 0xf8, 0x05, 0x00, 0x02};
    // uint8_t Data[8] =  {0xef, 0xbe, 0xad, 0xde, 0xef, 0xbe, 0xad, 0xde};
    EXPECT_CALL(*PeciMock, peci_CrashDump_Discovery(_, _, _, _, _, _, _, _))
        .WillOnce(DoAll(SetArgPointee<6>(u8CrashdumpDisabled),
                        SetArgPointee<7>(0x40), Return(PECI_CC_SUCCESS)))
        .WillOnce(DoAll(SetArgPointee<6>(u16CrashdumpNumAgents),
                        SetArgPointee<7>(0x40), Return(PECI_CC_SUCCESS)))
        .WillOnce(DoAll(SetArgPointee<6>(u64UniqueId), SetArgPointee<7>(0x40),
                        Return(PECI_CC_SUCCESS)))
        .WillOnce(DoAll(SetArgPointee<6>(u64PayloadExp), SetArgPointee<7>(0x40),
                        Return(PECI_CC_SUCCESS)));
    EXPECT_CALL(*PeciMock, peci_CrashDump_GetFrame(_, _, _, _, _, _, _))
        .Times(255) // 2(1528) + 512 = 3568 + 2(size)
        .WillRepeatedly(DoAll(SetArrayArgument<5>(VerSizeRaw, VerSizeRaw + 8),
                              SetArgPointee<6>(0x40), Return(PECI_CC_SUCCESS)));
    cJSON* root = nullptr;
    cJSON* expected = nullptr;
    CPUInfo cpuInfo = {};
    root = cJSON_CreateObject();
    cpuInfo.model = cpu::icx2;
    cpuInfo.coreMask = 0x1;
    int ret = logCrashdump(cpuInfo, root);
    // printJson(root);
    expected = cJSON_CreateObject();
    expected = cJSON_GetObjectItemCaseSensitive(root->child->child,
                                                "crashlog_version");
    ASSERT_NE(expected, nullptr);
    EXPECT_STREQ(expected->valuestring, "0x0401e002");
    expected =
        cJSON_GetObjectItemCaseSensitive(root->child->child, "CS_SELECTOR");
    ASSERT_NE(expected, nullptr);
    EXPECT_STREQ(expected->valuestring, "0x05f8");
    expected = cJSON_GetObjectItemCaseSensitive(root->child->child, "LER_INFO");
    ASSERT_NE(expected, nullptr);
    EXPECT_STREQ(expected->valuestring, "0x020005f80401e002");
    expected =
        cJSON_GetObjectItemCaseSensitive(root->child->child->next, "entry0");
    ASSERT_NE(expected, nullptr);
    EXPECT_STREQ(expected->valuestring, "0x020005f80401e002");
    EXPECT_EQ(ret, 0);
    cJSON_Delete(root);
    expected = nullptr;
}
