/******************************************************************************
 *
 * INTEL CONFIDENTIAL
 *
 * Copyright 2020 Intel Corporation.
 *
 * This software and the related documents are Intel copyrighted materials, and
 * your use of them is governed by the express license under which they were
 * provided to you ("License"). Unless the License provides otherwise, you may
 * not use, modify, copy, publish, distribute, disclose or transmit this
 * software or the related documents without Intel's prior written permission.
 *
 * This software and the related documents are provided as is, with no express
 * or implied warranties, other than those that are expressly stated in
 * the License.
 *
 ******************************************************************************/

#pragma once
#include <array>
#include <map>
#include <utility>
#include <vector>

#include <tor_defs.hpp>

const std::array<const char*, 27> ICX_PORT_ID = {
    "UPI0",
    "UPI1",
    "UPI2",
    "not implemented",
    "IMC0",
    "IMC1",
    "IMC2",
    "IMC3",
    "not implemented",
    "not implemented",
    "not implemented",
    "not implemented",
    "not implemented",
    "not implemented",
    "not implemented",
    "not implemented",
    "not implemented",
    "not implemented",
    "not implemented",
    "not implemented",
    "PCI0",
    "PCI1",
    "PCI2",
    "PCI3",
    "not implemented",
    "not implemented",
    "UBOX",
};

const std::array<const char*, 16> ICX_LLCS = {
    "not implemented",
    "SF_S",
    "SF_E",
    "SF_H",
    "not implemented",
    "not implemented",
    "not implemented",
    "not implemented",
    "LLC_I",
    "LLC_S",
    "LLC_E",
    "LLC_M",
    "LLC_F",
    "LLC_D",
    "not implemented",
    "LLC_P",
};

const std::map<uint8_t, uint8_t> IcxfirstErrorCha = {
    {0x40, 0},  {0x41, 1},  {0x42, 2},  {0x43, 3},  {0x44, 4},
    {0x45, 5},  {0x46, 6},  {0x47, 7},  {0x48, 8},  {0x49, 9},
    {0x4a, 10}, {0x4b, 11}, {0x4c, 12}, {0x4d, 13}, {0x4e, 14},
    {0x4f, 15}, {0x50, 16}, {0x51, 17}, {0x52, 18}, {0x53, 19},
    {0x54, 20}, {0x55, 21}, {0x56, 22}, {0x57, 23}, {0x58, 24},
    {0x59, 25}, {0x5a, 26}, {0x5b, 27}, {0x5c, 28}, {0x5d, 29},
    {0x5e, 30}, {0x5f, 31}, {0x60, 32}, {0x61, 33}, {0x62, 34},
    {0x63, 35}, {0x64, 36}, {0x65, 37}
};

