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

const std::array<const char*, 27> CPX_PORT_ID = {
    "KTI0",
    "KTI1",
    "KTI2",
    "KTI3",
    "not implemented",
    "not implemented",
    "not implemented",
    "not implemented",
    "not implemented",
    "not implemented",
    "not implemented",
    "not implemented",
    "IMC0",
    "IMC1",
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
    "PCI4",
    "not implemented",
    "UBOX",
};

const std::array<const char*, 16> CPX_LLCS = {
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
    "not implemented",
    "not implemented",
    "not implemented",
    "not implemented",
};

const std::map<uint8_t, uint8_t> CpxfirstErrorCha = {
    {0x0c0, 0},  {0x0c1, 1},  {0x0c2, 2},  {0x0c3, 3},  {0x0c4, 4},
    {0x0c5, 5},  {0x0c6, 6},  {0x0c7, 7},  {0x0c8, 8},  {0x0c9, 9},
    {0x0ca, 10}, {0x0cb, 11}, {0x0cc, 12}, {0x0cd, 13}, {0x0ce, 14},
    {0x0cf, 15}, {0x0d0, 16}, {0x0d1, 17}, {0x0d2, 18}, {0x0d3, 19},
    {0x0d4, 20}, {0x0d5, 21}, {0x0d6, 22}, {0x0d7, 23}, {0x0d8, 24},
    {0x0d9, 25}, {0x0da, 26}, {0x0db, 27}, {0x0dc, 28}, {0x0dd, 29},
    {0x0de, 30}, {0x0df, 31}, {0x0e0, 32}, {0x0e1, 33}, {0x0e2, 34},
    {0x0e3, 35}, {0x0e4, 36}, {0x0e5, 37}, {0x0e6, 38}, {0x0e7, 39},
    {0x0e8, 40}, {0x0e9, 41}, {0x0ea, 42}, {0x0eb, 43}, {0x0ec, 44},
    {0x0ed, 45}, {0x0ee, 46}, {0x0ef, 47}, {0x0f0, 48}, {0x0f1, 49},
    {0x0f2, 50}, {0x0f3, 51}, {0x0f4, 52}, {0x0f5, 53}, {0x0f6, 54},
    {0x0f7, 55}, {0x0f8, 56}, {0x0f9, 57}, {0x0fa, 58}, {0x0fb, 59},
    {0x0fc, 60}, {0x0fd, 61}, {0x0fe, 62}, {0x0ff, 63}
};

const std::map<uint8_t, const char*> CpxfirstError = {
    {0x002, "UPI 0, bank 5"},
    {0x003, "UPI 1, bank 12"},
    {0x006, "UPI 2, bank 19"},
    {0x007, "UPI 3, bank 20"},
    {0x00a, "UPI 4, bank 21"},
    {0x00b, "UPI 5, bank 22"},
    {0x044, "PCU, bank 4"},
    {0x060, "IMC 0, bank 13,14,17"},
    {0x062, "M2MEM0, bank 7"},
    {0x064, "IMC 1, bank 15,16,18"},
    {0x066, "M2MEM1, bank 8"},
    {0x0a0, "IIO0, bank 6"},
    {0x0a1, "IIO0, bank 6"},
    {0x0a2, "IIO0, bank 6"},
    {0x0a3, "IIO0, bank 6"},
    {0x0a4, "IIO1, bank 6"},
    {0x0a5, "IIO1, bank 6"},
    {0x0a6, "IIO1, bank 6"},
    {0x0a7, "IIO1, bank 6"},
    {0x0a8, "IIO2, bank 6"},
    {0x0a9, "IIO2, bank 6"},
    {0x0aa, "IIO2, bank 6"},
    {0x0ab, "IIO2, bank 6"},
    {0x0ac, "CBDMA, bank 6"},
    {0x0ad, "CBDMA, bank 6"},
    {0x0ae, "CBDMA, bank 6"},
    {0x0af, "CBDMA, bank 6"},
    {0x0b0, "MCP0, bank 6"},
    {0x0b1, "MCP0, bank 6"},
    {0x0b2, "MCP0, bank 6"},
    {0x0b3, "MCP0, bank 6"},
    {0x0b4, "MCP1, bank 6"},
    {0x0b5, "MCP1, bank 6"},
    {0x0b6, "MCP1, bank 6"},
    {0x0b7, "MCP1, bank 6"},
    {0x0c0, "CHA0, bank 9"},
    {0x0c1, "CHA1, bank 10"},
    {0x0c2, "CHA2, bank 11"},
    {0x0c3, "CHA3, bank 9"},
    {0x0c4, "CHA4, bank 10"},
    {0x0c5, "CHA5, bank 11"},
    {0x0c6, "CHA6, bank 9"},
    {0x0c7, "CHA7, bank 10"},
    {0x0c8, "CHA8, bank 11"},
    {0x0c9, "CHA9, bank 9"},
    {0x0ca, "CHA10, bank 10"},
    {0x0cb, "CHA11, bank 11"},
    {0x0cc, "CHA12, bank 9"},
    {0x0cd, "CHA13, bank 10"},
    {0x0ce, "CHA14, bank 11"},
    {0x0cf, "CHA15, bank 9"},
    {0x0d0, "CHA16, bank 10"},
    {0x0d1, "CHA17, bank 11"},
    {0x0d2, "CHA18, bank 9"},
    {0x0d3, "CHA19, bank 10"},
    {0x0d4, "CHA20, bank 11"},
    {0x0d5, "CHA21, bank 9"},
    {0x0d6, "CHA22, bank 10"},
    {0x0d7, "CHA23, bank 11"},
    {0x0d8, "CHA24, bank 9"},
    {0x0d9, "CHA25, bank 10"},
    {0x0da, "CHA26, bank 11"},
    {0x0db, "CHA27, bank 9"},
    {0x0dc, "CHA28"},
    {0x0dd, "CHA29"},
    {0x0de, "CHA30"},
    {0x0df, "CHA31"},
    {0x0e0, "CHA32"},
    {0x0e1, "CHA33"},
    {0x0e2, "CHA34"},
    {0x0e3, "CHA35"},
    {0x0e4, "CHA36"},
    {0x0e5, "CHA37"},
    {0x0e6, "CHA38"},
    {0x0e7, "CHA39"},
    {0x0e8, "CHA40"},
    {0x0e9, "CHA41"},
    {0x0ea, "CHA42"},
    {0x0eb, "CHA43"},
    {0x0ec, "CHA44"},
    {0x0ed, "CHA45"},
    {0x0ee, "CHA46"},
    {0x0ef, "CHA47"},
    {0x0f0, "CHA48"},
    {0x0f1, "CHA49"},
    {0x0f2, "CHA50"},
    {0x0f3, "CHA51"},
    {0x0f4, "CHA52"},
    {0x0f5, "CHA53"},
    {0x0f6, "CHA54"},
    {0x0f7, "CHA55"},
    {0x0f8, "CHA56"},
    {0x0f9, "CHA57"},
    {0x0fa, "CHA58"},
    {0x0fb, "CHA59"},
    {0x0fc, "CHA60"},
    {0x0fd, "CHA61"},
    {0x0fe, "CHA62"},
    {0x0ff, "CHA63"}
};

