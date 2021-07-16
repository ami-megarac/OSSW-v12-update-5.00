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
#include <string>
#include <sstream>

#include <mca_defs.hpp>
#include <tor_defs_spr.hpp>
#include <utils.hpp>

using json = nlohmann::json;

class SprMcaDecoder : public McaDecoder
{
  public:
    SprMcaDecoder(const MCAData& mca) : McaDecoder(mca) {};

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
        {0x7, "Intel UPI 1"},
        {0x8, "Intel UPI 2"},
        {0x9, "CHA_A"},
        {0xA, "CHA_B"},
        {0xB, "CHA_C"},
        {0xC, "M2M0"},
        {0xD, "IMC 0, channel 0"},
        {0xE, "IMC 0, channel 1"},
        {0xF, "IMC 0, channel 2"},
        {0x10, "M2M1"},
        {0x11, "IMC 1, channel 0"},
        {0x12, "IMC 1, channel 1"},
        {0x13, "IMC 1, channel 2"},
        {0x14, "M2M2"},
        {0x15, "IMC 2, channel 0"},
        {0x16, "IMC 2, channel 1"},
        {0x17, "IMC 2, channel 2"},
        {0x18, "M2M3"},
        {0x19, "IMC 3, channel 0"},
        {0x1A, "IMC 3, channel 1"},
        {0x1B, "IMC 3, channel 2"},
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

    const std::map<uint8_t, const char*> ErrSpecific = {
        {0x5, "Parity error on structures"},
    };

