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
#include "CrashdumpSections/UncoreMca.hpp"

#include <safe_mem_lib.h>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace crashdump;

TEST_F(PeciTestFixture, logUncoreMca_null)
{
    cJSON* root = nullptr;
    CPUInfo cpuInfo = {};
    root = nullptr;
    int ret = logUncoreMca(cpuInfo, root);
    EXPECT_EQ(ret, 1);
}

TEST_F(PeciTestFixture, cpx_logUncoreMca_null)
{
    uint8_t Data[4] = {0xef, 0xbe, 0xad, 0xde};
    EXPECT_CALL(*PeciMock, peci_Lock(_, _))
        .Times(2)
        .WillRepeatedly(DoAll(Return(PECI_CC_SUCCESS)));
    EXPECT_CALL(*PeciMock, peci_WrPkgConfig_seq(_, _, _, _, _, _, _))
        .Times(57)
        .WillRepeatedly(DoAll(SetArgPointee<6>(0x40), Return(PECI_CC_SUCCESS)));
    EXPECT_CALL(*PeciMock, peci_RdPkgConfig_seq(_, _, _, _, _, _, _))
        .Times(190)
        .WillRepeatedly(DoAll(SetArrayArgument<4>(Data, Data + 4),
                              SetArgPointee<6>(0x40), Return(PECI_CC_SUCCESS)));
    cJSON* root = nullptr;
    root = cJSON_CreateObject();
    CPUInfo cpuInfo = {};
    cpuInfo.model = cpu::cpx;
    int ret = logUncoreMca(cpuInfo, root);
    // printJson(root);
    cJSON* expected = nullptr;
    expected = cJSON_CreateObject();
    expected = cJSON_GetObjectItemCaseSensitive(root->child->child, "mc4_ctl");
    ASSERT_NE(expected, nullptr);
    EXPECT_STREQ(expected->valuestring, "0xdeadbeefdeadbeef");
    expected =
        cJSON_GetObjectItemCaseSensitive(root->child->child, "mc4_status");
    ASSERT_NE(expected, nullptr);
    EXPECT_STREQ(expected->valuestring, "0xdeadbeefdeadbeef");
    expected = cJSON_GetObjectItemCaseSensitive(root->child->child, "mc4_addr");
    ASSERT_NE(expected, nullptr);
    EXPECT_STREQ(expected->valuestring, "0xdeadbeefdeadbeef");
    expected = cJSON_GetObjectItemCaseSensitive(root->child->child, "mc4_misc");
    ASSERT_NE(expected, nullptr);
    EXPECT_STREQ(expected->valuestring, "0xdeadbeefdeadbeef");
    expected = cJSON_GetObjectItemCaseSensitive(root->child->child, "mc4_ctl2");
    ASSERT_NE(expected, nullptr);
    EXPECT_STREQ(expected->valuestring, "0xdeadbeefdeadbeef");
    EXPECT_EQ(ret, 0);
    cJSON_Delete(root);
    expected = nullptr;
}

TEST_F(PeciTestFixture, icx_logUncoreMca_null)
{
    EXPECT_CALL(*PeciMock, peci_RdIAMSR(_, _, _, _, _))
        .Times(91)
        .WillRepeatedly(DoAll(SetArgPointee<3>(0xdeadbeefdeadbeef),
                              SetArgPointee<4>(0x40), Return(PECI_CC_SUCCESS)));
    cJSON* root = nullptr;
    root = cJSON_CreateObject();
    CPUInfo cpuInfo = {};
    cpuInfo.model = cpu::icx2;
    int ret = logUncoreMca(cpuInfo, root);
    // printJson(root);
    cJSON* expected = nullptr;
    expected = cJSON_CreateObject();
    expected = cJSON_GetObjectItemCaseSensitive(root->child->child, "mc4_ctl");
    ASSERT_NE(expected, nullptr);
    EXPECT_STREQ(expected->valuestring, "0xdeadbeefdeadbeef");
    expected =
        cJSON_GetObjectItemCaseSensitive(root->child->child, "mc4_status");
    ASSERT_NE(expected, nullptr);
    EXPECT_STREQ(expected->valuestring, "0xdeadbeefdeadbeef");
    expected = cJSON_GetObjectItemCaseSensitive(root->child->child, "mc4_addr");
    ASSERT_NE(expected, nullptr);
    EXPECT_STREQ(expected->valuestring, "0xdeadbeefdeadbeef");
    expected = cJSON_GetObjectItemCaseSensitive(root->child->child, "mc4_misc");
    ASSERT_NE(expected, nullptr);
    EXPECT_STREQ(expected->valuestring, "0xdeadbeefdeadbeef");
    expected = cJSON_GetObjectItemCaseSensitive(root->child->child, "mc4_ctl2");
    ASSERT_NE(expected, nullptr);
    EXPECT_STREQ(expected->valuestring, "0xdeadbeefdeadbeef");
    EXPECT_EQ(ret, 0);
    cJSON_Delete(root);
    expected = nullptr;
}