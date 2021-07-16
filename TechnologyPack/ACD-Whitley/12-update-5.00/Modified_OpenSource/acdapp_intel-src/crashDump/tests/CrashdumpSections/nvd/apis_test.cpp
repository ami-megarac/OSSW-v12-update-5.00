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
#include "crashdump.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace ::testing;
using ::testing::Return;
using namespace crashdump;

// change to ifdef or adjust CMakeList.txt to test against hardware
#ifndef MOCK
class ApiTestFixture : public Test
{
  public:
    void SetUp() override
    {
        peciStatus = peci_Lock(&fd, PECI_TIMEOUT_MS);
        EXPECT_EQ(PECI_CC_SUCCESS, peciStatus);
        buffer = readTestFile("/tmp/payload.json");
    }

    void TearDown() override
    {
        peci_Unlock(fd);
    }

    int fd = -1;
    EPECIStatus peciStatus;
    uint32_t apiStatus = 0;
    uint8_t addr = 48;
    uint8_t cc = 0;
    uint8_t dimm = 1;
    char* buffer = NULL;
    char* output = NULL;
    uint32_t expected = 0;
    cJSON* logSection = NULL;
    cJSON* json = NULL;
};

TEST(discovery, cpu0_two_dimms_cpu1_no_dimm)
{
    int addr = 48;
    int fd = -1;
    uint16_t mask = 0;
    uint16_t failMask = 0;

    EPECIStatus peciStatus = peci_Lock(&fd, PECI_TIMEOUT_MS);
    EXPECT_EQ(PECI_CC_SUCCESS, peciStatus);

    // requirements: nvd in slot 1 & slot 3
    failMask = discovery(addr, fd, &mask);
    EXPECT_EQ(0b0, failMask);
    EXPECT_EQ(0b1010, mask);

    mask = 0;
    addr = 49;
    failMask = discovery(addr, fd, &mask);
    EXPECT_EQ(0b0, failMask);
    EXPECT_EQ(0b0, mask);

    peci_Unlock(fd);
}

TEST_F(ApiTestFixture, identifyDimm)
{
    uint32_t payloadSize = 20;
    uint32_t* payload = (uint32_t*)calloc(payloadSize, sizeof(uint32_t));

    apiStatus = identifyDimm(addr, dimm, payload, payloadSize, fd, &cc);
    EXPECT_EQ(0x1, apiStatus);

    if (buffer == NULL)
    {
        FAIL();
    }

    int idx = 0;
    for (int i = 0; i < payloadSize; i++)
    {
        json = cJSON_Parse(buffer);
        logSection = cJSON_GetArrayItem(json, idx++);
        output = cJSON_Print(logSection);
        expected = strtoull(removeQuotes(output), NULL, 16);
        EXPECT_EQ(expected, payload[i]);
    }

    FREE(payload);
}

#endif