    const std::map<uint16_t, const char*> MscodGeneric2 = {
        {0x1, "Address Parity Error"},
        {0x2, "Wr data parity Error"},
        {0x3, "CMI Uncorr/Corr ECC error"},
        {0x4, "Wr BE parity Error"},
        {0x8, "Corr Patrol Scrub Error"},
        {0x10, "UnCorr Patrol Scrub Error"},
        {0x20, "Corr Spare Error"},
        {0x40, "UnCorr Spare Error"},
        {0x80, "Corr Rd Error"},
        {0xa0, "UnCorr Rd Error"},
        {0xc0, "UnCorr MetaData"},
        {0x104, "RPQ1 parity error"},
        {0x105, "WPQ parity error"},
        {0x106, "DDRT write data parity error"},
        {0x107, "DDRT write BE parity error"},
        {0x108, "DDR/DDRT link fail"},
        {0x111, "PCLS CAM error"},
        {0x112, "PCLS data error"},
        {0x200, "DDR4 CA Parity"},
        {0x400, "Scheduler address parity error"},
        {0x800, "Unrecognized request type"},
        {0x801, "Read response to an invalid scoreboard entry"},
        {0x802, "Unexpected read response"},
        {0x803, "DDR4 completion to an invalid scoreboard entry"},
        {0x804, "DDRT completion to an invalid scoreboard entry"},
        {0x805, "Completion FIFO overflow"},
        {0x806, "ERID Correctable parity error"},
        {0x807, "ERID Uncorrectable error"},
        {0x808, "Interrupt received while outstanding interrupt was not ACKed"},
        {0x809, "ERID FIFO overflow"},
        {0x80a, "Error on Write credits"},
        {0x80b, "Error on Read credits"},
        {0x80c, "Scheduler error"},
        {0x80d, "DDRT error event"},
        {0x80e, "FNV thermal error"},
        {0x80f, "CMI packet while idle"},
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

class SprMcaBankIfu final : public SprMcaDecoder
{
  private:
    union MC_STATUS
    {
      struct
      {
          uint64_t mcacod : 11, enh_mca_avail0: 5, mscod: 5, enh_mca_avail1: 17,
          corrected_err_cnt: 14, reserved: 1, enh_mca_avail2: 2, ar: 1, s: 1,
          pcc: 1, addrv: 1, miscv: 1, en: 1, uc: 1, overflow: 1, valid: 1;
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
        {0xc, "IFU Poison Error"},
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
        SprMcaBankIfu(const MCAData& mca) : SprMcaDecoder(mca)
    {
        status_decoded.mc_status = mca.mc_status;
    }

    json decode_status() override
    {
        entry["MCACOD"] =
            int_to_hex(static_cast<uint16_t>(status_decoded.mcacod));
        entry["ENH_MCA_AVAIL0"] =
          int_to_hex(static_cast<uint16_t>(status_decoded.enh_mca_avail0));
        entry["MSCOD"] =
            int_to_hex(static_cast<uint16_t>(status_decoded.mscod));
        entry["ENH_MCA_AVAIL1"] =
          int_to_hex(static_cast<uint16_t>(status_decoded.enh_mca_avail1));
        entry["CORRECTED_ERROR_COUNT"] = int_to_hex(
            static_cast<uint16_t>(status_decoded.corrected_err_cnt));
        entry["RESERVED"] =
            int_to_hex(static_cast<uint8_t>(status_decoded.reserved));
        entry["ENH_MCA_AVAIL2"] =
          int_to_hex(static_cast<uint16_t>(status_decoded.enh_mca_avail2));
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

class SprMcaBankDcu final : public SprMcaDecoder
{
  private:
    union MC_STATUS
    {
      struct
      {
          uint64_t mcacod: 16, mscod: 22, corrected_err_cnt: 15,
          enh_mca_avail: 2, ar: 1, s: 1, pcc: 1, addrv: 1,
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
    SprMcaBankDcu(const MCAData& mca) : SprMcaDecoder(mca)
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
      entry["ENH_MCA_AVAIL"] = int_to_hex(
          static_cast<uint8_t>(status_decoded.enh_mca_avail));
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

class SprMcaBankDtlb final : public SprMcaDecoder
{
private:
    union MC_STATUS
    {
      struct
      {
          uint64_t mcacod: 16, mscod: 16, enh_mca_avail0: 6, corrected_err_cnt: 15,
          enh_mca_avail1: 4, pcc: 1, addrv: 1, miscv: 1, en: 1, uc: 1, overflow: 1,
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
    SprMcaBankDtlb(const MCAData& mca) : SprMcaDecoder(mca)
    {
        status_decoded.mc_status = mca.mc_status;
    }

    json decode_status() override
    {
        entry["MCACOD"] =
            int_to_hex(static_cast<uint16_t>(status_decoded.mcacod));
        entry["MSCOD"] =
            int_to_hex(static_cast<uint16_t>(status_decoded.mscod));
        entry["ENH_MCA_AVAIL0"] =
            int_to_hex(static_cast<uint8_t>(status_decoded.enh_mca_avail0));
        entry["CORRECTED_ERROR_COUNT"] = int_to_hex(
            static_cast<uint16_t>(status_decoded.corrected_err_cnt));
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

  class SprMcaBankMlc final : public SprMcaDecoder
{
    private:
    union MC_STATUS
    {
        struct
        {
            uint64_t mcacod: 16, mscod: 16, enh_mca_avail0: 6, corrected_err_cnt: 15,
            green_tracking: 1, yellow_tracking: 1, ar: 1, s: 1, pcc: 1, addrv: 1,
            miscv: 1, en: 1, uc: 1, overflow: 1, valid: 1;
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
    SprMcaBankMlc(const MCAData& mca) : SprMcaDecoder(mca)
    {
        status_decoded.mc_status = mca.mc_status;
    }

    json decode_status() override
    {
        entry["MCACOD"] =
            int_to_hex(static_cast<uint16_t>(status_decoded.mcacod));
        entry["MSCOD"] =
            int_to_hex(static_cast<uint16_t>(status_decoded.mscod));
        entry["ENH_MCA_AVAIL0"] =
            int_to_hex(static_cast<uint8_t>(status_decoded.enh_mca_avail0));
        entry["CORRECTED_ERROR_COUNT"] = int_to_hex(
            static_cast<uint16_t>(status_decoded.corrected_err_cnt));
        entry["GREEN_TRACKING"] = int_to_hex(
            static_cast<bool>(status_decoded.green_tracking));
        entry["YELLOW_TRACKING"] = int_to_hex(
            static_cast<bool>(status_decoded.yellow_tracking));
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

class SprMcaBankPcu final : public SprMcaDecoder
{
    union MC_STATUS
    {
        struct
        {
            uint64_t mcacod: 16, mscod_16_19: 4, mscod_20_23: 4, mscod_24_31: 8,
            other_info: 5, fw_upd: 1, corrected_err_cnt: 15, thrs_err_st: 2,
            ar: 1, s: 1, pcc: 1, addrv: 1, miscv: 1, en: 1, uc: 1, overflow: 1,
            valid: 1;
        };
        uint64_t mc_status;
    };

    const std::map<uint16_t, const char*> decodeMcacod = {
        {0x400, "Core 3-strike, see firstierrsrcid in ubox for which core"},
        {0x401, "WB Access to APIC memory"},
        {0x402, "PCU errors"},
        {0x405, "Internal errors"},
        {0x40B, "Internal Firmware errors"},
        {0x40C, "Internal errors"},
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
        {0x4, "Clock/power IP response timeout"},
        {0x5, "SMBus controller raised SMI"},
        {0x9, "PM controller received invalid transaction"},
    };

    const std::map<uint8_t, const char*> decodeMscod2431 = {
        {0xd, "LLC_BIST_ACTIVE_TIMEOUT"},
        {0xe, "DMI_TRAINING_TIMEOUT"},
        {0xf, "DMI_STRAP_SET_ARRIVAL_TIMEOUT"},
        {0x10, "DMI_CPU_RESET_ACK_TIMEOUT"},
        {0x11, "MORE_THAN_ONE_LT_AGENT"},
        {0x14, "INCOMPATIBLE_PCH_TYPE"},
        {0x1e, "BIOS_RST_CPL_INVALID_SEQ"},
        {0x1f, "BIOS_INVALID_PKG_STATE_CONFIG"},
        {0x2d, "PCU_PMAX_CALIB_ERROR"},
        {0x2e, "TSC100_SYNC_TIMEOUT"},
        {0x3a, "GPSB_TIMEOUT"},
        {0x3b, "PMSB_TIMEOUT"},
        {0x3e, "IOSFSB_PMREQ_CMP_TIMEOUT"},
        {0x40, "SVID_VCCIN_VR_ICC_MAX_FAILURE"},
        {0x42, "SVID_VCCIN_VR_VOUT_FAILURE"},
        {0x43, "SVID_CPU_VR_CAPABILITY_ERROR"},
        {0x44, "SVID_CRITICAL_VR_FAILED"},
        {0x45, "SVID_SA_ITD_ERROR"},
        {0x46, "SVID_READ_REG_FAILED"},
        {0x47, "SVID_WRITE_REG_FAILED"},
        {0x4a, "SVID_PKGC_REQUEST_FAILED"},
        {0x4b, "SVID_IMON_REQUEST_FAILED"},
        {0x4c, "SVID_ALERT_REQUEST_FAILED"},
        {0x4d, "SVID_MCP_VR_RAMP_ERROR"},
        {0x56, "FIVR_PD_HARDERR"},
        {0x58, "WATCHDOG_TIMEOUT_PKGC_SLAVE"},
        {0x59, "WATCHDOG_TIMEOUT_PKGC_MASTER"},
        {0x5a, "WATCHDOG_TIMEOUT_PKGS_MASTER"},
        {0x5b, "WATCHDOG_TIMEOUT_MSG_CH_FSM"},
        {0x5c, "WATCHDOG_TIMEOUT_BULK_CR_FSM"},
        {0x5d, "WATCHDOG_TIMEOUT_IOSFSB_FSM"},
        {0x60, "PKGS_SAFE_WP_TIMEOUT"},
        {0x61, "PKGS_CPD_UNCPD_TIMEOUT"},
        {0x62, "PKGS_INVALID_REQ_PCH"},
        {0x63, "PKGS_INVALID_REQ_INTERNAL"},
        {0x64, "PKGS_INVALID_RSP_INTERNAL"},
        {0x65, "PKGS_RESET_PREP_TIMEOUT"},
        {0x66, "PKGS_RESET_PREP_TIMEOUT"},
        {0x67, "PKGS_RESET_PREP_TIMEOUT"},
        {0x68, "PKGS_RESET_PREP_TIMEOUT"},
        {0x69, "PKGS_RESET_PREP_TIMEOUT"},
        {0x6a, "PKGS_RESET_PREP_TIMEOUT"},
        {0x6b, "PKGS_RESET_PREP_TIMEOUT"},
        {0x6c, "PKGS_RESET_PREP_TIMEOUT"},
        {0x6d, "PKGS_RESET_PREP_TIMEOUT"},
        {0x6e, "PKGS_RESET_PREP_TIMEOUT"},
        {0x6f, "PKGS_RESET_PREP_TIMEOUT"},
        {0x70, "PKGS_RESET_PREP_TIMEOUT"},
        {0x71, "PKGS_RESET_PREP_TIMEOUT"},
        {0x72, "PKGS_RESET_PREP_TIMEOUT"},
        {0x73, "PKGS_RESET_PREP_TIMEOUT"},
        {0x74, "PKGS_RESET_PREP_TIMEOUT"},
        {0x75, "PKGS_RESET_PREP_TIMEOUT"},
        {0x76, "PKGS_RESET_PREP_TIMEOUT"},
        {0x77, "PKGS_RESET_PREP_TIMEOUT"},
        {0x78, "PKGS_RESET_PREP_TIMEOUT"},
        {0x79, "PKGS_RESET_PREP_TIMEOUT"},
        {0x7a, "PKGS_RESET_PREP_TIMEOUT"},
        {0x80, "MCA_PKGS_UNIT_TYPE_SPDI3C_TIMEOUT"},
        {0x86, "PKGS_SMBUS_VPP_PAUSE_TIMEOUT"},
        {0x87, "PKGS_SMBUS_MCP_PAUSE_TIMEOUT"},
        {0x88, "PKGS_SMBUS_SPD_PAUSE_TIMEOUT"},
        {0x91, "PKGC_INVALID_RSP_PCH"},
        {0x93, "PKGC_WATCHDOG_HANG_CBZ_DOWN"},
        {0x94, "PKGC_WATCHDOG_HANG_CBZ_UP"},
        {0x98, "PKGC_WATCHDOG_HANG_C2_BLKMASTER"},
        {0x99, "PKGC_WATCHDOG_HANG_C2_PSLIMIT"},
        {0x9a, "PKGC_WATCHDOG_HANG_SETDISP"},
        {0x9d, "PKGC_ALLOW_L1_ERROR"},
        {0xa0, "RECOVERABLE_DIE_THERMAL_TOO_HOT"},
        {0xb0, "ADR_SIGNAL_TIMEOUT"},
        {0xb1, "BCLK_FREQ_OC_ABOVE_THRESHOLD"},
        {0xc0, "DISPATCHER_RUN_BUSY_TIMEOUT"},
    };

    MC_STATUS status_decoded;
    json entry;

    void decodeKnownFields()
    {
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
    SprMcaBankPcu(const MCAData& mca) : SprMcaDecoder(mca)
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
        entry["OTHER_INFO"] =
            int_to_hex(static_cast<uint8_t>(status_decoded.other_info));
        entry["FW_UPD"] = int_to_hex(static_cast<bool>(status_decoded.fw_upd));
        entry["CORRECTED_ERROR_COUNT"] = int_to_hex(
            static_cast<uint16_t>(status_decoded.corrected_err_cnt));
        entry["THRS_ERR_ST"] = int_to_hex(
            static_cast<uint8_t>(status_decoded.thrs_err_st));
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

class SprMcaBankUpi final : public SprMcaDecoder
{
    private:
    union MC_STATUS
    {
        struct
        {
            uint64_t mcacod_ll: 2, mcacod_ii: 2, mcacod_rrrr: 4, mcacod_t : 1,
            mcacod_pp: 2, mcacod_int: 1, mcacod_rsvd: 4, mscod: 6, mscod_spare: 10,
            other_info: 5, fw_upd: 1, corrected_err_cnt: 15, corr_err_status_ind: 2,
            ar: 1, s: 1, pcc: 1, addrv: 1, miscv: 1, en: 1, uc: 1, overflow: 1,
            valid: 1;
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

    const std::map<uint8_t, const char*> decodeMcacod_pp = {
        {0x2, "Unsupported/Undefined Packet"},
        {0x3, "Other Errors"},
    };

    const std::map<uint8_t, const char*> decodeMcacod_ii = {
        {0x0, "Memory"},
        {0x1, "Reserved"},
        {0x2, "I/O"},
        {0x3, "Other"},
    };

    const std::map<uint8_t, const char*> decodeMcacod_ll = {
        {0x0, "L0"},
        {0x1, "L1"},
        {0x2, "L2"},
        {0x3, "Generic LL"},
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

        auto mcacod_ppDecoded = getDecoded(
            decodeMcacod_pp, static_cast<uint8_t>(status_decoded.mcacod_pp));
        if (mcacod_ppDecoded)
        {
            entry["MCACOD_PP_decoded"] = *mcacod_ppDecoded;
        }

        auto mcacod_iiDecoded = getDecoded(
            decodeMcacod_ii, static_cast<uint8_t>(status_decoded.mcacod_ii));
        if (mcacod_iiDecoded)
        {
            entry["MCACOD_II_decoded"] = *mcacod_iiDecoded;
        }

        auto mcacod_llDecoded = getDecoded(
            decodeMcacod_ll, static_cast<uint8_t>(status_decoded.mcacod_ll));
        if (mcacod_llDecoded)
        {
            entry["MCACOD_LL_decoded"] = *mcacod_llDecoded;
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
    SprMcaBankUpi(const MCAData& mca) : SprMcaDecoder(mca)
    {
        status_decoded.mc_status = mca.mc_status;
    }

    json decode_status() override
    {
        entry["MCACOD_LL"] =
            int_to_hex(static_cast<uint8_t>(status_decoded.mcacod_ll));
        entry["MCACOD_II"] =
            int_to_hex(static_cast<uint8_t>(status_decoded.mcacod_ii));
        entry["MCACOD_RRRR"] =
            int_to_hex(static_cast<uint8_t>(status_decoded.mcacod_rrrr));
        entry["MCACOD_T"] =
            int_to_hex(static_cast<uint8_t>(status_decoded.mcacod_t));
        entry["MCACOD_PP"] =
            int_to_hex(static_cast<uint8_t>(status_decoded.mcacod_pp));
        entry["MCACOD_INT"] =
            int_to_hex(static_cast<uint8_t>(status_decoded.mcacod_int));
        entry["MCACOD_RSVD"] =
            int_to_hex(static_cast<uint8_t>(status_decoded.mcacod_rsvd));
        entry["MSCOD"] =
            int_to_hex(static_cast<uint8_t>(status_decoded.mscod));
        entry["MSCOD_SPARE"] =
            int_to_hex(static_cast<uint16_t>(status_decoded.mscod_spare));
        entry["OTHER_INFO"] =
            int_to_hex(static_cast<uint8_t>(status_decoded.other_info));
        entry["FW_UPD"] = int_to_hex(static_cast<bool>(status_decoded.fw_upd));
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

class SprMcaBankIio final : public SprMcaDecoder
{
    private:
    union MC_STATUS
    {
        struct
        {
            uint64_t mcacod: 16, mscod: 16, other_info: 5, fw_upd: 1,
            corrected_err_cnt: 15, corr_err_status_ind: 2, ar: 1, s: 1, pcc: 1,
            addrv: 1, miscv: 1, en: 1, uc: 1, overflow: 1, valid: 1;
        };
        uint64_t mc_status;
    };

    const std::map<uint16_t, const char*> decodeMscod = {
        {0x8000, "Poison"},
        {0x8001, "Unsupported Opcode"},
        {0x8002, "Misaligned CFG Rd (SMM)"},
        {0x8003, "Misaligned CFG Wr (SMM)"},
        {0x8004, "Misaligned CFG Rd (Non-SMM)"},
        {0x8005, "Misaligned CFG Wr (Non-SMM)"},
        {0x8006, "Misaligned MMIO Rd (SMM)"},
        {0x8007, "Misaligned MMIO Wr (SMM)"},
        {0x8008, "Misaligned MMIO Rd (Non-SMM)"},
        {0x8009, "Misaligned MMIO Wr (Non-SMM)"},
        {0x800a, "SMI Timeout"},
        {0x800b, "Lock Master Timeout"},
        {0x800e, "Semaphore Error"},
        {0x800f, "AK Egress Write Valid Entry"},
        {0x8010, "BL Egress Write Valid Entry"},
        {0x8011, "AD Egress Write Valid Entry"},
        {0x8012, "AK Egress Overflow"},
        {0x8013, "BL Egress Overflow"},
        {0x8014, "AD Egress Overflow"},
        {0x8015, "CHA to Ubox Overflow (NCB)"},
        {0x8016, "CHA to Ubox Overflow (NCS)"},
        {0x8017, "UPI to Ubox Overflow (NCB)"},
        {0x8018, "UPI to Ubox Overflow (NCS)"},
        {0x8019, "Ingress Parity Error"},
        {0x801a, "MS2IOSF to Ubox Overflow (NCB)"},
        {0x801b, "MS2IOSF to Ubox Overflow (NCS)"},
    };

    const std::map<uint16_t, const char*> decodeMcacod = {
        {0xe0b, "Generic I/O Error"},
        {0x405, "Generic I/O Error"},
        {0x406, "Internal Firmware Error"},
        {0x407, "General UBOX Error"},
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
    SprMcaBankIio(const MCAData& mca) : SprMcaDecoder(mca)
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
        entry["FW_UPD"] = int_to_hex(static_cast<bool>(status_decoded.ar));
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

class SprMcaBankM2m final : public SprMcaDecoder
{
    private:
    union MC_STATUS
    {
        struct
        {
            uint64_t mcacod: 16, mscod: 8, mscodddrtype: 2,
            mscodfailoverwhileresetprep: 1, mscodmiscerrs: 5, other_info: 6,
            corrected_err_cnt: 15, corr_err_status_ind: 2, ar: 1, s: 1, pcc: 1,
            addrv: 1, miscv: 1, en: 1, uc: 1, overflow: 1, valid: 1;
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

    const std::map<uint16_t, const char*> decodeMscod = {
        {0x0, "No error (default)"},
        {0x1, "Read ECC error (MemSpecRd, MemRd, MemRdData, MemRdXto*, MemInv, MemInvXto*, MemInvItoX)"},
        {0x2, "Bucket1 error"},
        {0x3, "RdTrkr Parity error"},
        {0x4, "Secure mismatch"},
        {0x5, "Prefetch channel mismatch"},
        {0x6, "Failover while reset prep"},
        {0x7, "Read completion parity error"},
        {0x8, "Response parity error"},
        {0x9, "Timeout error"},
        {0xa, "CMI reserved credit pool error"},
        {0xb, "CMI total credit count error"},
        {0xc, "CMI credit oversubscription error"},
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
        {0x405, "Parity error on CMI"},
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
        {0xf, "Channel not specified"},
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
        auto corrErrStsIndDecoded = getDecoded(CorrErrStsInd,
            static_cast<uint8_t>(status_decoded.corr_err_status_ind));
        if (corrErrStsIndDecoded)
        {
            entry["CORR_ERR_STS_IND_decoded"] = *corrErrStsIndDecoded;
        }

        auto mscodDdrType = getDecoded(decodeDdrType,
            static_cast<uint8_t>(status_decoded.mscodddrtype));
        if (mscodDdrType)
        {
            entry["MSCODDDRTYPE_decoded"] = *mscodDdrType;
        }

        auto mscodDecoded = getDecoded(
            decodeMscod, static_cast<uint16_t>(status_decoded.mscod));
        if (mscodDecoded)
        {
            entry["MSCOD_decoded"] = *mscodDecoded;
        }

        entry["MCACOD_decoded"] =
            decodeMcacod(static_cast<uint16_t>(status_decoded.mcacod));
    }

    public:
    SprMcaBankM2m(const MCAData& mca) : SprMcaDecoder(mca)
    {
        status_decoded.mc_status = mca.mc_status;
    }

    json decode_status() override
    {
        entry["MCACOD"] =
            int_to_hex(static_cast<uint16_t>(status_decoded.mcacod));
        entry["MSCOD"] =
            int_to_hex(static_cast<uint16_t>(status_decoded.mscod));
        entry["MSCODDDRTYPE"] =
            int_to_hex(static_cast<uint8_t>(status_decoded.mscodddrtype));
        entry["MSCOD_FAILOVER_WHILE_RESET_PREP"] = int_to_hex(
            static_cast<bool>(status_decoded.mscodfailoverwhileresetprep));
        entry["MSCOD_MISC_ERRS"] =
            int_to_hex(static_cast<uint8_t>(status_decoded.mscodmiscerrs));
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

class SprMcaBankCha final : public SprMcaDecoder
{
    private:
    const std::map<uint8_t, const char*> rawChaDecoding9 = {
        {0x0, "Most severe Error ID = RAW CHA0"},
        {0x1, "Most severe Error ID = RAW CHA3"},
        {0x2, "Most severe Error ID = RAW CHA6"},
        {0x3, "Most severe Error ID = RAW CHA9"},
        {0x4, "Most severe Error ID = RAW CHA12"},
        {0x5, "Most severe Error ID = RAW CHA15"},
        {0x6, "Most severe Error ID = RAW CHA18"},
        {0x7, "Most severe Error ID = RAW CHA21"},
        {0x8, "Most severe Error ID = RAW CHA24"},
        {0x9, "Most severe Error ID = RAW CHA27"},
        {0xA, "Most severe Error ID = RAW CHA30"},
        {0xB, "Most severe Error ID = RAW CHA33"},
        {0xC, "Most severe Error ID = RAW CHA36"},
        {0xD, "Most severe Error ID = RAW CHA39"}
    };

    const std::map<uint8_t, const char*> rawChaDecoding10 = {
        {0x0, "Most severe Error ID = RAW CHA1"},
        {0x1, "Most severe Error ID = RAW CHA4"},
        {0x2, "Most severe Error ID = RAW CHA7"},
        {0x3, "Most severe Error ID = RAW CHA10"},
        {0x4, "Most severe Error ID = RAW CHA13"},
        {0x5, "Most severe Error ID = RAW CHA16"},
        {0x6, "Most severe Error ID = RAW CHA19"},
        {0x7, "Most severe Error ID = RAW CHA22"},
        {0x8, "Most severe Error ID = RAW CHA25"},
        {0x9, "Most severe Error ID = RAW CHA28"},
        {0xA, "Most severe Error ID = RAW CHA31"},
        {0xB, "Most severe Error ID = RAW CHA34"},
        {0xC, "Most severe Error ID = RAW CHA37"}
    };

    const std::map<uint8_t, const char*> rawChaDecoding11 = {
        {0x0, "Most severe Error ID = RAW CHA2"},
        {0x1, "Most severe Error ID = RAW CHA5"},
        {0x2, "Most severe Error ID = RAW CHA8"},
        {0x3, "Most severe Error ID = RAW CHA11"},
        {0x4, "Most severe Error ID = RAW CHA14"},
        {0x5, "Most severe Error ID = RAW CHA17"},
        {0x6, "Most severe Error ID = RAW CHA20"},
        {0x7, "Most severe Error ID = RAW CHA23"},
        {0x8, "Most severe Error ID = RAW CHA26"},
        {0x9, "Most severe Error ID = RAW CHA29"},
        {0xA, "Most severe Error ID = RAW CHA32"},
        {0xB, "Most severe Error ID = RAW CHA35"},
        {0xC, "Most severe Error ID = RAW CHA38"},
        {0xD, "Most severe Error ID = RAW CHA41"}
    };
    union MC_STATUS
    {
         struct
        {
            uint64_t aggregation : 32, max_instance: 5, health0: 1,
                corrected_error_count: 15, raw_cha: 5, ar: 1, s: 1, pcc: 1,
                uc: 1, health1 : 1, valid: 1;
        };
        uint64_t mc_status;
    };

    MC_STATUS status_decoded;
    json entry;

    void decodeKnownFields()
    {
        std::optional<std::string> rawChaDecoded;
        if (mca.bank == 9)
        {
            rawChaDecoded = getDecoded(
                rawChaDecoding9, static_cast<uint8_t>(status_decoded.raw_cha));
        }
        else if (mca.bank == 10)
        {
            rawChaDecoded = getDecoded(
                rawChaDecoding10, static_cast<uint8_t>(status_decoded.raw_cha));
        }
        else if (mca.bank == 11)
        {
            rawChaDecoded = getDecoded(
                rawChaDecoding11, static_cast<uint8_t>(status_decoded.raw_cha));
        }
        if (rawChaDecoded)
        {
            entry["INSTANCE_ID_decoded"] = *rawChaDecoded;
        }
    }

    public:
    SprMcaBankCha(const MCAData& mca) : SprMcaDecoder(mca)
    {
        status_decoded.mc_status = mca.mc_status;
    }

    json decode_status() override
    {
        entry["AGGREGATION"] =
            int_to_hex(static_cast<uint32_t>(status_decoded.aggregation));
        entry["MAX_INSTANCE"] =
            int_to_hex(static_cast<uint8_t>(status_decoded.max_instance));
        entry["HEALTH0"] =
            int_to_hex(static_cast<bool>(status_decoded.health0));
        entry["CORRECTED_ERROR_COUNT"] = int_to_hex(
            static_cast<uint16_t>(status_decoded.corrected_error_count));
        entry["INSTANCE_ID"] =
            int_to_hex(static_cast<uint8_t>(status_decoded.raw_cha));
        entry["AR"] = int_to_hex(static_cast<bool>(status_decoded.ar));
        entry["S"] = int_to_hex(static_cast<bool>(status_decoded.s));
        entry["PCC"] = int_to_hex(static_cast<bool>(status_decoded.pcc));
        entry["UC"] = int_to_hex(static_cast<bool>(status_decoded.uc));
        entry["HEALTH1"] =
            int_to_hex(static_cast<bool>(status_decoded.health1));
        entry["VALID"] = int_to_hex(static_cast<bool>(status_decoded.valid));
        decodeKnownFields();

        return entry;
    }
};

class SprMcaBankImc final : public SprMcaDecoder
{
    private:
    union MC_STATUS
    {
        struct
        {
            uint64_t mcacod: 16, mscod: 16, other_info: 5, fw_upd: 1,
            corrected_err_cnt: 15, corr_err_status_ind: 2, ar: 1, s: 1, pcc: 1,
            addrv: 1, miscv: 1, en: 1, uc: 1, overflow: 1, valid: 1;
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
    SprMcaBankImc(const MCAData& mca) : SprMcaDecoder(mca)
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
        entry["FW_UPD"] = int_to_hex(static_cast<bool>(status_decoded.fw_upd));
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

class SprMcaBankCbo final : public SprMcaDecoder
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
            uint64_t reserved: 37, torid: 5, origreq: 11,
            reserved2: 11;
        };
        uint64_t mc_misc;
    };

    const std::map<uint16_t, const char*> decodeMscod = {
        {0x0001, "Uncorrectable Data Error"},
        {0x0002, "Uncorrectable Tag Error"},
        {0x0003, "SAD_ERR_WB_TO_MMIO"},
        {0x0004, "SAD_ERR_IA_ACCESS_TO_GSM"},
        {0x0005, "SAD_ERR_CORRUPTING_OTHER"},
        {0x0006, "SAD_ERR_NON_CORRUPTING_OTHER"},
        {0x0007, "Correctable Data Error"},
        {0x0008, "MEM_POISON_DATA_ERROR"},
        {0x0009, "SAD_ERR_CRABABORT"},
        {0x000A, "Parity Data Error"},
        {0x000B, "CORE_WB_MISS_LCC"},
        {0x000C, "TOR Time Out"},
        {0x000D, "ISMQ_REQ_2_INVLD_TOR_ENTRY"},
        {0x000E, "HA Parity Tracker Error"},
        {0x000F, "COH_TT_ERR"},
        {0x0011, "LLC_TAG_CORR_ERR"},
        {0x0012, "LLC_STATE_CORR_ERR"},
        {0x0013, "LLC_STATE_UNCORR_ERR"},
        {0x0016, "MULT_TOR_ENTRY_MATCH"},
        {0x0017, "MULT_LCC_WAY_TAG_MATCH"},
        {0x0018, "BL_REG_RTID_TABLE_MISS"},
        {0x0019, "AK_REG_RTID_TABLE_MISS"},
        {0x001C, "IDI_JITTER_ERROR"},
        {0x001D, "SEGR_PARITY_ERROR"},
        {0x001E, "SINGR_PARITY_ERROR"},
        {0x001F, "ADDR_PARITY_ERROR"},
        {0x0021, "UNCORRECTABLE_SF_TAG_ERROR"},
        {0x0022, "SnoopFilter_TAG_CORR_ERR"},
        {0x0023, "SnoopFilter_STATE_CORR_ERR"},
        {0x0024, "SnoopFilter_STATE_UNCORR_ERR"},
        {0x0027, "SAD_ERR_LTMEMLOCK"},
        {0x0028, "LLC_TWOLM_CORR_ERR"},
        {0x0029, "LLC_TWOLM_UNCORR_ERR"},
        {0x002A, "ISMQ_UNEXPECTED_RSP"},
        {0x002B, "TWOLM_MULT_HIT"},
        {0x002C, "HA_UNEXPECTED_RSP"},
        {0x002D, "SAD_ERR_RRQWBQ_TO_NONHOM"},
        {0x002E, "SAD_ERR_IIOTONONHOM"},
        {0x002F, "PARITY_REPAIR_ERROR"},
        {0x0031, "SF_TWOLM_CORR_ERR"},
        {0x0032, "SF_TWOLM_UNCORR_ERR"},
        {0x0033, "AK_BL_UQID_PTY_ERROR"},
        {0x0034, "WXSNP_WITH_SNPCOUNT_ZERO"},
        {0x0035, "MEM_PUSH_WR_NS_S"},
        {0x0036, "SAD_ERR_UNSECURE_UPI_ACCESS"},
        {0x0037, "CLFLUSH_MMIO_HIT_M"},
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
            auto origreqDecoded = getDecoded(SprOpCodeDecode,
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
            entry["MCACOD_decoded"] = decodeMcacodGeneric1(status_decoded.mcacod);
        }
    }

  public:
    SprMcaBankCbo(const MCAData& mca) : SprMcaDecoder(mca)
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