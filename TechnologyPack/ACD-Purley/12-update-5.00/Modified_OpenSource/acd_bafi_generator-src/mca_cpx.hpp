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
#include <map>
#include <nlohmann/json.hpp>
#include <string>
#include <sstream>

#include <mca_defs.hpp>
#include <utils.hpp>

using json = nlohmann::json;

class CpxMcaDecoder : public McaDecoder
{
  public:
    CpxMcaDecoder(const MCAData& mca) : McaDecoder(mca) {};

    json decode()
    {
        json entry;
        entry["Core"] = mca.core;
        entry["Thread"] = mca.thread;
        entry["Bank"] = decodeBankName(mca.bank);
        entry["Status"] = int_to_hex(static_cast<uint64_t>(mca.mc_status));
        entry["Address"] = int_to_hex(static_cast<uint64_t>(mca.address));
        entry["Misc"] = int_to_hex(static_cast<uint64_t>(mca.misc));
        entry["CTL"] = int_to_hex(static_cast<uint64_t>(mca.ctl));
        entry["Status_decoded"] = decode_status();

        return entry;
    }

    virtual json decode_status() = 0;

  protected:
    const std::map<uint8_t, const char*> BankNames = {
        {0x0, "IFU"},
        {0x1, "DCU"},
        {0x2, "DTLB"},
        {0x3, "MLC"},
        {0x4, "PCU"},
        {0x5, "Intel UPI 0"},
        {0x6, "IIO"},
        {0x7, "IMC 0, Main"},
        {0x8, "IMC 1, Main"},
        {0x9, "CHA [0]"},
        {0xA, "CHA [1]"},
        {0xB, "CHA [2]"},
        {0xC, "Intel UPI 1"},
        {0xD, "IMC 0, channel 0"},
        {0xE, "IMC 0, channel 1"},
        {0xF, "IMC 1, channel 0"},
        {0x10, "IMC 1, channel 1"},
        {0x11, "IMC 0, channel 2"},
        {0x12, "IMC 1, channel 2"},
        {0x13, "Intel UPI 2"},
        {0x14, "Intel UPI 3"},
        {0x15, "Intel UPI 4"},
        {0x16, "Intel UPI 5"},
    };

    const std::map<uint8_t, const char*> CorrErrStsInd = {
        {0x0, "No hardware status tracking is provided for the structure "
              "reporting this event"},
        {0x1, "Green - The current status is green (below threshold) for the "
              "structure posting the event"},
        {0x2, "Yellow - The current status is yellow (above threshold) for the "
              "structure posting the event"},
        {0x3, "Reserved"}
    };

    const std::map<uint8_t, const char*> ErrorRequestType = {
        {0x0, "GENERIC_ERROR"}, {0x1, "GENERIC_READ"},
        {0x2, "GENERIC_WRITE"}, {0x3, "DATA_READ"},
        {0x4, "DATA_WRITE"},    {0x5, "INSTRUCTION_FETCH"},
        {0x6, "PREFETCH"},      {0x7, "EVICTION"},
        {0x8, "SNOOP"},         {0x9, "RESERVED"},
        {0xa, "RESERVED"},      {0xb, "RESERVED"},
        {0xc, "RESERVED"},      {0xd, "RESERVED"},
        {0xe, "RESERVED"},      {0xf, "RESERVED"},
    };

    const std::map<uint8_t, const char*> ErrorTransactionType = {
        {0x0, "INSTRUCTION"},
        {0x1, "DATA"},
        {0x2, "GENERIC_ERROR_TRANSACTION_TYPE"},
        {0x3, "RESERVED"},
    };

    const std::map<uint8_t, const char*> ErrorLevelEncoding = {
        {0x0, "LEVEL_0 (core L1 cache)"},
        {0x1, "LEVEL_1 (core L2 cache)"},
        {0x2, "LEVEL_2 (L3/LLC cache)"},
        {0x3, "GENERIC_ERROR_LEVEL_ENCODING"},
    };

    const std::map<uint8_t, const char*> DdrType = {
        {0x0, "No error logged (DDR4 or NVMDIMM)"},
        {0x1, "Error specifically on DDR4"},
        {0x2, "Error specifically on NVMDIMM"},
        {0x3,
         "Error for this transaction was detected on both DDR4 and NVMDIMM"},
    };

    const std::map<uint16_t, const char*> Mcacod = {
        {0xc0f, "Unsupported/Undefined Packet"},
        {0xe0f, "For all other corrected and uncorrected errors"},
    };

    const std::map<uint8_t, const char*> Participation = {
        {0x2, "Unsupported/Undefined Packet"},
        {0x3, "Other Errors"},
    };

    const std::map<uint8_t, const char*> Request = {
        {0x0, "Memory or I/O"},
        {0x3, "Level encoding"},
    };

    const std::map<uint8_t, const char*> ErrSpecific = {
        {0x5, "Parity error on structures"},
    };

    const std::map<uint8_t, const char*> Error = {
        {0x0, "Generic undefined request"}, {0x1, "Memory Read Error"},
        {0x2, "Memory Write Error"},        {0x3, "Address/Command Error"},
        {0x4, "Memory Scrubbing Error"},
    };

    const std::map<uint8_t, const char*> LlError = {
        {0x0, "UC PHY Initialization Failure"},
        {0x1, "UC PHY Detected Drift Buffer Alarm"},
        {0x2, "UC PHY Detected Latency Buffer Rollover"},
        {0x10, "UC LL Rx detected CRC error"},
        {0x11, "UC LL Rx Unsupported/Undefined packet"},
        {0x13, "UC LL Rx Parameter Exception"},
        {0x1f, "UC LL Detected Control Error"},
        {0x20, "COR PHY Initialization Abort"},
        {0x22, "COR PHY Lane failure"},
        {0x25, "COR PHY L0p"},
        {0x30, "COR LL Rx detected CRC error"},
        {0x31, "COR LL Rx detected CRC error"},
    };

    const std::map<uint8_t, const char*> Channel = {
        {0x0, "Channel_0"},  {0x1, "Channel_1"},
        {0x2, "Channel_2"},  {0x3, "Channel_3"},
        {0x4, "Channel_4"},  {0x5, "Channel_5"},
        {0x6, "Channel_6"},  {0x7, "Channel_7"},
        {0x8, "Channel_8"},  {0x9, "Channel_9"},
        {0xa, "Channel_10"}, {0xb, "Channel_11"},
        {0xc, "Channel_12"}, {0xd, "Channel_13"},
        {0xe, "Channel_14"}, {0xf, "Channel_not_specified"},
    };

    const std::map<uint16_t, const char*> Mscod2231 = {
        {0x1, "Phy Control Error"},
        {0x2, "Unexpected Retry Ack flit"},
        {0x4, "Unexpected Retry Req flit"},
        {0x8, "RF Parity error"},
        {0x10, "Routeback Table Error"},
        {0x20, "Unexpected Tx Protocol Flit"},
        {0x40, "Rx Header-or-CreditBGF credit overflow/underflow"},
        {0x80, "Link Layer Reset still in progress when Phy enters L0"},
        {0x100, "Link Layer reset initiated while protocol traffic not idle"},
        {0x200, "Link layer Tx Parity Error"},
    };

    const std::map<uint8_t, const char*> Mscod1621 = {
        {0x0, "UC Phy Initialization Failure"},
        {0x1, "UC Phy Detected Drift Buffer Alarm"},
        {0x2, "UC Phy Detected Latency Buffer Rollover"},
        {0x10, "UC LL Rx detected CRC error"},
        {0x11, "UC LL Rx Unsupported/Undefined packet"},
        {0x12, "UC LL or Phy Control"},
        {0x13, "UC LL Rx Parameter Exception"},
        {0x1f, "UC LL Detected Control Error from M3UPI Correctable (COR)"},
        {0x20, "COR Phy Initialization Abort"},
        {0x21, "COR Phy Reset"},
        {0x22, "COR Phy Lane failure, recovery in x8 width"},
        {0x23, "COR Phy L0c error"},
        {0x25, "COR Phy L0p exit error triggering Phy Reset"},
        {0x30,
         "COR LL Rx detected CRC error: successful LLR without Phy Reinit"},
        {0x31, "COR LL Rx detected CRC error: successful LLR with Phy Reinit"},
    };

    const std::map<uint16_t, const char*> Mcacod150 = {
        {0x400, "Time-out"},
        {0x5, "Parity error on internal Mesh2mem"},
        {0x21, "Read Error"},
        {0x22, "PWR Error"},
        {0xa, "PWR Error, Error from last level of memory(pmem/block/1LM DDR)"},
        {0x9,
         "Read Error, Error from last level of memory(pmem/block/1LM DDR)"},
    };

    const std::map<uint8_t, const char*> Mcacod157 = {
        {0x4, "Near-Memory Cache controller error"},
        {0x1, "Last level memory controller error."},
    };

    const std::map<uint8_t, const char*> Mcacod64 = {
        {0x0, "Generic undefined request"},
        {0x1, "Memory Read Error"},
        {0x2, "Memory Write Error"},
        {0x3, "Address/Command Error"},
    };

    const std::map<uint8_t, const char*> Mcacod30 = {
        {0x0, "Error on channel 0"},
        {0x1, "Error on channel 1"},
        {0x2, "Error on channel 2"},
    };

    // MSCOD decode table for banks 9,10,11
    const std::map<uint16_t, const char*> MscodGeneric1 = {
        {0x1, "UNCORRECTABLE_DATA_ERROR"},
        {0x2, "UNCORRECTABLE_TAG_ERROR"},
        {0x3, "SAD_ERR_WB_TO_MMIO"},
        {0x4, "SAD_ERR_IA_ACCESS_TO_GSM"},
        {0x5, "SAD_ERR_CORRUPTING_OTHER"},
        {0x6, "SAD_ERR_NON_CORRUPTING_OTHER"},
        {0x7, "CORRECTABLE_DATA_ERROR"},
        {0x8, "MEM_POISON_DATA_ERROR"},
        {0xa, "PARITY_DATA_ERROR"},
        {0xb, "CORE_WB_MISS_LLC"},
        {0xc, "TOR_TIMEOUT"},
        {0xd, "Internal Error 0x000E "},
        {0xf, "Internal Error"},
        {0x11, "LLC_TAG_CORR_ERR"},
        {0x12, "LLC_STATE_CORR_ERR"},
        {0x13, "LLC_STATE_UNCORR_ERR"},
        {0x14, "LLC_CV_CORR_ERR"},
        {0x15, "LLC_CV_UNCORR_ERR"},
        {0x16, "Internal Protocol Error"},
        {0x17, "MULT_LLC_WAY_TAG_MATCH"},
        {0x18, "BL_REQ_RTID_TABLE_MISS"},
        {0x19, "AK_REQ_RTID_TABLE_MISS"},
        {0x1c, "IDI_JITTER_ERROR"},
        {0x1d, "Internal Parity Error 0x001E "},
        {0x1f, "Internal Parity Error"},
        {0x21, "UNCORRECTABLE_SnoopFilter_TAG_ERROR"},
        {0x22, "SnoopFilter_TAG_CORR_ERR"},
        {0x23, "SnoopFilter_STATE_CORR_ERR"},
        {0x24, "SnoopFilter_STATE_UNCORR_ERR"},
        {0x25, "SnoopFilter_CV_CORR_ERR"},
        {0x26, "SnoopFilter_CV_UNCORR_ERR"},
        {0x27, "SAD_ERR_LTMEMLOCK"},
        {0x28, "LLC_TWOLM_CORR_ERR"},
        {0x29, "LLC_TWOLM_UNCORR_ERR"},
        {0x2a, "ISMQ_UNEXP_RSP"},
        {0x2b, "TWOLM_MULT_HIT"},
        {0x2c, "HA_UNEXP_RSP"},
        {0x2d, "SAD_ERR_RRQWBQ_TO_NONHOM"},
        {0x2e, "SAD_ERR_IIOTONONHOM"},
    };

