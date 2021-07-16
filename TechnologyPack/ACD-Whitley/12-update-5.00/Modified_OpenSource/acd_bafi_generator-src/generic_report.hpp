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
#include <map>
#include <nlohmann/json.hpp>
#include <optional>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

#include <aer.hpp>
#include <mca_defs.hpp>
#include <summary.hpp>
#include <tor_defs.hpp>
#include <utils.hpp>

using json = nlohmann::json;

class GenericReport
{
  public:
    GenericReport() = delete;
    GenericReport(Summary& summary) : summary(summary) {};

    [[nodiscard]] json createGenericReport() {
        json report;
        json& summaryReport = report["summary"];
        json& mcaReport = report["MCA"];

        for (auto const& [socketId, socketMcasData] : summary.mca)
        {
            std::vector<std::array<std::string, 3>> mceSummary;
            std::string socket = "socket" + std::to_string(socketId);
            json& mcaEntry = mcaReport[socket];
            mcaEntry = json::array();
            for (auto const& mcaData : socketMcasData)
            {
                auto entry = createMcaEntry(mcaData);
                if (entry.empty())
                {
                    continue;
                }
                std::array<std::string, 3> summaryData = {"", "", ""};
                summaryData[0] = entry["Bank"];
                if (entry["Status_decoded"].contains("MCACOD_decoded"))
                {
                    summaryData[1] = "MCACOD_decoded: " +
                        std::string(entry["Status_decoded"]["MCACOD_decoded"]);
                }
                if (entry["Status_decoded"].contains("MSCOD_decoded"))
                {
                    summaryData[2] = "MSCOD_decoded: " +
                        std::string(entry["Status_decoded"]["MSCOD_decoded"]);
                }
                if (entry["Status_decoded"].contains("INSTANCE_ID_decoded"))
                {
                    summaryData[2] = "INSTANCE_ID_decoded: " +
                        std::string(
                            entry["Status_decoded"]["INSTANCE_ID_decoded"]);
                }
                if (entry["Status_decoded"].contains("MSEC_FW_decoded"))
                {
                    summaryData[2] = "MSEC_FW_decoded: " +
                        std::string(entry["Status_decoded"]["MSEC_FW_decoded"]);
                }
                mceSummary.push_back(summaryData);
                mcaEntry.push_back(entry);
            }
            if (mceSummary.size() > 0)
            {
                json mceSummaryEntry;
                mceSummaryEntry["MCE"] = createMceSummaryString(mceSummary);
                summaryReport[socket].push_back(mceSummaryEntry);
            }
        }

        for (auto const& [socketId, socketAerData] : summary.uncAer)
        {
            if (socketAerData.size() > 0)
            {
                std::string socket = "socket" + std::to_string(socketId);
                json aerEntry;
                json& entry = aerEntry["PCIe_AER_Uncorrectable_errors"];
                for (auto const& aerData : socketAerData)
                {
                    entry[aerData.address] = createUncAerString(aerData);
                }
                summaryReport[socket].push_back(aerEntry);
            }
        }

        for (auto const& [socketId, socketAerData] : summary.corAer)
        {
            if (socketAerData.size() > 0)
            {
                std::string socket = "socket" + std::to_string(socketId);
                json aerEntry;
                json& entry = aerEntry["PCIe_AER_Correctable_errors"];
                for (auto const& aerData : socketAerData)
                {
                    entry[aerData.address] = createCorAerString(aerData);
                }
                summaryReport[socket].push_back(aerEntry);
            }
        }
        std::vector<uint64_t> smallestValuesFromSockets;
        for (auto const& [socketId, socketTscData] : summary.tsc)
        {
            if (socketTscData.pcu_first_mcerr_tsc_cfg != 0 &&
             socketTscData.pcu_first_ierr_tsc_cfg != 0){
                if (socketTscData.pcu_first_ierr_tsc_cfg >
                 socketTscData.pcu_first_mcerr_tsc_cfg)
                {
                    smallestValuesFromSockets
                    .push_back(socketTscData.pcu_first_mcerr_tsc_cfg);
                }
                else
                {
                    smallestValuesFromSockets
                    .push_back(socketTscData.pcu_first_ierr_tsc_cfg);
                }
            }
            else if (socketTscData.pcu_first_ierr_tsc_cfg == 0 &&
             socketTscData.pcu_first_mcerr_tsc_cfg != 0)
            {
                smallestValuesFromSockets
                .push_back(socketTscData.pcu_first_mcerr_tsc_cfg);
            }
            else if (socketTscData.pcu_first_mcerr_tsc_cfg == 0 &&
             socketTscData.pcu_first_ierr_tsc_cfg != 0)
            {
                smallestValuesFromSockets
                .push_back(socketTscData.pcu_first_ierr_tsc_cfg);
            }
        }

        for (auto const& [socketId, socketTscData] : summary.tsc)
        {
            json tscEntry;
            std::string socket = "socket" + std::to_string(socketId);
            auto tscData = json::array();
            if (socketTscData.pcu_first_ierr_tsc_cfg != 0 ||
                socketTscData.pcu_first_mcerr_tsc_cfg != 0)
            {
                std::stringstream ss1;
                ss1 << "pcu_first_ierr_tsc_cfg: " <<
                    int_to_hex(socketTscData.pcu_first_ierr_tsc_cfg);
                if (socketTscData.pcu_first_ierr_tsc_cfg ==
                 *std::min_element(smallestValuesFromSockets.begin(),
                 smallestValuesFromSockets.end()))
                 ss1 << " (Occurred first between all TSCs)";
                json entry1 = ss1.str();
                tscData.push_back(entry1);

                std::stringstream ss2;
                ss2 << "pcu_first_mcerr_tsc_cfg: " <<
                    int_to_hex(socketTscData.pcu_first_mcerr_tsc_cfg);
                if (socketTscData.pcu_first_mcerr_tsc_cfg ==
                 *std::min_element(smallestValuesFromSockets.begin(),
                 smallestValuesFromSockets.end()))
                 ss2 << " (Occurred first between all TSCs)";
                json entry2 = ss2.str();
                tscData.push_back(entry2);

                std::stringstream ss3;
                ss3 << "IERR occurred: " << std::boolalpha
                    << socketTscData.ierr_on_socket;
                json entry3 = ss3.str();
                tscData.push_back(entry3);

                std::stringstream ss4;
                ss4 << "MCERR occurred: " << std::boolalpha
                    << socketTscData.mcerr_on_socket;
                json entry4 = ss4.str();
                tscData.push_back(entry4);

                tscEntry["TSC"] = tscData;
                summaryReport[socket].push_back(tscEntry);
            }
        }
        return report;
    }

