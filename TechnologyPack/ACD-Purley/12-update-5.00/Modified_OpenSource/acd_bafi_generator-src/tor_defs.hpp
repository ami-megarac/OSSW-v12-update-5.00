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

union IerrLoggingReg
{
    struct
    {
        uint32_t firstIerrSrcIdCha : 8, firstIerrValid : 1,
            firstIerrSrcFromCbo : 1, reserved0 : 22;
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
    McerrLoggingReg mcerr;
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