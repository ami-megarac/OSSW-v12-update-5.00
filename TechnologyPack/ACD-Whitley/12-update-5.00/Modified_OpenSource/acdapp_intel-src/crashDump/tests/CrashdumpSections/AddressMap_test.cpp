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
#include "CrashdumpSections/AddressMap.hpp"

#include <safe_mem_lib.h>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace crashdump;

TEST_F(PeciTestFixture, logAddressMap_null)
{
    cJSON* root = nullptr;
    CPUInfo cpuInfo = {};
    int ret = logAddressMap(cpuInfo, root);
    EXPECT_EQ(ret, 1);
}

TEST_F(PeciTestFixture, logAddressMap_unknowm_cpu)
{
    cJSON* root = nullptr;
    CPUInfo cpuInfo = {};
    root = cJSON_CreateObject();
    cpuInfo.model = (cpu::Model)10; // Unknown Processor
    int ret = logAddressMap(cpuInfo, root);
    EXPECT_EQ(ret, 1);
    cJSON_Delete(root);
}

TEST_F(PeciTestFixture, cpx_logAddressMap_peci_lock_failure)
{
    EXPECT_CALL(*PeciMock, peci_Lock(_, _))
        .Times(1)
        .WillOnce(DoAll(Return(PECI_CC_TIMEOUT)));
    cJSON* root = nullptr;
    CPUInfo cpuInfo = {};
    root = cJSON_CreateObject();
    cpuInfo.model = cpu::cpx;
    int ret = logAddressMap(cpuInfo, root);
    EXPECT_EQ(ret, 6);
    cJSON_Delete(root);
}

TEST_F(PeciTestFixture, icx_logAddressMap)
{
    cJSON* root = nullptr;
    CPUInfo cpuInfo = {};
    EXPECT_CALL(*PeciMock,
                peci_RdEndPointConfigPciLocal_seq(_, _, _, _, _, _, _, _, _, _))
        .Times(AnyNumber())
        .WillRepeatedly(DoAll(SetArgPointee<9>(0x40), Return(PECI_CC_SUCCESS)));
    root = cJSON_CreateObject();
    cpuInfo.model = cpu::icx;
    int ret = logAddressMap(cpuInfo, root);
    cJSON* expected = nullptr;
    expected = cJSON_GetObjectItemCaseSensitive(root, "MMCFG_BASE");
    ASSERT_NE(expected, nullptr);
    EXPECT_STREQ(expected->valuestring, "0xdeadbeefdeadbeef");
    expected = cJSON_GetObjectItemCaseSensitive(root, "TOLM");
    ASSERT_NE(expected, nullptr);
    EXPECT_STREQ(expected->valuestring, "0xdeadbeef");
    expected = cJSON_GetObjectItemCaseSensitive(root, "TOCM");
    ASSERT_NE(expected, nullptr);
    EXPECT_STREQ(expected->valuestring, "0xef");
    EXPECT_EQ(ret, 0);
    cJSON_Delete(root);
    expected = nullptr;
}