    Summary& summary;

  protected:
    [[nodiscard]] json createMcaEntry(MCAData mcaData)
    {
        auto bankDecoder = mcaDecoderFactory(mcaData, summary.cpuType);
        json entry;
        if (!bankDecoder)
        {
            return entry;
        }
        entry = bankDecoder->decode();
        // decode CHA bank address according to memory map
        if (mcaData.cbo)
        {
            auto addressMapped = mapToMemory(mcaData.address);
            if (addressMapped)
            {
                std::stringstream ss;
                ss << entry["Address"].get<std::string>() << " (" <<
                    *addressMapped << ")";
                entry["Address"] = ss.str();
            }
        }
        return entry;
    }

    [[nodiscard]] std::optional<std::string> mapToMemory(uint64_t address)
    {
        for (auto const& [name, limits] : summary.memoryMap)
        {
            if (address >= limits[0] && address <= limits[1])
            {
                return name;
            }
        }
        return {};
    }

    [[nodiscard]] std::optional<uint8_t> getFirstErrorCha(
        std::map<uint8_t, uint8_t> decodingTable, uint32_t toDecode)
    {
        const auto& item = decodingTable.find(toDecode);
        if (item != decodingTable.cend())
        {
            return item->second;
        }
        return {};
    }

    [[nodiscard]] std::string
        BDFtoString(const std::tuple<uint8_t, uint8_t, uint8_t>& bdf)
    {
        auto& [bus, dev, func] = bdf;
        if (bus == 0 && dev == 0 && func == 0)
        {
            return "Please refer to system address map";
        }
        std::stringstream ss;
        ss << "Bus: " << int_to_hex(bus, false) << ", ";
        ss << "Device: " << int_to_hex(dev, false) << ", ";
        ss << "Function: " << int_to_hex(func, false);
        return ss.str();
    }

    [[nodiscard]] std::tuple<uint8_t, uint8_t, uint8_t>
        getBDFFromAddress(uint32_t address)
    {
        BDFFormatter bdf;
        bdf.address = address;
        return std::make_tuple(static_cast<uint8_t>(bdf.bus),
            static_cast<uint8_t>(bdf.dev), static_cast<uint8_t>(bdf.func));
    }

