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

#include "../../test_utils.hpp"
#include "CrashdumpSections/nvd/apis.hpp"
#include "CrashdumpSections/nvd/csrs.hpp"
#include "CrashdumpSections/nvd/logdata.hpp"
#include "crashdump.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace ::testing;
using ::testing::Return;
using namespace crashdump;

// change to ifdef or adjust CMakeList.txt to test against hardware
#ifndef MOCK
class LogDataTestFixture : public Test
{
  public:
    void SetUp() override
    {
        buffer = readTestFile("/tmp/crashdump_nvd.json");
        // Initialize json object
        root = cJSON_CreateObject();
    }

    cJSON* root = NULL;
    cJSON* expected = NULL;
    char* buffer = NULL;
    uint8_t addr = 0x30;
    uint8_t cpuNum = 0;
};

TEST_F(LogDataTestFixture, main_test)
{
    logNvdJson(addr, cpuNum, root);
    expected = cJSON_Parse(buffer);
    EXPECT_EQ(true, cJSON_Compare(expected, root, true));
    cJSON_Delete(root);
}

#endif