TEST_F(PeciTestFixture, icx_logAddressMap_UA_on_first_read_size8_size4_size1)
{
    EXPECT_CALL(*PeciMock,
                peci_RdEndPointConfigPciLocal_seq(_, _, _, _, _, _, _, _, _, _))
        .Times(AnyNumber())
        .WillRepeatedly(DoAll(SetArgPointee<9>(0x40), Return(PECI_CC_SUCCESS)));
    EXPECT_CALL(*PeciMock,
                peci_RdEndPointConfigPciLocal_seq(_, _, 0, 0, 0, 0x90, _, _, _,
                                                  _)) // Size 8 first read
        .WillOnce(DoAll(SetArgPointee<9>(0x90), Return(PECI_CC_SUCCESS)));
    EXPECT_CALL(*PeciMock,
                peci_RdEndPointConfigPciLocal_seq(_, _, 0, 0, 0, 0x94, _, _, _,
                                                  _)) // Size 8 second read
        .WillOnce(DoAll(SetArgPointee<9>(0x40), Return(PECI_CC_SUCCESS)));
    EXPECT_CALL(*PeciMock,
                peci_RdEndPointConfigPciLocal_seq(_, _, 0, 0, 0, 0xC0, _, _, _,
                                                  _)) // Size 1 first read
        .WillOnce(DoAll(SetArgPointee<9>(0x90), Return(PECI_CC_SUCCESS)));
    EXPECT_CALL(*PeciMock,
                peci_RdEndPointConfigPciLocal_seq(_, _, 0, 0, 0, 0xD0, _, _, _,
                                                  _)) // Size 4 first read
        .WillOnce(DoAll(SetArgPointee<9>(0x90), Return(PECI_CC_SUCCESS)));
    cJSON* root = nullptr;
    CPUInfo cpuInfo = {};
    root = cJSON_CreateObject();
    cpuInfo.model = cpu::icx;
    int ret = logAddressMap(cpuInfo, root);
    // printJson(root);
    cJSON* expected = nullptr;
    expected = cJSON_GetObjectItemCaseSensitive(root, "MMCFG_BASE");
    ASSERT_NE(expected, nullptr);
    EXPECT_STREQ(expected->valuestring, "UA:0x90");
    expected = cJSON_GetObjectItemCaseSensitive(root, "TOLM");
    ASSERT_NE(expected, nullptr);
    EXPECT_STREQ(expected->valuestring, "UA:0x90");
    expected = cJSON_GetObjectItemCaseSensitive(root, "TOCM");
    ASSERT_NE(expected, nullptr);
    EXPECT_STREQ(expected->valuestring, "UA:0x90");
    EXPECT_EQ(ret, 0);
    cJSON_Delete(root);
    expected = nullptr;
}

TEST_F(PeciTestFixture, icx_logAddressMap_UA_on_second_read_size8)
{
    EXPECT_CALL(*PeciMock,
                peci_RdEndPointConfigPciLocal_seq(_, _, _, _, _, _, _, _, _, _))
        .Times(AnyNumber())
        .WillRepeatedly(DoAll(SetArgPointee<9>(0x40), Return(PECI_CC_SUCCESS)));
    EXPECT_CALL(*PeciMock,
                peci_RdEndPointConfigPciLocal_seq(_, _, 0, 0, 0, 0x90, _, _, _,
                                                  _)) // Size 8 first read
        .WillOnce(DoAll(SetArgPointee<9>(0x40), Return(PECI_CC_SUCCESS)));
    EXPECT_CALL(*PeciMock,
                peci_RdEndPointConfigPciLocal_seq(_, _, 0, 0, 0, 0x94, _, _, _,
                                                  _)) // Size 8 second read
        .WillOnce(DoAll(SetArgPointee<9>(0x91), Return(PECI_CC_SUCCESS)));
    cJSON* root = nullptr;
    CPUInfo cpuInfo = {};
    root = cJSON_CreateObject();
    cpuInfo.model = cpu::icx;
    int ret = logAddressMap(cpuInfo, root);
    // printJson(root);
    cJSON* expected = nullptr;
    expected = cJSON_GetObjectItemCaseSensitive(root, "MMCFG_BASE");
    ASSERT_NE(expected, nullptr);
    EXPECT_STREQ(expected->valuestring, "UA:0x91");
    EXPECT_EQ(ret, 0);
    cJSON_Delete(root);
    expected = nullptr;
}