const std::map<uint32_t, const char*> CpxOpCodeDecode = {
    {0x201, "CRd"},
    {0x202, "DRd"},
    {0x206, "DRdPTE"},
    {0x207, "PRd"},
    {0x258, "PrefRFO"},
    {0x25a, "PrefData"},
    {0x259, "PrefCode"},
    {0x203, "Monitor"},
    {0x200, "RFO"},
    {0x20c, "WCiLF"},
    {0x20d, "WCiL"},
    {0x20f, "WiL"},
    {0x213, "PCIWiL"},
    {0x214, "PCIWiLF"},
    {0x218, "CLFlush"},
    {0x219, "CLCleanse"},
    {0x21e, "PCIRdCur"},
    {0x229, "LLCWBInv"},
    {0x228, "LLCInv"},
    {0x248, "ItoM"},
    {0x244, "WbMtoI"},
    {0x245, "WbMtoE"},
    {0x246, "WbEFtoI"},
    {0x247, "WbEFtoE"},
    {0x243, "WbPushHint"},
    {0x270, "RdMonitor"},
    {0x271, "ClrMonitor"},
    {0x27c, "PortIn"},
    {0x274, "IntA"},
    {0x27f, "Lock"},
    {0x27e, "SplitLock"},
    {0x273, "Unlock"},
    {0x260, "SpCyc"},
    {0x275, "PortOut"},
    {0x27b, "IntPriUp"},
    {0x279, "IntLog"},
    {0x27a, "IntPhy"},
    {0x278, "EOI"},
    {0x276, "FERR"},
    {0x180, "SnpCur"},
    {0x181, "SnpCode"},
    {0x182, "SnpData"},
    {0x183, "SnpDataMigratory"},
    {0x184, "SnpInvOwn"},
    {0x185, "SnpInvItoE"},
    {0x18f, "PrefetchHint"},
    {0x11b, "SFVictim"},
    {0x131, "LLCVictim"},
    {0x080, "KRdCur"},
    {0x081, "KRdCode"},
    {0x082, "KRdData"},
    {0x083, "KRdDataMig"},
    {0x084, "KRdInvOwn"},
    {0x085, "KInvXtoI"},
    {0x086, "KPushHint"},
    {0x087, "KInvItoE"},
    {0x08c, "KRdInv"},
    {0x08f, "KInvItoM"},
    {0x000, "KWbMtoI"},
    {0x001, "KWbMtoS"},
    {0x002, "KWbMtoE"},
    {0x003, "KNonSnpWr"},
    {0x004, "KWbMtoIPtl"},
    {0x006, "KWbMtoEPtl"},
    {0x007, "KNonSnpWrPtl"},
    {0x008, "KWbPushMtoI"},
    {0x00b, "KWbFlush"},
    {0x00c, "KEvctCln"},
    {0x00d, "KNonSnpRd"},
    {0x00f, "Slot0LLCTRL"}
};

struct CpxTORData
{
    union
    {
        struct
        {
            uint32_t master_valid : 1, valid : 1, retry : 1, in_pipe : 1,
                cha_inside : 5, tor : 5, core_id : 6, thread_id : 1,
                request_opCode : 11;
        };
        uint32_t subindex0;
    };
    union
    {
        struct
        {
            uint32_t addr_lo : 14, fsm : 6, target : 5, sad : 3, lcs : 4;
        };
        uint32_t subindex1;
    };
    union
    {
        uint32_t address;
        uint32_t subindex2;
    };

    uint32_t cha;
    uint32_t idx;
};

using CpxTOR =
    std::map<uint32_t, std::pair<SocketCtx, std::vector<CpxTORData>>>;