    // MSCOD decode table for banks 13,14,15,16,17,18
    const std::map<uint16_t, const char*> MscodGeneric2 = {
        {0x1, "Address Parity Error"},
        {0x2, "HA Wr data parity Error"},
        {0x4, "HA Wr BE parity Error"},
        {0x8, "Corr Patrol Scrub Error"},
        {0x10, "UnCorr Patrol Scrub Error"},
        {0x20, "Corr Spare Error"},
        {0x40, "UnCorr Spare Error"},
        {0x80, "Any Ha Rd Error"},
        {0x100, "WDB Read Parity Error"},
        {0x102, "DDRT WDB parity error"},
        {0x103, "RPQ0 parity error"},
        {0x104, "RPQ1 parity error"},
        {0x105, "WPQ parity error"},
        {0x106, "DDRT write data parity error"},
        {0x107, "DDRT write BE parity error"},
        {0x108, "DDRT link fail"},
        {0x200, "DDR4 CA Parity"},
        {0x400, "UnCorr Address Parity Error"},
        {0x800, "Unrecognized request type"},
        {0x801, "Read response to an invalid scoreboard entry"},
        {0x802, "Unexpected read response"},
        {0x803, "DDR4 completion to an invalid scoreboard entry"},
        {0x804, "Completion to an invalid scoreboard entry"},
        {0x805, "Completion FIFO overflow"},
        {0x806, "Correctable parity error"},
        {0x808, "Interrupt received while outstanding interrupt was not ACKed"},
        {0x809, "ERID FIFO overflow"},
        {0x80a, "Error on Write credits"},
        {0x80b, "Error on Read credits"},
        {0x80c, "Scheduler error"},
        {0x80d, "DDRT link retry"},
        {0x80e, "FNV thermal error"},
    };

    // Decoding rules for MCACOD for banks 9,10,11 (for exact match for whole
    // mcacod data)
    const std::map<uint16_t, const char*> McacodGeneric1 = {
        {0x136, "Error during data read (UCNA)"},
        {0x17a, "Error during Explicit Write Back (SRAO)"},
        {0x152, "Error during instruction read (UCNA)"},
    };

    union MscodDataGeneric1
    {
        struct
        {
            uint16_t mc_read_err : 1, reserved0 : 1, mc_partial_write_err : 1,
                full_write_err : 1, m2m_cdc_buf_err : 1, m2m_timeout : 1,
                m2m_tracker_parity_err : 1, bucket1_err : 1, ddr_err : 2,
                reserved1 : 6;
        };
        uint16_t mscod;
    };

    union MscodDataGeneric2
    {
        struct
        {
            uint16_t tx_parity_err : 1, ll_reset : 1, ll_reset_in_progress : 1,
                rx_header : 1, tx_flit : 1, table_err : 1, rf_parity_err : 1,
                retry_req_flit : 1, retry_ack_flit : 1, phy_ctl_err : 1,
                reserved0 : 6;
        };
        struct
        {
            uint16_t ll_err : 8, reserved1 : 8;
        };
        uint16_t mscod;
    };

    union McacodDataGeneric1
    {
        struct
        {
            uint16_t mcacod_1_0 : 2, mcacod_3_2 : 2, mcacod_7_4 : 4,
                mcacod_8_8 : 1, reserved : 7;
        };
        uint16_t mcacod_15_0;
    };

    union McacodDataGeneric2
    {
        struct
        {
            uint16_t mcacod_3_0 : 4, mcacod_6_4 : 3, mcacod_15_7 : 9;
        };
        uint16_t mcacod_15_0;
    };

    union McacodDataGeneric3
    {
        struct
        {
            uint16_t reserved0 : 2, request : 2, reserved1 : 4, timeout : 1,
                participation : 2, reserved2 : 5;
        };
        uint16_t mcacod_15_0;
    };

    union McaCodDataGeneric4
    {
        struct
        {
            uint16_t channel : 4, err : 3, imc_err : 1, reserved0 : 8;
        };
        struct
        {
            uint16_t err_specific : 8, reserved1 : 8;
        };

        uint16_t mcacod_15_0;
    };

    std::string decodeMscodGeneric1(uint16_t mscod)
    {
        std::stringstream ss;
        MscodDataGeneric1 mscodDecoded;
        mscodDecoded.mscod = mscod;

        auto ddrType = getDecoded(DdrType,
                                   static_cast<uint8_t>(mscodDecoded.ddr_err));

        if (ddrType)
        {
            ss << *ddrType << "|";
        }
        if (mscodDecoded.bucket1_err)
        {
            ss << "Bucket1 error|";
        }
        if (mscodDecoded.m2m_tracker_parity_err)
        {
            ss << "M2M tracker parity error|";
        }
        if (mscodDecoded.m2m_timeout)
        {
            ss << "M2M Timeout|";
        }
        if (mscodDecoded.m2m_cdc_buf_err)
        {
            ss << "M2M clock-domain-crossing buffer (BGF) error|";
        }
        if (mscodDecoded.full_write_err)
        {
            ss << "Full Write Data Error|";
        }
        if (mscodDecoded.mc_partial_write_err)
        {
            ss << "MC partial write data error|";
        }
        if (mscodDecoded.mc_read_err)
        {
            ss << "MC read data error";
        }

        return ss.str();
    }

    std::string decodeMscodGeneric2(uint16_t mscod)
    {
        std::stringstream ss;
        MscodDataGeneric2 mscodDecoded;
        mscodDecoded.mscod = mscod;

        if (mscodDecoded.tx_parity_err)
        {
            ss << "Link Layer Tx Parity Error|";
        }
        if (mscodDecoded.ll_reset)
        {
            ss << "Link Layer reset initiated while protocol traffic not idle|";
        }
        if (mscodDecoded.ll_reset_in_progress)
        {
            ss << "Link Layer Reset still in progress|";
        }
        if (mscodDecoded.rx_header)
        {
            ss << "Rx Header or Credit BGF credit overflow/underflow|";
        }
        if (mscodDecoded.tx_flit)
        {
            ss << "Unexpected Tx Protocol Flit (EOP, Header or Data)|";
        }
        if (mscodDecoded.table_err)
        {
            ss << "Routeback Table Error|";
        }
        if (mscodDecoded.rf_parity_err)
        {
            ss << "RF parity error|";
        }
        if (mscodDecoded.retry_req_flit)
        {
            ss << "Unexpected Retry.Req flit|";
        }
        if (mscodDecoded.retry_ack_flit)
        {
            ss << "Unexpected Retry.Ack flit|";
        }
        if (mscodDecoded.phy_ctl_err)
        {
            ss << "PHY Control Error|";
        }

        auto llErr =
            getDecoded(LlError, static_cast<uint8_t>(mscodDecoded.ll_err));
        if (llErr)
        {
            ss << *llErr;
        }

        return ss.str();
    }

    std::string decodeMcacodGeneric1(uint16_t mcacod)
    {
        std::stringstream ss;
        McacodDataGeneric1 mcacodDecoded;
        mcacodDecoded.mcacod_15_0 = mcacod;

        auto errorLevelEncoding =
            getDecoded(ErrorLevelEncoding,
                        static_cast<uint8_t>(mcacodDecoded.mcacod_1_0));
        auto errorTransactionType =
            getDecoded(ErrorTransactionType,
                        static_cast<uint8_t>(mcacodDecoded.mcacod_3_2));
        auto errorRequestType =
            getDecoded(ErrorRequestType,
                        static_cast<uint8_t>(mcacodDecoded.mcacod_7_4));

        if (mcacodDecoded.mcacod_8_8 == 1)
        {
            ss << "Cache_Hierarchy_Error|";
        }

        ss << *errorRequestType << "|" << *errorTransactionType << "|"
           << *errorLevelEncoding;

        return ss.str();
    }

    std::string decodeMcacodGeneric2(uint16_t mcacod)
    {
        std::stringstream ss;
        McacodDataGeneric2 mcacodDecoded;
        mcacodDecoded.mcacod_15_0 = mcacod;

        auto mcacod150 = getDecoded(
            Mcacod150, static_cast<uint16_t>(mcacodDecoded.mcacod_15_0));
        auto mcacod157 = getDecoded(
            Mcacod157, static_cast<uint8_t>(mcacodDecoded.mcacod_15_7));
        auto mcacod64 = getDecoded(
            Mcacod64, static_cast<uint8_t>(mcacodDecoded.mcacod_6_4));
        auto mcacod30 = getDecoded(
            Mcacod30, static_cast<uint8_t>(mcacodDecoded.mcacod_3_0));

        if (mcacod150)
        {
            ss << *mcacod150 << "|";
        }
        if (mcacod157)
        {
            ss << *mcacod157 << "|";
        }
        if (mcacod64)
        {
            ss << *mcacod64 << "|";
        }
        if (mcacod30)
        {
            ss << *mcacod30;
        }

        return ss.str();
    }

    std::string decodeMcacodGeneric3(uint16_t mcacod)
    {
        std::stringstream ss;
        McaCodDataGeneric4 mcacodDecoded;
        mcacodDecoded.mcacod_15_0 = mcacod;

        auto channel =
            getDecoded(Channel, static_cast<uint8_t>(mcacodDecoded.channel));
        auto error =
            getDecoded(Error, static_cast<uint8_t>(mcacodDecoded.err));
        auto errorSpecific = getDecoded(ErrSpecific,
            static_cast<uint8_t>(mcacodDecoded.err_specific));

        if (mcacodDecoded.imc_err == 1)
        {
            ss << "IMC_Error|";
        }
        if (errorSpecific)
        {
            ss << *errorSpecific << "|";
        }
        if (error)
        {
            ss << *error << "|";
        }
        if (channel)
        {
            ss << *channel;
        }

        return ss.str();
    }

    std::string decodeMcacodGeneric4(uint16_t mcacod)
    {
        std::stringstream ss;
        McacodDataGeneric3 mcacodDecoded;
        mcacodDecoded.mcacod_15_0 = mcacod;

        auto request = getDecoded(Request,
            static_cast<uint8_t>(mcacodDecoded.request));
        auto participation = getDecoded(Participation,
            static_cast<uint8_t>(mcacodDecoded.participation));

        if (request)
        {
            ss << *request << "|";
        }
        if (mcacodDecoded.timeout == 1)
        {
            ss << "Timeout|";
        }
        if (participation)
        {
            ss << *participation;
        }

        return ss.str();
    }

    std::string decodeBankName(uint32_t mcaBank)
    {
        std::stringstream ss;
        ss << mcaBank;
        auto bankNameDecoded =
            getDecoded(BankNames, static_cast<uint8_t>(mcaBank));
        if (bankNameDecoded)
        {
            ss << " (" << *bankNameDecoded << ")";
        }

        return ss.str();
    }
};

class CpxMcaBank0 final : public CpxMcaDecoder
{
  private:
    union MC_STATUS
    {
        struct
        {
            uint64_t mcacod : 16, mscod : 16, reserved0 : 6,
                corrected_error_count : 15, corr_err_status_ind : 2, ar : 1,
                s : 1, pcc : 1, addrv : 1, miscv : 1, en : 1, uc : 1,
                overflow : 1, valid : 1;
        };
        uint64_t mc_status;
    };

    const std::map<uint16_t, const char*> decodeMscod = {
        {0x0, "Internal Error"},
        {0x1, "Internal Error"},
        {0x2, "Internal Error"},
        {0x3, "Internal Error"},
        {0x4, "IFU Icache Data Parity/Poison Error"},
        {0x5, "IFU Icache Tag Parity Error"},
        {0x6, "ITLB Parity Error"},
        {0x7, "Internal Error"},
        {0x8, "Internal Error"},
        {0x9, "Internal Error"},
        {0xa, "Internal Error"},
        {0xb, "Internal Error"},
        {0xc, "Parity Error on MLC data"},
        {0xd, "Internal Error"},
        {0xe, "Internal Error"},
        {0xf, "Internal Error"},
    };

    const std::map<uint16_t, const char*> decodeMcacod = {
        {0x5, "IFU Internal Error"},
        {0x150, "IFU Icache Data Parity/Data Posion/Tag Parity Error"},
        {0x10, "ITLB Parity error"},
    };

    MC_STATUS status_decoded;
    json entry;

    void decodeKnownFields()
    {
        auto corrErrStsIndDecoded = getDecoded(CorrErrStsInd,
            static_cast<uint8_t>(status_decoded.corr_err_status_ind));
        if (corrErrStsIndDecoded)
        {
            entry["CORR_ERR_STATUS_IND_decoded"] = *corrErrStsIndDecoded;
        }

        auto mscodDecoded = getDecoded(decodeMscod,
            static_cast<uint16_t>(status_decoded.mscod));
        if (mscodDecoded)
        {
            entry["MSCOD_decoded"] = *mscodDecoded;
        }

        auto mcacodDecoded = getDecoded(decodeMcacod,
            static_cast<uint16_t>(status_decoded.mcacod));
        if (mcacodDecoded)
        {
            entry["MCACOD_decoded"] = *mcacodDecoded;
        }
        else
        {
            entry["MCACOD_decoded"] =
                decodeMcacodGeneric1(status_decoded.mcacod);
        }
    }

  public:
    CpxMcaBank0(const MCAData& mca) : CpxMcaDecoder(mca)
    {
        status_decoded.mc_status = mca.mc_status;
    }