TEST_F(PeciTestFixture, icx_logAddressMap_DF_on_first_read_size8_size4_size1)
{
    EXPECT_CALL(*PeciMock,
                peci_RdEndPointConfigPciLocal_seq(_, _, _, _, _, _, _, _, _, _))
        .Times(AnyNumber())
        .WillRepeatedly(DoAll(SetArgPointee<9>(0x40), Return(PECI_CC_SUCCESS)));
    EXPECT_CALL(*PeciMock,
                peci_RdEndPointConfigPciLocal_seq(_, _, 0, 0, 0, 0x90, _, _, _,
                                                  _)) // Size 8 first read
        .WillOnce(DoAll(SetArgPointee<9>(0x80), Return(PECI_CC_TIMEOUT)));
    EXPECT_CALL(*PeciMock,
                peci_RdEndPointConfigPciLocal_seq(_, _, 0, 0, 0, 0xC0, _, _, _,
                                                  _)) // Size 1 first read
        .WillOnce(DoAll(SetArgPointee<9>(0x80), Return(PECI_CC_TIMEOUT)));
    EXPECT_CALL(*PeciMock,
                peci_RdEndPointConfigPciLocal_seq(_, _, 0, 0, 0, 0xD0, _, _, _,
                                                  _)) // Size 4 first read
        .WillOnce(DoAll(SetArgPointee<9>(0x80), Return(PECI_CC_TIMEOUT)));
    cJSON* root = nullptr;
    CPUInfo cpuInfo = {};
    root = cJSON_CreateObject();
    cpuInfo.model = cpu::icx;
    int ret = logAddressMap(cpuInfo, root);
    // printJson(root);
    cJSON* expected = nullptr;
    expected = cJSON_GetObjectItemCaseSensitive(root, "MMCFG_BASE");
    ASSERT_NE(expected, nullptr);
    EXPECT_STREQ(expected->valuestring, "UA:0x80,DF:0x6");
    expected = cJSON_GetObjectItemCaseSensitive(root, "TOLM");
    ASSERT_NE(expected, nullptr);
    EXPECT_STREQ(expected->valuestring, "UA:0x80,DF:0x6");
    expected = cJSON_GetObjectItemCaseSensitive(root, "TOCM");
    ASSERT_NE(expected, nullptr);
    EXPECT_STREQ(expected->valuestring, "UA:0x80,DF:0x6");
    EXPECT_EQ(ret, 0);
    cJSON_Delete(root);
    expected = nullptr;
}

TEST_F(PeciTestFixture, icx_logAddressMap_DF_on_second_read)
{
    EXPECT_CALL(*PeciMock,
                peci_RdEndPointConfigPciLocal_seq(_, _, _, _, _, _, _, _, _, _))
        .Times(AnyNumber())
        .WillRepeatedly(DoAll(SetArgPointee<9>(0x40), Return(PECI_CC_SUCCESS)));
    EXPECT_CALL(*PeciMock,
                peci_RdEndPointConfigPciLocal_seq(_, _, 0, 0, 0, 0x90, _, _, _,
                                                  _)) // Size 8 first read
        .WillOnce(DoAll(SetArgPointee<9>(0x40), Return(PECI_CC_SUCCESS)));
    EXPECT_CALL(*PeciMock,
                peci_RdEndPointConfigPciLocal_seq(_, _, 0, 0, 0, 0x94, _, _, _,
                                                  _)) // Size 8 first read
        .WillOnce(DoAll(SetArgPointee<9>(0x80), Return(PECI_CC_TIMEOUT)));
    cJSON* root = nullptr;
    CPUInfo cpuInfo = {};
    root = cJSON_CreateObject();
    cpuInfo.model = cpu::icx;
    int ret = logAddressMap(cpuInfo, root);
    // printJson(root);
    cJSON* expected = nullptr;
    expected = cJSON_GetObjectItemCaseSensitive(root, "MMCFG_BASE");
    ASSERT_NE(expected, nullptr);
    EXPECT_STREQ(expected->valuestring, "UA:0x80,DF:0x6");
    EXPECT_EQ(ret, 0);
    cJSON_Delete(root);
    expected = nullptr;
}

