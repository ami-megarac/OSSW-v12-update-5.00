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
#include <tor_defs_cpx.hpp>
#include <utils.hpp>
#include <tor.hpp>

using json = nlohmann::json;

class CpxCpu final : public Cpu
{
  public:
    // indexes for ierr and mcerr tsc information
    TscVariablesNames tscVariables = {
        "B01_D30_F4_0xF0",
        "B01_D30_F4_0xF4",
        "B01_D30_F4_0xF8",
        "B01_D30_F4_0xFC",
    };
    // indexes for IERR, MCERR and MCERR source
    static constexpr const char* ierr_varname = "B00_D08_F0_0xA4";
    static constexpr const char* mcerr_varname = "B00_D08_F0_0xA8";
    static constexpr const char* mca_err_src_varname = "B01_D30_F2_0xEC";
    // indexes and bitmasks of uncorectable and corectable AER errors to decode
    static constexpr const char* unc_err_index = "0x14C";
    static const uint32_t unc_spec_err_mask = 0x3FF030;
    static constexpr const char* cor_err_index = "0x158";
    static const uint32_t cor_spec_err_mask = 0x31C1;
    // decode only B*_D(0-3)_F* for CPX
    static constexpr const char* decode_only_index_1 = "_D00_";
    static constexpr const char* decode_only_index_2 = "_D01_";
    static constexpr const char* decode_only_index_3 = "_D02_";
    static constexpr const char* decode_only_index_4 = "_D03_";

    [[nodiscard]] std::map<uint32_t, TscData> getTscData(const json& input)
    {
        auto cpuSections = prepareJson(input);
        return getTscDataForProcessorType(cpuSections, tscVariables);
    }

    [[nodiscard]] UncAer analyzeUncAer(const json& input)
    {
        UncAer output;
        auto cpuSections = prepareJson(input);
        for (auto const& [cpu, cpuSection] : cpuSections)
        {
            uint32_t socketId;
            if (!str2uint(cpu.substr(3), socketId, decimal))
            {
                continue;
            }
            auto allUncErr = parseUncErrSts(cpuSection);
            output.insert({socketId, allUncErr});
        }
        return output;
    }

    [[nodiscard]] std::vector<UncAerData> parseUncErrSts(const json& input)
    {
        std::vector<UncAerData> allUncErrs;
        for (const auto [pciKey, pciVal] : input["uncore"].items())
        {
            uint32_t temp;
            if (checkInproperValue(pciVal) || pciVal == "0x0")
            {
                continue;
            }
            else if (pciKey.find(unc_err_index) != std::string::npos)
            {
                // decode only D00 - D03 indexes
                if (pciKey.find(decode_only_index_1) == std::string::npos &&
                    pciKey.find(decode_only_index_2) == std::string::npos &&
                    pciKey.find(decode_only_index_3) == std::string::npos &&
                    pciKey.find(decode_only_index_4) == std::string::npos)
                {
                    continue;
                }
                if (!str2uint(pciVal, temp))
                {
                    continue;
                }
                temp = temp & unc_spec_err_mask;
            }
            else
            {
                continue;
            }
            UncAerData uncErr;
            uncErr.error_status = temp;
            if (uncErr.error_status != 0)
            {
                uncErr.address = pciKey;
                allUncErrs.push_back(uncErr);
            }
        }
        return allUncErrs;
    }

    [[nodiscard]] CorAer analyzeCorAer(const json& input)
    {
        CorAer output;
        auto cpuSections = prepareJson(input);
        for (auto const& [cpu, cpuSection] : cpuSections)
        {
            uint32_t socketId;
            if (!str2uint(cpu.substr(3), socketId, decimal))
            {
                continue;
            }
            auto allCorErr = parseCorErrSts(cpuSection);
            output.insert({socketId, allCorErr});
        }
        return output;
    }

