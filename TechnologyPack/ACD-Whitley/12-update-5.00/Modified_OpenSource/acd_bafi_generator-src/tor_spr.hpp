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
#include <regex>

#include <aer.hpp>
#include <mca_defs.hpp>
#include <tor_defs_spr.hpp>
#include <utils.hpp>
#include <tor_eaglestream.hpp>
#include <cpu.hpp>

using json = nlohmann::json;

class SprCpu final : public EaglestreamCpu, public CpuGeneric
{
    public:
    // indexes for ierr and mcerr tsc information
    TscVariablesNames tscVariables = {
        "B31_D30_F4_0xf0",
        "B31_D30_F4_0xf4",
        "B31_D30_F4_0xf8",
        "B31_D30_F4_0xfc",
    };
    // indexes for IERR, MCERR and MCERR source
    static constexpr const char* ierr_varname = "B30_D00_F0_0xa4";
    static constexpr const char* mcerr_varname = "B30_D00_F0_0xa8";
    static constexpr const char* mca_err_src_varname = "B31_D30_F2_0xec";
    // bigcore MCAs indexes
    static constexpr const char* bigcore_mc0 = "ifu_cr_mc0";
    static constexpr const char* bigcore_mc1 = "dcu_cr_mc1";
    static constexpr const char* bigcore_mc2 = "dtlb_cr_mc2";
    static constexpr const char* bigcore_mc3 = "ml2_cr_mc3";
    // indexes and masks of uncorrectable and correctable AER errors to decode
    static constexpr const char* unc_err_index1 = "B08_D03_F0_0x104";
    static constexpr const char* unc_err_index2 = "B08_D03_F0_0x170";
    static const uint32_t unc_err_mask = 0x7FFF030;
    static constexpr const char* cor_err_index1 = "B08_D03_F0_0x110";
    static constexpr const char* cor_err_index2 = "B08_D03_F0_0x174";
    static const uint32_t cor_err_mask = 0xF1C1;
    // index and bit mask of uncorrectable AER that requires different decoding
    // rules
    static constexpr const char* unc_spec_err_index = "B00_D03_F0_0x14c";
    static const uint32_t unc_spec_err_mask = 0x47FF030;
    // index and bit mask of correctable AER that requires different decoding
    // rule
    static constexpr const char* cor_spec_err_index = "B00_D03_F0_0x158";
    static const uint32_t cor_spec_err_mask = 0x31C1;

    std::string bigcore_mcas[4] = {bigcore_mc0, bigcore_mc1, bigcore_mc2,
                                   bigcore_mc3};

    [[nodiscard]] virtual std::map<std::string, std::array<uint64_t, 2>>
    getMemoryMap(const json& input)
    {
        return EaglestreamCpu::getMemoryMap(input);
    }

    [[nodiscard]] virtual std::map<uint32_t, TscData> getTscData(const json& input)
    {
        auto cpuSections = prepareJson(input);
        return getTscDataForProcessorType(cpuSections, tscVariables);
    }

    [[nodiscard]] virtual std::optional<std::map<uint32_t, PackageThermStatus>>
        getThermData(const json& input)
    {
        return {};
    }

    [[nodiscard]] virtual UncAer analyzeUncAer(const json& input)
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
        std::regex unc_decode_index("B00_D0._F0_0x104");