    [[nodiscard]] std::string createSummaryString(SocketCtx ctx,
        const std::map<uint16_t, const char*> firstErrorTable, std::string Cpu, json& summaryEntry)
    {
        std::stringstream ss;
        auto firstIerrSrc = getDecoded(firstErrorTable,
            static_cast<uint16_t>(ctx.ierr.firstIerrSrcIdCha));
        auto firstMcerrSrc = getDecoded(firstErrorTable,
            static_cast<uint16_t>(ctx.mcerr.firstMcerrSrcIdCha));
        if (Cpu == "SPR")
        {
            firstIerrSrc = getDecoded(firstErrorTable,
                static_cast<uint16_t>(ctx.ierrSpr.firstIerrSrcIdCha));
            firstMcerrSrc = getDecoded(firstErrorTable,
                static_cast<uint16_t>(ctx.mcerrSpr.firstMcerrSrcIdCha));
        }
        else
        {
            ctx.ierrSpr.firstIerrValid = 0;
            ctx.mcerrSpr.firstMcerrValid = 0;
        }
        if (ctx.mcerrErr.value == 0)
        {
            ss << "MCA_ERR_SRC_LOG = N/A, ";
        }
        else
        {
            if (ctx.mcerrErr.msmi_internal)
            {
                ss << "IntMSMI, ";
            }
            if (ctx.mcerrErr.msmi_external)
            {
                ss << "ExtMSMI, ";
            }
            if (ctx.mcerrErr.caterr_interal)
            {
                ss << "IntCATERR, ";
            }
            if (ctx.mcerrErr.caterr_external)
            {
                ss << "ExtCATERR, ";
            }
        }
        if (firstIerrSrc && (ctx.ierr.firstIerrValid ||
            ctx.ierrSpr.firstIerrValid))
        {
            ss << "FirstIERR = " << *firstIerrSrc << ", ";
        }
        else if (ctx.ierr.firstIerrValid || ctx.ierrSpr.firstIerrValid)
        {
            ss << "FirstIERR = N/A, ";
        }
        else
        {
            ss << "FirstIERR = 0x0, ";
        }
        if (firstMcerrSrc && (ctx.mcerr.firstMcerrValid ||
            ctx.mcerrSpr.firstMcerrValid))
        {
            if (firstMcerrInMce(*firstMcerrSrc, summaryEntry))
            {
                ss << "FirstMCERR = " << *firstMcerrSrc;
            }
            else
            {
                ss << "FirstMCERR = " << *firstMcerrSrc << ", FirstMCERR value" 
                " doesn't match any MCE bank#, manual review may be required";
            }
        }
        else if (ctx.mcerr.firstMcerrValid || ctx.mcerrSpr.firstMcerrValid)
        {
            ss << "FirstMCERR = N/A";
        }
        else
        {
            ss << "FirstMCERR = 0x0";
        }
        return ss.str();
    }

    [[nodiscard]] std::string createUncAerString(UncAerData aerData)
    {
        std::stringstream ss;
        if (aerData.data_link_protocol_error)
        {
            ss << "data_link_protocol_error, ";
        }
        if (aerData.surprise_down_error)
        {
            ss << "surprise_down_error, ";
        }
        if (aerData.poisoned_tlp)
        {
            ss << "poisoned_tlp, ";
        }
        if (aerData.flow_control_protocol_error)
        {
            ss << "flow_control_protocol_error, ";
        }
        if (aerData.completition_timeout)
        {
            ss << "completition_timeout, ";
        }
        if (aerData.completer_abort)
        {
            ss << "completer_abort, ";
        }
        if (aerData.unexpected_completition)
        {
            ss << "unexpected_completition, ";
        }
        if (aerData.receiver_buffer_overflow)
        {
            ss << "receiver_buffer_overflow, ";
        }
        if (aerData.malformed_tlp)
        {
            ss << "malformed_tlp, ";
        }
        if (aerData.ecrc_error)
        {
            ss << "ecrc_error, ";
        }
        if (aerData.received_an_unsupported_request)
        {
            ss << "received_an_unsupported_request, ";
        }
        if (aerData.acs_violation)
        {
            ss << "acs_violation, ";
        }
        if (aerData.uncorrectable_internal_error)
        {
            ss << "uncorrectable_internal_error, ";
        }
        if (aerData.mc_blocked_tlp)
        {
            ss << "mc_blocked_tlp, ";
        }
        if (aerData.atomic_egress_blocked)
        {
            ss << "atomic_egress_blocked, ";
        }
        if (aerData.tlp_prefix_blocked)
        {
            ss << "tlp_prefix_blocked, ";
        }
        if (aerData.poisoned_tlp_egress_blocked)
        {
            ss << "poisoned_tlp_egress_blocked, ";
        }

        std::string output = ss.str();
        output.erase(output.end() - 2, output.end());
        return output;
    }

