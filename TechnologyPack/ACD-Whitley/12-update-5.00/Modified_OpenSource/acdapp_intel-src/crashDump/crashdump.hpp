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

#pragma once
#include "utils_dbusplus.hpp"

#ifndef SPX_BMC_ACD
#include <linux/peci-ioctl.h>
#include <peci.h>
#endif

#include <array>
#include <vector>

extern "C" {
#ifndef SPX_BMC_ACD
#include <cjson/cJSON.h>
#include "safe_str_lib.h"
#else
#include "cJSON.h"
#include "libpeci4.h"
#include "crashdumpRunControl.h"
#endif
}
#define CRASHDUMP_PRINT(level, fmt, ...) fprintf(fmt, __VA_ARGS__)
#define CRASHDUMP_VALUE_LEN 6

#define ICXD_MODEL 0x606C0

#define RESET_DETECTED_NAME "cpu%d.%s"

//peci-io
#define PECI_MBX_INDEX_CPU_ID           0  /* Package Identifier Read */
#define PECI_MBX_INDEX_VR_DEBUG         1  /* VR Debug */
#define PECI_MBX_INDEX_PKG_TEMP_READ        2  /* Package Temperature Read */
#define PECI_MBX_INDEX_ENERGY_COUNTER       3  /* Energy counter */
#define PECI_MBX_INDEX_ENERGY_STATUS        4  /* DDR Energy Status */
#define PECI_MBX_INDEX_WAKE_MODE_BIT        5  /* "Wake on PECI" Mode bit */
#define PECI_MBX_INDEX_EPI          6  /* Efficient Performance Indication */
#define PECI_MBX_INDEX_PKG_RAPL_PERF        8  /* Pkg RAPL Performance Status Read */
#define PECI_MBX_INDEX_PER_CORE_DTS_TEMP    9  /* Per Core DTS Temperature Read */
#define PECI_MBX_INDEX_DTS_MARGIN       10 /* DTS thermal margin */
#define PECI_MBX_INDEX_SKT_PWR_THRTL_DUR    11 /* Socket Power Throttled Duration */
#define PECI_MBX_INDEX_CFG_TDP_CONTROL      12 /* TDP Config Control */
#define PECI_MBX_INDEX_CFG_TDP_LEVELS       13 /* TDP Config Levels */
#define PECI_MBX_INDEX_DDR_DIMM_TEMP        14 /* DDR DIMM Temperature */
#define PECI_MBX_INDEX_CFG_ICCMAX       15 /* Configurable ICCMAX */
#define PECI_MBX_INDEX_TEMP_TARGET      16 /* Temperature Target Read */
#define PECI_MBX_INDEX_CURR_CFG_LIMIT       17 /* Current Config Limit */
#define PECI_MBX_INDEX_DIMM_TEMP_READ       20 /* Package Thermal Status Read */
#define PECI_MBX_INDEX_DRAM_IMC_TMP_READ    22 /* DRAM IMC Temperature Read */
#define PECI_MBX_INDEX_DDR_CH_THERM_STAT    23 /* DDR Channel Thermal Status */
#define PECI_MBX_INDEX_PKG_POWER_LIMIT1     26 /* Package Power Limit1 */
#define PECI_MBX_INDEX_PKG_POWER_LIMIT2     27 /* Package Power Limit2 */
#define PECI_MBX_INDEX_TDP          28 /* Thermal design power minimum */
#define PECI_MBX_INDEX_TDP_HIGH         29 /* Thermal design power maximum */
#define PECI_MBX_INDEX_TDP_UNITS        30 /* Units for power/energy registers */
#define PECI_MBX_INDEX_RUN_TIME         31 /* Accumulated Run Time */
#define PECI_MBX_INDEX_CONSTRAINED_TIME     32 /* Thermally Constrained Time Read */
#define PECI_MBX_INDEX_TURBO_RATIO      33 /* Turbo Activation Ratio */
#define PECI_MBX_INDEX_DDR_RAPL_PL1     34 /* DDR RAPL PL1 */
#define PECI_MBX_INDEX_DDR_PWR_INFO_HIGH    35 /* DRAM Power Info Read (high) */
#define PECI_MBX_INDEX_DDR_PWR_INFO_LOW     36 /* DRAM Power Info Read (low) */
#define PECI_MBX_INDEX_DDR_RAPL_PL2     37 /* DDR RAPL PL2 */
#define PECI_MBX_INDEX_DDR_RAPL_STATUS      38 /* DDR RAPL Performance Status */
#define PECI_MBX_INDEX_DDR_HOT_ABSOLUTE     43 /* DDR Hottest Dimm Absolute Temp */
#define PECI_MBX_INDEX_DDR_HOT_RELATIVE     44 /* DDR Hottest Dimm Relative Temp */
#define PECI_MBX_INDEX_DDR_THROTTLE_TIME    45 /* DDR Throttle Time */
#define PECI_MBX_INDEX_DDR_THERM_STATUS     46 /* DDR Thermal Status */
#define PECI_MBX_INDEX_TIME_AVG_TEMP        47 /* Package time-averaged temperature */
#define PECI_MBX_INDEX_TURBO_RATIO_LIMIT    49 /* Turbo Ratio Limit Read */
#define PECI_MBX_INDEX_HWP_AUTO_OOB     53 /* HWP Autonomous Out-of-band */
#define PECI_MBX_INDEX_DDR_WARM_BUDGET      55 /* DDR Warm Power Budget */
#define PECI_MBX_INDEX_DDR_HOT_BUDGET       56 /* DDR Hot Power Budget */
#define PECI_MBX_INDEX_PKG_PSYS_PWR_LIM3    57 /* Package/Psys Power Limit3 */
#define PECI_MBX_INDEX_PKG_PSYS_PWR_LIM1    58 /* Package/Psys Power Limit1 */
#define PECI_MBX_INDEX_PKG_PSYS_PWR_LIM2    59 /* Package/Psys Power Limit2 */
#define PECI_MBX_INDEX_PKG_PSYS_PWR_LIM4    60 /* Package/Psys Power Limit4 */
#define PECI_MBX_INDEX_PERF_LIMIT_REASON    65 /* Performance Limit Reasons */