        std::vector<UncAerData> allUncErrs;
        if (input.find("uncore") == input.cend())
        {
            return allUncErrs;
        }
        for (const auto [pciKey, pciVal] : input["uncore"].items())
        {
            uint32_t temp;
            if (checkInproperValue(pciVal) || pciVal == "0x0")
            {
                continue;
            }
            else if (pciKey == unc_spec_err_index)
            {
                if (!str2uint(pciVal, temp))
                {
                    continue;
                }
                // apply a mask to fit decoding rules for this register
                temp = temp & unc_spec_err_mask;
            }
            else if (pciKey == unc_err_index1 || pciKey == unc_err_index2 ||
            std::regex_search(std::string(pciKey), unc_decode_index))
            {
                if (!str2uint(pciVal, temp))
                {
                    continue;
                }
                // apply a mask to fit decoding rules for this registers
                temp = temp & unc_err_mask;
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

    [[nodiscard]] virtual CorAer analyzeCorAer(const json& input)
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
        std::regex cor_decode_index("B00_D0._F0_0x110");

        std::vector<CorAerData> allCorErrs;
        if (input.find("uncore") == input.cend())
        {
            return allCorErrs;
        }
        for (const auto [pciKey, pciVal] : input["uncore"].items())
        {
            uint32_t temp;
            if (checkInproperValue(pciVal) || pciVal == "0x0")
            {
                continue;
            }
            else if (pciKey == cor_spec_err_index)
            {
                if (!str2uint(pciVal, temp))
                {
                    continue;
                }
                // apply a mask to fit decoding rules
                temp = temp & cor_spec_err_mask;
            }
            else if (pciKey == cor_err_index1 || pciKey == cor_err_index1 ||
            std::regex_search(std::string(pciKey), cor_decode_index))
            {
                if (!str2uint(pciVal, temp))
                {
                    continue;
                }
                temp = temp & cor_err_mask;
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

    [[nodiscard]] virtual MCA analyzeMca(const json& input)
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
            std::vector<MCAData> bigCoreMcas =
                parseAllBigcoreMcas(cpuSection, bigcore_mcas);
            std::vector<MCAData> uncoreMcas = parseUncoreMcas(cpuSection);
            std::vector<MCAData> coreMcas = parseCoreMcas(cpuSection);
            std::vector<MCAData> allMcas;
            allMcas.reserve(bigCoreMcas.size() + uncoreMcas.size() +
                            coreMcas.size());
            allMcas.insert(allMcas.begin(), bigCoreMcas.begin(),
                           bigCoreMcas.end());
            allMcas.insert(allMcas.begin(), uncoreMcas.begin(),
                           uncoreMcas.end());
            allMcas.insert(allMcas.begin(), coreMcas.begin(), coreMcas.end());
            output.insert({socketId, allMcas});
        }
        return output;
    }

    [[nodiscard]] std::tuple<uint32_t, uint32_t, uint32_t> divideTordumps(
        const json& inputData)
    {
        uint32_t tordumpParsed0, tordumpParsed1, tordumpParsed2;
        std::string input = inputData;
        while (input.length() < 26)
        {
            input.insert(2, "0");
        }
        std::string tordump0 = input.substr(input.length() - 8, 8);
        std::string tordump1 = input.substr(input.length() - 16, 8);
        std::string tordump2 = input.substr(input.length() - 24, 8);
        if (!str2uint(tordump0, tordumpParsed0) ||
            !str2uint(tordump1, tordumpParsed1) ||
            !str2uint(tordump2, tordumpParsed2))
        {
            return std::make_tuple(0, 0, 0);
        }
        return std::make_tuple(tordumpParsed0, tordumpParsed1, tordumpParsed2);
    }

    [[nodiscard]] std::optional<TORDataGeneric> parseTorData(const json& index)
    {
        if (index.find("subindex0") == index.cend())
        {
            return {};
        }
        if (checkInproperValue(index["subindex0"]))
        {
            return {};
        }
        TORDataGeneric tor;
        std::tie(tor.tordump0_subindex0_spr, tor.tordump1_subindex0_spr,
            tor.tordump2_subindex0_spr) = divideTordumps(index["subindex0"]);
        if (!tor.valid2)
        {
            return {};
        }
        std::tie(tor.tordump0_subindex1_spr, tor.tordump1_subindex1_spr,
                    tor.tordump2_subindex1_spr) =
            divideTordumps(index["subindex1"]);
        std::tie(tor.tordump0_subindex2_spr, tor.tordump1_subindex2_spr,
                    tor.tordump2_subindex2_spr) =
            divideTordumps(index["subindex2"]);
        std::tie(tor.tordump0_subindex3_spr, tor.tordump1_subindex3_spr,
                    tor.tordump2_subindex3_spr) =
            divideTordumps(index["subindex3"]);
        std::tie(tor.tordump0_subindex4_spr, tor.tordump1_subindex4_spr,
                    tor.tordump2_subindex4_spr) =
            divideTordumps(index["subindex4"]);
         std::tie(tor.tordump0_subindex5_spr, tor.tordump1_subindex5_spr,
                    tor.tordump2_subindex5_spr) =
            divideTordumps(index["subindex5"]);
        std::tie(tor.tordump0_subindex7_spr, tor.tordump1_subindex7_spr,
                    tor.tordump2_subindex7_spr) =
            divideTordumps(index["subindex7"]);
        return tor;
    }

    [[nodiscard]] std::vector<TORDataGeneric> getTorsData(const json& input)
    {
        std::vector<TORDataGeneric> torsData;
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
                std::optional<TORDataGeneric> tor = parseTorData(indexDataValue);
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

    [[nodiscard]] virtual TORData analyze(const json& input)
    {
        TORData output;
        auto cpuSections = prepareJson(input);
        for (auto const& [cpu, cpuSection] : cpuSections)
        {
            std::optional ierr = getUncoreData(cpuSection, ierr_varname);
            std::optional mcerr = getUncoreData(cpuSection, mcerr_varname);
            std::optional mcerrErrSrc =
                getUncoreData(cpuSection, mca_err_src_varname);
            std::vector<TORDataGeneric> tors = getTorsData(cpuSection);
            uint32_t socketId;
            if (!str2uint(cpu.substr(3), socketId, decimal))
            {
                continue;
            }
            SocketCtx ctx;
            if (!ierr || !str2uint(*ierr, ctx.ierrSpr.value))
            {
                ctx.ierr.value = 0;
            }
            if (!mcerr || !str2uint(*mcerr, ctx.mcerrSpr.value))
            {
                ctx.mcerrSpr.value = 0;
            }
            if (!mcerrErrSrc || !str2uint(*mcerrErrSrc, ctx.mcerrErr.value))
            {
                ctx.mcerrErr.value = 0;
            }
            std::pair<SocketCtx, std::vector<TORDataGeneric>> tempData(ctx, tors);
            output.insert({socketId, tempData});
        }
        return output;
    }
};