    json decode_status() override
    {
        entry["MCACOD"] =
            int_to_hex(static_cast<uint16_t>(status_decoded.mcacod));
        entry["MSCOD"] =
            int_to_hex(static_cast<uint16_t>(status_decoded.mscod));
        entry["RESERVED"] =
            int_to_hex(static_cast<uint8_t>(status_decoded.reserved0));
        entry["CORRECTED_ERROR_COUNT"] = int_to_hex(
            static_cast<uint16_t>(status_decoded.corrected_error_count));
        entry["CORR_ERR_STATUS_IND"] = int_to_hex(
            static_cast<uint8_t>(status_decoded.corr_err_status_ind));
        entry["AR"] = int_to_hex(static_cast<bool>(status_decoded.ar));
        entry["S"] = int_to_hex(static_cast<bool>(status_decoded.s));
        entry["PCC"] = int_to_hex(static_cast<bool>(status_decoded.pcc));
        entry["ADDRV"] = int_to_hex(static_cast<bool>(status_decoded.addrv));
        entry["MISCV"] = int_to_hex(static_cast<bool>(status_decoded.miscv));
        entry["EN"] = int_to_hex(static_cast<bool>(status_decoded.en));
        entry["UC"] = int_to_hex(static_cast<bool>(status_decoded.uc));
        entry["OVERFLOW"] =
            int_to_hex(static_cast<bool>(status_decoded.overflow));
        entry["VALID"] = int_to_hex(static_cast<bool>(status_decoded.valid));
        decodeKnownFields();

        return entry;
    }
};

class CpxMcaBank1 final : public CpxMcaDecoder
{
  private:
    union MC_STATUS
    {
        struct
        {
            uint64_t mcacod : 16, mscod : 22, corrected_error_count : 15,
                corr_err_status_ind : 2, ar : 1, s : 1, pcc : 1, addrv : 1,
                miscv : 1, en : 1, uc : 1, overflow : 1, valid : 1;
        };
        uint64_t mc_status;
    };

    const std::map<uint32_t, const char*> decodeMscod = {
        {0x0, "Non-APIC Error"},
        {0x10, "WBINVD orPoison SRAR Error based on MCACOD values"},
        {0x20, "APIC Error"},
    };

    const std::map<uint16_t, const char*> decodeMcacod = {
        {0x174, "Eviction Error (e.g. WBINV)"},
        {0x134, "L1 Data Read Error"},
        {0x144, "L1 Data Store Error"},
        {0x164, "L1 Data Prefetch Error"},
        {0x184, "L1 Data Snoop Error"},
    };

    MC_STATUS status_decoded;
    json entry;

    void decodeKnownFields()
    {
        auto corrErrStsIndDecoded = getDecoded(CorrErrStsInd,
            static_cast<uint8_t>(status_decoded.corr_err_status_ind));
        if (corrErrStsIndDecoded)
        {
            entry["CORR_ERR_STS_IND_decoded"] = *corrErrStsIndDecoded;
        }

        auto mscodDecoded = getDecoded(
            decodeMscod, static_cast<uint32_t>(status_decoded.mscod));
        if (mscodDecoded)
        {
            entry["MSCOD_decoded"] = *mscodDecoded;
        }

        auto mcacodDecoded = getDecoded(
            decodeMcacod, static_cast<uint16_t>(status_decoded.mcacod));
        if (mcacodDecoded)
        {
            entry["MCACOD_decoded"] = *mcacodDecoded;
        }
        else
        {
            entry["MCACOD_decoded"] =
                decodeMcacodGeneric1(status_decoded.mcacod);
        }
    }

  public:
    CpxMcaBank1(const MCAData& mca) : CpxMcaDecoder(mca)
    {
        status_decoded.mc_status = mca.mc_status;
    }

    json decode_status() override
    {
        entry["MCACOD"] =
            int_to_hex(static_cast<uint16_t>(status_decoded.mcacod));
        entry["MSCOD"] =
            int_to_hex(static_cast<uint32_t>(status_decoded.mscod));
        entry["CORRECTED_ERROR_COUNT"] = int_to_hex(
            static_cast<uint16_t>(status_decoded.corrected_error_count));
        entry["CORR_ERR_STATUS_IND"] = int_to_hex(
            static_cast<uint8_t>(status_decoded.corr_err_status_ind));
        entry["AR"] = int_to_hex(static_cast<bool>(status_decoded.ar));
        entry["S"] = int_to_hex(static_cast<bool>(status_decoded.s));
        entry["PCC"] = int_to_hex(static_cast<bool>(status_decoded.pcc));
        entry["ADDRV"] = int_to_hex(static_cast<bool>(status_decoded.addrv));
        entry["MISCV"] = int_to_hex(static_cast<bool>(status_decoded.miscv));
        entry["EN"] = int_to_hex(static_cast<bool>(status_decoded.en));
        entry["UC"] = int_to_hex(static_cast<bool>(status_decoded.uc));
        entry["OVERFLOW"] =
            int_to_hex(static_cast<bool>(status_decoded.overflow));
        entry["VALID"] = int_to_hex(static_cast<bool>(status_decoded.valid));
        decodeKnownFields();

        return entry;
    }
};

class CpxMcaBank2 final : public CpxMcaDecoder
{
  private:
    union MC_STATUS
    {
        struct
        {
            uint64_t mcacod : 16, mscod : 16, reserved0 : 6,
                corrected_error_count : 15, reserved1 : 4, pcc : 1, addrv : 1,
                miscv : 1, en : 1, uc : 1, overflow : 1, valid : 1;
        };
        uint64_t mc_status;
    };

    union Mcacod
    {
        struct
        {
            uint16_t tlb_err_ll : 2, tlb_err_tt : 2, tlb_err : 1, reserved : 11;
        };
        uint16_t mcacod_15_0;
    };

    const std::map<uint16_t, const char*> decodeMscod = {
        {0x0, "Tag Parity Error"},
        {0x1, "Data Parity Error"},
        {0x10, "Tag Parity Error"},
        {0x11, "Data Parity Error"},
    };

    const std::map<uint16_t, const char*> decodeMcacod = {
        {0x14, "DTLB error code"},
    };

    const std::map<uint8_t, const char*> decodeTlbErrTt = {
        {0x0, "Instruction"},
        {0x1, "Data"},
        {0x2, "Generic"},
        {0x3, "Reserved"},
    };

    const std::map<uint8_t, const char*> decodeTlbErrLl = {
        {0x0, "Level 0"},
        {0x1, "Level 1"},
        {0x2, "Level 2"},
        {0x3, "Generic"},
    };

    MC_STATUS status_decoded;
    json entry;

    std::string decodedMcacod(uint16_t mcacod)
    {
        std::stringstream ss;
        Mcacod mcacodDecoded;
        mcacodDecoded.mcacod_15_0 = mcacod;

        auto tlbErrLl = getDecoded(
            decodeTlbErrLl, static_cast<uint8_t>(mcacodDecoded.tlb_err_ll));
        auto tlbErrTt = getDecoded(
            decodeTlbErrTt, static_cast<uint8_t>(mcacodDecoded.tlb_err_tt));

        if (mcacodDecoded.tlb_err == 1)
        {
            ss << "TLB_ERR|";
        }

        ss << *tlbErrTt << "|" << *tlbErrLl;

        return ss.str();
    }

    void decodeKnownFields()
    {
        auto mscodDecoded = getDecoded(
            decodeMscod, static_cast<uint16_t>(status_decoded.mscod));
        if (mscodDecoded)
        {
            entry["MSCOD_decoded"] = *mscodDecoded;
        }

        auto mcacodDecoded = getDecoded(
            decodeMcacod, static_cast<uint16_t>(status_decoded.mcacod));
        if (mcacodDecoded)
        {
            entry["MCACOD_decoded"] = *mcacodDecoded;
        }
        else
        {
            entry["MCACOD_decoded"] = decodedMcacod(status_decoded.mcacod);
        }
    }

  public:
    CpxMcaBank2(const MCAData& mca) : CpxMcaDecoder(mca)
    {
        status_decoded.mc_status = mca.mc_status;
    }

    json decode_status() override
    {
        entry["MCACOD"] =
            int_to_hex(static_cast<uint16_t>(status_decoded.mcacod));
        entry["MSCOD"] =
            int_to_hex(static_cast<uint16_t>(status_decoded.mscod));
        entry["RESERVED0"] =
            int_to_hex(static_cast<uint8_t>(status_decoded.reserved0));
        entry["CORRECTED_ERROR_COUNT"] = int_to_hex(
            static_cast<uint16_t>(status_decoded.corrected_error_count));
        entry["RESERVED1"] =
            int_to_hex(static_cast<uint8_t>(status_decoded.reserved1));
        entry["PCC"] = int_to_hex(static_cast<bool>(status_decoded.pcc));
        entry["ADDRV"] = int_to_hex(static_cast<bool>(status_decoded.addrv));
        entry["MISCV"] = int_to_hex(static_cast<bool>(status_decoded.miscv));
        entry["EN"] = int_to_hex(static_cast<bool>(status_decoded.en));
        entry["UC"] = int_to_hex(static_cast<bool>(status_decoded.uc));
        entry["OVERFLOW"] =
            int_to_hex(static_cast<bool>(status_decoded.overflow));
        entry["VALID"] = int_to_hex(static_cast<bool>(status_decoded.valid));
        decodeKnownFields();

        return entry;
    }
};

class CpxMcaBank3 final : public CpxMcaDecoder
{
  private:
    union MC_STATUS
    {
        struct
        {
            uint64_t mcacod : 16, mscod : 16, reserved0 : 6,
                corrected_error_count : 15, corr_err_status_ind : 2,
                reserved1 : 2, pcc : 1, addrv : 1, miscv : 1, en : 1, uc : 1,
                overflow : 1, valid : 1;
        };
        uint64_t mc_status;
    };

    const std::map<uint16_t, const char*> decodeMscod = {
        {0x80, "MLC Watchdog timer (3 strike) Error"},
        {0x30, "Poisoned Data detected"},
        {0x20, "Data Read Uncorrected error / prefetch Error"},
        {0x10, "Data Read Corrected error / prefetch Error"},
        {0x8, "MESI State Uncorrected Error"},
        {0x4, "MESI State Corrected Error"},
        {0x2, "Tag Uncorrected Error"},
        {0x1, "Tag Corrected Error"},
    };

    const std::map<uint16_t, const char*> decodeMcacod = {
        {0x400, "Watchdog Timer (3-strike timeout) Error"},
        {0x165, "Data Prefetch Errors"},
        {0x135, "Data Read Errors"},
        {0x109, "Tag/MESI State Error"},
    };

    MC_STATUS status_decoded;
    json entry;

    void decodeKnownFields()
    {
        auto corrErrStsIndDecoded = getDecoded(CorrErrStsInd,
            static_cast<uint8_t>(status_decoded.corr_err_status_ind));
        if (corrErrStsIndDecoded)
        {
            entry["CORR_ERR_STATUS_IND_decoded"] = *corrErrStsIndDecoded;
        }

        auto mscodDecoded = getDecoded(
            decodeMscod, static_cast<uint16_t>(status_decoded.mscod));
        if (mscodDecoded)
        {
            entry["MSCOD_decoded"] = *mscodDecoded;
        }

        auto mcacodDecoded = getDecoded(
            decodeMcacod, static_cast<uint16_t>(status_decoded.mcacod));
        if (mcacodDecoded)
        {
            entry["MCACOD_decoded"] = *mcacodDecoded;
        }
        else
        {
            entry["MCACOD_decoded"] =
                decodeMcacodGeneric1(status_decoded.mcacod);
        }
    }

  public:
    CpxMcaBank3(const MCAData& mca) : CpxMcaDecoder(mca)
    {
        status_decoded.mc_status = mca.mc_status;
    }

    json decode_status() override
    {
        entry["MCACOD"] =
            int_to_hex(static_cast<uint16_t>(status_decoded.mcacod));
        entry["MSCOD"] =
            int_to_hex(static_cast<uint16_t>(status_decoded.mscod));
        entry["RESERVED0"] =
            int_to_hex(static_cast<uint8_t>(status_decoded.reserved0));
        entry["CORRECTED_ERROR_COUNT"] = int_to_hex(
            static_cast<uint16_t>(status_decoded.corrected_error_count));
        entry["CORR_ERR_STATUS_IND"] = int_to_hex(
            static_cast<uint8_t>(status_decoded.corr_err_status_ind));
        entry["RESERVED1"] =
            int_to_hex(static_cast<uint8_t>(status_decoded.reserved1));
        entry["PCC"] = int_to_hex(static_cast<bool>(status_decoded.pcc));
        entry["ADDRV"] = int_to_hex(static_cast<bool>(status_decoded.addrv));
        entry["MISCV"] = int_to_hex(static_cast<bool>(status_decoded.miscv));
        entry["EN"] = int_to_hex(static_cast<bool>(status_decoded.en));
        entry["UC"] = int_to_hex(static_cast<bool>(status_decoded.uc));
        entry["OVERFLOW"] =
            int_to_hex(static_cast<bool>(status_decoded.overflow));
        entry["VALID"] = int_to_hex(static_cast<bool>(status_decoded.valid));
        decodeKnownFields();

        return entry;
    }
};