const std::map<uint8_t, const char*> IcxfirstError = {
    {0x00, "Core0, bank 0-3"},
    {0x01, "Core1, bank 0-3"},
    {0x02, "Core2, bank 0-3"},
    {0x03, "Core3, bank 0-3"},
    {0x04, "Core4, bank 0-3"},
    {0x05, "Core5, bank 0-3"},
    {0x06, "Core6, bank 0-3"},
    {0x07, "Core7, bank 0-3"},
    {0x08, "Core8, bank 0-3"},
    {0x09, "Core9, bank 0-3"},
    {0x0a, "Core10, bank 0-3"},
    {0x0b, "Core11, bank 0-3"},
    {0x0c, "Core12, bank 0-3"},
    {0x0d, "Core13, bank 0-3"},
    {0x0e, "Core14, bank 0-3"},
    {0x0f, "Core15, bank 0-3"},
    {0x10, "Core16, bank 0-3"},
    {0x11, "Core17, bank 0-3"},
    {0x12, "Core18, bank 0-3"},
    {0x13, "Core19, bank 0-3"},
    {0x14, "Core20, bank 0-3"},
    {0x15, "Core21, bank 0-3"},
    {0x16, "Core22, bank 0-3"},
    {0x17, "Core23, bank 0-3"},
    {0x18, "Core24, bank 0-3"},
    {0x19, "Core25, bank 0-3"},
    {0x1a, "Core26, bank 0-3"},
    {0x1b, "Core27, bank 0-3"},
    {0x1c, "Core28, bank 0-3"},
    {0x1d, "Core29, bank 0-3"},
    {0x1e, "Core30, bank 0-3"},
    {0x1f, "Core31, bank 0-3"},
    {0x20, "Core32, bank 0-3"},
    {0x21, "Core33, bank 0-3"},
    {0x22, "Core34, bank 0-3"},
    {0x23, "Core35, bank 0-3"},
    {0x24, "Core36, bank 0-3"},
    {0x25, "Core37, bank 0-3"},
    {0x30, "Chassis_Punit_FSM, bank 4"},
    {0x31, "Chassis_Punit_FSM2, bank 4"},
    {0x32, "Chassis_Punit_CR, bank 4"},
    {0x40, "CHA0, bank 9"},
    {0x41, "CHA1, bank 10"},
    {0x42, "CHA2, bank 11"},
    {0x43, "CHA3, bank 9"},
    {0x44, "CHA4, bank 10"},
    {0x45, "CHA5, bank 11"},
    {0x46, "CHA6, bank 9"},
    {0x47, "CHA7, bank 10"},
    {0x48, "CHA8, bank 11"},
    {0x49, "CHA9, bank 9"},
    {0x4a, "CHA10, bank 10"},
    {0x4b, "CHA11, bank 11"},
    {0x4c, "CHA12, bank 9"},
    {0x4d, "CHA13, bank 10"},
    {0x4e, "CHA14, bank 11"},
    {0x4f, "CHA15, bank 9"},
    {0x50, "CHA16, bank 10"},
    {0x51, "CHA17, bank 11"},
    {0x52, "CHA18, bank 9"},
    {0x53, "CHA19, bank 10"},
    {0x54, "CHA20, bank 11"},
    {0x55, "CHA21, bank 9"},
    {0x56, "CHA22, bank 10"},
    {0x57, "CHA23, bank 11"},
    {0x58, "CHA24, bank 9"},
    {0x59, "CHA25, bank 10"},
    {0x5a, "CHA26, bank 11"},
    {0x5b, "CHA27, bank 9"},
    {0x5c, "CHA28, bank 10"},
    {0x5d, "CHA29, bank 11"},
    {0x5e, "CHA30, bank 9"},
    {0x5f, "CHA31, bank 10"},
    {0x60, "CHA32, bank 11"},
    {0x61, "CHA33, bank 9"},
    {0x62, "CHA34, bank 10"},
    {0x63, "CHA35, bank 11"},
    {0x64, "CHA36, bank 9"},
    {0x65, "CHA37, bank 10"},
    {0x6d, "Intel UPI 0, bank 5"},
    {0x6e, "Intel UPI 1, bank 7"},
    {0x6f, "Intel UPI 2, bank 8"},
    {0x80, "IMC 0 - CH0-CH1, bank 13, 14"},
    {0x81, "IMC 1 - CH0-CH1, bank 17, 18"},
    {0x82, "IMC 2 - CH0-CH1, bank 21, 22"},
    {0x83, "IMC 3 - CH0-CH1, bank 25, 26"},
    {0x86, "M2MEM 0, bank 12"},
    {0x87, "M2MEM 1, bank 16"},
    {0x88, "M2MEM 2, bank 20"},
    {0x89, "M2MEM 3, bank 24"},
    {0xc7, "Global IEH/UBOX (IOMCA), bank 6"}
};

