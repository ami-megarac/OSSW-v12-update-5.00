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
#include <tor_defs_icx.hpp>
#include <utils.hpp>
#include <tor.hpp>

using json = nlohmann::json;

class IcxCpu final : public Cpu
{
  public:
    // indexes for ierr and mcerr tsc information
    TscVariablesNames tscVariables = {
        "B31_D30_F4_0xF0",
        "B31_D30_F4_0xF4",
        "B31_D30_F4_0xF8",
        "B31_D30_F4_0xFC",
    };
    // index for PACKAGE_THERM_STATUS
    static constexpr const char* package_therm_status_varname = "RDIAMSR_0x1B1";
    // indexes for IERR, MCERR and MCERR source
    static constexpr const char* ierr_varname = "B30_D00_F0_0xA4";
    static constexpr const char* mcerr_varname = "B30_D00_F0_0xA8";
    static constexpr const char* mca_err_src_varname = "B31_D30_F2_0xEC";
    // bigcore MCAs indexes
    static constexpr const char* bigcore_mc0 = "ifu_cr_mc0";
    static constexpr const char* bigcore_mc1 = "dcu_cr_mc1";
    static constexpr const char* bigcore_mc2 = "dtlb_cr_mc2";
    static constexpr const char* bigcore_mc3 = "ml2_cr_mc3";
    // indexes and masks of uncorrectable and correctable AER errors to decode
    static constexpr const char* unc_err_index = "0x104";
    static const uint32_t unc_err_mask = 0x7FFF030;
    static constexpr const char* cor_err_index = "0x110";
    static const uint32_t cor_err_mask = 0xF1C1;
    // index and bit mask of uncorrectable AER that requires different decoding
    // rules
    static constexpr const char* unc_spec_err_index = "B00_D03_F0_0x14C";
    static const uint32_t unc_spec_err_mask = 0x47FF030;
    // index and bit mask of correctable AER that requires different decoding
    // rule
    static constexpr const char* cor_spec_err_index = "B00_D03_F0_0x158";
    static const uint32_t cor_spec_err_mask = 0x31C1;
    // index that should be excluded from decoding
    static constexpr const char* exclude_index_1 = "B30_";
    static constexpr const char* exclude_index_2 = "B31_";

    std::string bigcore_mcas[4] = {bigcore_mc0, bigcore_mc1, bigcore_mc2,
                                   bigcore_mc3};

    [[nodiscard]] std::map<uint32_t, TscData> getTscData(const json& input)
    {
        auto cpuSections = prepareJson(input);
        return getTscDataForProcessorType(cpuSections, tscVariables);
    }

    [[nodiscard]] std::optional<std::map<uint32_t, PackageThermStatus>>
        getThermData(const json& input)
    {
        auto cpuSections = prepareJson(input);
        return getThermDataForProcessorType(cpuSections,
            package_therm_status_varname);
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
            // do not decode excluded registers
            else if (startsWith(pciKey, exclude_index_1) ||
                     startsWith(pciKey, exclude_index_2))
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
            else if (pciKey.find(unc_err_index) != std::string::npos)
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
            // do not decode excluded registers
            else if (startsWith(pciKey, exclude_index_1) ||
                     startsWith(pciKey, exclude_index_2))
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
            else if (pciKey.find(cor_err_index) != std::string::npos)
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

    [[nodiscard]] std::optional<IcxTORData> parseTorData(const json& index)
    {
        if (index.find("subindex0") == index.cend())
        {
            return {};
        }
        if (checkInproperValue(index["subindex0"]))
        {
            return {};
        }
        IcxTORData tor;
        std::tie(tor.tordump0_subindex0, tor.tordump1_subindex0,
            tor.tordump2_subindex0) = divideTordumps(index["subindex0"]);
        if (!tor.valid)
        {
            return {};
        }
        std::tie(tor.tordump0_subindex1, tor.tordump1_subindex1,
                    tor.tordump2_subindex1) =
            divideTordumps(index["subindex1"]);
        std::tie(tor.tordump0_subindex2, tor.tordump1_subindex2,
                    tor.tordump2_subindex2) =
            divideTordumps(index["subindex2"]);
        std::tie(tor.tordump0_subindex3, tor.tordump1_subindex3,
                    tor.tordump2_subindex3) =
            divideTordumps(index["subindex3"]);
        std::tie(tor.tordump0_subindex4, tor.tordump1_subindex4,
                    tor.tordump2_subindex4) =
            divideTordumps(index["subindex4"]);
        std::tie(tor.tordump0_subindex7, tor.tordump1_subindex7,
                    tor.tordump2_subindex7) =
            divideTordumps(index["subindex7"]);
        return tor;
    }

    [[nodiscard]] std::vector<IcxTORData> getTorsData(const json& input)
    {
        std::vector<IcxTORData> torsData;
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
                std::optional<IcxTORData> tor = parseTorData(indexDataValue);
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

    [[nodiscard]] IcxTOR analyze(const json& input)
    {
        IcxTOR output;
        auto cpuSections = prepareJson(input);
        for (auto const& [cpu, cpuSection] : cpuSections)
        {
            std::optional ierr = getUncoreData(cpuSection, ierr_varname);
            std::optional mcerr = getUncoreData(cpuSection, mcerr_varname);
            std::optional mcerrErrSrc =
                getUncoreData(cpuSection, mca_err_src_varname);
            std::vector<IcxTORData> tors = getTorsData(cpuSection);
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
            std::pair<SocketCtx, std::vector<IcxTORData>> tempData(ctx, tors);
            output.insert({socketId, tempData});
        }
        return output;
    }
};