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
#include <tor_defs_spr.hpp>
#include <utils.hpp>

using json = nlohmann::json;

class EaglestreamCpu
{
    public:
    [[nodiscard]] std::map<std::string, std::array<uint64_t, 2>>
    getMemoryMap(const json& input)
    {
        // map is the same for all CPUs, so if once created other CPUs can be skipped
        bool mapCreated = false;
        std::map<std::string, std::array<uint64_t, 2>> memoryMap;
        auto cpuSections = prepareJson(input);
        for (auto const& [cpu, cpuSection] : cpuSections)
        {
            if (mapCreated)
            {
                break;
            }
            if  (cpuSection.get().find("address_map") == cpuSection.get().cend())
            {
                continue;
            }
            for (auto const& [name, value] : cpuSection.get()["address_map"].items())
            {
                if (name == "_record_enable" && value == false)
                {
                   break;
                }
                if (name == "_version")
                {
                    continue;
                }
                if (checkInproperValue(value))
                {
                    continue;
                }
                uint64_t uintValue;
                if (!str2uint(value, uintValue))
                {
                    continue;
                }
                if (name.find("_BASE") != std::string::npos)
                {
                    auto shortName = name.substr(0, name.find("_BASE"));
                    memoryMap[shortName][0] = uintValue;
                }
                else if (name.find("_LIMIT") != std::string::npos)
                {
                    auto shortName = name.substr(0, name.find("_LIMIT"));
                    memoryMap[shortName][1] = uintValue;
                }
            }
            mapCreated = true;
        }
        return memoryMap;
    }

    [[nodiscard]] std::optional<std::string>
        getUncoreData(const json& input, std::string varName)
    {
        if (input.find("uncore") == input.cend())
        {
            return {};
        }
        if (input["uncore"].find(varName) == input["uncore"].cend())
        {
            return {};
        }
        if (!input["uncore"][varName].is_string())
        {
            return {};
        }
        if (checkInproperValue(input["uncore"][varName]))
        {
            return {};
        }
        return input["uncore"][varName].get<std::string>();
    }

    [[nodiscard]] std::optional<MCAData> parseBigCoreMca(
        const json& input, std::string bigCoreStr)
    {
        std::string bigcore_status =
            std::string(bigCoreStr) + std::string("_status");
        std::string bigcore_ctl = std::string(bigCoreStr) + std::string("_ctl");
        std::string bigcore_addr =
            std::string(bigCoreStr) + std::string("_addr");
        std::string bigcore_misc =
            std::string(bigCoreStr) + std::string("_misc");
        MCAData mc = {0, 0, 0, 0, 0, 0, 0, false};

        if (!input.contains(bigcore_status) || !input.contains(bigcore_ctl)
            || !input.contains(bigcore_addr) || !input.contains(bigcore_misc))
        {
            return {};
        }
        if (checkInproperValue(input[bigcore_status]))
        {
            return {};
        }
        if (!str2uint(input[bigcore_status], mc.mc_status))
        {
            return {};
        }
        if (!mc.valid)
        {
            return {};
        }
        if (checkInproperValue(input[bigcore_ctl]))
        {
            mc.ctl = 0;
        }
        else if (!str2uint(input[bigcore_ctl], mc.ctl))
        {
            return {};
        }
        if (checkInproperValue(input[bigcore_addr]))
        {
            mc.address = 0;
        }
        else if (!str2uint(input[bigcore_addr], mc.address))
        {
            return {};
        }
        if (checkInproperValue(input[bigcore_misc]))
        {
            mc.misc = 0;
        }
        else if (!str2uint(input[bigcore_misc], mc.misc))
        {
            return {};
        }
        return mc;
    }