const std::map<uint32_t, const char*> IcxOpCodeDecode = {
    {0x107, "PRd"},
    {0x11e, "RdCurr"},
    {0x505, "KInvXtoI"},
    {0x002, "RspDataM"},
    {0x148, "Invd"},
    {0x18c, "WbStoI"},
    {0x102, "DRd"},
    {0x40c, "KEvctCln"},
    {0x104, "DRd_Opt"},
    {0x149, "WbInvd"},
    {0x023, "RspIFwdFE"},
    {0x11c, "CLWB"},
    {0x500, "KRdCur"},
    {0x180, "CLDemote"},
    {0x404, "KWbIDataPtl"},
    {0x003, "RspIFwdM"},
    {0x504, "KRdInvOwn"},
    {0x01d, "GsrIsInMstate"},
    {0x008, "Cmp_FwdInvItoE"},
    {0x188, "ItoM"},
    {0x181, "ItoMWr_NS"},
    {0x408, "KWbPushMtoI"},
    {0x01b, "Victim"},
    {0x106, "DRdPTE"},
    {0x111, "CRd_Pref"},
    {0x009, "Cmp_PullData"},
    {0x00c, "DataC_Cmp"},
    {0x199, "PrefCode"},
    {0x19d, "MemPushWr_NS"},
    {0x116, "WCiLF_NS"},
    {0x103, "SetMonitor"},
    {0x110, "RFO_Pref"},
    {0x502, "KRdData"},
    {0x18b, "ItoMWr_WT"},
    {0x101, "CRd"},
    {0x10c, "WCiLF"},
    {0x1a4, "WbPushHint"},
    {0x004, "PullData"},
    {0x01e, "DataNC"},
    {0x010, "FakeCycle"},
    {0x18d, "MemPushWr"},
    {0x100, "RFO"},
    {0x40d, "KNonSnpRd"},
    {0x187, "WbEFtoE"},
    {0x507, "KInvItoE"},
    {0x11f, "RFOWr"},
    {0x50f, "KInvItoM"},
    {0x705, "SnpInvItoE"},
    {0x1c0, "SpCyc"},
    {0x113, "DRd_NS"},
    {0x10d, "WCiL"},
    {0x40b, "KWbFlush"},
    {0x025, "FwdCnflt"},
    {0x702, "SnpData"},
    {0x033, "RspIFwdMPtl"},
    {0x114, "DRd_Opt_Pref"},
    {0x704, "SnpInvOwn"},
    {0x501, "KRdCode"},
    {0x701, "SnpCode"},
    {0x406, "KWbEDataPtl"},
    {0x118, "CLFlush"},
    {0x400, "KWbIData"},
    {0x001, "RspS"},
    {0x19a, "PrefData"},
    {0x1d8, "CBO_EOI"},
    {0x026, "RspV"},
    {0x000, "RspI"},
    {0x185, "WbMtoE"},
    {0x1de, "SplitLock"},
    {0x14b, "LLCWB"},
    {0x1d5, "PortOut"},
    {0x198, "PrefRFO"},
    {0x403, "KNonSnpWr"},
    {0x184, "WbMtoI"},
    {0x007, "Cmp_FwdCode"},
    {0x1d4, "IntA"},
    {0x109, "PCommit"},
    {0x703, "SnpDataMigratory"},
    {0x032, "RspNack"},
    {0x006, "Cmp"},
    {0x139, "PMSeqInvd"},
    {0x031, "LLCVictim"},
    {0x1d1, "ClrMonitor"},
    {0x402, "KWbEData"},
    {0x1df, "Lock"},
    {0x105, "CRd_UC"},
    {0x186, "WbEFtoI"},
    {0x10e, "UCRdF"},
    {0x13c, "CLCleanse"},
    {0x401, "KWbSData"},
    {0x407, "KNonSnpWrPtl"},
    {0x020, "DataC"},
    {0x030, "TOR_TimeOut"},
    {0x00b, "Cmp_FwdInvOwn"},
    {0x1a8, "ItoMCacheNear"},
    {0x197, "ItoMWR_WT_NS"},
    {0x1da, "IntPhy"},
    {0x115, "WCiL_NS"},
    {0x1d9, "IntLog"},
    {0x11a, "CLFlush_Opt"},
    {0x503, "KRdDataMigratory"},
    {0x189, "ItoMWr"},
    {0x1a9, "RdCurrCacheNear"},
    {0x10f, "WiL"},
    {0x1dc, "PortIn"},
    {0x112, "DRd_Pref"},
    {0x005, "PullDataBogus"},
    {0x700, "SnpCur"},
    {0x027, "RspVFwdV"},
    {0x195, "WbOtoE"},
    {0x50c, "KRdInvOwnE"},
    {0x194, "WbOtoI"},
    {0x18a, "SpecItoM"},
    {0x1d3, "Unlock"},
    {0x1db, "IntPriUp"},
    {0x024, "RspSFwdFE"}
};

