#include "libpeci_mock.hpp"

PeciTestFixture::PeciTestFixture()
{
    PeciMock = std::make_unique<NiceMock<PeciMOCK>>();
}

PeciTestFixture::~PeciTestFixture()
{
    PeciMock.reset();
}
void PeciTestFixture::SetUp()
{
}

std::unique_ptr<PeciMOCK> PeciTestFixture::PeciMock;

EPECIStatus peci_Lock(int* peci_fd, int timeout)
{
    return PeciTestFixture::PeciMock->peci_Lock(peci_fd, timeout);
}
EPECIStatus peci_RdPCIConfigLocal_seq(uint8_t target, uint8_t u8Bus,
                                      uint8_t u8Device, uint8_t u8Fcn,
                                      uint16_t u16Reg, uint8_t u8ReadLen,
                                      uint8_t* pPCIReg, int peci_fd,
                                      uint8_t* cc)
{
    uint8_t data[] = {0xef, 0xbe, 0xad, 0xde};
    memcpy(pPCIReg, data, u8ReadLen);

    return PeciTestFixture::PeciMock->peci_RdPCIConfigLocal_seq(
        target, u8Bus, u8Device, u8Fcn, u16Reg, u8ReadLen, pPCIReg, peci_fd,
        cc);
}
EPECIStatus peci_RdEndPointConfigPciLocal_seq(uint8_t target, uint8_t u8Seg,
                                              uint8_t u8Bus, uint8_t u8Device,
                                              uint8_t u8Fcn, uint16_t u16Reg,
                                              uint8_t u8ReadLen,
                                              uint8_t* pPCIData, int peci_fd,
                                              uint8_t* cc)
{
    uint8_t data[] = {0xef, 0xbe, 0xad, 0xde};
    memcpy(pPCIData, data, u8ReadLen);
    return PeciTestFixture::PeciMock->peci_RdEndPointConfigPciLocal_seq(
        target, u8Seg, u8Bus, u8Device, u8Fcn, u16Reg, u8ReadLen, pPCIData,
        peci_fd, cc);
}

EPECIStatus peci_WrPkgConfig_seq(uint8_t target, uint8_t u8Index,
                                 uint16_t u16Param, uint32_t u32Value,
                                 uint8_t u8WriteLen, int peci_fd, uint8_t* cc)
{
    return PeciTestFixture::PeciMock->peci_WrPkgConfig_seq(
        target, u8Index, u16Param, u32Value, u8WriteLen, peci_fd, cc);
}

EPECIStatus peci_RdPkgConfig_seq(uint8_t target, uint8_t u8Index,
                                 uint16_t u16Value, uint8_t u8ReadLen,
                                 uint8_t* pPkgConfig, int peci_fd, uint8_t* cc)
{
    uint8_t data[] = {0x00, 0x00, 0x00, 0x00};
    memcpy(pPkgConfig, data, u8ReadLen);
    return PeciTestFixture::PeciMock->peci_RdPkgConfig_seq(
        target, u8Index, u16Value, u8ReadLen, pPkgConfig, peci_fd, cc);
}

EPECIStatus peci_CrashDump_Discovery(uint8_t target, uint8_t subopcode,
                                     uint8_t param0, uint16_t param1,
                                     uint8_t param2, uint8_t u8ReadLen,
                                     uint8_t* pData, uint8_t* cc)
{
    uint8_t data[] = {0x00, 0x00, 0x00, 0x00};
    memcpy(pData, data, u8ReadLen);
    return PeciTestFixture::PeciMock->peci_CrashDump_Discovery(
        target, subopcode, param0, param1, param2, u8ReadLen, pData, cc);
}

EPECIStatus peci_CrashDump_GetFrame(uint8_t target, uint16_t param0,
                                    uint16_t param1, uint16_t param2,
                                    uint8_t u8ReadLen, uint8_t* pData,
                                    uint8_t* cc)
{
    uint8_t data[] = {0x00, 0x00, 0x00, 0x00};
    memcpy(pData, data, u8ReadLen);
    return PeciTestFixture::PeciMock->peci_CrashDump_GetFrame(
        target, param0, param1, param2, u8ReadLen, pData, cc);
}

EPECIStatus peci_RdIAMSR(uint8_t target, uint8_t threadID, uint16_t MSRAddress,
                         uint64_t* u64MsrVal, uint8_t* cc)
{
    uint8_t data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    memcpy(u64MsrVal, data, 8);
    return PeciTestFixture::PeciMock->peci_RdIAMSR(target, threadID, MSRAddress,
                                                   u64MsrVal, cc);
}

EPECIStatus peci_RdPkgConfig(uint8_t target, uint8_t u8Index, uint16_t u16Value,
                             uint8_t u8ReadLen, uint8_t* pPkgConfig,
                             uint8_t* cc)
{
    uint8_t data[] = {0x00, 0x00, 0x00, 0x00};
    memcpy(pPkgConfig, data, 4);
    return PeciTestFixture::PeciMock->peci_RdPkgConfig(
        target, u8Index, u16Value, u8ReadLen, pPkgConfig, cc);
}

void printJson(cJSON* value)
{
    char* jsonStr = cJSON_Print(value);
    if (jsonStr != NULL)
    {
        printf("%s\n", jsonStr);
        free(jsonStr);
        jsonStr = nullptr;
    }
}