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
#include "CrashdumpSections/nvd/csrs.hpp"
#include "CrashdumpSections/nvd/registers.hpp"
#include "crashdump.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace ::testing;
using ::testing::Return;
using namespace crashdump;

enum regname
{
    smb_cmd_cfg_spd0,
    smb_cmd_cfg_spd1,
    smb_data_cfg_spd0,
    smb_data_cfg_spd1,
    smb_status_cfg_spd0,
    smb_status_cfg_spd1,
};

// change to ifdef or adjust CMakeList.txt to test against hardware
#ifndef MOCK
class RegisterTestFixture : public Test
{
  public:
    void SetUp() override
    {
        peci_Lock(&fd, PECI_TIMEOUT_MS);
        buffer = readTestFile("/tmp/test.json");
    }

    void TearDown() override
    {
        peci_Unlock(fd);
    }

    RegEntry reg = {};
    EPECIStatus status = PECI_CC_INVALID_REQ;
    uint8_t addr = 0x30;
    uint32_t val = 0;
    uint32_t expected = 0;
    uint8_t cc = 0;
    uint8_t dimm = 1;
    uint8_t bus = 0;
    char* output = NULL;
    char* buffer = NULL;
    cJSON* logSection = NULL;
    cJSON* json = NULL;
    int fd = -1;
};

TEST_F(RegisterTestFixture, regRd)
{
    int idx = 0;

    if (buffer == NULL)
    {
        FAIL();
    }

    // read registers and compare to test.json file values
    for (int i = smb_data_cfg_spd0; i <= smb_status_cfg_spd1; i++)
    {
        regRd(addr, regs[i], &val, fd, &cc);
        DEBUG_CC_PRINT(cc, val);
        json = cJSON_Parse(buffer);
        logSection = cJSON_GetArrayItem(json, idx++);
        output = cJSON_Print(logSection);
        expected = strtoull(removeQuotes(output), NULL, 16);
        EXPECT_EQ(expected, val);
    }
}

TEST_F(RegisterTestFixture, regWr)
{
    for (int i = smb_cmd_cfg_spd0; i <= smb_cmd_cfg_spd1; i++)
    {
        regRd(addr, regs[i], &val, fd, &cc);
        expected = val + 1;
        regWr(addr, regs[i], ++val, fd, &cc);
        regRd(addr, regs[i], &val, fd, &cc);
        EXPECT_EQ(expected, val);
    }
}

TEST_F(RegisterTestFixture, smbusRd_smbusWr_devid)
{
    uint8_t dev = 1;
    uint8_t func = 4;
    uint8_t offset = 0x008;
    uint32_t val0 = 0;
    uint32_t val1 = 0;
    uint32_t val2 = 0;
    uint32_t devid = 0;
    uint32_t expected = 0x8086097b;
    uint8_t cmdByte = 0x81;
    uint8_t base = 0xb0;
    uint8_t saddr = base + dimm * 2;
    NVDStatus status = NVD_FAIL;

    status = smbusWr(addr, bus, saddr, cmdByte, 2,
                     (func & 0x7) | ((dev & 0x1f) << 3), fd, &cc, true);
    cmdByte = 0x41;
    status = smbusWr(addr, bus, saddr, cmdByte, 2, offset, fd, &cc, true);
    EXPECT_EQ(NVD_SUCCESS, status);
    EXPECT_FALSE(PECI_CC_UA(cc));
    cmdByte = 0x81;
    status = smbusRd(addr, bus, saddr, cmdByte, 2, &val0, fd, &cc, true);
    EXPECT_EQ(NVD_SUCCESS, status);
    EXPECT_FALSE(PECI_CC_UA(cc));
    cmdByte = 0x01;
    status = smbusRd(addr, bus, saddr, cmdByte, 2, &val1, fd, &cc, true);
    EXPECT_EQ(NVD_SUCCESS, status);
    EXPECT_FALSE(PECI_CC_UA(cc));
    cmdByte = 0x41;
    status = smbusRd(addr, bus, saddr, cmdByte, 1, &val2, fd, &cc, true);
    EXPECT_EQ(NVD_SUCCESS, status);
    EXPECT_FALSE(PECI_CC_UA(cc));
    devid = ((val0 & 0xff) << 24) | ((val1 & 0xffff) << 8) | (val2 & 0xff);
    EXPECT_EQ(expected, devid);
}

TEST_F(RegisterTestFixture, csrRd_devid_mfgid)
{
    NVDStatus status = NVD_FAIL;
    uint32_t val = 0;
    expected = 0x8086097b;

    status = csrRd(addr, dimm, csrs[vendor_device_id], &val, fd, &cc, true);
    EXPECT_EQ(NVD_SUCCESS, status);
    EXPECT_FALSE(PECI_CC_UA(cc));
    EXPECT_EQ(expected, val);

    expected = 0x4;
    status = csrRd(addr, dimm, csrs[revision_mfg_id], &val, fd, &cc, true);
    EXPECT_EQ(NVD_SUCCESS, status);
    EXPECT_FALSE(PECI_CC_UA(cc));
    EXPECT_EQ(expected, val);
}

TEST_F(RegisterTestFixture, csrWr)
{
    bool check = true;
    NVDStatus status = NVD_FAIL;

    status = csrRd(addr, dimm, csrs[mb_smbus_cmd], &val, fd, &cc, check);
    EXPECT_EQ(NVD_SUCCESS, status);
    EXPECT_FALSE(PECI_CC_UA(cc));
    expected = val + 1;
    csrWr(addr, dimm, csrs[mb_smbus_cmd], ++val, fd, &cc, check);
    EXPECT_EQ(NVD_SUCCESS, status);
    EXPECT_FALSE(PECI_CC_UA(cc));
    csrRd(addr, dimm, csrs[mb_smbus_cmd], &val, fd, &cc, check);
    EXPECT_EQ(NVD_SUCCESS, status);
    EXPECT_FALSE(PECI_CC_UA(cc));
    EXPECT_EQ(expected, val);
}

TEST_F(RegisterTestFixture, csrRd_payload_readable)
{
    bool check = true;
    NVDStatus status = NVD_FAIL;

    for (int i = mb_smbus_input_payload_0; i <= mb_smbus_input_payload_31; i++)
    {
        status = csrRd(addr, dimm, csrs[i], &val, fd, &cc, check);
        EXPECT_EQ(NVD_SUCCESS, status);
        EXPECT_FALSE(PECI_CC_UA(cc));
    }

    for (int i = mb_smbus_output_payload_0; i <= mb_smbus_output_payload_31;
         i++)
    {
        status = csrRd(addr, dimm, csrs[i], &val, fd, &cc, check);
        EXPECT_EQ(NVD_SUCCESS, status);
        EXPECT_FALSE(PECI_CC_UA(cc));
    }
}

#endif