TEST_F(PeciTestFixture, cpx_logAddressMap)
{
    EXPECT_CALL(*PeciMock, peci_RdPCIConfigLocal_seq(_, _, _, _, _, _, _, _, _))
        .Times(AnyNumber())
        .WillRepeatedly(DoAll(SetArgPointee<8>(0x40), Return(PECI_CC_SUCCESS)));
    cJSON* root = nullptr;
    CPUInfo cpuInfo = {};
    root = cJSON_CreateObject();
    cpuInfo.model = cpu::cpx;

    cJSON* expected = cJSON_CreateObject();
    int ret = logAddressMap(cpuInfo, root);
    expected = cJSON_GetObjectItemCaseSensitive(root, "MMCFG_BASE");
    ASSERT_NE(expected, nullptr);
    EXPECT_STREQ(expected->valuestring, "0xdeadbeefdeadbeef");
    expected = cJSON_GetObjectItemCaseSensitive(root, "TOLM");
    ASSERT_NE(expected, nullptr);
    EXPECT_STREQ(expected->valuestring, "0xdeadbeef");
    EXPECT_EQ(ret, 0);
    cJSON_Delete(root);
    expected = nullptr;
}

TEST_F(PeciTestFixture, cpx_logAddressMap_UA_on_first_read_size8_size4)
{
    EXPECT_CALL(*PeciMock, peci_RdPCIConfigLocal_seq(_, _, _, _, _, _, _, _, _))
        .Times(AnyNumber())
        .WillRepeatedly(DoAll(SetArgPointee<8>(0x40), Return(PECI_CC_SUCCESS)));
    EXPECT_CALL(*PeciMock,
                peci_RdPCIConfigLocal_seq(_, 0, 5, 0, 0x90, _, _, _, _))
        .Times(1)
        .WillOnce(DoAll(SetArgPointee<8>(0x91), Return(PECI_CC_SUCCESS)));
    EXPECT_CALL(*PeciMock,
                peci_RdPCIConfigLocal_seq(_, 0, 5, 0, 0x94, _, _, _, _))
        .Times(1)
        .WillOnce(DoAll(SetArgPointee<8>(0x40), Return(PECI_CC_SUCCESS)));
    EXPECT_CALL(*PeciMock,
                peci_RdPCIConfigLocal_seq(_, 0, 5, 0, 0xD0, _, _, _, _))
        .Times(1)
        .WillOnce(DoAll(SetArgPointee<8>(0x91), Return(PECI_CC_SUCCESS)));
    cJSON* root = nullptr;
    CPUInfo cpuInfo = {};
    root = cJSON_CreateObject();
    cpuInfo.model = cpu::cpx;
    cJSON* expected = cJSON_CreateObject();
    int ret = logAddressMap(cpuInfo, root);
    // printJson(root);
    expected = cJSON_GetObjectItemCaseSensitive(root, "MMCFG_BASE");
    ASSERT_NE(expected, nullptr);
    EXPECT_STREQ(expected->valuestring, "0xdeadbeefdeadbeef,UA:0x91");
    expected = cJSON_GetObjectItemCaseSensitive(root, "TOLM");
    ASSERT_NE(expected, nullptr);
    EXPECT_STREQ(expected->valuestring, "0xdeadbeef,UA:0x91");
    EXPECT_EQ(ret, 0);
    cJSON_Delete(root);
    expected = nullptr;
}

TEST_F(PeciTestFixture, cpx_logAddressMap_UA_on_second_read_size8)
{
    EXPECT_CALL(*PeciMock, peci_RdPCIConfigLocal_seq(_, _, _, _, _, _, _, _, _))
        .Times(AnyNumber())
        .WillRepeatedly(DoAll(SetArgPointee<8>(0x40), Return(PECI_CC_SUCCESS)));
    EXPECT_CALL(*PeciMock,
                peci_RdPCIConfigLocal_seq(_, 0, 5, 0, 0x90, _, _, _, _))
        .Times(1)
        .WillOnce(DoAll(SetArgPointee<8>(0x92), Return(PECI_CC_SUCCESS)));
    EXPECT_CALL(*PeciMock,
                peci_RdPCIConfigLocal_seq(_, 0, 5, 0, 0x94, _, _, _, _))
        .Times(1)
        .WillOnce(DoAll(SetArgPointee<8>(0x91), Return(PECI_CC_SUCCESS)));
    cJSON* root = nullptr;
    CPUInfo cpuInfo = {};
    root = cJSON_CreateObject();
    cpuInfo.model = cpu::cpx;
    cJSON* expected = cJSON_CreateObject();
    int ret = logAddressMap(cpuInfo, root);
    // printJson(root);
    expected = cJSON_GetObjectItemCaseSensitive(root, "MMCFG_BASE");
    ASSERT_NE(expected, nullptr);
    EXPECT_STREQ(expected->valuestring, "0xdeadbeefdeadbeef,UA:0x91");
    EXPECT_EQ(ret, 0);
    cJSON_Delete(root);
    expected = nullptr;
}

