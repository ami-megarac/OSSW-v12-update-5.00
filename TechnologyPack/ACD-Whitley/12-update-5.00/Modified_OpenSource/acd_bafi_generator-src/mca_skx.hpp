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
#include <map>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <sstream>

#include <mca_defs.hpp>
#include <tor_defs_skx.hpp>
#include <utils.hpp>

using json = nlohmann::json;

class SkxMcaDecoder : public McaDecoder
{
  public:
    SkxMcaDecoder(const MCAData& mca) : McaDecoder(mca) {};

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
        {0x6, "UBOX"},
        {0x7, "M2M0"},
        {0x8, "M2M1"},
        {0x9, "CHA_A"},
        {0xA, "CHA_B"},
        {0xB, "CHA_C"},
        {0xC, "Intel UPI 1"},
        {0xD, "IMC 0, channel 0"},
        {0xE, "IMC 0, channel 1"},
        {0xF, "IMC 1, channel 0"},
        {0x10, "IMC 1, channel 1"},
        {0x11, "IMC 0, channel 2"},
        {0x12, "IMC 1, channel 0"},
        {0x13, "Intel UPI 2"},
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

    const std::map<uint8_t, const char*> ErrorLevelEncoding = {
        {0x0, "LEVEL_0 (core L1 cache)"},
        {0x1, "LEVEL_1 (core L2 cache)"},
        {0x2, "LEVEL_2 (L3/LLC cache)"},
        {0x3, "GENERIC_ERROR_LEVEL_ENCODING"},
    };

    const std::map<uint8_t, const char*> ErrorTransactionType = {
        {0x0, "INSTRUCTION"},
        {0x1, "DATA"},
        {0x2, "GENERIC_ERROR_TRANSACTION_TYPE"},
        {0x3, "RESERVED"},
    };