#define PECI_PKG_ID_CPU_ID          0x0000  /* CPUID Info */
#define PECI_PKG_ID_PLATFORM_ID         0x0001  /* Platform ID */
#define PECI_PKG_ID_UNCORE_ID           0x0002  /* Uncore Device ID */
#define PECI_PKG_ID_MAX_THREAD_ID       0x0003  /* Max Thread ID */
#define PECI_PKG_ID_MICROCODE_REV       0x0004  /* CPU Microcode Update Revision */
#define PECI_PKG_ID_MACHINE_CHECK_STATUS    0x0005  /* Machine Check Status */

#define PECI_CRASHDUMP_ENABLED      0x00
#define PECI_CRASHDUMP_NUM_AGENTS   0x01
#define PECI_CRASHDUMP_AGENT_DATA   0x02

#define PECI_CRASHDUMP_CORE     0x00
#define PECI_CRASHDUMP_TOR      0x01

/* Crashdump Agent Param */
#define PECI_CRASHDUMP_PAYLOAD_SIZE 0x00

/* Crashdump Agent Data Param */
#define PECI_CRASHDUMP_AGENT_ID     0x00
#define PECI_CRASHDUMP_AGENT_PARAM  0x01

/* Device Specific Completion Code (CC) Definition */
#define PECI_DEV_CC_SUCCESS             0x40
#define PECI_DEV_CC_NEED_RETRY              0x80
#define PECI_DEV_CC_OUT_OF_RESOURCE         0x81
#define PECI_DEV_CC_UNAVAIL_RESOURCE            0x82
#define PECI_DEV_CC_INVALID_REQ             0x90
#define PECI_DEV_CC_MCA_ERROR               0x91
#define PECI_DEV_CC_CATASTROPHIC_MCA_ERROR      0x93
#define PECI_DEV_CC_FATAL_MCA_DETECTED          0x94
#define PECI_DEV_CC_PARITY_ERROR_ON_GPSB_OR_PMSB    0x98
#define PECI_DEV_CC_PARITY_ERROR_ON_GPSB_OR_PMSB_IERR   0x9B
#define PECI_DEV_CC_PARITY_ERROR_ON_GPSB_OR_PMSB_MCA    0x9C
//---
typedef enum
{
    EMERG,
    ALERT,
    CRIT,
    ERR,
    WARNING,
    NOTICE,
    INFO,
    DEBUG,
} severity;

typedef enum
{
    ACD_SUCCESS,
    ACD_FAILURE,
    ACD_INVALID_OBJECT,
    ACD_ALLOCATE_FAILURE
} acdStatus;
namespace cpuid
{
typedef enum
{
    STARTUP = 1,
    EVENT = 0,
    INVALID = 2,
    OVERWRITTEN = 3,
} cpuidState;
}

namespace record_type
{
constexpr const int offset = 24;
constexpr const int coreCrashLog = 0x04;
constexpr const int uncoreStatusLog = 0x08;
constexpr const int torDump = 0x09;
constexpr const int metadata = 0x0b;
constexpr const int pmInfo = 0x0c;
constexpr const int addressMap = 0x0d;
constexpr const int bmcAutonomous = 0x23;
constexpr const int mcaLog = 0x3e;
} // namespace record_type

