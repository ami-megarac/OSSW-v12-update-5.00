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
#include <nlohmann/json.hpp>
#include <string>
#include <regex>

#include <pcilookup.hpp>
#include <generic_report.hpp>
#include <tor_defs.hpp>
#include <tor_defs_icx.hpp>
#include <tor_defs_cpx.hpp>
#include <tor_defs_skx.hpp>
#include <tor_defs_spr.hpp>
#include <utils.hpp>
#include <tor_whitley.hpp>

using json = nlohmann::json;

template <typename T>
class Report : public GenericReport
{
  public:
    Report() = delete;
    Report(Summary& summary, T& tor, std::string cpu_type, json deviceMapFile, json silkscreenMapFile, const json& input) :
           GenericReport(summary), tor(tor), input(input) {
        Cpu = cpu_type;
        deviceMap = deviceMapFile;
        silkscreenMap = silkscreenMapFile;

        if (Cpu == "ICX")
        {
            PORT_ID = ICX_PORT_ID;
            LLCS = ICX_LLCS;
            FirstErrorCha = IcxfirstErrorCha;
            FirstError = IcxfirstError;
            OpCodeDecode = IcxOpCodeDecode;
        }
        else if (Cpu == "CPX" || Cpu == "CLX")
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
        else if (Cpu == "SPR")
        {
            PORT_ID = SPR_PORT_ID;
            LLCS = SPR_LLCS;
            FirstErrorCha = SprfirstErrorCha;
            FirstError = SprfirstError;
            OpCodeDecode = SprOpCodeDecode;
        }
    };