class CpxMcaBank4 final : public CpxMcaDecoder
{
  private:
    union MC_STATUS
    {
        struct
        {
            uint64_t mcacod : 16, mscod_16_19 : 4, mscod_20_23 : 4,
                mscod_24_31 : 8, enh_mca_avail0 : 6, corrected_error_count : 15,
                corr_err_status_ind : 2, enh_mca_avail1 : 2, pcc : 1, addrv : 1,
                miscv : 1, en : 1, uc : 1, overflow : 1, valid : 1;
        };
        uint64_t mc_status;
    };

    const std::map<uint16_t, const char*> decodeMcacod = {
        {0x402, "PCU"},
        {0x403, "VCU"},
        {0x406, "Internal firmware errors"},
        {0x407, "Other UBOX errors"},
    };

    const std::map<uint8_t, const char*> decodeMscod1619 = {
        {0x1, "Instruction address out of valid space"},
        {0x2, "Double bit RAM error on Instruction Fetch"},
        {0x3, "Invalid Opcode seen"},
        {0x4, "Stack Underflow"},
        {0x5, "Stack Overflow"},
        {0x6, "Data address out of valid space"},
        {0x7, "Double bit RAM error on Data Fetch"},
    };

    const std::map<uint8_t, const char*> decodeMscod2023 = {
        {0x3, "Trusted Path Error"},
        {0x4, "Other Ubox error"},
    };

    const std::map<uint8_t, const char*> decodeMscod2431 = {
        {0xe, "MCA_DMI_TRAINING_TIMEOUT"},
        {0xf, "MCA_DMI_STRAP_SET_ARRIVAL_TIMEOUT"},
        {0x10, "MCA_DMI_CPU_RESET_ACK_TIMEOUT"},
        {0x11, "MCA_MORE_THAN_ONE_LT_AGENT"},
        {0x12, "MCA_MEMORY_RCOMP_TIMEOUT"},
        {0x14, "MCA_INCOMPATIBLE_PCH_TYPE"},
        {0x1e, "MCA_BIOS_RST_CPL_INVALID_SEQ"},
        {0x1f, "MCA_BIOS_INVALID_PKG_STATE_CONFIG"},
        {0x25, "MCA_MESSAGE_CHANNEL_TIMEOUT"},
        {0x27, "MCA_MSGCH_PMREQ_CMP_TIMEOUT"},
        {0x30, "MCA_PKGC_DIRECT_WAKE_RING_TIMEOUT"},
        {0x31, "MCA_PKGC_INVALID_RSP_PCH"},
        {0x33, "MCA_PKGC_WATCHDOG_HANG_CBZ_DOWN"},
        {0x34, "MCA_PKGC_WATCHDOG_HANG_CBZ_UP"},
        {0x38, "MCA_PKGC_WATCHDOG_HANG_C3_UP_SF"},
        {0x40, "MCA_SVID_VCCIN_VR_ICC_MAX_FAILURE"},
        {0x41, "MCA_SVID_COMMAND_TIMEOUT"},
        {0x42, "MCA_SVID_VCCIN_VR_VOUT_FAILURE"},
        {0x43, "MCA_SVID_CPU_VR_CAPABILITY_ERROR"},
        {0x44, "MCA_SVID_CRITICAL_VR_FAILED"},
        {0x45, "MCA_SVID_SA_ITD_ERROR"},
        {0x46, "MCA_SVID_READ_REG_FAILED"},
        {0x47, "MCA_SVID_WRITE_REG_FAILED"},
        {0x48, "MCA_SVID_PKGC_INIT_FAILED"},
        {0x49, "MCA_SVID_PKGC_CONFIG_FAILED"},
        {0x4a, "MCA_SVID_PKGC_REQUEST_FAILED"},
        {0x4b, "MCA_SVID_IMON_REQUEST_FAILED"},
        {0x4c, "MCA_SVID_ALERT_REQUEST_FAILED"},
        {0x4d, "MCA_SVID_MCP_VR_RAMP_ERROR"},
        {0x4e, "MCA_MEMORY_CAPACITY_EXCEEDED"},
        {0x51, "MCA_FIVR_CATAS_OVERVOL_FAULT"},
        {0x52, "MCA_FIVR_CATAS_OVERCUR_FAULT"},
        {0x58, "MCA_WATCHDOG_TIMEOUT_PKGC_SLAVE"},
        {0x59, "MCA_WATCHDOG_TIMEOUT_PKGC_MASTER"},
        {0x5a, "MCA_WATCHDOG_TIMEOUT_PKGS_MASTER"},
        {0x61, "MCA_PKGS_CPD_UNCPD_TIMEOUT"},
        {0x63, "MCA_PKGS_INVALID_REQ_PCH"},
        {0x64, "MCA_PKGS_INVALID_REQ_INTERNAL"},
        {0x65, "MCA_PKGS_INVALID_RSP_INTERNAL"},
        {0x6b, "MCA_PKGS_SMBUS_VPP_PAUSE_TIMEOUT"},
    };

    MC_STATUS status_decoded;
    json entry;

    void decodeKnownFields()
    {
        auto corrErrStsIndDecoded = getDecoded(CorrErrStsInd,
            static_cast<uint8_t>(status_decoded.corr_err_status_ind));
        if (corrErrStsIndDecoded)
        {
            entry["CORR_ERR_STATUS_IND_decoded"] = *corrErrStsIndDecoded;
        }

        auto mscod1619Decoded = getDecoded(
            decodeMscod1619, static_cast<uint8_t>(status_decoded.mscod_16_19));
        if (mscod1619Decoded)
        {
            entry["MSCOD_16_19_decoded"] = *mscod1619Decoded;
        }

        auto mscod2023Decoded = getDecoded(
            decodeMscod2023, static_cast<uint8_t>(status_decoded.mscod_20_23));
        if (mscod2023Decoded)
        {
            entry["MSCOD_20_23_decoded"] = *mscod2023Decoded;
        }

        auto mscod2431Decoded = getDecoded(
            decodeMscod2431, static_cast<uint8_t>(status_decoded.mscod_24_31));
        if (mscod2431Decoded)
        {
            entry["MSCOD_24_31_decoded"] = *mscod2431Decoded;
        }
        else
        {
            entry["MSCOD_24_31_decoded"] = "Internal error";
        }

        auto mcacod = getDecoded(
            decodeMcacod, static_cast<uint16_t>(status_decoded.mcacod));
        if (mcacod)
        {
            entry["MCACOD_decoded"] = *mcacod;
        }
    }

  public:
    CpxMcaBank4(const MCAData& mca) : CpxMcaDecoder(mca)
    {
        status_decoded.mc_status = mca.mc_status;
    }

    json decode_status() override
    {
        entry["MCACOD"] =
            int_to_hex(static_cast<uint16_t>(status_decoded.mcacod));
        entry["MSCOD_16_19"] =
            int_to_hex(static_cast<uint8_t>(status_decoded.mscod_16_19));
        entry["MSCOD_20_23"] =
            int_to_hex(static_cast<uint8_t>(status_decoded.mscod_20_23));
        entry["MSCOD_24_31"] =
            int_to_hex(static_cast<uint8_t>(status_decoded.mscod_24_31));
        entry["ENH_MCA_AVAIL0"] =
            int_to_hex(static_cast<uint8_t>(status_decoded.enh_mca_avail0));
        entry["CORRECTED_ERROR_COUNT"] = int_to_hex(
            static_cast<uint16_t>(status_decoded.corrected_error_count));
        entry["CORR_ERR_STATUS"] = int_to_hex(
            static_cast<uint8_t>(status_decoded.corr_err_status_ind));
        entry["ENH_MCA_AVAIL1"] =
            int_to_hex(static_cast<uint8_t>(status_decoded.enh_mca_avail1));
        entry["PCC"] = int_to_hex(static_cast<bool>(status_decoded.pcc));
        entry["ADDRV"] = int_to_hex(static_cast<bool>(status_decoded.addrv));
        entry["MISCV"] = int_to_hex(static_cast<bool>(status_decoded.miscv));
        entry["EN"] = int_to_hex(static_cast<bool>(status_decoded.en));
        entry["UC"] = int_to_hex(static_cast<bool>(status_decoded.uc));
        entry["OVERFLOW"] =
            int_to_hex(static_cast<bool>(status_decoded.overflow));
        entry["VALID"] = int_to_hex(static_cast<bool>(status_decoded.valid));
        decodeKnownFields();

        return entry;
    }
};

class CpxMcaBank5 final : public CpxMcaDecoder
{
  private:
    union MC_STATUS
    {
        struct
        {
            uint64_t mcacod : 16, mscod_16_21 : 6, mscod_22_31 : 10,
                other_info : 6, corrected_error_count : 15,
                corr_err_status_ind : 2, ar : 1, s : 1, pcc : 1, addrv : 1,
                miscv : 1, en : 1, uc : 1, overflow : 1, valid : 1;
        };
        uint64_t mc_status;
    };

    MC_STATUS status_decoded;
    json entry;

    void decodeKnownFields()
    {
        auto corrErrStsIndDecoded = getDecoded(CorrErrStsInd,
            static_cast<uint8_t>(status_decoded.corr_err_status_ind));
        if (corrErrStsIndDecoded)
        {
            entry["CORR_ERR_STATUS_IND_decoded"] = *corrErrStsIndDecoded;
        }

        auto mscod1621Decoded = getDecoded(
            Mscod1621, static_cast<uint8_t>(status_decoded.mscod_16_21));
        if (mscod1621Decoded)
        {
            entry["MSCOD_16_21_decoded"] = *mscod1621Decoded;
        }

        auto mscod2231Decoded = getDecoded(
            Mscod2231, static_cast<uint16_t>(status_decoded.mscod_22_31));
        if (mscod2231Decoded)
        {
            entry["MCACOD_22_31_decoded"] = *mscod2231Decoded;
        }

        auto mcacodDecoded = getDecoded(
            Mcacod, static_cast<uint16_t>(status_decoded.mcacod));
        if (mcacodDecoded)
        {
            entry["MCACOD_decoded"] = *mcacodDecoded;
        }
        else
        {
            entry["MCACOD_decoded"] =
                decodeMcacodGeneric4(status_decoded.mcacod);
        }
    }

  public:
    CpxMcaBank5(const MCAData& mca) : CpxMcaDecoder(mca)
    {
        status_decoded.mc_status = mca.mc_status;
    }

    json decode_status() override
    {
        entry["MCACOD"] =
            int_to_hex(static_cast<uint16_t>(status_decoded.mcacod));
        entry["MSCOD_16_21"] =
            int_to_hex(static_cast<uint8_t>(status_decoded.mscod_16_21));
        entry["MSCOD_22_31"] =
            int_to_hex(static_cast<uint16_t>(status_decoded.mscod_22_31));
        entry["OTHER_INFO"] =
            int_to_hex(static_cast<uint8_t>(status_decoded.other_info));
        entry["CORRECTED_ERROR_COUNT"] = int_to_hex(
            static_cast<uint16_t>(status_decoded.corrected_error_count));
        entry["CORR_ERR_STATUS_IND"] = int_to_hex(
            static_cast<uint8_t>(status_decoded.corr_err_status_ind));
        entry["AR"] = int_to_hex(static_cast<bool>(status_decoded.ar));
        entry["S"] = int_to_hex(static_cast<bool>(status_decoded.s));
        entry["PCC"] = int_to_hex(static_cast<bool>(status_decoded.pcc));
        entry["ADDRV"] = int_to_hex(static_cast<bool>(status_decoded.addrv));
        entry["MISCV"] = int_to_hex(static_cast<bool>(status_decoded.miscv));
        entry["EN"] = int_to_hex(static_cast<bool>(status_decoded.en));
        entry["UC"] = int_to_hex(static_cast<bool>(status_decoded.uc));
        entry["OVERFLOW"] =
            int_to_hex(static_cast<bool>(status_decoded.overflow));
        entry["VALID"] = int_to_hex(static_cast<bool>(status_decoded.valid));
        decodeKnownFields();

        return entry;
    }
};

class CpxMcaBank6 final : public CpxMcaDecoder
{
  private:
    union MC_STATUS
    {
        struct
        {
            uint64_t mcacod : 16, mscod : 16, other_info : 6, corr_err_cnt : 15,
                corr_err_status_ind : 2, ar : 1, s : 1, pcc : 1, addrv : 1,
                miscv : 1, en : 1, uc : 1, overflow : 1, valid : 1;
        };
        uint64_t mc_status;
    };

    const std::map<uint16_t, const char*> decodeMscod = {
        {0x0, "This value is always 0"},
    };

    const std::map<uint16_t, const char*> decodeMcacod = {
        {0xe0b, "Generic I/O Error"},
        {0x405, "Generic I/O Error"},
    };

