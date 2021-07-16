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

#include <pcilookup.hpp>
#include <report.hpp>
#include <tor_defs.hpp>
#include <tor_defs_skx.hpp>

using json = nlohmann::json;

class SkxReport : public Report
{
    public:
    SkxReport() = delete;
    SkxReport(Summary& summary, SkxTOR& tor) : Report(summary), tor(tor) {};

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
            json firstMcerr;
            socketEntry = json::array();
            errorsPerSocket["Errors Per Socket"] =
                createSummaryString(ctxSocket, SkxfirstError);
            summaryEntry.push_back(errorsPerSocket);
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
                    SkxOpCodeDecode,
                    static_cast<uint32_t>(torEntry.request_opCode));
                if (requestDecoded)
                {
                    details["Request.decoded"] = *requestDecoded;
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
                if (static_cast<uint8_t>(torEntry.lcs) < SKX_LLCS.size())
                {
                    details["LLC.decoded"] = SKX_LLCS[torEntry.lcs];
                }
                details["Target"] =
                    int_to_hex(static_cast<uint8_t>(torEntry.target));
                if (static_cast<uint8_t>(torEntry.target) < SKX_PORT_ID.size())
                {
                    details["Target.decoded"] = SKX_PORT_ID[torEntry.target];
                }
                details["SAD"] = int_to_hex(static_cast<uint8_t>(torEntry.sad));
                if (static_cast<uint8_t>(torEntry.sad) < SAD_RESULT.size())
                {
                    details["SAD.decoded"] = SAD_RESULT[torEntry.sad];
                }

                json& decoded = entry["Decoded"];

                if (torEntry.sad == static_cast<uint8_t>(SadValues::CFG))
                {
                    decoded["ErrorType"] = "TOR Timeout Error";
                    decoded["ErrorSubType"] =
                        "Type 1: PCIe* MMCFG access cause TOR Timeout";
                    if (static_cast<uint8_t>(torEntry.target) <
                        SKX_PORT_ID.size())
                    {
                        decoded["Port"] = SKX_PORT_ID[torEntry.target];
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
                        SKX_PORT_ID.size())
                    {
                        decoded["Port"] = SKX_PORT_ID[torEntry.target];
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
                        SKX_PORT_ID.size())
                    {
                        decoded["Port"] = SKX_PORT_ID[torEntry.target];
                    }
                    decoded["BDF"] =
                        BDFtoString(PciBdfLookup::lookup(getAddress(torEntry)));
                }
                auto firstMcerrCha = getFirstErrorCha(
                    SkxfirstErrorCha, ctxSocket.mcerr.firstMcerrSrcIdCha);
                if (firstMcerrCha == static_cast<uint8_t>(torEntry.cha) &&
                    ctxSocket.mcerr.firstMcerrValid)
                {
                    firstMcerr["First.MCERR"] = entry["Decoded"];
                    firstMcerr["First.MCERR"]["Address"] = details["Address"];
                    if (!entry["Decoded"].empty())
                    {
                        summaryEntry.push_back(firstMcerr);
                    }
                    entry["First.MCERR"] = true;
                }
                else
                {
                    entry["First.MCERR"] = false;
                }
                auto firstIerrCha = getFirstErrorCha(
                    SkxfirstErrorCha, ctxSocket.ierr.firstIerrSrcIdCha);
                if (firstIerrCha == static_cast<uint8_t>(torEntry.cha) &&
                    ctxSocket.ierr.firstIerrValid)
                {
                    entry["First.IERR"] = true;
                }
                else
                {
                    entry["First.IERR"] = false;
                }
                socketEntry.push_back(entry);
            }
        }
        return report;
    }

  private:
    [[nodiscard]] uint64_t getAddress(const SkxTORData& torEntry)
    {
        return static_cast<uint64_t>(torEntry.address) << 14 |
               static_cast<uint64_t>(torEntry.addr_lo);
    }

    const SkxTOR& tor;
};