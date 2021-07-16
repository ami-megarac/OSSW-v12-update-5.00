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
#include <nlohmann/json.hpp>
#include <string>
#include <regex>

#include <pcilookup.hpp>
#include <generic_report.hpp>
#include <tor_defs.hpp>
#include <tor_defs_icx.hpp>
#include <tor_defs_cpx.hpp>
#include <tor_defs_skx.hpp>

using json = nlohmann::json;

template <typename T>
class Report : public GenericReport
{
  public:
    Report() = delete;
    Report(Summary& summary, T& tor, std::string cpu_type) : GenericReport(summary), tor(tor) {
        Cpu = cpu_type;

        if (Cpu == "ICX")
        {
            PORT_ID = ICX_PORT_ID;
            LLCS = ICX_LLCS;
            FirstErrorCha = IcxfirstErrorCha;
            FirstError = IcxfirstError;
            OpCodeDecode = IcxOpCodeDecode;
        }
        else if (Cpu == "CPX")
        {
            PORT_ID = CPX_PORT_ID;
            LLCS = CPX_LLCS;
            FirstErrorCha = CpxfirstErrorCha;
            FirstError = CpxfirstError;
            OpCodeDecode = CpxOpCodeDecode;
        }
        else if (Cpu == "SKX")
        {
            PORT_ID = SKX_PORT_ID;
            LLCS = SKX_LLCS;
            FirstErrorCha = SkxfirstErrorCha;
            FirstError = SkxfirstError;
            OpCodeDecode = SkxOpCodeDecode;
        }
    };

    [[nodiscard]] json createJSONReport() {
        json report = createGenericReport();
        json& torReport = report["TOR"];
        json& summaryReport = report["summary"];

        for (auto const& [socketId, reportData] : tor)
        {
            std::string socket = "socket" + std::to_string(socketId);
            auto ctxSocket = reportData.first;
            auto tors = reportData.second;
            json& summaryEntry = summaryReport[socket];
            json& socketEntry = torReport[socket];
            json errorsPerSocket;
            json uboxAdditionalInformation;
            json thermalStatus;
            json firstMcerr;
            socketEntry = json::array();
            errorsPerSocket["Errors_Per_Socket"] =
                createSummaryString(ctxSocket, FirstError);
            std::regex uboxMCERR("FirstMCERR = .*, bank 6");
            std::regex uboxMCE("UBOX");
            if(std::regex_search(std::string(errorsPerSocket["Errors_Per_Socket"]), uboxMCERR))
            {
                for (auto const& bankError : summaryEntry[0]["MCE"])
                {
                    if (std::regex_search(std::string(bankError), uboxMCE))
                    {
                        std::size_t left = std::string(bankError).find("Bus");
                        if (left > std::string(bankError).size())
                            continue;

                        std::size_t right = std::string(bankError).find(" from");
                        uboxAdditionalInformation["IO_Errors"].push_back(std::string(bankError)
                        .substr(left, right - left));
                    }
                }
            }
            summaryEntry.push_back(errorsPerSocket);
            if (!uboxAdditionalInformation.empty())
                summaryEntry.push_back(uboxAdditionalInformation);

            auto packageThermStatus = createThermStatusString(socketId);
            if (packageThermStatus)
            {
                thermalStatus["Package_Therm_Status"] = *packageThermStatus;
                summaryEntry.push_back(thermalStatus);
            }
            for (const auto& torEntry : tors)
            {
                json entry;
                json& details = entry["Details"];
                details["CoreId"] =
                    int_to_hex(static_cast<uint8_t>(torEntry.core_id));
                details["ThreadId"] =
                    int_to_hex(static_cast<uint8_t>(torEntry.thread_id));
                details["CHA"] = int_to_hex(static_cast<uint8_t>(torEntry.cha));
                details["IDX"] = int_to_hex(static_cast<uint8_t>(torEntry.idx));
                details["Request"] =
                    int_to_hex(static_cast<uint16_t>(torEntry.request_opCode));
                auto requestDecoded = getDecoded(
                    OpCodeDecode,
                    static_cast<uint32_t>(torEntry.request_opCode));
                if (requestDecoded)
                {
                    details["Request_decoded"] = *requestDecoded;
                }
                uint64_t address = getAddress(torEntry);
                auto addressMapped = mapToMemory(address);
                if (addressMapped)
                {
                    details["Address"] = int_to_hex(address) + " ("
                        + *addressMapped + ")";
                }
                else
                {
                    details["Address"] = int_to_hex(address);
                }
                details["InPipe"] =
                    int_to_hex(static_cast<uint8_t>(torEntry.in_pipe));
                details["Retry"] =
                    int_to_hex(static_cast<uint8_t>(torEntry.retry));
                details["TorFSMState"] =
                    int_to_hex(static_cast<uint8_t>(torEntry.fsm));
                details["LLC"] = int_to_hex(static_cast<uint8_t>(torEntry.lcs));
                if (static_cast<uint8_t>(torEntry.lcs) < LLCS.size())
                {
                    details["LLC_decoded"] = LLCS[torEntry.lcs];
                }
                details["Target"] =
                    int_to_hex(static_cast<uint8_t>(torEntry.target));
                if (static_cast<uint8_t>(torEntry.target) < PORT_ID.size())
                {
                    details["Target_decoded"] = PORT_ID[torEntry.target];
                }
                details["SAD"] = int_to_hex(static_cast<uint8_t>(torEntry.sad));
                if (static_cast<uint8_t>(torEntry.sad) < SAD_RESULT.size())
                {
                    details["SAD_decoded"] = SAD_RESULT[torEntry.sad];
                }

                json& decoded = entry["Decoded"];

                if (torEntry.sad == static_cast<uint8_t>(SadValues::CFG))
                {
                    decoded["ErrorType"] = "TOR Timeout Error";
                    decoded["ErrorSubType"] =
                        "Type 1: PCIe* MMCFG access cause TOR Timeout";
                    if (static_cast<uint8_t>(torEntry.target) <
                        PORT_ID.size())
                    {
                        decoded["Port"] = PORT_ID[torEntry.target];
                    }
                    decoded["BDF"] =
                        BDFtoString(getBDFFromAddress(getAddress(torEntry)));
                }
                else if (torEntry.sad ==
                         static_cast<uint8_t>(SadValues::MMIOPartialRead))
                {
                    decoded["ErrorType"] = "TOR Timeout Error";
                    decoded["ErrorSubType"] =
                        "Type 2: PCIe* MMIO access cause TOR timeout.";
                    if (static_cast<uint8_t>(torEntry.target) <
                        PORT_ID.size())
                    {
                        decoded["Port"] = PORT_ID[torEntry.target];
                    }
                    decoded["BDF"] =
                        BDFtoString(PciBdfLookup::lookup(getAddress(torEntry)));
                }
                else if (torEntry.sad == static_cast<uint8_t>(SadValues::IO))
                {
                    decoded["ErrorType"] = "TOR Timeout Error";
                    decoded["ErrorSubType"] =
                        "Type 3: I/O Port in access cause TOR timeout.";
                    if (static_cast<uint8_t>(torEntry.target) <
                        PORT_ID.size())
                    {
                        decoded["Port"] = PORT_ID[torEntry.target];
                    }
                    decoded["BDF"] =
                        BDFtoString(PciBdfLookup::lookup(getAddress(torEntry)));
                }
                auto firstMcerrCha = getFirstErrorCha(
                    FirstErrorCha, ctxSocket.mcerr.firstMcerrSrcIdCha);
                if (firstMcerrCha == static_cast<uint8_t>(torEntry.cha) &&
                    ctxSocket.mcerr.firstMcerrValid)
                {
                    firstMcerr["First_MCERR"] = entry["Decoded"];
                    firstMcerr["First_MCERR"]["Address"] = details["Address"];
                    if (!entry["Decoded"].empty())
                    {
                        summaryEntry.push_back(firstMcerr);
                    }
                    entry["First_MCERR"] = true;
                }
                else
                {
                    entry["First_MCERR"] = false;
                }
                auto firstIerrCha = getFirstErrorCha(
                    FirstErrorCha, ctxSocket.ierr.firstIerrSrcIdCha);
                if (firstIerrCha == static_cast<uint8_t>(torEntry.cha) &&
                    ctxSocket.ierr.firstIerrValid)
                {
                    entry["First_IERR"] = true;
                }
                else
                {
                    entry["First_IERR"] = false;
                }
                socketEntry.push_back(entry);
            }
        }
        return report;
    }