    MC_STATUS status_decoded;
    json entry;

    void decodeKnownFields()
    {
        auto corrErrStsIndDecoded = getDecoded(
            CorrErrStsInd,
            static_cast<uint8_t>(status_decoded.corr_err_status_ind));
        if (corrErrStsIndDecoded)
        {
            entry["CORR_ERR_STATUS_IND_decoded"] = *corrErrStsIndDecoded;
        }

        auto mscodDecoded = getDecoded(
            decodeMscod, static_cast<uint16_t>(status_decoded.mscod));
        if (mscodDecoded)
        {
            entry["MSCOD_decoded"] = *mscodDecoded;
        }

        auto mcacodDecoded = getDecoded(
            decodeMcacod, static_cast<uint16_t>(status_decoded.mcacod));
        if (mcacodDecoded)
        {
            entry["MCACOD_decoded"] = *mcacodDecoded;
        }
    }

  public:
    CpxMcaBank6(const MCAData& mca) : CpxMcaDecoder(mca)
    {
        status_decoded.mc_status = mca.mc_status;
    }

    json decode_status() override
    {
        entry["MCACOD"] =
            int_to_hex(static_cast<uint16_t>(status_decoded.mcacod));
        entry["MSCOD"] =
            int_to_hex(static_cast<uint16_t>(status_decoded.mscod));
        entry["OTHER_INFO"] =
            int_to_hex(static_cast<uint8_t>(status_decoded.other_info));
        entry["CORR_ERR_CNT"] =
            int_to_hex(static_cast<uint16_t>(status_decoded.corr_err_cnt));
        entry["CORR_ERR_STATUS_IND"] = int_to_hex(
            static_cast<uint8_t>(status_decoded.corr_err_status_ind));
        entry["AR"] = int_to_hex(static_cast<bool>(status_decoded.ar));
        entry["S"] = int_to_hex(static_cast<bool>(status_decoded.s));
        entry["PCC"] = int_to_hex(static_cast<bool>(status_decoded.pcc));
        entry["ADDRV"] = int_to_hex(static_cast<bool>(status_decoded.addrv));
        entry["MISCV"] = int_to_hex(static_cast<bool>(status_decoded.miscv));
        entry["EN"] = int_to_hex(static_cast<bool>(status_decoded.en));
        entry["UC"] = int_to_hex(static_cast<bool>(status_decoded.uc));
        entry["OVERFLOW"] =
            int_to_hex(static_cast<bool>(status_decoded.overflow));
        entry["VALID"] = int_to_hex(static_cast<bool>(status_decoded.valid));
        decodeKnownFields();

        return entry;
    }
};

class CpxMcaBank7 final : public CpxMcaDecoder
{
  private:
    union MC_STATUS
    {
        struct
        {
            uint64_t mcacod : 16, mscod : 16, other_info : 6,
                corrected_error_count : 15, corr_err_status_ind : 2, ar : 1,
                s : 1, pcc : 1, addrv : 1, miscv : 1, en : 1, uc : 1,
                overflow : 1, valid : 1;
        };
        uint64_t mc_status;
    };

    MC_STATUS status_decoded;
    json entry;

    void decodeKnownFields()
    {
        auto corrErrStsIndDecoded = getDecoded(CorrErrStsInd,
            static_cast<uint8_t>(status_decoded.corr_err_status_ind));
        if (corrErrStsIndDecoded)
        {
            entry["CORR_ERR_STATUS_IND_decoded"] = *corrErrStsIndDecoded;
        }

        entry["MSCOD_decoded"] = decodeMscodGeneric1(status_decoded.mscod);
        entry["MCACOD_decoded"] = decodeMcacodGeneric2(status_decoded.mcacod);
    }

  public:
    CpxMcaBank7(const MCAData& mca) : CpxMcaDecoder(mca)
    {
        status_decoded.mc_status = mca.mc_status;
    }

    json decode_status() override
    {
        entry["MCACOD"] =
            int_to_hex(static_cast<uint16_t>(status_decoded.mcacod));
        entry["MSCOD"] =
            int_to_hex(static_cast<uint16_t>(status_decoded.mscod));
        entry["OTHER_INFO"] =
            int_to_hex(static_cast<uint8_t>(status_decoded.other_info));
        entry["CORRECTED_ERROR_COUNT"] = int_to_hex(
            static_cast<uint16_t>(status_decoded.corrected_error_count));
        entry["CORR_ERR_STATUS_IND"] = int_to_hex(
            static_cast<uint8_t>(status_decoded.corr_err_status_ind));
        entry["AR"] = int_to_hex(static_cast<bool>(status_decoded.ar));
        entry["S"] = int_to_hex(static_cast<bool>(status_decoded.s));
        entry["PCC"] = int_to_hex(static_cast<bool>(status_decoded.pcc));
        entry["ADDRV"] = int_to_hex(static_cast<bool>(status_decoded.addrv));
        entry["MISCV"] = int_to_hex(static_cast<bool>(status_decoded.miscv));
        entry["EN"] = int_to_hex(static_cast<bool>(status_decoded.en));
        entry["UC"] = int_to_hex(static_cast<bool>(status_decoded.uc));
        entry["OVERFLOW"] =
            int_to_hex(static_cast<bool>(status_decoded.overflow));
        entry["VALID"] = int_to_hex(static_cast<bool>(status_decoded.valid));
        decodeKnownFields();

        return entry;
    }
};

class CpxMcaBank8 final : public CpxMcaDecoder
{
  private:
    union MC_STATUS
    {
        struct
        {
            uint64_t mcacod : 16, mscod : 16, other_info : 6,
                corrected_error_count : 15, corr_err_status_ind : 2, ar : 1,
                s : 1, pcc : 1, addrv : 1, miscv : 1, en : 1, uc : 1,
                overflow : 1, valid : 1;
        };
        uint64_t mc_status;
    };

    MC_STATUS status_decoded;
    json entry;

    void decodeKnownFields()
    {
        auto corrErrStsIndDecoded = getDecoded(CorrErrStsInd,
            static_cast<uint8_t>(status_decoded.corr_err_status_ind));
        if (corrErrStsIndDecoded)
        {
            entry["CORR_ERR_STATUS_IND_decoded"] = *corrErrStsIndDecoded;
        }

        entry["MSCOD_decoded"] = decodeMscodGeneric1(status_decoded.mscod);
        entry["MCACOD_decoded"] = decodeMcacodGeneric2(status_decoded.mcacod);
    }

  public:
    CpxMcaBank8(const MCAData& mca) : CpxMcaDecoder(mca)
    {
        status_decoded.mc_status = mca.mc_status;
    }

    json decode_status() override
    {
        entry["MCACOD"] =
            int_to_hex(static_cast<uint16_t>(status_decoded.mcacod));
        entry["MSCOD"] =
            int_to_hex(static_cast<uint16_t>(status_decoded.mscod));
        entry["OTHER_INFO"] =
            int_to_hex(static_cast<uint8_t>(status_decoded.other_info));
        entry["CORRECTED_ERROR_COUNT"] = int_to_hex(
            static_cast<uint16_t>(status_decoded.corrected_error_count));
        entry["CORR_ERR_STATUS_IND"] = int_to_hex(
            static_cast<uint8_t>(status_decoded.corr_err_status_ind));
        entry["AR"] = int_to_hex(static_cast<bool>(status_decoded.ar));
        entry["S"] = int_to_hex(static_cast<bool>(status_decoded.s));
        entry["PCC"] = int_to_hex(static_cast<bool>(status_decoded.pcc));
        entry["ADDRV"] = int_to_hex(static_cast<bool>(status_decoded.addrv));
        entry["MISCV"] = int_to_hex(static_cast<bool>(status_decoded.miscv));
        entry["EN"] = int_to_hex(static_cast<bool>(status_decoded.en));
        entry["UC"] = int_to_hex(static_cast<bool>(status_decoded.uc));
        entry["OVERFLOW"] =
            int_to_hex(static_cast<bool>(status_decoded.overflow));
        entry["VALID"] = int_to_hex(static_cast<bool>(status_decoded.valid));
        decodeKnownFields();

        return entry;
    }
};

class CpxMcaBank9 final : public CpxMcaDecoder
{
  private:
    union MC_STATUS
    {
        struct
        {
            uint64_t mcacod : 16, mscod : 16, other_info : 6,
                corrected_error_count : 15, corr_err_status_ind : 2, ar : 1,
                s : 1, pcc : 1, addrv : 1, miscv : 1, en : 1, uc : 1,
                overflow : 1, valid : 1;
        };
        uint64_t mc_status;
    };

    MC_STATUS status_decoded;
    json entry;

    void decodeKnownFields()
    {
        auto corrErrStsIndDecoded = getDecoded(CorrErrStsInd,
            static_cast<uint8_t>(status_decoded.corr_err_status_ind));
        if (corrErrStsIndDecoded)
        {
            entry["CORR_ERR_STATUS_IND_decoded"] = *corrErrStsIndDecoded;
        }

        auto mscodDecoded = getDecoded(MscodGeneric1,
            static_cast<uint16_t>(status_decoded.mscod));
        if (mscodDecoded)
        {
            entry["MSCOD_decoded"] = *mscodDecoded;
        }

        auto mcacodDecoded = getDecoded(McacodGeneric1,
            static_cast<uint16_t>(status_decoded.mcacod));
        if (mcacodDecoded)
        {
            entry["MCACOD_decoded"] = *mcacodDecoded;
        }
        else
        {
            entry["MCACOD_decoded"] =
                decodeMcacodGeneric1(status_decoded.mcacod);
        }
    }

  public:
    CpxMcaBank9(const MCAData& mca) : CpxMcaDecoder(mca)
    {
        status_decoded.mc_status = mca.mc_status;
    }

    json decode_status() override
    {
        entry["MCACOD"] =
            int_to_hex(static_cast<uint16_t>(status_decoded.mcacod));
        entry["MSCOD"] =
            int_to_hex(static_cast<uint16_t>(status_decoded.mscod));
        entry["OTHER_INFO"] =
            int_to_hex(static_cast<uint8_t>(status_decoded.other_info));
        entry["CORRECTED_ERROR_COUNT"] = int_to_hex(
            static_cast<uint16_t>(status_decoded.corrected_error_count));
        entry["CORR_ERR_STATUS_IND"] = int_to_hex(
            static_cast<uint8_t>(status_decoded.corr_err_status_ind));
        entry["AR"] = int_to_hex(static_cast<bool>(status_decoded.ar));
        entry["S"] = int_to_hex(static_cast<bool>(status_decoded.s));
        entry["PCC"] = int_to_hex(static_cast<bool>(status_decoded.pcc));
        entry["ADDRV"] = int_to_hex(static_cast<bool>(status_decoded.addrv));
        entry["MISCV"] = int_to_hex(static_cast<bool>(status_decoded.miscv));
        entry["EN"] = int_to_hex(static_cast<bool>(status_decoded.en));
        entry["UC"] = int_to_hex(static_cast<bool>(status_decoded.uc));
        entry["OVERFLOW"] =
            int_to_hex(static_cast<bool>(status_decoded.overflow));
        entry["VALID"] = int_to_hex(static_cast<bool>(status_decoded.valid));
        decodeKnownFields();

        return entry;
    }
};

class CpxMcaBank10 final : public CpxMcaDecoder
{
  private:
    union MC_STATUS
    {
        struct
        {
            uint64_t mcacod : 16, mscod : 16, other_info : 6,
                corrected_error_count : 15, corr_err_status_ind : 2, ar : 1,
                s : 1, pcc : 1, addrv : 1, miscv : 1, en : 1, uc : 1,
                overflow : 1, valid : 1;
        };
        uint64_t mc_status;
    };

    MC_STATUS status_decoded;
    json entry;

    void decodeKnownFields()
    {
        auto corrErrStsIndDecoded = getDecoded(CorrErrStsInd,
            static_cast<uint8_t>(status_decoded.corr_err_status_ind));
        if (corrErrStsIndDecoded)
        {
            entry["CORR_ERR_STATUS_IND_decoded"] = *corrErrStsIndDecoded;
        }

        auto mscodDecoded = getDecoded(
            MscodGeneric1, static_cast<uint16_t>(status_decoded.mscod));
        if (mscodDecoded)
        {
            entry["MSCOD_decoded"] = *mscodDecoded;
        }

        auto mcacodDecoded = getDecoded(McacodGeneric1,
            static_cast<uint16_t>(status_decoded.mcacod));
        if (mcacodDecoded)
        {
            entry["MCACOD_decoded"] = *mcacodDecoded;
        }
        else
        {
            entry["MCACOD_decoded"] =
                decodeMcacodGeneric1(status_decoded.mcacod);
        }
    }

