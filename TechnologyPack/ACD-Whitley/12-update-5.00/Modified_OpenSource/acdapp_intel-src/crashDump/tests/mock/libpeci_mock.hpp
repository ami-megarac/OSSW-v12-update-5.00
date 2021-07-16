#pragma once

#include "gmock/gmock.h"
#include "gtest/gtest.h"

extern "C" {
#include <cjson/cJSON.h>
#include <peci.h>
}

using namespace ::testing;
using ::testing::Return;
void printJson(cJSON* value);
class PeciMOCK
{
  public:
    virtual ~PeciMOCK()
    {
    }
    MOCK_METHOD2(peci_Lock, EPECIStatus(int*, int));
    MOCK_METHOD9(peci_RdPCIConfigLocal_seq,
                 EPECIStatus(uint8_t, uint8_t, uint8_t, uint8_t, uint16_t,
                             uint8_t, uint8_t*, int, uint8_t*));
    MOCK_METHOD10(peci_RdEndPointConfigPciLocal_seq,
                  EPECIStatus(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t,
                              uint16_t, uint8_t, uint8_t*, int, uint8_t*));
    MOCK_METHOD7(peci_WrPkgConfig_seq,
                 EPECIStatus(uint8_t, uint8_t, uint16_t, uint32_t, uint8_t, int,
                             uint8_t*));
    MOCK_METHOD7(peci_RdPkgConfig_seq,
                 EPECIStatus(uint8_t, uint8_t, uint16_t, uint8_t, uint8_t*, int,
                             uint8_t*));
    MOCK_METHOD8(peci_CrashDump_Discovery,
                 EPECIStatus(uint8_t, uint8_t, uint8_t, uint16_t, uint8_t,
                             uint8_t, uint8_t*, uint8_t*));
    MOCK_METHOD7(peci_CrashDump_GetFrame,
                 EPECIStatus(uint8_t, uint16_t, uint16_t, uint16_t, uint8_t,
                             uint8_t*, uint8_t*));
    MOCK_METHOD5(peci_RdIAMSR,
                 EPECIStatus(uint8_t, uint8_t, uint16_t, uint64_t*, uint8_t*));
    MOCK_METHOD6(peci_RdPkgConfig, EPECIStatus(uint8_t, uint8_t, uint16_t,
                                               uint8_t, uint8_t*, uint8_t*));
};

class PeciTestFixture : public Test
{
  public:
    PeciTestFixture();

    ~PeciTestFixture();
    void SetUp() override;
    static std::unique_ptr<PeciMOCK> PeciMock;
};