    [[nodiscard]] std::vector<CorAerData> parseCorErrSts(const json& input)
    {
        std::vector<CorAerData> allCorErrs;
        for (const auto [pciKey, pciVal] : input["uncore"].items())
        {
            uint32_t temp;
            if (checkInproperValue(pciVal) || pciVal == "0x0")
            {
                continue;
            }
            else if (pciKey.find(cor_err_index) != std::string::npos)
            {
                // decode only D00 - D03 indexes
                if (pciKey.find(decode_only_index_1) == std::string::npos &&
                    pciKey.find(decode_only_index_2) == std::string::npos &&
                    pciKey.find(decode_only_index_3) == std::string::npos &&
                    pciKey.find(decode_only_index_4) == std::string::npos)
                {
                    continue;
                }
                if (!str2uint(pciVal, temp))
                {
                    continue;
                }
                temp = temp & cor_spec_err_mask;
            }
            else
            {
                continue;
            }

            CorAerData corErr;
            corErr.error_status = temp;
            if (corErr.error_status != 0)
            {
                corErr.address = pciKey;
                allCorErrs.push_back(corErr);
            }
        }
        return allCorErrs;
    }

    [[nodiscard]] MCA analyzeMca(const json& input)
    {
        MCA output;
        auto cpuSections = prepareJson(input);
        std::vector<MCAData> allMcs;
        for (auto const& [cpu, cpuSection] : cpuSections)
        {
            uint32_t socketId;
            if (!str2uint(cpu.substr(3), socketId, decimal))
            {
                continue;
            }
            std::vector<MCAData> uncoreMcas = parseUncoreMcas(cpuSection);
            std::vector<MCAData> coreMcas = parseCoreMcas(cpuSection);
            std::vector<MCAData> allMcas;
            allMcas.reserve(uncoreMcas.size() + coreMcas.size());
            allMcas.insert(allMcas.begin(), uncoreMcas.begin(),
                           uncoreMcas.end());
            allMcas.insert(allMcas.begin(), coreMcas.begin(), coreMcas.end());
            output.insert({socketId, allMcas});
        }
        return output;
    }

    [[nodiscard]] std::optional<CpxTORData> parseTorData(const json& index)
    {
        if (index.find("subindex0") == index.cend())
        {
            return {};
        }
        if (checkInproperValue(index["subindex0"]))
        {
            return {};
        }
        CpxTORData tor;
        if (!str2uint(index["subindex0"], tor.subindex0) ||
            !str2uint(index["subindex1"], tor.subindex1) ||
            !str2uint(index["subindex2"], tor.subindex2))
        {
            return {};
        }
        if (!tor.valid)
        {
            return {};
        }
        return tor;
    }

    [[nodiscard]] std::vector<CpxTORData> getTorsData(const json& input)
    {
        std::vector<CpxTORData> torsData;
        if (input.find("TOR") == input.cend())
        {
            return torsData;
        }
        for (const auto& [chaItemKey, chaItemValue] : input["TOR"].items())
        {
            if (!startsWith(chaItemKey, "cha"))
            {
                continue;
            }
            for (const auto& [indexDataKey, indexDataValue] :
                    chaItemValue.items())
            {
                std::optional<CpxTORData> tor = parseTorData(indexDataValue);
                if (!tor)
                {
                    continue;
                }
                if (str2uint(chaItemKey.substr(3), tor->cha, decimal) &&
                    str2uint(indexDataKey.substr(5), tor->idx, decimal))
                {
                    torsData.push_back(*tor);
                }
            }
        }
        return torsData;
    }

    [[nodiscard]] CpxTOR analyze(const json& input)
    {
        CpxTOR output;
        auto cpuSections = prepareJson(input);
        for (auto const& [cpu, cpuSection] : cpuSections)
        {
            std::optional ierr = getUncoreData(cpuSection, ierr_varname);
            std::optional mcerr = getUncoreData(cpuSection, mcerr_varname);
            std::optional mcerrErrSrc =
                getUncoreData(cpuSection, mca_err_src_varname);
            std::vector<CpxTORData> tors = getTorsData(cpuSection);
            uint32_t socketId;
            if (!str2uint(cpu.substr(3), socketId, decimal))
            {
                continue;
            }
            SocketCtx ctx;
            if (!ierr || !str2uint(*ierr, ctx.ierr.value))
            {
                ctx.ierr.value = 0;
            }
            if (!mcerr || !str2uint(*mcerr, ctx.mcerr.value))
            {
                ctx.mcerr.value = 0;
            }
            if (!mcerrErrSrc || !str2uint(*mcerrErrSrc, ctx.mcerrErr.value))
            {
                ctx.mcerrErr.value = 0;
            }
            std::pair<SocketCtx, std::vector<CpxTORData>> tempData(ctx, tors);
            output.insert({socketId, tempData});
        }
        return output;
    }
};