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
#include <algorithm>
#include <array>
#include <functional>
#include <map>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include <aer.hpp>
#include <mca_defs.hpp>
#include <tor_defs_skx.hpp>
#include <utils.hpp>

using json = nlohmann::json;

[[nodiscard]] std::optional<std::reference_wrapper<const json>>
    getSocketSection(std::string socket, const json& input)
{
    const auto& socketSection = input.find(socket);
    if (socketSection != input.cend())
    {
        return *socketSection;
    }
    return {};
}

[[nodiscard]] std::map<std::string, std::reference_wrapper<const json>>
    getAllSocketSections(const json& dumpSection,
    std::vector<std::string> sockets)
{
    std::map<std::string, std::reference_wrapper<const json>> allSocketsSections;
    for (const auto& socket : sockets)
    {
        auto cpuSection = getSocketSection(socket, dumpSection);
        if (!dumpSection)
        {
            continue;
        }
        allSocketsSections.insert(
            std::pair<std::string, std::reference_wrapper<const json>>(
            socket, dumpSection));
    }
    return allSocketsSections;
}

[[nodiscard]] std::vector<std::string> findSocketsPurley(const json& input)
{
    std::vector<std::string> cpus;
    const auto& metaData = input.find("sys_info");
    if (metaData != input.cend())
    {
        for (const auto& metaDataItem : metaData.value().items())
        {
            if (startsWith(metaDataItem.key(), "socket"))
            {
                cpus.push_back(metaDataItem.key());
            }
        }
    }
    return cpus;
}

class PurleyCpu
{
    public:
    [[nodiscard]] std::optional<std::string>
        getUncoreData(const json& input, std::string socket, std::string varName)
    {
        if (input.contains("uncore_status_regs"))
        {
            if (input["uncore_status_regs"].contains(socket))
            {
                if (input["uncore_status_regs"][socket].find(varName) !=
                 input["uncore_status_regs"][socket].cend())
                {
                    if (input["uncore_status_regs"][socket][varName]
                    .is_string() && !checkInproperValue(
                    input["uncore_status_regs"][socket][varName]))
                    {
                        return input["uncore_status_regs"][socket][varName]
                        .get<std::string>();
                    }
                }
            }
        }
        if (input.contains("crashdump"))
        {
            if (input["crashdump"].contains("uncore_regs"))
            {
                if (input["crashdump"][socket]["uncore_regs"].find(varName) !=
                 input["crashdump"][socket]["uncore_regs"].cend())
                 {
                    if (input["crashdump"][socket]["uncore_regs"][varName]
                    .is_string() && !checkInproperValue(
                    input["crashdump"][socket]["uncore_regs"][varName]))
                    {
                        return input["crashdump"][socket]["uncore_regs"][varName]
                        .get<std::string>();
                    }
                 }
            }
        }
        return {};
    }

    [[nodiscard]] std::optional<MCAData> parseMca(const json& mcSection,
        const json& mcData, uint32_t coreId)
    {
        if (mcData.is_string())
        {
            return {};
        }
        MCAData mc = {0, 0, 0, 0, 0, 0, 0, false};
        mc.core = coreId;
        std::string ctl = "CTL";
        std::string status = "STATUS";
        std::string addr = "ADDR";
        std::string misc = "MISC";
        if (mcData.find(status) == mcData.cend())
        {
            return {};
        }
        if (checkInproperValue(mcData[status]))
        {
            return {};
        }
        if (!str2uint(mcData[status], mc.mc_status))
        {
            return {};
        }
        if (!mc.valid)
        {
            return {};
        }
        if (startsWith(mcSection, "MC"))
        {
            mc.cbo = false;
            if (!str2uint(std::string(mcSection).substr(2), mc.bank, 10))
            {
                return {};
            }
        }
        else
        {
            return {};
        }
        if (mcData.find(ctl) == mcData.cend() || checkInproperValue(mcData[ctl]))
        {
            mc.ctl = 0;
        }
        else if (!str2uint(mcData[ctl], mc.ctl))
        {
            return {};
        }
        if (mcData.find(addr) == mcData.cend() || checkInproperValue(mcData[addr]))
        {
            mc.address = 0;
        }
        else if (!str2uint(mcData[addr], mc.address))
        {
            return {};
        }

        if (mcData.find(misc) == mcData.cend() || checkInproperValue(mcData[misc]))
        {
            mc.misc = 0;
        }
        else if (!str2uint(mcData[misc], mc.misc))
        {
            return {};
        }
        return mc;
    }

    [[nodiscard]] std::vector<MCAData>
        parseCoreMcas(const json& input, std::string socket)
    {
        std::vector<MCAData> allCoreMcas;
        if (input["core_MCA"].find(socket) == input["core_MCA"].cend())
        {
            return allCoreMcas;
        }
        for (auto const& [core, coreData] : input["core_MCA"][socket].items())
        {
            if (!startsWith(core, "core"))
            {
                continue;
            }
            uint32_t coreId;
            if (!str2uint(core.substr(4), coreId, 10))
            {
                continue;
            }
            for (auto const& [mcSection, mcData] : coreData.items())
            {
                auto coreMca =
                    parseMca(mcSection, mcData, coreId);
                if (coreMca)
                {
                    allCoreMcas.push_back(*coreMca);
                }
            }
        }
        return allCoreMcas;
    }

    [[nodiscard]] std::vector<MCAData> parseUncoreMcas(
        const json& input, std::string socket)
    {
        std::vector<MCAData> output;
        if (input["uncore_MCA"].find(socket) == input["uncore_MCA"].cend())
        {
            return output;
        }
        for (auto const& [mcSection, mcData] :
        input["uncore_MCA"][socket].items())
        {
            auto uncoreMc = parseMca(mcSection, mcData, 0);
            if (uncoreMc)
            {
                output.push_back(*uncoreMc);
            }
        }
        return output;
    }
};