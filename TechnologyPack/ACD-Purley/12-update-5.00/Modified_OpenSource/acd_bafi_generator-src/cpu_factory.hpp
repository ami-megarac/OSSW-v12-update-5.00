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
#include <string>
#include <utility>
#include <memory>

#include <cpu.hpp>
#include <tor_icx.hpp>
#include <tor_cpx.hpp>
#include <tor_skx.hpp>

class CpuFactory
{
    public:
    static std::shared_ptr<CpuGeneric> CreateInstance(std::string cpu_name)
    {
        CpuGeneric *instance = nullptr;

        if (cpu_name == "ICX")
            instance = new IcxCpu();
        if (cpu_name == "CPX")
            instance = new CpxCpu();
        if (cpu_name == "SKX")
            instance = new SkxCpu();

        if (instance != nullptr)
            return std::shared_ptr<CpuGeneric>(instance);
        else
            return nullptr;
    }
};