  public:
    CpxMcaBank10(const MCAData& mca) : CpxMcaDecoder(mca)
    {
        status_decoded.mc_status = mca.mc_status;
    }

    json decode_status() override
    {
        entry["MCACOD"] =
            int_to_hex(static_cast<uint16_t>(status_decoded.mcacod));
        entry["MSCOD"] =
            int_to_hex(static_cast<uint16_t>(status_decoded.mscod));
        entry["OTHER_INFO"] =
            int_to_hex(static_cast<uint8_t>(status_decoded.other_info));
        entry["CORRECTED_ERROR_COUNT"] = int_to_hex(
            static_cast<uint16_t>(status_decoded.corrected_error_count));
        entry["CORR_ERR_STATUS_IND"] = int_to_hex(
            static_cast<uint8_t>(status_decoded.corr_err_status_ind));
        entry["AR"] = int_to_hex(static_cast<bool>(status_decoded.ar));
        entry["S"] = int_to_hex(static_cast<bool>(status_decoded.s));
        entry["PCC"] = int_to_hex(static_cast<bool>(status_decoded.pcc));
        entry["ADDRV"] = int_to_hex(static_cast<bool>(status_decoded.addrv));
        entry["MISCV"] = int_to_hex(static_cast<bool>(status_decoded.miscv));
        entry["EN"] = int_to_hex(static_cast<bool>(status_decoded.en));
        entry["UC"] = int_to_hex(static_cast<bool>(status_decoded.uc));
        entry["OVERFLOW"] =
            int_to_hex(static_cast<bool>(status_decoded.overflow));
        entry["VALID"] = int_to_hex(static_cast<bool>(status_decoded.valid));
        decodeKnownFields();

        return entry;
    }
};

class CpxMcaBank11 final : public CpxMcaDecoder
{
  private:
    union MC_STATUS
    {
        struct
        {
            uint64_t mcacod : 16, mscod : 16, other_info : 6,
                corrected_error_count : 15, corr_err_status_ind : 2, ar : 1,
                s : 1, pcc : 1, addrv : 1, miscv : 1, en : 1, uc : 1,
                overflow : 1, valid : 1;
        };
        uint64_t mc_status;
    };

    MC_STATUS status_decoded;
    json entry;

    void decodeKnownFields()
    {
        auto corrErrStsIndDecoded = getDecoded(CorrErrStsInd,
            static_cast<uint8_t>(status_decoded.corr_err_status_ind));
        if (corrErrStsIndDecoded)
        {
            entry["CORR_ERR_STATUS_IND_decoded"] = *corrErrStsIndDecoded;
        }

        auto mscodDecoded = getDecoded(MscodGeneric1,
            static_cast<uint16_t>(status_decoded.mscod));
        if (mscodDecoded)
        {
            entry["MSCOD_decoded"] = *mscodDecoded;
        }

        auto mcacodDecoded = getDecoded(McacodGeneric1,
            static_cast<uint16_t>(status_decoded.mcacod));
        if (mcacodDecoded)
        {
            entry["MCACOD_decoded"] = *mcacodDecoded;
        }
        else
        {
            entry["MCACOD_decoded"] =
                decodeMcacodGeneric1(status_decoded.mcacod);
        }
    }

  public:
    CpxMcaBank11(const MCAData& mca) : CpxMcaDecoder(mca)
    {
        status_decoded.mc_status = mca.mc_status;
    }

    json decode_status() override
    {
        entry["MCACOD"] =
            int_to_hex(static_cast<uint16_t>(status_decoded.mcacod));
        entry["MSCOD"] =
            int_to_hex(static_cast<uint16_t>(status_decoded.mscod));
        entry["OTHER_INFO"] =
            int_to_hex(static_cast<uint8_t>(status_decoded.other_info));
        entry["CORRECTED_ERROR_COUNT"] = int_to_hex(
            static_cast<uint16_t>(status_decoded.corrected_error_count));
        entry["CORR_ERR_STATUS_IND"] = int_to_hex(
            static_cast<uint8_t>(status_decoded.corr_err_status_ind));
        entry["AR"] = int_to_hex(static_cast<bool>(status_decoded.ar));
        entry["S"] = int_to_hex(static_cast<bool>(status_decoded.s));
        entry["PCC"] = int_to_hex(static_cast<bool>(status_decoded.pcc));
        entry["ADDRV"] = int_to_hex(static_cast<bool>(status_decoded.addrv));
        entry["MISCV"] = int_to_hex(static_cast<bool>(status_decoded.miscv));
        entry["EN"] = int_to_hex(static_cast<bool>(status_decoded.en));
        entry["UC"] = int_to_hex(static_cast<bool>(status_decoded.uc));
        entry["OVERFLOW"] =
            int_to_hex(static_cast<bool>(status_decoded.overflow));
        entry["VALID"] = int_to_hex(static_cast<bool>(status_decoded.valid));
        decodeKnownFields();

        return entry;
    }
};

class CpxMcaBank12 final : public CpxMcaDecoder
{
  private:
    union MC_STATUS
    {
        struct
        {
            uint64_t mcacod : 16, mscod_16_21 : 6, mscod_22_31 : 10,
                other_info : 6, corrected_error_count : 15,
                corr_err_status_ind : 2, ar : 1, s : 1, pcc : 1, addrv : 1,
                miscv : 1, en : 1, uc : 1, overflow : 1, valid : 1;
        };
        uint64_t mc_status;
    };

    MC_STATUS status_decoded;
    json entry;

    void decodeKnownFields()
    {
        auto corrErrStsIndDecoded = getDecoded(CorrErrStsInd,
            static_cast<uint8_t>(status_decoded.corr_err_status_ind));
        if (corrErrStsIndDecoded)
        {
            entry["CORR_ERR_STATUS_IND_decoded"] = *corrErrStsIndDecoded;
        }

        auto mscod1621Decoded = getDecoded(Mscod1621,
            static_cast<uint8_t>(status_decoded.mscod_16_21));
        if (mscod1621Decoded)
        {
            entry["MSCOD_16_21_decoded"] = *mscod1621Decoded;
        }

        auto mscod2231Decoded = getDecoded(Mscod2231,
            static_cast<uint16_t>(status_decoded.mscod_22_31));
        if (mscod2231Decoded)
        {
            entry["MCACOD_22_31_decoded"] = *mscod2231Decoded;
        }

        auto mcacodDecoded = getDecoded(Mcacod,
            static_cast<uint16_t>(status_decoded.mcacod));
        if (mcacodDecoded)
        {
            entry["MCACOD_decoded"] = *mcacodDecoded;
        }
        else
        {
            entry["MCACOD_decoded"] =
                decodeMcacodGeneric4(status_decoded.mcacod);
        }
    }

  public:
    CpxMcaBank12(const MCAData& mca) : CpxMcaDecoder(mca)
    {
        status_decoded.mc_status = mca.mc_status;
    }

    json decode_status() override
    {
        entry["MCACOD"] =
            int_to_hex(static_cast<uint16_t>(status_decoded.mcacod));
        entry["MSCOD_16_21"] =
            int_to_hex(static_cast<uint8_t>(status_decoded.mscod_16_21));
        entry["MSCOD_22_31"] =
            int_to_hex(static_cast<uint16_t>(status_decoded.mscod_22_31));
        entry["OTHER_INFO"] =
            int_to_hex(static_cast<uint8_t>(status_decoded.other_info));
        entry["CORRECTED_ERROR_COUNT"] = int_to_hex(
            static_cast<uint16_t>(status_decoded.corrected_error_count));
        entry["CORR_ERR_STATUS_IND"] = int_to_hex(
            static_cast<uint8_t>(status_decoded.corr_err_status_ind));
        entry["AR"] = int_to_hex(static_cast<bool>(status_decoded.ar));
        entry["S"] = int_to_hex(static_cast<bool>(status_decoded.s));
        entry["PCC"] = int_to_hex(static_cast<bool>(status_decoded.pcc));
        entry["ADDRV"] = int_to_hex(static_cast<bool>(status_decoded.addrv));
        entry["MISCV"] = int_to_hex(static_cast<bool>(status_decoded.miscv));
        entry["EN"] = int_to_hex(static_cast<bool>(status_decoded.en));
        entry["UC"] = int_to_hex(static_cast<bool>(status_decoded.uc));
        entry["OVERFLOW"] =
            int_to_hex(static_cast<bool>(status_decoded.overflow));
        entry["VALID"] = int_to_hex(static_cast<bool>(status_decoded.valid));
        decodeKnownFields();

        return entry;
    }
};

class CpxMcaBank13 final : public CpxMcaDecoder
{
  private:
    union MC_STATUS
    {
        struct
        {
            uint64_t mcacod : 16, mscod : 16, other_info : 6,
                corrected_error_count : 15, corr_err_status_ind : 2, ar : 1,
                s : 1, pcc : 1, addrv : 1, miscv : 1, en : 1, uc : 1,
                overflow : 1, valid : 1;
        };
        uint64_t mc_status;
    };

    MC_STATUS status_decoded;
    json entry;

    void decodeKnownFields()
    {
        auto corrErrStsIndDecoded = getDecoded(CorrErrStsInd,
            static_cast<uint8_t>(status_decoded.corr_err_status_ind));
        if (corrErrStsIndDecoded)
        {
            entry["CORR_ERR_STATUS_IND_decoded"] = *corrErrStsIndDecoded;
        }

        auto mscodDecoded = getDecoded(MscodGeneric2,
            static_cast<uint16_t>(status_decoded.mscod));
        if (mscodDecoded)
        {
            entry["MSCOD_decoded"] = *mscodDecoded;
        }

        entry["MCACOD_decoded"] = decodeMcacodGeneric3(status_decoded.mcacod);
    }

  public:
    CpxMcaBank13(const MCAData& mca) : CpxMcaDecoder(mca)
    {
        status_decoded.mc_status = mca.mc_status;
    }

    json decode_status() override
    {
        entry["MCACOD"] =
            int_to_hex(static_cast<uint16_t>(status_decoded.mcacod));
        entry["MSCOD"] =
            int_to_hex(static_cast<uint16_t>(status_decoded.mscod));
        entry["OTHER_INFO"] =
            int_to_hex(static_cast<uint8_t>(status_decoded.other_info));
        entry["CORRECTED_ERROR_COUNT"] = int_to_hex(
            static_cast<uint16_t>(status_decoded.corrected_error_count));
        entry["CORR_ERR_STATUS_IND"] = int_to_hex(
            static_cast<uint8_t>(status_decoded.corr_err_status_ind));
        entry["AR"] = int_to_hex(static_cast<bool>(status_decoded.ar));
        entry["S"] = int_to_hex(static_cast<bool>(status_decoded.s));
        entry["PCC"] = int_to_hex(static_cast<bool>(status_decoded.pcc));
        entry["ADDRV"] = int_to_hex(static_cast<bool>(status_decoded.addrv));
        entry["MISCV"] = int_to_hex(static_cast<bool>(status_decoded.miscv));
        entry["EN"] = int_to_hex(static_cast<bool>(status_decoded.en));
        entry["UC"] = int_to_hex(static_cast<bool>(status_decoded.uc));
        entry["OVERFLOW"] =
            int_to_hex(static_cast<bool>(status_decoded.overflow));
        entry["VALID"] = int_to_hex(static_cast<bool>(status_decoded.valid));
        decodeKnownFields();

        return entry;
    }
};

class CpxMcaBank14 final : public CpxMcaDecoder
{
  private:
    union MC_STATUS
    {
        struct
        {
            uint64_t mcacod : 16, mscod : 16, other_info : 6,
                corrected_error_count : 15, corr_err_status_ind : 2, ar : 1,
                s : 1, pcc : 1, addrv : 1, miscv : 1, en : 1, uc : 1,
                overflow : 1, valid : 1;
        };
        uint64_t mc_status;
    };

    MC_STATUS status_decoded;
    json entry;

    void decodeKnownFields()
    {
        auto corrErrStsIndDecoded = getDecoded(CorrErrStsInd,
            static_cast<uint8_t>(status_decoded.corr_err_status_ind));
        if (corrErrStsIndDecoded)
        {
            entry["CORR_ERR_STATUS_IND_decoded"] = *corrErrStsIndDecoded;
        }

        auto mscodDecoded = getDecoded(MscodGeneric2,
            static_cast<uint16_t>(status_decoded.mscod));
        if (mscodDecoded)
        {
            entry["MSCOD_decoded"] = *mscodDecoded;
        }

        entry["MCACOD_decoded"] = decodeMcacodGeneric3(status_decoded.mcacod);
    }

  public:
    CpxMcaBank14(const MCAData& mca) : CpxMcaDecoder(mca)
    {
        status_decoded.mc_status = mca.mc_status;
    }