union PackageThermStatus
{
    struct
    {
        uint32_t thermal_monitor_status : 1, thermal_monitor_log : 1,
            prochot_status : 1, prochot_log : 1, out_of_spec_status: 1,
            out_of_spec_log: 1, threshold1_status: 1, threshold1_log: 1,
            threshold2_status: 1, threshold2_log: 1, power_limitation_status: 1,
            power_limitation_log: 1, pmax_status: 1, pmax_log: 1, reserved1: 2,
            temperature: 7, reserved2: 3, hw_feedback_notification_log: 1,
            resolution: 4, valid: 1;
    };
    uint32_t package_therm_status;
};

struct IcxTORData
{
    union
    {
        struct
        {
            uint32_t reserved0 : 2, valid : 1, reserved1 : 2, retry : 1,
                reserved2 : 1, in_pipe : 1, reserved3 : 24;
        };
        uint32_t tordump0_subindex0;
    };
    union
    {
        struct
        {
            uint32_t reserved4 : 29, address_8_6 : 3;
        };
        uint32_t tordump1_subindex0;
    };
    union
    {
        struct
        {
            uint32_t address_16_9 : 8, reserved5 : 24;
        };
        uint32_t tordump2_subindex0;
    };
    union
    {
        struct
        {
            uint32_t reserved6 : 8, thread_id : 3, reserved7 : 21;
        };
        uint32_t tordump0_subindex1;
    };
    union
    {
        struct
        {
            uint32_t reserved8 : 29, address_19_17 : 3;
        };
        uint32_t tordump1_subindex1;
    };
    union
    {
        struct
        {
            uint32_t address_27_20 : 8, reserved9 : 24;
        };
        uint32_t tordump2_subindex1;
    };
    union
    {
        struct
        {
            uint32_t reserved10 : 7, target : 5, reserved11 : 6, sad : 3,
                reserved12 : 11;
        };
        uint32_t tordump0_subindex2;
    };
    union
    {
        struct
        {
            uint32_t reserved13 : 29, address_30_28 : 3;
        };
        uint32_t tordump1_subindex2;
    };
    union
    {
        struct
        {
            uint32_t address_38_31 : 8, reserved14 : 24;
        };
        uint32_t tordump2_subindex2;
    };
    union
    {
        struct
        {
            uint32_t reserved15 : 21, core_id : 6, reserved16 : 5;
        };
        uint32_t tordump0_subindex3;
    };
    union
    {
        struct
        {
            uint32_t reserved17 : 29, address_41_39 : 3;
        };
        uint32_t tordump1_subindex3;
    };
    union
    {
        struct
        {
            uint32_t address_49_42 : 8, reserved18 : 24;
        };
        uint32_t tordump2_subindex3;
    };
    union
    {
        struct
        {
            uint32_t reserved19 : 17, request_opCode : 11, reserved20 : 4;
        };
        uint32_t tordump0_subindex4;
    };
    union
    {
        struct
        {
            uint32_t reserved21 : 29, address_51_50 : 2, reserved22 : 1;
        };
        uint32_t tordump1_subindex4;
    };
    union
    {
        struct
        {
            uint32_t reserved23 : 32;
        };
        uint32_t tordump2_subindex4;
    };
    union
    {
        struct
        {
            uint32_t reserved24 : 3, lcs : 4, reserved25 : 25;
        };
        uint32_t tordump0_subindex7;
    };
    union
    {
        struct
        {
            uint32_t reserved26 : 1, fsm : 5, reserved27 : 26;
        };
        uint32_t tordump1_subindex7;
    };
    union
    {
        struct
        {
            uint32_t reserved28 : 32;
        };
        uint32_t tordump2_subindex7;
    };

    uint32_t cha;
    uint32_t idx;
};

using IcxTOR =
    std::map<uint32_t, std::pair<SocketCtx, std::vector<IcxTORData>>>;