    [[nodiscard]] std::string createCorAerString(CorAerData aerData)
    {
        std::stringstream ss;
        if (aerData.receiver_error)
        {
            ss << "receiver_error, ";
        }
        if (aerData.bad_tlp)
        {
            ss << "bad_tlp, ";
        }
        if (aerData.bad_dllp)
        {
            ss << "bad_dllp, ";
        }
        if (aerData.replay_num_rollover)
        {
            ss << "replay_num_rollover, ";
        }
        if (aerData.replay_timer_timeout)
        {
            ss << "replay_timer_timeout, ";
        }
        if (aerData.advisory_non_fatal_error)
        {
            ss << "advisory_non_fatal_error, ";
        }
        if (aerData.correctable_internal_error)
        {
            ss << "correctable_internal_error, ";
        }
        if (aerData.header_log_overflow_error)
        {
            ss << "header_log_overflow_error, ";
        }

        std::string output = ss.str();
        output.erase(output.end() - 2, output.end());
        return output;
    }

    [[nodiscard]] json createMceSummaryString(
        std::vector<std::array<std::string, 3>> mceSummaryData)
    {
        auto mcaEntry = json::array();
        for (auto const& mcaData : mceSummaryData)
        {
            std::stringstream ss;
            ss << "bank " << mcaData[0];
            if (mcaData[1] != "")
            {
                ss << ", " << mcaData[1];
            }
            if (mcaData[2] != "")
            {
                ss << ", " << mcaData[2];
            }
            mcaEntry.push_back(ss.str());
        }
        return mcaEntry;
    }

    [[nodiscard]] std::optional<std::string> createThermStatusString(
        uint32_t socketId)
    {
        std::stringstream ss;
        if (summary.thermStatus.find(socketId) == summary.thermStatus.end())
        {
            return {};
        }
        ss << "PROCHOT_LOG=";
        ss << int_to_hex(bool(summary.thermStatus[socketId].prochot_log));
        ss << ", PROCHOT_STATUS=";
        ss << int_to_hex(bool(summary.thermStatus[socketId].prochot_status));
        ss << ", PMAX_LOG=";
        ss << int_to_hex(bool(summary.thermStatus[socketId].pmax_log));
        ss << ", PMAX_STATUS=";
        ss << int_to_hex(bool(summary.thermStatus[socketId].pmax_status));
        ss << ", OUT_OF_SPEC_LOG=";
        ss << int_to_hex(bool(summary.thermStatus[socketId].out_of_spec_log));
        ss << ", OUT_OF_SPEC_STATUS=";
        ss << int_to_hex(bool(
            summary.thermStatus[socketId].out_of_spec_status));
        ss << ", THERMAL_MONITOR_LOG=";
        ss << int_to_hex(bool(
            summary.thermStatus[socketId].thermal_monitor_log));
        ss << ", THERMAL_MONITOR_STATUS=";
        ss << int_to_hex(bool(
            summary.thermStatus[socketId].thermal_monitor_status));
        return ss.str();
    }

    [[nodiscard]] bool firstMcerrInMce(std::string firstMcerrSrc,
                                        json& summaryEntry)
    {
        bool bankPresentInMce = false;
        std::regex bankRegex("bank ([0-9]*)[, ]*([0-9]*)[, ]*([0-9]*)");
        std::regex coreRegex("bank ([0-9])-([0-9])");
        std::smatch matches;
        if (summaryEntry != nullptr && summaryEntry[0].contains("MCE"))
        {
            if (std::regex_search(firstMcerrSrc, matches, coreRegex))
            {
                for (int i = std::stoi(matches[1].str());
                i <= std::stoi(matches[2].str()); i++)
                {
                    std::string bankNumber = "bank " + std::to_string(i) + " ";
                    for (auto const& bankError : summaryEntry[0]["MCE"])
                    {
                        if (std::string(bankError).find(bankNumber) !=
                            std::string::npos)
                        {
                            bankPresentInMce = true;
                            break;
                        }
                    }

                    if (bankPresentInMce)
                        break;
                }
            }
            else if (std::regex_search(firstMcerrSrc, matches, bankRegex))
            {
                uint32_t i = 1;
                while (matches[i].str() != "")
                {
                    std::string bankNumber = "bank " + matches[i].str() + " ";
                    for (auto const& bankError : summaryEntry[0]["MCE"])
                    {
                        if (std::string(bankError).find(bankNumber) !=
                            std::string::npos)
                        {
                            bankPresentInMce = true;
                            break;
                        }
                    }

                    if (bankPresentInMce)
                        break;
                    i++;
                }
            }
        }
        return bankPresentInMce;
    }
};