    json decode_status() override
    {
        entry["MCACOD"] =
            int_to_hex(static_cast<uint16_t>(status_decoded.mcacod));
        entry["MSCOD"] =
            int_to_hex(static_cast<uint16_t>(status_decoded.mscod));
        entry["OTHER_INFO"] =
            int_to_hex(static_cast<uint8_t>(status_decoded.other_info));
        entry["CORRECTED_ERROR_COUNT"] = int_to_hex(
            static_cast<uint16_t>(status_decoded.corrected_error_count));
        entry["CORR_ERR_STATUS_IND"] = int_to_hex(
            static_cast<uint8_t>(status_decoded.corr_err_status_ind));
        entry["AR"] = int_to_hex(static_cast<bool>(status_decoded.ar));
        entry["S"] = int_to_hex(static_cast<bool>(status_decoded.s));
        entry["PCC"] = int_to_hex(static_cast<bool>(status_decoded.pcc));
        entry["ADDRV"] = int_to_hex(static_cast<bool>(status_decoded.addrv));
        entry["MISCV"] = int_to_hex(static_cast<bool>(status_decoded.miscv));
        entry["EN"] = int_to_hex(static_cast<bool>(status_decoded.en));
        entry["UC"] = int_to_hex(static_cast<bool>(status_decoded.uc));
        entry["OVERFLOW"] =
            int_to_hex(static_cast<bool>(status_decoded.overflow));
        entry["VALID"] = int_to_hex(static_cast<bool>(status_decoded.valid));
        decodeKnownFields();

        return entry;
    }
};

class CpxMcaBank15 final : public CpxMcaDecoder
{
  private:
    union MC_STATUS
    {
        struct
        {
            uint64_t mcacod : 16, mscod : 16, fw : 1,
                corrected_error_count : 15, corr_err_status_ind : 2, ar : 1,
                s : 1, pcc : 1, addrv : 1, miscv : 1, en : 1, uc : 1,
                overflow : 1, valid : 1;
        };
        uint64_t mc_status;
    };

    MC_STATUS status_decoded;
    json entry;

    void decodeKnownFields()
    {
        auto corrErrStsIndDecoded = getDecoded(CorrErrStsInd,
            static_cast<uint8_t>(status_decoded.corr_err_status_ind));
        if (corrErrStsIndDecoded)
        {
            entry["CORR_ERR_STATUS_IND_decoded"] = *corrErrStsIndDecoded;
        }

        auto mscodDecoded = getDecoded(MscodGeneric2,
            static_cast<uint16_t>(status_decoded.mscod));
        if (mscodDecoded)
        {
            entry["MSCOD_decoded"] = *mscodDecoded;
        }

        entry["MCACOD_decoded"] = decodeMcacodGeneric3(status_decoded.mcacod);
    }

  public:
    CpxMcaBank15(const MCAData& mca) : CpxMcaDecoder(mca)
    {
        status_decoded.mc_status = mca.mc_status;
    }

    json decode_status() override
    {
        entry["MCACOD"] =
            int_to_hex(static_cast<uint16_t>(status_decoded.mcacod));
        entry["MSCOD"] =
            int_to_hex(static_cast<uint16_t>(status_decoded.mscod));
        entry["FW"] = int_to_hex(static_cast<bool>(status_decoded.fw));
        entry["CORRECTED_ERROR_COUNT"] = int_to_hex(
            static_cast<uint16_t>(status_decoded.corrected_error_count));
        entry["CORR_ERR_STATUS_IND"] = int_to_hex(
            static_cast<uint8_t>(status_decoded.corr_err_status_ind));
        entry["AR"] = int_to_hex(static_cast<bool>(status_decoded.ar));
        entry["S"] = int_to_hex(static_cast<bool>(status_decoded.s));
        entry["PCC"] = int_to_hex(static_cast<bool>(status_decoded.pcc));
        entry["ADDRV"] = int_to_hex(static_cast<bool>(status_decoded.addrv));
        entry["MISCV"] = int_to_hex(static_cast<bool>(status_decoded.miscv));
        entry["EN"] = int_to_hex(static_cast<bool>(status_decoded.en));
        entry["UC"] = int_to_hex(static_cast<bool>(status_decoded.uc));
        entry["OVERFLOW"] =
            int_to_hex(static_cast<bool>(status_decoded.overflow));
        entry["VALID"] = int_to_hex(static_cast<bool>(status_decoded.valid));
        decodeKnownFields();

        return entry;
    }
};

class CpxMcaBank16 final : public CpxMcaDecoder
{
  private:
    union MC_STATUS
    {
        struct
        {
            uint64_t mcacod : 16, mscod : 16, other_info : 6,
                corrected_error_count : 15, corr_err_status_ind : 2, ar : 1,
                s : 1, pcc : 1, addrv : 1, miscv : 1, en : 1, uc : 1,
                overflow : 1, valid : 1;
        };
        uint64_t mc_status;
    };

    MC_STATUS status_decoded;
    json entry;

    void decodeKnownFields()
    {
        auto corrErrStsIndDecoded = getDecoded(CorrErrStsInd,
            static_cast<uint8_t>(status_decoded.corr_err_status_ind));
        if (corrErrStsIndDecoded)
        {
            entry["CORR_ERR_STATUS_IND_decoded"] = *corrErrStsIndDecoded;
        }

        auto mscodDecoded = getDecoded(MscodGeneric2,
            static_cast<uint16_t>(status_decoded.mscod));
        if (mscodDecoded)
        {
            entry["MSCOD_decoded"] = *mscodDecoded;
        }

        entry["MCACOD_decoded"] = decodeMcacodGeneric3(status_decoded.mcacod);
    }

  public:
    CpxMcaBank16(const MCAData& mca) : CpxMcaDecoder(mca)
    {
        status_decoded.mc_status = mca.mc_status;
    }

    json decode_status() override
    {
        entry["MCACOD"] =
            int_to_hex(static_cast<uint16_t>(status_decoded.mcacod));
        entry["MSCOD"] =
            int_to_hex(static_cast<uint16_t>(status_decoded.mscod));
        entry["OTHER_INFO"] =
            int_to_hex(static_cast<uint8_t>(status_decoded.other_info));
        entry["CORRECTED_ERROR_COUNT"] = int_to_hex(
            static_cast<uint16_t>(status_decoded.corrected_error_count));
        entry["CORR_ERR_STATUS_IND"] = int_to_hex(
            static_cast<uint8_t>(status_decoded.corr_err_status_ind));
        entry["AR"] = int_to_hex(static_cast<bool>(status_decoded.ar));
        entry["S"] = int_to_hex(static_cast<bool>(status_decoded.s));
        entry["PCC"] = int_to_hex(static_cast<bool>(status_decoded.pcc));
        entry["ADDRV"] = int_to_hex(static_cast<bool>(status_decoded.addrv));
        entry["MISCV"] = int_to_hex(static_cast<bool>(status_decoded.miscv));
        entry["EN"] = int_to_hex(static_cast<bool>(status_decoded.en));
        entry["UC"] = int_to_hex(static_cast<bool>(status_decoded.uc));
        entry["OVERFLOW"] =
            int_to_hex(static_cast<bool>(status_decoded.overflow));
        entry["VALID"] = int_to_hex(static_cast<bool>(status_decoded.valid));
        decodeKnownFields();

        return entry;
    }
};

class CpxMcaBank17 final : public CpxMcaDecoder
{
  private:
    union MC_STATUS
    {
        struct
        {
            uint64_t mcacod : 16, mscod : 16, other_info : 6,
                corrected_error_count : 15, corr_err_status_ind : 2, ar : 1,
                s : 1, pcc : 1, addrv : 1, miscv : 1, en : 1, uc : 1,
                overflow : 1, valid : 1;
        };
        uint64_t mc_status;
    };

    MC_STATUS status_decoded;
    json entry;

    void decodeKnownFields()
    {
        auto corrErrStsIndDecoded = getDecoded(CorrErrStsInd,
            static_cast<uint8_t>(status_decoded.corr_err_status_ind));
        if (corrErrStsIndDecoded)
        {
            entry["CORR_ERR_STATUS_IND_decoded"] = *corrErrStsIndDecoded;
        }

        auto mscodDecoded = getDecoded(MscodGeneric2,
            static_cast<uint16_t>(status_decoded.mscod));
        if (mscodDecoded)
        {
            entry["MSCOD_decoded"] = *mscodDecoded;
        }

        entry["MCACOD_decoded"] = decodeMcacodGeneric3(status_decoded.mcacod);
    }

  public:
    CpxMcaBank17(const MCAData& mca) : CpxMcaDecoder(mca)
    {
        status_decoded.mc_status = mca.mc_status;
    }

    json decode_status() override
    {
        entry["MCACOD"] =
            int_to_hex(static_cast<uint16_t>(status_decoded.mcacod));
        entry["MSCOD"] =
            int_to_hex(static_cast<uint16_t>(status_decoded.mscod));
        entry["OTHER_INFO"] =
            int_to_hex(static_cast<uint8_t>(status_decoded.other_info));
        entry["CORRECTED_ERROR_COUNT"] = int_to_hex(
            static_cast<uint16_t>(status_decoded.corrected_error_count));
        entry["CORR_ERR_STATUS_IND"] = int_to_hex(
            static_cast<uint8_t>(status_decoded.corr_err_status_ind));
        entry["AR"] = int_to_hex(static_cast<bool>(status_decoded.ar));
        entry["S"] = int_to_hex(static_cast<bool>(status_decoded.s));
        entry["PCC"] = int_to_hex(static_cast<bool>(status_decoded.pcc));
        entry["ADDRV"] = int_to_hex(static_cast<bool>(status_decoded.addrv));
        entry["MISCV"] = int_to_hex(static_cast<bool>(status_decoded.miscv));
        entry["EN"] = int_to_hex(static_cast<bool>(status_decoded.en));
        entry["UC"] = int_to_hex(static_cast<bool>(status_decoded.uc));
        entry["OVERFLOW"] =
            int_to_hex(static_cast<bool>(status_decoded.overflow));
        entry["VALID"] = int_to_hex(static_cast<bool>(status_decoded.valid));
        decodeKnownFields();

        return entry;
    }
};

class CpxMcaBank18 final : public CpxMcaDecoder
{
  private:
    union MC_STATUS
    {
        struct
        {
            uint64_t mcacod : 16, mscod : 16, other_info : 6,
                corrected_error_count : 15, corr_err_status_ind : 2, ar : 1,
                s : 1, pcc : 1, addrv : 1, miscv : 1, en : 1, uc : 1,
                overflow : 1, valid : 1;
        };
        uint64_t mc_status;
    };

    MC_STATUS status_decoded;
    json entry;

    void decodeKnownFields()
    {
        auto corrErrStsIndDecoded = getDecoded(CorrErrStsInd,
            static_cast<uint8_t>(status_decoded.corr_err_status_ind));
        if (corrErrStsIndDecoded)
        {
            entry["CORR_ERR_STATUS_IND_decoded"] = *corrErrStsIndDecoded;
        }

        auto mscodDecoded = getDecoded(MscodGeneric2,
            static_cast<uint16_t>(status_decoded.mscod));
        if (mscodDecoded)
        {
            entry["MSCOD_decoded"] = *mscodDecoded;
        }

        entry["MCACOD_decoded"] = decodeMcacodGeneric3(status_decoded.mcacod);
    }

  public:
    CpxMcaBank18(const MCAData& mca) : CpxMcaDecoder(mca)
    {
        status_decoded.mc_status = mca.mc_status;
    }

    json decode_status() override
    {
        entry["MCACOD"] =
            int_to_hex(static_cast<uint16_t>(status_decoded.mcacod));
        entry["MSCOD"] =
            int_to_hex(static_cast<uint16_t>(status_decoded.mscod));
        entry["OTHER_INFO"] =
            int_to_hex(static_cast<uint8_t>(status_decoded.other_info));
        entry["CORRECTED_ERROR_COUNT"] = int_to_hex(
            static_cast<uint16_t>(status_decoded.corrected_error_count));
        entry["CORR_ERR_STATUS_IND"] = int_to_hex(
            static_cast<uint8_t>(status_decoded.corr_err_status_ind));
        entry["AR"] = int_to_hex(static_cast<bool>(status_decoded.ar));
        entry["S"] = int_to_hex(static_cast<bool>(status_decoded.s));
        entry["PCC"] = int_to_hex(static_cast<bool>(status_decoded.pcc));
        entry["ADDRV"] = int_to_hex(static_cast<bool>(status_decoded.addrv));
        entry["MISCV"] = int_to_hex(static_cast<bool>(status_decoded.miscv));
        entry["EN"] = int_to_hex(static_cast<bool>(status_decoded.en));
        entry["UC"] = int_to_hex(static_cast<bool>(status_decoded.uc));
        entry["OVERFLOW"] =
            int_to_hex(static_cast<bool>(status_decoded.overflow));
        entry["VALID"] = int_to_hex(static_cast<bool>(status_decoded.valid));
        decodeKnownFields();

        return entry;
    }
};