  private :
    std::string Cpu;
    std::array<const char*, 27> PORT_ID;
    std::array<const char*, 16> LLCS;
    std::map<uint8_t, uint8_t> FirstErrorCha;
    std::map<uint8_t, const char*> FirstError;
    std::map<uint32_t, const char*> OpCodeDecode;

    [[nodiscard]] uint64_t getAddress(const IcxTORData& torEntry)
    {
        return static_cast<uint64_t>(torEntry.address_8_6) << 6 |
               static_cast<uint64_t>(torEntry.address_16_9) << 9 |
               static_cast<uint64_t>(torEntry.address_19_17) << 17 |
               static_cast<uint64_t>(torEntry.address_27_20) << 20 |
               static_cast<uint64_t>(torEntry.address_30_28) << 28 |
               static_cast<uint64_t>(torEntry.address_38_31) << 31 |
               static_cast<uint64_t>(torEntry.address_41_39) << 39 |
               static_cast<uint64_t>(torEntry.address_49_42) << 42 |
               static_cast<uint64_t>(torEntry.address_51_50) << 50;
    }

    [[nodiscard]] uint64_t getAddress(const CpxTORData& torEntry)
    {
        return static_cast<uint64_t>(torEntry.address) << 14 |
               static_cast<uint64_t>(torEntry.addr_lo);
    }

    [[nodiscard]] uint64_t getAddress(const SkxTORData& torEntry)
    {
        return static_cast<uint64_t>(torEntry.address) << 14 |
               static_cast<uint64_t>(torEntry.addr_lo);
    }


    const T& tor;
};