    const std::map<uint8_t, const char*> ErrSpecific = {
        {0x5, "Parity error on structures"},
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
        {0xc, "Memory or I/O"},
        {0x3, "Level encoding"},
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

    const std::map<uint8_t, const char*> Error = {
        {0x0, "Generic undefined request"}, {0x1, "Memory Read Error"},
        {0x2, "Memory Write Error"},        {0x3, "Address/Command Error"},
        {0x4, "Memory Scrubbing Error"},
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

    // MSCOD decode for banks
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

    const std::map<uint16_t, const char*> McacodGeneric1 = {
        {0x136, "Error during data read (UCNA)"},
        {0x17a, "Error during Explicit Write Back (SRAO)"},
        {0x152, "Error during instruction read (UCNA)"},
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

    union McacodDataGeneric3
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

    union McacodDataGeneric4
    {
        struct
        {
            uint16_t reserved0 : 2, participation: 2, reserved1: 4,
            request: 4, timeout: 1, reserved2: 5;
        };
        uint16_t mcacod_15_0;
    };

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

    std::string decodeMcacodGeneric3(uint16_t mcacod)
    {
        std::stringstream ss;
        McacodDataGeneric3 mcacodDecoded;
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
        McacodDataGeneric4 mcacodDecoded;
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
        if (mca.cbo)
        {
            ss << "CHA" << mca.bank;
        }
        else
        {
            ss << mca.bank;
            auto bankNameDecoded =
                getDecoded(BankNames, static_cast<uint8_t>(mca.bank));
            if (bankNameDecoded)
            {
                ss << " (" << *bankNameDecoded << ")";
            }
        }
        return ss.str();
    }
};

class SkxMcaBankIfu final : public SkxMcaDecoder
{
    private:
      union MC_STATUS
      {
        struct
        {
            uint64_t mcacod : 16, mscod: 16, reserved: 6,
            corrected_err_cnt : 15, corr_err_status_ind: 2, ar: 1, s: 1, pcc: 1,
            addrv: 1, miscv: 1, en: 1, uc: 1, overflow: 1, valid: 1;
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
        SkxMcaBankIfu(const MCAData& mca) : SkxMcaDecoder(mca)
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
            int_to_hex(static_cast<uint8_t>(status_decoded.reserved));
        entry["CORRECTED_ERROR_COUNT"] = int_to_hex(
            static_cast<uint16_t>(status_decoded.corrected_err_cnt));
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

class SkxMcaBankDcu final : public SkxMcaDecoder
{
    private:
      union MC_STATUS
      {
        struct
        {
            uint64_t mcacod: 16, mscod: 16, corrected_err_cnt: 15,
            corr_err_status_ind: 2, ar: 1, s: 1, pcc: 1, addrv: 1,
            miscv: 1, en: 1, uc: 1, overflow: 1, valid: 1;
        };
          uint64_t mc_status;
      };

      const std::map<uint32_t, const char*> decodeMscod = {
        {0x0, "Non-APIC Error"},
        {0x10, "WBINVD or Poison SRAR Error based on MCACOD values"},
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
      SkxMcaBankDcu(const MCAData& mca) : SkxMcaDecoder(mca)
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
            static_cast<uint16_t>(status_decoded.corrected_err_cnt));
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

class SkxMcaBankDtlb final : public SkxMcaDecoder
{
private:
    union MC_STATUS
    {
        struct
        {
            uint64_t mcacod: 16, mscod: 16, reserved0: 6,  corrected_err_cnt: 15,
            reserved1: 4, pcc: 1, addrv: 1, miscv: 1, en: 1, uc: 1, overflow: 1,
            valid: 1;
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
        {0x2, "Tag Parity Error"},
        {0x3, "Data Parity Error"},
        {0x4, "SRF"},
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

    MC_STATUS status_decoded;
    json entry;

    void decodeKnownFields()
    {
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
            entry["MCACOD_decoded"] = decodedMcacod(status_decoded.mcacod);
        }
    }

    public:
    SkxMcaBankDtlb(const MCAData& mca) : SkxMcaDecoder(mca)
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
            static_cast<uint16_t>(status_decoded.corrected_err_cnt));
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

class SkxMcaBankMlc final : public SkxMcaDecoder
{
    private:
    union MC_STATUS
    {
        struct
        {
            uint64_t mcacod: 16, mscod: 16, reserved0: 6, corrected_err_cnt: 15,
            corr_err_status_ind: 2, reserved1: 2, pcc: 1, addrv: 1, miscv: 1,
            en: 1, uc: 1, overflow: 1, valid: 1;
        };
        uint64_t mc_status;
    };

    const std::map<uint16_t, const char*> decodeMscod = {
        {0x0080, "Watchdog timer (3-strike) Error"},
        {0x0040, "Internal Error"},
        {0x0030, "Poisoned Data detected"},
        {0x0020, "Data Read Uncorrected error / prefetch Error"},
        {0x0010, "Data Read Corrected error / prefetch Error"},
        {0x0008, "MESI State Uncorrected Error"},
        {0x0004, "MESI State Corrected Error"},
        {0x0002, "Tag Uncorrected Error"},
        {0x0001, "Tag Corrected Error"},
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
    SkxMcaBankMlc(const MCAData& mca) : SkxMcaDecoder(mca)
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
            static_cast<uint16_t>(status_decoded.corrected_err_cnt));
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

class SkxMcaBankPcu final : public SkxMcaDecoder
{
    union MC_STATUS
    {
        struct
        {
            uint64_t mcacod: 16, mscod_16_19: 4, mscod_20_23: 4, mscod_24_31: 8,
            enh_mca_avail0: 6, corrected_err_cnt: 15, corr_err_status_ind: 2,
            enh_mca_avail1: 2, pcc: 1, addrv: 1, miscv: 1, en: 1, uc: 1,
            overflow: 1, valid: 1;
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

        std::stringstream ss;
        auto mscod1619Decoded = getDecoded(
            decodeMscod1619, static_cast<uint8_t>(status_decoded.mscod_16_19));
        if (mscod1619Decoded)
        {
            entry["MSCOD_16_19_decoded"] = *mscod1619Decoded;
            ss << *mscod1619Decoded << " | ";
        }

        auto mscod2023Decoded = getDecoded(
            decodeMscod2023, static_cast<uint8_t>(status_decoded.mscod_20_23));
        if (mscod2023Decoded)
        {
            entry["MSCOD_20_23_decoded"] = *mscod2023Decoded;
            ss << *mscod2023Decoded << " | ";
        }

        auto mscod2431Decoded = getDecoded(
            decodeMscod2431, static_cast<uint8_t>(status_decoded.mscod_24_31));
        if (mscod2431Decoded)
        {
            entry["MSCOD_24_31_decoded"] = *mscod2431Decoded;
            ss << *mscod2431Decoded;
        }
        else
        {
            entry["MSCOD_24_31_decoded"] = "Internal error";
            ss << "Internal error";
        }

        entry["MSCOD_decoded"] = ss.str();

        auto mcacod = getDecoded(
            decodeMcacod, static_cast<uint16_t>(status_decoded.mcacod));
        if (mcacod)
        {
            entry["MCACOD_decoded"] = *mcacod;
        }
    }

  public:
    SkxMcaBankPcu(const MCAData& mca) : SkxMcaDecoder(mca)
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
            static_cast<uint16_t>(status_decoded.corrected_err_cnt));
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

class SkxMcaBankUpi final : public SkxMcaDecoder
{
    private:
    union MC_STATUS
    {
        struct
        {
            uint64_t mcacod: 16, mscod: 6, mscod_spare: 10, other_info: 6,
            corrected_err_cnt: 15, corr_err_status_ind: 2, ar: 1, s: 1, pcc: 1,
            addrv: 1, miscv: 1, en: 1, uc: 1, overflow: 1, valid: 1;
        };
        uint64_t mc_status;
    };

    union MscodData
    {
        struct
        {
            uint16_t tx_parity_err : 1, ll_reset : 1, ll_reset_in_progress : 1,
            rx_header: 1, tx_flit: 1, table_err: 1, rf_parity_err : 1,
            retry_req_flit : 1, retry_ack_flit : 1, phy_ctl_err : 1,
            reserved0 : 6;
        };
        uint16_t mscod;
    };

    MC_STATUS status_decoded;
    json entry;

    const std::map<uint8_t, const char*> decodeMscod = {
       {0x00, "UC Phy Initialization Failure"},
       {0x01, "UC Phy Detected Drift Buffer Alarm"},
       {0x02, "UC Phy Detected Latency Buffer Rollover"},
       {0x10, "UC LL Rx detected CRC error"},
       {0x11, "UC LL Rx Unsupported/Undefined packet"},
       {0x12, "UC LL or Phy Control Error: unexpected Tx Protocol flit"},
       {0x13, "UC LL Rx Parameter Exception"},
       {0x1F, "UC LL Detected Control Error from M3UPI Correctable (COR)"},
       {0x20, "COR Phy Initialization Abort"},
       {0x21, "COR Phy Reset"},
       {0x22, "COR Phy Lane failure, recovery in x8 width"},
       {0x23, "COR Phy L0c error"},
       {0x24, "COR Phy L0c error triggering Phy Reset"},
       {0x25, "COR Phy L0p exit error triggering Phy Reset"},
       {0x30, "COR LL Rx detected CRC error: successful LLR without Phy Reinit"},
       {0x31, "COR LL Rx detected CRC error: successful LLR with Phy Reinit"},
    };

    std::string decodeMscodSpare(uint16_t mscod)
    {
        std::stringstream ss;
        MscodData mscodDecoded;
        mscodDecoded.mscod = mscod;

        if (mscodDecoded.tx_parity_err)
        {
            ss << "Link Layer Tx Parity Error|";
        }
        if (mscodDecoded.ll_reset)
        {
            ss << "Link Layer reset initiated while protocol traffic not idle"
               << "|";
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

        return ss.str();
    }

    void decodeKnownFields()
    {
        auto corrErrStsIndDecoded = getDecoded(CorrErrStsInd,
            static_cast<uint8_t>(status_decoded.corr_err_status_ind));
        if (corrErrStsIndDecoded)
        {
            entry["CORR_ERR_STATUS_IND_decoded"] = *corrErrStsIndDecoded;
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

        auto mscod = getDecoded(decodeMscod,
            static_cast<uint8_t>(status_decoded.mscod));
        if (mscod)
        {
            entry["MSCOD_decoded"] = *mscod;
        }

        entry["MSCOD_SPARE_decoded"] =
            decodeMscodSpare(status_decoded.mscod_spare);
    }

    public:
    SkxMcaBankUpi(const MCAData& mca) : SkxMcaDecoder(mca)
    {
        status_decoded.mc_status = mca.mc_status;
    }

    json decode_status() override
    {
        entry["MCACOD"] =
            int_to_hex(static_cast<uint16_t>(status_decoded.mcacod));
        entry["MSCOD"] =
            int_to_hex(static_cast<uint8_t>(status_decoded.mscod));
        entry["MSCOD_SPARE"] =
            int_to_hex(static_cast<uint16_t>(status_decoded.mscod_spare));
        entry["OTHER_INFO"] =
            int_to_hex(static_cast<uint8_t>(status_decoded.other_info));
        entry["CORRECTED_ERROR_COUNT"] = int_to_hex(
            static_cast<uint16_t>(status_decoded.corrected_err_cnt));
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

class SkxMcaBankIio final : public SkxMcaDecoder
{
    private:
    union MC_STATUS
    {
        struct
        {
            uint64_t mcacod: 16, mscod: 16, other_info: 6,
            corrected_err_cnt: 15, corr_err_status_ind: 2, ar: 1, s: 1, pcc: 1,
            addrv: 1, miscv: 1, en: 1, uc: 1, overflow: 1, valid: 1;
        };
        uint64_t mc_status;
    };

    union MC_MISC
    {
        struct
        {
            uint64_t reserved0 : 8, segment_log : 8, function_log : 3,
                device_log : 5, bus_log : 8, reserved1 : 32;
        };
        uint64_t mc_misc;
    };

    const std::map<uint16_t, const char*> decodeMscod = {
        {0x0, "This value is always 0"},
    };

    const std::map<uint16_t, const char*> decodeMcacod = {
        {0xe0b, "Generic I/O Error"},
        {0x405, "Generic I/O Error"},
    };

    MC_STATUS status_decoded;
    MC_MISC misc_decoded;
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
            std::stringstream ss;
            ss << *mcacodDecoded;
            if (status_decoded.miscv == 1)
            {
                ss << " on Bus["
                   << int_to_hex(static_cast<uint8_t>(misc_decoded.bus_log))
                   << "] Device["
                   << int_to_hex(static_cast<uint8_t>(misc_decoded.device_log))
                   << "] Function["
                   << int_to_hex(static_cast<uint8_t>(misc_decoded.function_log))
                   << "] from Segment["
                   << int_to_hex(static_cast<uint8_t>(misc_decoded.segment_log))
                   << "]";
            }
            entry["MCACOD_decoded"] = ss.str();
        }
    }

    public:
    SkxMcaBankIio(const MCAData& mca) : SkxMcaDecoder(mca)
    {
        status_decoded.mc_status = mca.mc_status;
        misc_decoded.mc_misc = mca.misc;
    }

    json decode_status() override
    {
        entry["MCACOD"] =
            int_to_hex(static_cast<uint16_t>(status_decoded.mcacod));
        entry["MSCOD"] =
            int_to_hex(static_cast<uint16_t>(status_decoded.mscod));
        entry["OTHER_INFO"] =
            int_to_hex(static_cast<uint8_t>(status_decoded.other_info));
        entry["CORRECTED_ERROR_COUNT"] =
            int_to_hex(static_cast<uint16_t>(status_decoded.corrected_err_cnt));
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

class SkxMcaBankM2m final : public SkxMcaDecoder
{
    private:
    union MC_STATUS
    {
        struct
        {
            uint64_t mcacod: 16, mscoddatarderr: 1, reserved0: 1,
            mscodptlwrerr: 1, mscodfullwrerr: 1, mscodbgferr: 1,
            mscodtimeout: 1, mscodparerr: 1, mscodbucket1err: 1,
            mscodddrtype: 2, reserved1: 6, other_info: 6, corrected_err_cnt: 15,
            corr_err_status_ind: 2, ar: 1, s: 1, pcc: 1, addrv: 1, miscv: 1,
            en: 1, uc: 1, overflow: 1, valid: 1;
        };
        uint64_t mc_status;
    };

    union mcacodData
    {
        struct
        {
            uint16_t mcacod_3_0 : 4, mcacod_6_4 : 3, mcacod_15_7 : 9;
        };
        uint16_t mcacod_15_0;
    };

    const std::map<uint8_t, const char*> decodeDdrType = {
       {0x0, "Error not logged neither on DDR4 nor NVMDIMM"},
       {0x1, "Error specifically on DDR4"},
       {0x2, "Error specifically on NVMDIMM"},
       {0x3, "Error for this transaction was detected on both DDR4 and NVMDIMM"},
    };

    const std::map<uint16_t, const char*> decodeMcacod150 = {
        {0x400, "Time-out"},
        {0x5, "Parity error on internal Mesh2mem structures"},
    };

    const std::map<uint8_t, const char*> decodeMcacod157 = {
        {0x4, "Near-Memory Cache controller error"},
        {0x1, "Last level memory controller error."},
    };

    const std::map<uint8_t, const char*> decodeMcacod64 = {
        {0x0, "Generic undefined request"},
        {0x1, "Memory Read Error"},
        {0x2, "Memory Write Error"},
        {0x3, "Address/Command Error"},
        {0x4, "Memory Scrubbing Error"},
    };

    const std::map<uint8_t, const char*> decodeMcacod30 = {
        {0x0, "Error on channel 0"},
        {0x1, "Error on channel 1"},
        {0x2, "Error on channel 2"},
        {0x15, "Channel not specified"},
    };

    std::string decodeMcacod(uint16_t mcacod)
    {
        std::stringstream ss;
        mcacodData mcacodDecoded;
        mcacodDecoded.mcacod_15_0 = mcacod;

        auto mcacod150 = getDecoded(
            decodeMcacod150, static_cast<uint16_t>(mcacodDecoded.mcacod_15_0));

        if (mcacod150)
        {
            ss << *mcacod150 << "|";
        }
        else
        {
            auto mcacod157 = getDecoded(decodeMcacod157,
                 static_cast<uint8_t>(mcacodDecoded.mcacod_15_7));
            if (mcacod157)
            {
                ss << *mcacod157 << "|";
                auto mcacod64 = getDecoded(decodeMcacod64,
                    static_cast<uint8_t>(mcacodDecoded.mcacod_6_4));
                auto mcacod30 = getDecoded(decodeMcacod30,
                    static_cast<uint8_t>(mcacodDecoded.mcacod_3_0));
                if (mcacod64)
                {
                    ss << *mcacod64 << "|";
                }
                if (mcacod30)
                {
                    ss << *mcacod30;
                }
            }
        }

        return ss.str();
    }

    MC_STATUS status_decoded;
    json entry;

    void decodeKnownFields()
    {
        auto mscodDdrType = getDecoded(decodeDdrType,
            static_cast<uint8_t>(status_decoded.mscodddrtype));
        if (mscodDdrType)
        {
            entry["MSCODDDRTYPE_decoded"] = *mscodDdrType;
        }

        auto corrErrStsIndDecoded = getDecoded(CorrErrStsInd,
            static_cast<uint8_t>(status_decoded.corr_err_status_ind));
        if (corrErrStsIndDecoded)
        {
            entry["CORR_ERR_STS_IND_decoded"] = *corrErrStsIndDecoded;
        }

        if (status_decoded.mscoddatarderr == 1)
        {
            entry["MSCODDATARDERR_decoded"] = "MC read data error";
        }

        if (status_decoded.mscodptlwrerr == 1)
        {
            entry["MSCODPTLWRERR_decoded"] = "MC partial write data error";
        }

        if (status_decoded.mscodfullwrerr == 1)
        {
            entry["MSCODFULLWRERR_decoded"] = "Full write data error";
        }

        if (status_decoded.mscodbgferr == 1)
        {
            entry["MSCODBGFERR_decoded"] = "M2M clock-domain-crossing buffer "
                "(BGF) error";
        }

        if (status_decoded.mscodtimeout == 1)
        {
            entry["MSCODTIMEOUT_decoded"] = "M2M timeout";
        }

        if (status_decoded.mscodparerr == 1)
        {
            entry["MSCODPARERR_decoded"] = "M2M tracker parity error";
        }

        if (status_decoded.mscodbucket1err == 1)
        {
            entry["MSCODBUCKET1ERR_decoded"] = "Bucket1 error";
        }

        entry["MCACOD_decoded"] =
            decodeMcacod(static_cast<uint16_t>(status_decoded.mcacod));
    }

    public:
    SkxMcaBankM2m(const MCAData& mca) : SkxMcaDecoder(mca)
    {
        status_decoded.mc_status = mca.mc_status;
    }

    json decode_status() override
    {
        entry["MCACOD"] =
            int_to_hex(static_cast<uint16_t>(status_decoded.mcacod));
        entry["MSCODDATARDERR"] =
            int_to_hex(static_cast<bool>(status_decoded.mscoddatarderr));
        entry["MSCODPTLWRERR"] =
            int_to_hex(static_cast<bool>(status_decoded.mscodptlwrerr));
        entry["MSCODFULLWRERR"] =
            int_to_hex(static_cast<bool>(status_decoded.mscodfullwrerr));
        entry["MSCODBGFERR"] =
            int_to_hex(static_cast<bool>(status_decoded.mscodbgferr));
        entry["MSCODTIMEOUT"] =
            int_to_hex(static_cast<bool>(status_decoded.mscodtimeout));
        entry["MSCODPARERR"] =
            int_to_hex(static_cast<bool>(status_decoded.mscodparerr));
        entry["MSCODBUCKET1ERR"] =
            int_to_hex(static_cast<bool>(status_decoded.mscodbucket1err));
        entry["MSCODDDRTYPE"] =
            int_to_hex(static_cast<uint8_t>(status_decoded.mscodddrtype));
        entry["OTHER_INFO"] =
            int_to_hex(static_cast<uint8_t>(status_decoded.other_info));
        entry["CORRECTED_ERROR_COUNT"] = int_to_hex(
            static_cast<uint16_t>(status_decoded.corrected_err_cnt));
        entry["CORR_ERR_STS_IND"] =
            int_to_hex(static_cast<uint8_t>(status_decoded.corr_err_status_ind));
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

class SkxMcaBankCha final : public SkxMcaDecoder
{
    private:
    union MC_STATUS
    {
        struct
        {
            uint64_t mcacod: 16, mscod: 16, other_info: 6, corrected_err_cnt: 15,
            corr_err_status_ind: 2, ar: 1, s: 1, pcc: 1, addrv: 1, miscv: 1,
            en: 1, uc: 1, overflow: 1, valid: 1;
        };
        uint64_t mc_status;
    };

    union MC_MISC
    {
        struct
        {
            uint64_t reserved: 39, torid: 5, origreq: 10,
            reserved2: 10;
        };
        uint64_t mc_misc;
    };

    MC_STATUS status_decoded;
    MC_MISC misc_decoded;
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
            std::stringstream ss;
            ss << *mscodDecoded;
            auto origreqDecoded = getDecoded(SkxOpCodeDecode,
            static_cast<uint32_t>(misc_decoded.origreq));
            if (origreqDecoded)
            {
                ss << "_OriginalReq[" + *origreqDecoded + "]_TorID[" +
                    std::to_string(misc_decoded.torid) + "]";
            }

            entry["MSCOD_decoded"] = ss.str();
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
    SkxMcaBankCha(const MCAData& mca) : SkxMcaDecoder(mca)
    {
        status_decoded.mc_status = mca.mc_status;
        misc_decoded.mc_misc = mca.misc;
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
            static_cast<uint16_t>(status_decoded.corrected_err_cnt));
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

class SkxMcaBankImc final : public SkxMcaDecoder
{
    private:
    union MC_STATUS
    {
        struct
        {
            uint64_t mcacod: 16, mscod: 16, other_info: 6, corrected_err_cnt: 15,
            corr_err_status_ind: 2, ar: 1, s: 1, pcc: 1, addrv: 1, miscv: 1,
            en: 1, uc: 1, overflow: 1, valid: 1;
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
    SkxMcaBankImc(const MCAData& mca) : SkxMcaDecoder(mca)
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
            static_cast<uint16_t>(status_decoded.corrected_err_cnt));
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

class SkxMcaBankCbo final : public SkxMcaDecoder
{
  private:
    union MC_STATUS
    {
        struct
        {
            uint64_t mcacod : 16, mscod : 16, other_info : 6,
                corrected_error_count : 15, corr_err_sts_ind : 2, ar : 1, s : 1,
                pcc : 1, addrv : 1, miscv : 1, en : 1, uc : 1, overflow : 1,
                valid : 1;
        };
        uint64_t mc_status;
    };

    union MC_MISC
    {
        struct
        {
            uint64_t reserved: 39, torid: 5, origreq: 10,
            reserved2: 10;
        };
        uint64_t mc_misc;
    };

    const std::map<uint16_t, const char*> decodeMscod = {
        {0x0001, "UNCORRECTABLE_DATA_ERROR"},
        {0x0002, "UNCORRECTABLE_TAG_ERROR"},
        {0x0003, "SAD_ERR_WB_TO_MMIO"},
        {0x0004, "SAD_ERR_IA_ACCESS_TO_GSM"},
        {0x0005, "SAD_ERR_CORRUPTING_OTHER"},
        {0x0006, "SAD_ERR_NON_CORRUPTING_OTHER"},
        {0x0007, "CORRECTABLE_DATA_ERROR"},
        {0x0008, "MEM_POISON_DATA_ERROR"},
        {0x000A, "PARITY_DATA_ERROR"},
        {0x000B, "CORE_WB_MISS_LLC"},
        {0x000C, "TOR_TIMEOUT"},
        {0x000D, "Internal Error"},
        {0x000E, "Internal Error"},
        {0x000F, "Internal Error"},
        {0x0011, "LLC_TAG_CORR_ERR"},
        {0x0012, "LLC_STATE_CORR_ERR"},
        {0x0013, "LLC_STATE_UNCORR_ERR"},
        {0x0014, "LLC_CV_CORR_ERR"},
        {0x0015, "LLC_CV_UNCORR_ERR"},
        {0x0016, "Internal Protocol Error"},
        {0x0017, "MULT_LLC_WAY_TAG_MATCH"},
        {0x0018, "BL_REQ_RTID_TABLE_MISS"},
        {0x0019, "AK_REQ_RTID_TABLE_MISS"},
        {0x001C, "IDI_JITTER_ERROR"},
        {0x001D, "Internal Parity Error"},
        {0x001E, "Internal Parity Error"},
        {0x001F, "Internal Parity Error"},
        {0x0021, "UNCORRECTABLE_SnoopFilter_TAG_ERROR"},
        {0x0022, "SnoopFilter_TAG_CORR_ERR"},
        {0x0023, "SnoopFilter_STATE_CORR_ERR"},
        {0x0024, "SnoopFilter_STATE_UNCORR_ERR"},
        {0x0025, "SnoopFilter_CV_CORR_ERR"},
        {0x0026, "SnoopFilter_CV_UNCORR_ERR"},
        {0x0027, "SAD_ERR_LTMEMLOCK"},
        {0x0028, "LLC_TWOLM_CORR_ERR"},
        {0x0029, "LLC_TWOLM_UNCORR_ERR"},
        {0x002A, "ISMQ_UNEXP_RSP"},
        {0x002B, "TWOLM_MULT_HIT"},
        {0x002C, "HA_UNEXP_RSP"},
        {0x002D, "SAD_ERR_RRQWBQ_TO_NONHOM"},
        {0x002E, "SAD_ERR_IIOTONONHOM"},
    };

    MC_STATUS status_decoded;
    MC_MISC misc_decoded;
    json entry;

    void decodeKnownFields()
    {
        auto corrErrStsIndDecoded =
            getDecoded(CorrErrStsInd,
                        static_cast<uint8_t>(status_decoded.corr_err_sts_ind));
        if (corrErrStsIndDecoded)
        {
            entry["CORR_ERR_STS_IND_decoded"] = *corrErrStsIndDecoded;
        }

        auto mscod = getDecoded(decodeMscod,
                                  static_cast<uint16_t>(status_decoded.mscod));
        if (mscod)
        {
            std::stringstream ss;
            ss << *mscod;
            auto origreqDecoded = getDecoded(SkxOpCodeDecode,
            static_cast<uint32_t>(misc_decoded.origreq));
            if (origreqDecoded)
            {
                ss << "_OriginalReq[" + *origreqDecoded + "]_TorID[" +
                    std::to_string(misc_decoded.torid) + "]";
            }

            entry["MSCOD_decoded"] = ss.str();
        }

        auto mcacod = getDecoded(
            McacodGeneric1, static_cast<uint16_t>(status_decoded.mcacod));
        if (mcacod)
        {
            entry["MCACOD_decoded"] = *mcacod;
        }
        else
        {
            entry["MCACOD_decoded"] =
                decodeMcacodGeneric1(status_decoded.mcacod);
        }
    }

  public:
    SkxMcaBankCbo(const MCAData& mca) : SkxMcaDecoder(mca)
    {
        status_decoded.mc_status = mca.mc_status;
        misc_decoded.mc_misc = mca.misc;
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
        entry["CORR_ERR_STS_IND"] =
            int_to_hex(static_cast<uint8_t>(status_decoded.corr_err_sts_ind));
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