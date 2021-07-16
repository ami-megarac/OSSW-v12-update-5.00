/******************************************************************************
 *
 * INTEL CONFIDENTIAL
 *
 * Copyright 2021 Intel Corporation.
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

union IerrLoggingReg
{
    struct
    {
        uint32_t firstIerrSrcIdCha : 8, firstIerrValid : 1,
            firstIerrSrcFromCbo : 1, reserved0 : 22;
    };
    uint32_t value;
};

union IerrLoggingRegSpr
{
    struct
    {
        uint32_t firstIerrSrcIdCha : 10, firstIerrValid : 1,
            firstIerrSrcFromCbo : 1, reserved0 : 20;
    };
    uint32_t value;
};

union McerrLoggingReg
{
    struct
    {
        uint32_t firstMcerrSrcIdCha : 8, firstMcerrValid : 1,
            firstMCerrSrcFromCbo : 1, reserved0 : 22;
    };
    uint32_t value;
};

union McerrLoggingRegSpr
{
    struct
    {
        uint32_t firstMcerrSrcIdCha : 10, firstMcerrValid : 1,
            firstMCerrSrcFromCbo : 1, reserved0 : 20;
    };
    uint32_t value;
};

union McerrErrorSource
{
    struct
    {
        uint32_t reserved0 : 20, msmi_internal : 1, reserved1 : 2,
            msmi_external : 1, reserved2 : 4, caterr_interal : 1, reserved3 : 2,
            caterr_external: 1;
    };
    uint32_t value;
};

struct SocketCtx
{
    IerrLoggingReg ierr;
    IerrLoggingRegSpr ierrSpr;
    McerrLoggingReg mcerr;
    McerrLoggingRegSpr mcerrSpr;
    McerrErrorSource mcerrErr;
};

enum class SadValues
{
    HOM = 0,
    MMIO,
    CFG,
    MMIOPartialRead,
    IO,
    IntelReserved0,
    SPC,
    IntelReserved1
};

union BDFFormatter
{
    struct
    {
        uint32_t reserved0 : 12, func : 3, dev : 5, bus : 8, reserved1 : 4;
    };
    uint32_t address;
};

const std::array<const char*, 8> SAD_RESULT = {
    "HOM", "MMIO",           "CFG", "MMIO Partial Read",
    "IO",  "Intel Reserved", "SPC", "Intel Reserved",
};

struct TORDataGeneric
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

    // ICX
    union
    {
        struct
        {
            uint32_t reserved0 : 2, valid1 : 1, reserved1 : 2, retry1 : 1,
                reserved2 : 1, in_pipe1 : 1, reserved3 : 24;
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
            uint32_t reserved6 : 8, thread_id1 : 3, reserved7 : 21;
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
            uint32_t reserved10 : 7, target1 : 5, reserved11 : 6, sad1 : 3,
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
            uint32_t reserved15 : 21, core_id1 : 6, reserved16 : 5;
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
            uint32_t reserved19 : 17, request_opCode1 : 11, reserved20 : 4;
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
            uint32_t reserved24 : 3, lcs1 : 4, reserved25 : 25;
        };
        uint32_t tordump0_subindex7;
    };
    union
    {
        struct
        {
            uint32_t reserved26 : 1, fsm1 : 5, reserved27 : 26;
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
    // SPR
    union
    {
        struct
        {
            uint32_t valid2: 1, reserved29 : 2, retry2 : 1, reserved30 : 3,
                in_pipe2 : 1, reserved31 : 24;
        };
        uint32_t tordump0_subindex0_spr;
    };
    union
    {
        struct
        {
            uint32_t reserved32 : 31, set_0_0 : 1;
        };
        uint32_t tordump1_subindex0_spr;
    };
    union
    {
        struct
        {
            uint32_t set_10_1 : 10, reserved33 : 22;
        };
        uint32_t tordump2_subindex0_spr;
    };
    union
    {
        struct
        {
            uint32_t reserved34 : 9, thread_id2 : 3, reserved35 : 20;
        };
        uint32_t tordump0_subindex1_spr;
    };
    union
    {
        struct
        {
            uint32_t reserved36 : 31, address_0_0_spr : 1;
        };
        uint32_t tordump1_subindex1_spr;
    };
    union
    {
        struct
        {
            uint32_t address_10_1_spr : 10, reserved37 : 22;
        };
        uint32_t tordump2_subindex1_spr;
    };
    union
    {
        struct
        {
            uint32_t reserved38 : 7, target2 : 5, reserved39 : 9, sad2 : 3,
                reserved40 : 8;
        };
        uint32_t tordump0_subindex2_spr;
    };
    union
    {
        struct
        {
            uint32_t reserved41 : 31, address_11_11_spr : 1;
        };
        uint32_t tordump1_subindex2_spr;
    };
    union
    {
        struct
        {
            uint32_t address_21_12_spr : 10, reserved42 : 22;
        };
        uint32_t tordump2_subindex2_spr;
    };
    union
    {
        struct
        {
            uint32_t reserved43 : 28, core_id2_2_0 : 3, reserved44 : 1;
        };
        uint32_t tordump0_subindex3_spr;
    };
    union
    {
        struct
        {
            uint32_t reserved45 : 31, address_22_22_spr : 1;
        };
        uint32_t tordump1_subindex3_spr;
    };
    union
    {
        struct
        {
            uint32_t address_32_23_spr : 10, reserved46 : 22;
        };
        uint32_t tordump2_subindex3_spr;
    };
    union
    {
        struct
        {
            uint32_t core_id2_6_3 : 4, reserved47 : 20, request_opCode2_6_0 : 7,
            reserved48 : 1;
        };
        uint32_t tordump0_subindex4_spr;
    };
    union
    {
        struct
        {
            uint32_t reserved49 : 31, address_33_33_spr : 1;
        };
        uint32_t tordump1_subindex4_spr;
    };
    union
    {
        struct
        {
            uint32_t address_34_34_spr : 1, reserved51 : 31;
        };
        uint32_t tordump2_subindex4_spr;
    };
    union
    {
        struct
        {
            uint32_t request_opCode2_10_7: 4, reserved52: 28;
        };
        uint32_t tordump0_subindex5_spr;
    };
    union
    {
        struct
        {
            uint32_t reserved53: 32;
        };
        uint32_t tordump1_subindex5_spr;
    };
    union
    {
        struct
        {
            uint32_t reserved54: 32;
        };
        uint32_t tordump2_subindex5_spr;
    };
    union
    {
        struct
        {
            uint32_t reserved55 : 3, lcs2 : 4, reserved56: 4, fsm2: 6, reserved57 : 15;
        };
        uint32_t tordump0_subindex7_spr;
    };
    union
    {
        struct
        {
            uint32_t reserved58: 32;
        };
        uint32_t tordump1_subindex7_spr;
    };
    union
    {
        struct
        {
            uint32_t reserved59 : 32;
        };
        uint32_t tordump2_subindex7_spr;
    };

    uint32_t cha;
    uint32_t idx;
};

using TORData =
    std::map<uint32_t, std::pair<SocketCtx, std::vector<TORDataGeneric>>>;