class CpxMcaBank19 final : public CpxMcaDecoder
{
  private:
    union MC_STATUS
    {
        struct
        {
            uint64_t mcacod : 16, mscod_16_21 : 6, mscod_22_31 : 10,
                other_info : 6, corrected_error_count : 15,
                corr_err_status_ind : 2, ar : 1, s : 1, pcc : 1, addrv : 1,
                miscv : 1, en : 1, uc : 1, overflow : 1, valid : 1;
        };
        uint64_t mc_status;
    };

    MC_STATUS status_decoded;
    json entry;

    void decodeKnownFields()
    {
        auto corrErrStsIndDecoded = getDecoded(CorrErrStsInd,
            static_cast<uint8_t>(status_decoded.corr_err_status_ind));
        if (corrErrStsIndDecoded)
        {
            entry["CORR_ERR_STATUS_IND_decoded"] = *corrErrStsIndDecoded;
        }

        auto mscod1621Decoded = getDecoded(Mscod1621,
            static_cast<uint8_t>(status_decoded.mscod_16_21));
        if (mscod1621Decoded)
        {
            entry["MSCOD_16_21_decoded"] = *mscod1621Decoded;
        }

        auto mscod2231Decoded = getDecoded(Mscod2231,
            static_cast<uint16_t>(status_decoded.mscod_22_31));
        if (mscod2231Decoded)
        {
            entry["MCACOD_22_31_decoded"] = *mscod2231Decoded;
        }

        auto mcacodDecoded = getDecoded(Mcacod,
            static_cast<uint16_t>(status_decoded.mcacod));
        if (mcacodDecoded)
        {
            entry["MCACOD_decoded"] = *mcacodDecoded;
        }
        else
        {
            entry["MCACOD_decoded"] =
                decodeMcacodGeneric4(status_decoded.mcacod);
        }
    }

  public:
    CpxMcaBank19(const MCAData& mca) : CpxMcaDecoder(mca)
    {
        status_decoded.mc_status = mca.mc_status;
    }

    json decode_status() override
    {
        entry["MCACOD"] =
            int_to_hex(static_cast<uint16_t>(status_decoded.mcacod));
        entry["MSCOD_16_21"] =
            int_to_hex(static_cast<uint8_t>(status_decoded.mscod_16_21));
        entry["MSCOD_22_31"] =
            int_to_hex(static_cast<uint16_t>(status_decoded.mscod_22_31));
        entry["OTHER_INFO"] =
            int_to_hex(static_cast<uint8_t>(status_decoded.other_info));
        entry["CORRECTED_ERROR_COUNT"] = int_to_hex(
            static_cast<uint16_t>(status_decoded.corrected_error_count));
        entry["CORR_ERR_STATUS_IND"] = int_to_hex(
            static_cast<uint8_t>(status_decoded.corr_err_status_ind));
        entry["AR"] = int_to_hex(static_cast<bool>(status_decoded.ar));
        entry["S"] = int_to_hex(static_cast<bool>(status_decoded.s));
        entry["PCC"] = int_to_hex(static_cast<bool>(status_decoded.pcc));
        entry["ADDRV"] = int_to_hex(static_cast<bool>(status_decoded.addrv));
        entry["MISCV"] = int_to_hex(static_cast<bool>(status_decoded.miscv));
        entry["EN"] = int_to_hex(static_cast<bool>(status_decoded.en));
        entry["UC"] = int_to_hex(static_cast<bool>(status_decoded.uc));
        entry["OVERFLOW"] =
            int_to_hex(static_cast<bool>(status_decoded.overflow));
        entry["VALID"] = int_to_hex(static_cast<bool>(status_decoded.valid));
        decodeKnownFields();

        return entry;
    }
};

class CpxMcaBank20 final : public CpxMcaDecoder
{
  private:
    union MC_STATUS
    {
        struct
        {
            uint64_t mcacod : 16, mscod : 16, other_info : 6,
                corrected_error_count : 15, corr_err_status_ind : 2, ar : 1,
                s : 1, pcc : 1, addrv : 1, miscv : 1, en : 1, uc : 1,
                overflow : 1, valid : 1;
        };
        uint64_t mc_status;
    };

    MC_STATUS status_decoded;
    json entry;

    void decodeKnownFields()
    {
        auto corrErrStsIndDecoded = getDecoded(CorrErrStsInd,
            static_cast<uint8_t>(status_decoded.corr_err_status_ind));
        if (corrErrStsIndDecoded)
        {
            entry["CORR_ERR_STATUS_IND_decoded"] = *corrErrStsIndDecoded;
        }

        entry["MSCOD_decoded"] = decodeMscodGeneric2(status_decoded.mscod);

        auto mcacodDecoded = getDecoded(Mcacod,
            static_cast<uint16_t>(status_decoded.mcacod));
        if (mcacodDecoded)
        {
            entry["MCACOD_decoded"] = *mcacodDecoded;
        }
        else
        {
            entry["MCACOD_decoded"] =
                decodeMcacodGeneric4(status_decoded.mcacod);
        }
    }

  public:
    CpxMcaBank20(const MCAData& mca) : CpxMcaDecoder(mca)
    {
        status_decoded.mc_status = mca.mc_status;
    }

    json decode_status() override
    {
        entry["MCACOD"] =
            int_to_hex(static_cast<uint16_t>(status_decoded.mcacod));
        entry["MSCOD"] =
            int_to_hex(static_cast<uint16_t>(status_decoded.mscod));
        entry["OTHER_INFO"] =
            int_to_hex(static_cast<uint8_t>(status_decoded.other_info));
        entry["CORRECTED_ERROR_COUNT"] = int_to_hex(
            static_cast<uint16_t>(status_decoded.corrected_error_count));
        entry["CORR_ERR_STATUS_IND"] = int_to_hex(
            static_cast<uint8_t>(status_decoded.corr_err_status_ind));
        entry["AR"] = int_to_hex(static_cast<bool>(status_decoded.ar));
        entry["S"] = int_to_hex(static_cast<bool>(status_decoded.s));
        entry["PCC"] = int_to_hex(static_cast<bool>(status_decoded.pcc));
        entry["ADDRV"] = int_to_hex(static_cast<bool>(status_decoded.addrv));
        entry["MISCV"] = int_to_hex(static_cast<bool>(status_decoded.miscv));
        entry["EN"] = int_to_hex(static_cast<bool>(status_decoded.en));
        entry["UC"] = int_to_hex(static_cast<bool>(status_decoded.uc));
        entry["OVERFLOW"] =
            int_to_hex(static_cast<bool>(status_decoded.overflow));
        entry["VALID"] = int_to_hex(static_cast<bool>(status_decoded.valid));
        decodeKnownFields();

        return entry;
    }
};

class CpxMcaBank21 final : public CpxMcaDecoder
{
  private:
    union MC_STATUS
    {
        struct
        {
            uint64_t mcacod : 16, mscod : 16, other_info : 6,
                corrected_error_count : 15, corr_err_status_ind : 2, ar : 1,
                s : 1, pcc : 1, addrv : 1, miscv : 1, en : 1, uc : 1,
                overflow : 1, valid : 1;
        };
        uint64_t mc_status;
    };

    MC_STATUS status_decoded;
    json entry;

    void decodeKnownFields()
    {
        auto corrErrStsIndDecoded = getDecoded(CorrErrStsInd,
            static_cast<uint8_t>(status_decoded.corr_err_status_ind));
        if (corrErrStsIndDecoded)
        {
            entry["CORR_ERR_STATUS_IND_decoded"] = *corrErrStsIndDecoded;
        }

        entry["MSCOD_decoded"] = decodeMscodGeneric2(status_decoded.mscod);

        auto mcacodDecoded = getDecoded(Mcacod,
            static_cast<uint16_t>(status_decoded.mcacod));
        if (mcacodDecoded)
        {
            entry["MCACOD_decoded"] = *mcacodDecoded;
        }
        else
        {
            entry["MCACOD_decoded"] =
                decodeMcacodGeneric4(status_decoded.mcacod);
        }
    }

  public:
    CpxMcaBank21(const MCAData& mca) : CpxMcaDecoder(mca)
    {
        status_decoded.mc_status = mca.mc_status;
    }

    json decode_status() override
    {
        entry["MCACOD"] =
            int_to_hex(static_cast<uint16_t>(status_decoded.mcacod));
        entry["MSCOD"] =
            int_to_hex(static_cast<uint16_t>(status_decoded.mscod));
        entry["OTHER_INFO"] =
            int_to_hex(static_cast<uint8_t>(status_decoded.other_info));
        entry["CORRECTED_ERROR_COUNT"] = int_to_hex(
            static_cast<uint16_t>(status_decoded.corrected_error_count));
        entry["CORR_ERR_STATUS_IND"] = int_to_hex(
            static_cast<uint8_t>(status_decoded.corr_err_status_ind));
        entry["AR"] = int_to_hex(static_cast<bool>(status_decoded.ar));
        entry["S"] = int_to_hex(static_cast<bool>(status_decoded.s));
        entry["PCC"] = int_to_hex(static_cast<bool>(status_decoded.pcc));
        entry["ADDRV"] = int_to_hex(static_cast<bool>(status_decoded.addrv));
        entry["MISCV"] = int_to_hex(static_cast<bool>(status_decoded.miscv));
        entry["EN"] = int_to_hex(static_cast<bool>(status_decoded.en));
        entry["UC"] = int_to_hex(static_cast<bool>(status_decoded.uc));
        entry["OVERFLOW"] =
            int_to_hex(static_cast<bool>(status_decoded.overflow));
        entry["VALID"] = int_to_hex(static_cast<bool>(status_decoded.valid));
        decodeKnownFields();

        return entry;
    }
};

class CpxMcaBank22 final : public CpxMcaDecoder
{
  private:
    union MC_STATUS
    {
        struct
        {
            uint64_t mcacod : 16, mscod : 16, other_info : 6,
                corrected_error_count : 15, corr_err_status_ind : 2, ar : 1,
                s : 1, pcc : 1, addrv : 1, miscv : 1, en : 1, uc : 1,
                overflow : 1, valid : 1;
        };
        uint64_t mc_status;
    };

    MC_STATUS status_decoded;
    json entry;

    void decodeKnownFields()
    {
        auto corrErrStsIndDecoded = getDecoded(CorrErrStsInd,
            static_cast<uint8_t>(status_decoded.corr_err_status_ind));
        if (corrErrStsIndDecoded)
        {
            entry["CORR_ERR_STATUS_IND_decoded"] = *corrErrStsIndDecoded;
        }

        entry["MSCOD_decoded"] = decodeMscodGeneric2(status_decoded.mscod);

        auto mcacodDecoded = getDecoded(Mcacod,
            static_cast<uint16_t>(status_decoded.mcacod));
        if (mcacodDecoded)
        {
            entry["MCACOD_decoded"] = *mcacodDecoded;
        }
        else
        {
            entry["MCACOD_decoded"] =
                decodeMcacodGeneric4(status_decoded.mcacod);
        }
    }

  public:
    CpxMcaBank22(const MCAData& mca) : CpxMcaDecoder(mca)
    {
        status_decoded.mc_status = mca.mc_status;
    }

    json decode_status() override
    {
        entry["MCACOD"] =
            int_to_hex(static_cast<uint16_t>(status_decoded.mcacod));
        entry["MSCOD"] =
            int_to_hex(static_cast<uint16_t>(status_decoded.mscod));
        entry["OTHER_INFO"] =
            int_to_hex(static_cast<uint8_t>(status_decoded.other_info));
        entry["CORRECTED_ERROR_COUNT"] = int_to_hex(
            static_cast<uint16_t>(status_decoded.corrected_error_count));
        entry["CORR_ERR_STATUS_IND"] = int_to_hex(
            static_cast<uint8_t>(status_decoded.corr_err_status_ind));
        entry["AR"] = int_to_hex(static_cast<bool>(status_decoded.ar));
        entry["S"] = int_to_hex(static_cast<bool>(status_decoded.s));
        entry["PCC"] = int_to_hex(static_cast<bool>(status_decoded.pcc));
        entry["ADDRV"] = int_to_hex(static_cast<bool>(status_decoded.addrv));
        entry["MISCV"] = int_to_hex(static_cast<bool>(status_decoded.miscv));
        entry["EN"] = int_to_hex(static_cast<bool>(status_decoded.en));
        entry["UC"] = int_to_hex(static_cast<bool>(status_decoded.uc));
        entry["OVERFLOW"] =
            int_to_hex(static_cast<bool>(status_decoded.overflow));
        entry["VALID"] = int_to_hex(static_cast<bool>(status_decoded.valid));
        decodeKnownFields();

        return entry;
    }
};