    [[nodiscard]] json createJSONReport() {
        json report = createGenericReport();
        json& torReport = report["TOR"];
        json& summaryReport = report["summary"];
        auto cpuSections = prepareJson(input);
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
                createSummaryString(ctxSocket, FirstError, Cpu, summaryEntry);
            std::regex uboxMCERR("FirstMCERR = .*, bank 6");
            std::regex uboxMCE("UBOX");
            if(std::regex_search(
                std::string(errorsPerSocket["Errors_Per_Socket"]), uboxMCERR))
            {
                for (auto const& bankError : summaryEntry[0]["MCE"])
                {
                    if (std::regex_search(std::string(bankError), uboxMCE))
                    {
                        std::size_t left = std::string(bankError).find("Bus");
                        if (left > std::string(bankError).size())
                            continue;

                        std::size_t right = std::string(bankError).find(" from");
                        uboxAdditionalInformation["IO_Errors"].push_back(
                            std::string(bankError).substr(left, right - left));
                        auto bdfObj = nlohmann::ordered_json::object();
                        getBdfFromIoErrorsSection(std::string(bankError).
                            substr(left, right - left), bdfObj);
                        showBdfDescription(deviceMap, bdfObj);
                        if (bdfObj.contains("Description"))
                        {
                            uboxAdditionalInformation["IO_Errors"].push_back(
                                std::string(bdfObj["Description"]));
                        }
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
            for (auto& torEntry : tors)
            {
                json entry;
                json& details = entry["Details"];
                if (Cpu == "ICX")
                {
                    torEntry.core_id = torEntry.core_id1;
                    torEntry.thread_id = torEntry.thread_id1;
                    torEntry.request_opCode = torEntry.request_opCode1;
                    torEntry.in_pipe = torEntry.in_pipe1;
                    torEntry.retry = torEntry.retry1;
                    torEntry.fsm = torEntry.fsm1;
                    torEntry.lcs = torEntry.lcs1;
                    torEntry.target = torEntry.target1;
                    torEntry.sad = torEntry.sad1;
                }

                if (Cpu == "SPR")
                {
                    torEntry.core_id = torEntry.core_id2_2_0 |
                     torEntry.core_id2_6_3 << 3;
                    torEntry.thread_id = torEntry.thread_id2;
                    torEntry.request_opCode = torEntry.request_opCode2_6_0 |
                     torEntry.request_opCode2_10_7 << 7;
                    torEntry.in_pipe = torEntry.in_pipe2;
                    torEntry.retry = torEntry.retry2;
                    torEntry.fsm = torEntry.fsm2;
                    torEntry.lcs = torEntry.lcs2;
                    torEntry.target = torEntry.target2;
                    torEntry.sad = torEntry.sad2;
                }
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
                    auto bdfObj = nlohmann::ordered_json::object();
                    getBdfFromFirstMcerrSection(decoded["BDF"], bdfObj);
                    if (bdfObj.contains("Bus"))
                        showBdfDescription(deviceMap, bdfObj);
                    if (bdfObj.contains("Description"))
                        decoded["Description"] = bdfObj["Description"];
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
                    auto bdfObj = nlohmann::ordered_json::object();
                    getBdfFromFirstMcerrSection(decoded["BDF"], bdfObj);
                    if (bdfObj.contains("Bus"))
                        showBdfDescription(deviceMap, bdfObj);
                    if (bdfObj.contains("Description"))
                        decoded["Description"] = bdfObj["Description"];
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
                    auto bdfObj = nlohmann::ordered_json::object();
                    getBdfFromFirstMcerrSection(decoded["BDF"], bdfObj);
                    if (bdfObj.contains("Bus"))
                        showBdfDescription(deviceMap, bdfObj);
                    if (bdfObj.contains("Description"))
                        decoded["Description"] = bdfObj["Description"];
                }
                auto firstMcerrCha = getFirstErrorCha(
                    FirstErrorCha, ctxSocket.mcerr.firstMcerrSrcIdCha);
                if (Cpu == "SPR")
                    firstMcerrCha = getFirstErrorCha(
                    FirstErrorCha, ctxSocket.mcerrSpr.firstMcerrSrcIdCha);
                if (firstMcerrCha == static_cast<uint8_t>(torEntry.cha) &&
                    (ctxSocket.mcerr.firstMcerrValid ||
                    ctxSocket.mcerrSpr.firstMcerrValid))
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
                if (Cpu == "SPR")
                    firstIerrCha = getFirstErrorCha(
                    FirstErrorCha, ctxSocket.ierrSpr.firstIerrSrcIdCha);
                if (firstIerrCha == static_cast<uint8_t>(torEntry.cha) &&
                    (ctxSocket.ierr.firstIerrValid ||
                    ctxSocket.ierrSpr.firstIerrValid))
                {
                    entry["First_IERR"] = true;
                }
                else
                {
                    entry["First_IERR"] = false;
                }
                socketEntry.push_back(entry);
            }
            for (auto const& [cpu, cpuSection] : cpuSections)
            {
                if (std::string(cpu) == "cpu" + std::to_string(socketId))
                {
                    auto memoryErrors = decodeRetryLog(cpuSection, silkscreenMap, std::to_string(socketId));
                    if (memoryErrors != nullptr)
                        summaryEntry.push_back(memoryErrors);
                }
            }
        }
        return report;
    }

  private :
    std::string Cpu;
    json deviceMap;
    json silkscreenMap;
    const json& input;
    std::array<const char*, 32> PORT_ID;
    std::array<const char*, 16> LLCS;
    std::map<uint8_t, uint8_t> FirstErrorCha;
    std::map<uint16_t, const char*> FirstError;
    std::map<uint32_t, const char*> OpCodeDecode;

    [[nodiscard]] uint64_t getAddress(const TORDataGeneric& torEntry)
    {
        if (Cpu == "ICX")
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
        else if (Cpu == "SPR")
        {
            return ((static_cast<uint64_t>(torEntry.address_0_0_spr) |
               static_cast<uint64_t>(torEntry.address_10_1_spr) << 1 |
               static_cast<uint64_t>(torEntry.address_11_11_spr) << 11 |
               static_cast<uint64_t>(torEntry.address_21_12_spr) << 12 |
               static_cast<uint64_t>(torEntry.address_22_22_spr) << 22 |
               static_cast<uint64_t>(torEntry.address_32_23_spr) << 23 |
               static_cast<uint64_t>(torEntry.address_33_33_spr) << 33 |
               static_cast<uint64_t>(torEntry.address_34_34_spr) << 34) << 17) +
               ((static_cast<uint64_t>(torEntry.set_0_0) |
               static_cast<uint64_t>(torEntry.set_10_1) << 1) << 6);
        }
        else
        {
            return static_cast<uint64_t>(torEntry.address) << 14 |
               static_cast<uint64_t>(torEntry.addr_lo);
        }
    }

    const T& tor;
};