TEST_F(PeciTestFixture, cpx_logAddressMap_DF_on_first_read_size8_size4)
{
    EXPECT_CALL(*PeciMock, peci_RdPCIConfigLocal_seq(_, _, _, _, _, _, _, _, _))
        .Times(AnyNumber())
        .WillRepeatedly(DoAll(SetArgPointee<8>(0x40), Return(PECI_CC_SUCCESS)));
    EXPECT_CALL(*PeciMock,
                peci_RdPCIConfigLocal_seq(_, 0, 5, 0, 0x90, _, _, _, _))
        .Times(1)
        .WillOnce(DoAll(SetArgPointee<8>(0x80), Return(PECI_CC_TIMEOUT)));
    EXPECT_CALL(*PeciMock,
                peci_RdPCIConfigLocal_seq(_, 0, 5, 0, 0xD0, _, _, _, _))
        .Times(1)
        .WillOnce(DoAll(SetArgPointee<8>(0x80), Return(PECI_CC_TIMEOUT)));
    cJSON* root = nullptr;
    CPUInfo cpuInfo = {};
    root = cJSON_CreateObject();
    cpuInfo.model = cpu::cpx;
    cJSON* expected = cJSON_CreateObject();
    int ret = logAddressMap(cpuInfo, root);
    // printJson(root);
    expected = cJSON_GetObjectItemCaseSensitive(root, "MMCFG_BASE");
    ASSERT_NE(expected, nullptr);
    EXPECT_STREQ(expected->valuestring, "0x0,UA:0x80,DF:0x6");
    expected = cJSON_GetObjectItemCaseSensitive(root, "TOLM");
    ASSERT_NE(expected, nullptr);
    EXPECT_STREQ(expected->valuestring, "0x0,UA:0x80,DF:0x6");
    EXPECT_EQ(ret, 0);
    cJSON_Delete(root);
    expected = nullptr;
}

TEST_F(PeciTestFixture, cpx_logAddressMap_DF_on_second_read_size8)
{
    EXPECT_CALL(*PeciMock, peci_RdPCIConfigLocal_seq(_, _, _, _, _, _, _, _, _))
        .Times(AnyNumber())
        .WillRepeatedly(DoAll(SetArgPointee<8>(0x40), Return(PECI_CC_SUCCESS)));
    EXPECT_CALL(*PeciMock,
                peci_RdPCIConfigLocal_seq(_, 0, 5, 0, 0x90, _, _, _, _))
        .Times(1)
        .WillOnce(DoAll(SetArgPointee<8>(0x40), Return(PECI_CC_SUCCESS)));
    EXPECT_CALL(*PeciMock,
                peci_RdPCIConfigLocal_seq(_, 0, 5, 0, 0x94, _, _, _, _))
        .Times(1)
        .WillOnce(DoAll(SetArgPointee<8>(0x80), Return(PECI_CC_TIMEOUT)));
    cJSON* root = nullptr;
    CPUInfo cpuInfo = {};
    root = cJSON_CreateObject();
    cpuInfo.model = cpu::cpx;
    cJSON* expected = cJSON_CreateObject();
    int ret = logAddressMap(cpuInfo, root);
    // printJson(root);
    expected = cJSON_GetObjectItemCaseSensitive(root, "MMCFG_BASE");
    ASSERT_NE(expected, nullptr);
    EXPECT_STREQ(expected->valuestring, "0x0,UA:0x80,DF:0x6");
    EXPECT_EQ(ret, 0);
    cJSON_Delete(root);
    expected = nullptr;
}