    [[nodiscard]] std::vector<MCAData>
        parseBigCoreMcas(std::reference_wrapper<const json> input,
            uint32_t coreId, uint32_t threadId, std::string bigcore_mcas[4])
    {
        std::vector<MCAData> output;
        auto mc0 = parseBigCoreMca(input, bigcore_mcas[0]);
        auto mc1 = parseBigCoreMca(input, bigcore_mcas[1]);
        auto mc2 = parseBigCoreMca(input, bigcore_mcas[2]);
        auto mc3 = parseBigCoreMca(input, bigcore_mcas[3]);
        if (mc0)
        {
            mc0->core = coreId;
            mc0->thread = threadId;
            mc0->bank = 0;
            mc0->cbo = false;
            output.push_back(*mc0);
        }
        if (mc1)
        {
            mc1->core = coreId;
            mc1->thread = threadId;
            mc1->bank = 1;
            mc1->cbo = false;
            output.push_back(*mc1);
        }
        if (mc2)
        {
            mc2->core = coreId;
            mc2->thread = threadId;
            mc2->bank = 2;
            mc2->cbo = false;
            output.push_back(*mc2);
        }
        if (mc3)
        {
            mc3->core = coreId;
            mc3->thread = threadId;
            mc3->bank = 3;
            mc3->cbo = false;
            output.push_back(*mc3);
        }
        return output;
    }