namespace crashdump
{
constexpr char const* dbgStatusItemName = "status";
constexpr const char* dbgFailedStatus = "N/A";

namespace cpu
{
enum Model
{
    skx,
    clx,
    cpx,
    icx,
    icx2,
    icxd,
    numberOfModels,
};
namespace stepping
{
constexpr const uint8_t skx = 0;
constexpr const uint8_t clx = 6;
constexpr const uint8_t cpx = 10;
constexpr const uint8_t icx = 0;
constexpr const uint8_t icx2 = 4;
constexpr const uint8_t icxd = 4;
} // namespace stepping
} // namespace cpu

typedef enum
{
    ON = 1,
    OFF = 0,
    UNKNOWN = -1,
} pwState;

typedef enum
{
    UNCORE,
    TOR,
    PM_INFO,
    ADDRESS_MAP,
    BIG_CORE,
    MCA,
    METADATA,
    NUMBER_OF_SECTIONS,
} Section;

const int NUMBER_OF_CPU_MODELS = crashdump::cpu::Model::numberOfModels;

typedef struct _InputFileInfo
{
    bool unique;
    char* filenames[NUMBER_OF_CPU_MODELS];
    cJSON* buffers[NUMBER_OF_CPU_MODELS];
} InputFileInfo;

typedef struct _JSONInfo
{
    char* filenamePtr;
    cJSON* bufferPtr;
} JSONInfo;

typedef struct COREMaskRead
{
    uint8_t coreMaskCc;
    int coreMaskRet;
    bool coreMaskValid;
    cpuid::cpuidState source;
} COREMaskRead;

typedef struct CHACountRead
{
    uint8_t chaCountCc;
    int chaCountRet;
    bool chaCountValid;
    cpuid::cpuidState source;
} CHACountRead;

typedef struct CPUIDRead
{
    uint8_t cpuidCc;
    int cpuidRet;
    CPUModel cpuModel;
    uint8_t stepping;
    bool cpuidValid;
    cpuid::cpuidState source;
} CPUIDRead;

typedef struct capidRead
{
    uint32_t capid2;
    uint8_t capid2Cc;
    int capid2Ret;
} CAPIDRead;

struct CPUInfo
{
    int clientAddr;
    cpu::Model model;
    uint64_t coreMask;
    uint64_t crashedCoreMask;
    uint8_t sectionMask = 0xff;
    size_t chaCount;
    pwState initialPeciWake;
    JSONInfo inputFile;
    CPUIDRead cpuidRead;
    CAPIDRead capidRead;
    CHACountRead chaCountRead;
    COREMaskRead coreMaskRead;
    struct timespec launchDelay;
};

typedef struct
{
    char* name;
    Section section;
    int record_type;
    int (*fptr)(CPUInfo& cpuInfo, cJSON* pJsonChild);
} CrashdumpSection;

typedef struct PlatformState
{
    bool resetDetected;
    int resetCpu;
    int resetSection;
    int currentSection;
    int currentCpu;
} PlatformState;

extern const CrashdumpSection sectionNames[NUMBER_OF_SECTIONS];

void setResetDetected();
} // namespace crashdump

namespace revision
{
constexpr const int offset = 0;
constexpr const int revision1 = 0x01;
static int revision_uncore = 0;
} // namespace revision

namespace product_type
{
constexpr const int offset = 12;
constexpr const int clxSP = 0x2C;
constexpr const int cpx = 0x34;
constexpr const int skxSP = 0x2A;
constexpr const int icxSP = 0x1A;
constexpr const int bmcAutonomous = 0x23;
constexpr const int icxdSP = 0x1B;
} // namespace product_type

int cd_snprintf_s(char* str, size_t len, const char* format, ...);

inline static void logCrashdumpVersion(cJSON* parent,
                                       crashdump::CPUInfo& cpuInfo,
                                       int recordType)
{
    struct VersionInfo
    {
        crashdump::cpu::Model cpuModel;
        int data;
    };

    static constexpr const std::array product{
        VersionInfo{crashdump::cpu::clx, product_type::clxSP},
        VersionInfo{crashdump::cpu::cpx, product_type::cpx},
        VersionInfo{crashdump::cpu::skx, product_type::skxSP},
        VersionInfo{crashdump::cpu::icx, product_type::icxSP},
        VersionInfo{crashdump::cpu::icx2, product_type::icxSP},
        VersionInfo{crashdump::cpu::icxd, product_type::icxdSP},
    };

    static constexpr const std::array revision{
        VersionInfo{crashdump::cpu::clx, revision::revision1},
        VersionInfo{crashdump::cpu::cpx, revision::revision1},
        VersionInfo{crashdump::cpu::skx, revision::revision1},
        VersionInfo{crashdump::cpu::icx, revision::revision1},
        VersionInfo{crashdump::cpu::icx2, revision::revision1},
        VersionInfo{crashdump::cpu::icxd, revision::revision1},
    };

    int productType = 0;
    for (const VersionInfo& cpuProduct : product)
    {
        if (cpuInfo.model == cpuProduct.cpuModel)
        {
            productType = cpuProduct.data;
        }
    }

    int revisionNum = 0;
    for (const VersionInfo& cpuRevision : revision)
    {
        if (cpuInfo.model == cpuRevision.cpuModel)
        {
            revisionNum = cpuRevision.data;
        }
    }

    if (recordType == record_type::uncoreStatusLog)
    {
        revisionNum = revision::revision_uncore;
    }

    // Build the version number:
    //  [31:30] Reserved
    //  [29:24] Crash Record Type
    //  [23:12] Product
    //  [11:8] Reserved
    //  [7:0] Revision
    int version = recordType << record_type::offset |
                  productType << product_type::offset |
                  revisionNum << revision::offset;
    char versionString[64];
    cd_snprintf_s(versionString, sizeof(versionString), "0x%x", version);
    cJSON_AddStringToObject(parent, "_version", versionString);
}