    [[nodiscard]] std::optional<MCAData> parseMca(const json& mcSection,
        const json& mcData, uint32_t coreId, uint32_t threadId)
    {
        if (mcData.is_string())
        {
            return {};
        }
        MCAData mc = {0, 0, 0, 0, 0, 0, 0, false};
        mc.core = coreId;
        mc.thread = threadId;
        std::string mcLower = mcSection;
        std::transform(mcLower.begin(), mcLower.end(), mcLower.begin(),
                       ::tolower);
        std::string status = mcLower + "_status";
        std::string ctl = mcLower + "_ctl";
        std::string addr = mcLower + "_addr";
        std::string misc = mcLower + "_misc";
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
        if (startsWith(mcLower, "mc"))
        {
            mc.cbo = false;
            uint32_t bank;
            if (!str2uint(mcLower.substr(2), bank, 10))
            {
                return {};
            }
            if (bank == 12 || bank == 16 || bank == 20 || bank == 24 || bank == 5
            || bank == 7 || bank == 8)
            {
                return {};
            }
            mc.bank = bank;
        }
        else if (startsWith(mcLower, "cbo"))
        {
            mc.cbo = true;
            if (!str2uint(mcLower.substr(3), mc.bank, 10))
            {
                return {};
            }
        }
        else if (startsWith(mcLower, "m2mem"))
        {
            mc.cbo = false;
            uint32_t bank;
            if (!str2uint(mcLower.substr(5), bank, 10))
            {
                return {};
            }
            if (bank == 0)
                mc.bank = 12;
            else if (bank == 1)
                mc.bank = 16;
            else if (bank == 2)
                mc.bank = 20;
            else if (bank == 3)
                mc.bank = 24;
        }
        else if (startsWith(mcLower, "upi"))
        {
            mc.cbo = false;
            uint32_t bank;
            if (!str2uint(mcLower.substr(3), bank, 10))
            {
                return {};
            }
            if (bank == 0)
                mc.bank = 5;
            else if (bank == 1)
                mc.bank = 7;
            else if (bank == 2)
                mc.bank = 8;
        }
        else if (startsWith(mcLower, "mdf"))
        {
            mc.cbo = false;
            if (!str2uint(mcLower.substr(3), mc.bank, 10))
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
        parseCoreMcas(std::reference_wrapper<const json> input)
    {
        std::vector<MCAData> allCoreMcas;
        if (input.get().find("MCA") == input.get().cend())
        {
            return allCoreMcas;
        }
        for (auto const& [core, threads] : input.get()["MCA"].items())
        {
            if (!startsWith(core, "core"))
            {
                continue;
            }
            uint32_t coreId;
            if (!str2uint(core.substr(4), coreId, decimal))
            {
                continue;
            }
            for (auto const& [thread, threadData] : threads.items())
            {
                if (!startsWith(thread, "thread"))
                {
                    continue;
                }
                uint32_t threadId;
                if (!str2uint(thread.substr(6), threadId, decimal))
                {
                    continue;
                }
                for (auto const& [mcSection, mcData] : threadData.items())
                {
                    auto coreMca =
                        parseMca(mcSection, mcData, coreId, threadId);
                    if (coreMca)
                    {
                        allCoreMcas.push_back(*coreMca);
                    }
                }
            }
        }
        return allCoreMcas;
    }

    [[nodiscard]] std::vector<MCAData> parseAllBigcoreMcas(
        std::reference_wrapper<const json> input, std::string bigcoreMcas[4])
    {
        std::vector<MCAData> allBigcoreMca;
        if (input.get().find("big_core") == input.get().cend())
        {
            return allBigcoreMca;
        }
        for (auto const& [core, threads] : input.get()["big_core"].items())
        {
            if (!startsWith(core, "core"))
            {
                continue;
            }
            uint32_t coreId;
            if (!str2uint(core.substr(4), coreId, decimal))
            {
                continue;
            }
            for (auto const& [thread, threadData] : threads.items())
            {
                if (!startsWith(thread, "thread") || threadData.is_string())
                {
                    continue;
                }
                uint32_t threadId;
                if (!str2uint(thread.substr(6), threadId, decimal))
                {
                    continue;
                }
                std::vector<MCAData> bigCoreMC =
                    parseBigCoreMcas(threadData, coreId, threadId, bigcoreMcas);
                for (auto const& mcData : bigCoreMC)
                {
                    allBigcoreMca.push_back(mcData);
                }
            }
        }
        return allBigcoreMca;
    }

    [[nodiscard]] std::vector<MCAData> parseUncoreMcas(
        std::reference_wrapper<const json> input)
    {
        std::vector<MCAData> output;
        if (input.get().find("MCA") == input.get().cend())
        {
            return output;
        }
        if (input.get()["MCA"].find("uncore") == input.get()["MCA"].cend())
        {
            return output;
        }
        for (auto const& [mcSection, mcData] :
                input.get()["MCA"]["uncore"].items())
        {
            auto uncoreMc = parseMca(mcSection, mcData, 0, 0);
            if (uncoreMc)
            {
                output.push_back(*uncoreMc);
            }
        }
        return output;
    }

    [[nodiscard]] std::map<uint32_t, TscData> getTscDataForProcessorType(
        const std::map<std::string, std::reference_wrapper<const json>>
        cpuSections, TscVariablesNames tscVariablesNamesForProcessor)
    {
        std::map<uint32_t, TscData> output;
        std::pair<uint32_t, uint64_t> firstIerrTimestampSocket;
        std::pair<uint32_t, uint64_t> firstMcerrTimestampSocket;
        for (auto const& [cpu, cpuSection] : cpuSections)
        {
            uint32_t socketId;
            TscData tsc;
            if (!str2uint(cpu.substr(3), socketId, decimal))
            {
                continue;
            }
            std::optional pcu_first_ierr_tsc_lo_cfg = getUncoreData(cpuSection,
                tscVariablesNamesForProcessor.pcu_first_ierr_tsc_lo_cfg_varname);
            if (!pcu_first_ierr_tsc_lo_cfg)
            {
                tsc.pcu_first_ierr_tsc_lo_cfg = "0x0";
            }
            else
            {
                tsc.pcu_first_ierr_tsc_lo_cfg = *pcu_first_ierr_tsc_lo_cfg;
            }
            std::optional pcu_first_ierr_tsc_hi_cfg = getUncoreData(cpuSection,
                tscVariablesNamesForProcessor.pcu_first_ierr_tsc_hi_cfg_varname);
            if (!pcu_first_ierr_tsc_hi_cfg)
            {
                tsc.pcu_first_ierr_tsc_hi_cfg = "0x0";
            }
            else
            {
                tsc.pcu_first_ierr_tsc_hi_cfg = *pcu_first_ierr_tsc_hi_cfg;
            }
            std::optional pcu_first_mcerr_tsc_lo_cfg = getUncoreData(cpuSection,
                tscVariablesNamesForProcessor.pcu_first_mcerr_tsc_lo_cfg_varname);
            if (!pcu_first_mcerr_tsc_lo_cfg)
            {
                tsc.pcu_first_mcerr_tsc_lo_cfg = "0x0";
            }
            else
            {
                tsc.pcu_first_mcerr_tsc_lo_cfg = *pcu_first_mcerr_tsc_lo_cfg;
            }
            std::optional pcu_first_mcerr_tsc_hi_cfg = getUncoreData(cpuSection,
                tscVariablesNamesForProcessor.pcu_first_mcerr_tsc_hi_cfg_varname);
            if (!pcu_first_mcerr_tsc_hi_cfg)
            {
                tsc.pcu_first_mcerr_tsc_hi_cfg = "0x0";
            }
            else
            {
                tsc.pcu_first_mcerr_tsc_hi_cfg = *pcu_first_mcerr_tsc_hi_cfg;
            }
            if (countTscCfg(tsc))
            {
                locateFirstErrorsOnSockets(socketId, firstIerrTimestampSocket,
                    firstMcerrTimestampSocket, tsc);
                output.insert({socketId, tsc});
            }
        }
        if (firstIerrTimestampSocket.second != 0)
        {
            output.at(firstIerrTimestampSocket.first).ierr_occured_first = true;
        }
        if (firstMcerrTimestampSocket.second != 0)
        {
            output.at(firstMcerrTimestampSocket.first).mcerr_occured_first = true;
        }
        return output;
    }

    [[nodiscard]] bool countTscCfg(TscData& socketTscData)
    {
        uint32_t pcu_first_ierr_tsc_lo_cfg;
        bool status1 = str2uint(socketTscData.pcu_first_ierr_tsc_lo_cfg,
                               pcu_first_ierr_tsc_lo_cfg);
        uint32_t pcu_first_ierr_tsc_hi_cfg;
        bool status2 = str2uint(socketTscData.pcu_first_ierr_tsc_hi_cfg,
                                pcu_first_ierr_tsc_hi_cfg);
        uint32_t pcu_first_mcerr_tsc_lo_cfg;
        bool status3 = str2uint(socketTscData.pcu_first_mcerr_tsc_lo_cfg,
                                pcu_first_mcerr_tsc_lo_cfg);
        uint32_t pcu_first_mcerr_tsc_hi_cfg;
        bool status4 = str2uint(socketTscData.pcu_first_mcerr_tsc_hi_cfg,
                                pcu_first_mcerr_tsc_hi_cfg);
        if (!status1 || !status2 || !status3 || !status4)
        {
            return false;
        }
        socketTscData.pcu_first_ierr_tsc_cfg =
            static_cast<uint64_t>(pcu_first_ierr_tsc_lo_cfg) |
            static_cast<uint64_t>(pcu_first_ierr_tsc_hi_cfg) << 32;
        socketTscData.pcu_first_mcerr_tsc_cfg =
            static_cast<uint64_t>(pcu_first_mcerr_tsc_lo_cfg) |
            static_cast<uint64_t>(pcu_first_mcerr_tsc_hi_cfg) << 32;

        if (socketTscData.pcu_first_ierr_tsc_cfg != 0)
        {
            socketTscData.ierr_on_socket = true;
        }
        if (socketTscData.pcu_first_mcerr_tsc_cfg != 0)
        {
            socketTscData.mcerr_on_socket = true;
        }
        return true;
    }

    void locateFirstErrorsOnSockets(uint32_t socketId,
         std::pair<uint32_t, uint64_t>& firstIerrTimestampSocket,
         std::pair<uint32_t, uint64_t>& firstMcerrTimestampSocket, TscData tsc)
    {
        if (tsc.ierr_on_socket && firstIerrTimestampSocket.second == 0)
        {
            firstIerrTimestampSocket =
                std::make_pair(socketId, tsc.pcu_first_ierr_tsc_cfg);
        }
        else if (tsc.ierr_on_socket &&
                 firstIerrTimestampSocket.second > tsc.pcu_first_ierr_tsc_cfg)
        {
            firstIerrTimestampSocket =
                std::make_pair(socketId,tsc.pcu_first_ierr_tsc_cfg);
        }
        if (tsc.mcerr_on_socket && firstMcerrTimestampSocket.second == 0)
        {
            firstMcerrTimestampSocket =
                std::make_pair(socketId, tsc.pcu_first_mcerr_tsc_cfg);
        }
        else if (tsc.mcerr_on_socket &&
                 firstMcerrTimestampSocket.second > tsc.pcu_first_mcerr_tsc_cfg)
        {
            firstMcerrTimestampSocket =
                std::make_pair(socketId, tsc.pcu_first_mcerr_tsc_cfg);
        }
    }
};