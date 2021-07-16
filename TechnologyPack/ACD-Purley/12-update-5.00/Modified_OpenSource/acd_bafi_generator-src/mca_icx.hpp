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
#include <optional>
#include <string>
#include <sstream>

#include <mca_defs.hpp>
#include <utils.hpp>

using json = nlohmann::json;

class IcxMcaDecoder : public McaDecoder
{
  public:
    IcxMcaDecoder(const MCAData& mca) : McaDecoder(mca) {};

    json decode()
    {
        json entry;
        entry["Core"] = mca.core;
        entry["Thread"] = mca.thread;
        entry["Bank"] = decodeBankName(mca);
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

    const std::map<uint16_t, const char*> Mcacod150 = {
        {0x400, "Time-out"},
        {0x5, "Parity error on internal Mesh2mem structures"},
        {0x21, "Read Error, Error from NM"},
        {0x22, "PWR Error, Error from NM"},
        {0xa, "PWR Error, Error from last level of memory(pmem/block/1LM DDR)"},
        {0x9, "Read Error, Error from last level of memory(pmem/block/1LM DDR)"},
    };

    // MSCOD decode for banks 12, 16, 20, 24
    const std::map<uint8_t, const char*> MscodGeneric1 = {
        {0x0, "No error"},
        {0x1, "Read ECC error"},
        {0x2, "Bucket1 error"},
        {0x3, "RdTrkr parity error"},
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

    // MSCOD decode for banks 13, 14, 17, 18, 21, 22, 25, 26
    const std::map<uint16_t, const char*> MscodGeneric2 = {
        {0x80, "HA_RD_ERR"},
        {0x1, "UC_APPP_ADDR_PAR_ERR"},
        {0x2, "HA_WR_DAT_PARERR"},
        {0x3, "HA_WR_DAT_CMI_ECC_ERR"},
        {0x4, "HA_WR_BE_PARERR"},
        {0x105, "WPQ_PARERR"},
        {0x106, "DDRT_HA_WR_DAT_PARERR"},
        {0x7, "UC_APPP_TID_PAR_ERR"},
        {0x8, "CORR_PATSCRUB_ERR"},
        {0x809, "DDRT_ERID_FIFO_OVERFLOW"},
        {0x80a, "DDRT_FNV_WR_CRDT_ERROR"},
        {0x80b, "DDRT_FNV_RD_CRDT_ERROR"},
        {0xc0, "2LM_METADATA_ERR"},
        {0x102, "VDB_RD_PARERR"},
        {0x100, "WDB_RD_PARERR"},
        {0x80f, "DDRT_UNEXP_PKT_CMI_IDLE"},
        {0x10, "UC_PATSCRUB_ERR"},
        {0x111, "PCLS_CSR_ADDR_PAR_ERR"},
        {0x112, "PCLS_ILLEGAL_CFG_ADDDC"},
        {0x113, "PCLS_ILLEGAL_CFG_SDDC"},
        {0x814, "CMI_CREDIT_TOTAL_ERR"},
        {0x815, "CMI_CREDIT_RSVD_POOL_ERR"},
        {0x200, "DDR4_CA_PARERR"},
        {0x817, "WDB_FIFO_ERR"},
        {0x807, "DDRT_ERID_UC"},
        {0x104, "RPQ1_PARERR"},
        {0x81a, "CMI_RSP_FIFO_OVERFLOW"},
        {0x81b, "CMI_RSP_FIFO_UNDERFLOW"},
        {0x81c, "CMI_MISC_MC_CRDT_ERRORS"},
        {0x81d, "CMI_MISC_MC_ARB_ERRORS"},
        {0x81e, "DDRT_WR_CMPL_FIFO_OVERFLOW"},
        {0x805, "2LM_CMP_FIFO_OVFLW_ERR"},
        {0x20, "COR_SPARE_ERR"},
        {0x221, "HBM_DATA_PARERR"},
        {0x822, "TME_KEY_PAR_ERR"},
        {0x803, "2LM_INV_DDR4_CMP_ERR"},
        {0x824, "TME_CMI_OVFL_ERR"},
        {0x806, "DDRT_ERID_PAR"},
        {0x400, "RPQ0_PARERR"},
        {0x827, "TME_UFILL_PAR_ERR"},
        {0x107, "DDRT_HA_WR_BE_PARERR"},
        {0x101, "VMSE_ERR"},
        {0x108, "LINK_FAIL"},
        {0x819, "CMI_REQ_FIFO_UNDERFLOW"},
        {0x808, "DDRT_INTR"},
        {0x81f, "DDRT_WR_CMPL_FIFO_UNDERFLOW"},
        {0x804, "2LM_INV_DDRT_CMP_ERR"},
        {0x820, "CMI_RD_CPL_FIFO_OVERFLOW"},
        {0x40, "UC_SPARE_ERR"},
        {0xa0, "HA_UNCORR_RD_ERR"},
        {0x802, "2LM_UNEXP_RDRSP_ERR"},
        {0x800, "2LM_BAD_REQ_ERR"},
        {0x821, "CMI_RD_CPL_FIFO_UNDERFLOW"},
        {0x80c, "DDRT_DDRT_SCHED_ERROR"},
        {0x80d, "DDRT_FNV_ERR_ERROR"},
        {0x823, "TME_CMI_MISC_ERR"},
        {0x80e, "DDRT_FNV_THERMAL_ERROR"},
        {0x220, "HBM_CA_PARERR"},
        {0x816, "DDRT_RD_ERROR"},
        {0x825, "TME_CMI_UFL_ERR"},
        {0x810, "DDRT_RPQ_REQ_PARITY_ERR"},
        {0x801, "2LM_INV_RDRSP_ERR"},
        {0x826, "TME_TEM_SECURE_ERR"},
        {0x811, "DDRT_WPQ_REQ_PARITY_ERR"},
        {0x812, "2LM_NMFILLWR_CAM_ERR"},
        {0x818, "CMI_REQ_FIFO_OVERFLOW"},
        {0x813, "CMI_CREDIT_OVERSUB_ERR"},
    };

    union McacodDataGeneric1
    {
        struct
        {
            uint16_t mcacod_3_0 : 4, mcacod_6_4 : 3, mcacod_15_7 : 9;
        };
        uint16_t mcacod_15_0;
    };

    union McacodDataGeneric2
    {
        struct
        {
            uint16_t error_level_encoding : 2, error_transaction_type : 2,
                error_request_type : 4, cache_hierarchy: 1, reserved0: 7;
        };
        uint16_t mcacod_15_0;
    };

    std::string decodeMcacodGeneric(uint16_t mcacod)
    {
        std::stringstream ss;
        McacodDataGeneric2 mcacodDecoded;
        mcacodDecoded.mcacod_15_0 = mcacod;

        auto errorLevelEncoding = getDecoded(ErrorLevelEncoding,
            static_cast<uint8_t>(mcacodDecoded.error_level_encoding));
        auto errorTransactionType = getDecoded(ErrorTransactionType,
            static_cast<uint8_t>(mcacodDecoded.error_transaction_type));
        auto errorRequestType = getDecoded(ErrorRequestType,
            static_cast<uint8_t>(mcacodDecoded.error_request_type));

        if (mcacodDecoded.cache_hierarchy == 1)
        {
            ss << "Cache_Hierarchy_Error|";
            ss << *errorRequestType << "|" << *errorTransactionType << "|"
               << *errorLevelEncoding;
        }
        else
        {
            ss << "---";
        }
        return ss.str();
    }

    std::string decodeBankName(MCAData mca)
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

class IcxMcaBankIfu final : public IcxMcaDecoder
{
  private:
    union MC_STATUS
    {
        struct
        {
            uint64_t mcacod : 16, mscod: 16, enh_mca_avail : 6,
                corrected_err_cnt : 14, sticky : 1, enh_mca_avail2 : 2, ar : 1,
                s : 1, pcc : 1, addrv : 1, miscv : 1, en : 1, uc : 1,
                overflow : 1, valid : 1;
        };
        uint64_t mc_status;
    };

    const std::map<uint16_t, const char*> decodeMscod = {
        {0x0, "PRF Parity Error - correctable or uncorrectable"},
        {0x1, "DSB Data Parity Error- correctable (on BOM) or uncorrectable "
              "(non-BOM)"},
        {0x2, "DSB Hit with Parity Error + IFU Tag miss OR ms patch ram parity "
              "error"},
        {0x3, "IFU iTLB Parity Error with DSB hit OR IQ LIP, iqarrest Parity "
              "Error"},
        {0x4, "DSB offset / NATA (BPU-TA) Parity Error OR IFU IC Data Parity "
              "Error, correctable or uncorrectable (on FERestart)"},
        {0x5, "DSB Tag Parity Error OR IFU IC Tag Parity Error, correctable or "
              "uncorrectable (on FERestart)"},
        {0x6, "IFU iTLB Parity Error, correctable or uncorrectable "
              "(on FERestart)"},
        {0x7, "IDQ UOP Parity"},
        {0x8, "Bit Parity (bit + baqbrd)"},
        {0xc, "IFU Poison (poisoned data received on icache miss)"},
        {0xd, "SDB Parity Error"},
        {0xe, "RS / IDQ imm Parity Error - correctable or uncorrectable"},
        {0xf, "Execution Residue Checking - correctable or uncorrectable OR "
              "DSB hit with IC miss - flush (so it doesn’t become fatal "
              "inclusion error on FFEIP) OR Miss on FERestart"},
    };

    const std::map<uint16_t, const char*> decodeMcacod = {
        {0x5, "Internal Parity Error"},
        {0x10, "TLB Errors: I.L0"},
        {0x150, "Cache Errors: IRD.I.L0"},
        {0x406, "Trusted Paths"},
        {0x40a, "Internal Unclassified Errors"},
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
                decodeMcacodGeneric(status_decoded.mcacod);
        }
    }

  public:
    IcxMcaBankIfu(const MCAData& mca) : IcxMcaDecoder(mca)
    {
        status_decoded.mc_status = mca.mc_status;
    }

    json decode_status() override
    {
        entry["MCACOD"] =
            int_to_hex(static_cast<uint16_t>(status_decoded.mcacod));
        entry["MSCOD"] =
            int_to_hex(static_cast<uint16_t>(status_decoded.mscod));
        entry["ENH_MCA_AVAIL"] =
            int_to_hex(static_cast<uint8_t>(status_decoded.enh_mca_avail));
        entry["CORRECTED_ERR_CNT"] =
            int_to_hex(static_cast<uint16_t>(status_decoded.corrected_err_cnt));
        entry["STICKY"] =
            int_to_hex(static_cast<bool>(status_decoded.sticky));
        entry["ENH_MCA_AVAIL2"] =
            int_to_hex(static_cast<uint8_t>(status_decoded.enh_mca_avail2));
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

class IcxMcaBankDcu final : public IcxMcaDecoder
{
  private:
    union MC_STATUS
    {
        struct
        {
            uint64_t mcacod : 16, mscod : 22, cecnt : 15, enh_mca_avail : 2,
                ar : 1, s : 1, pcc : 1, addrv : 1, miscv : 1, en : 1, uc : 1,
                overflow : 1, valid : 1;
        };
        uint64_t mc_status;
    };

    const std::map<uint32_t, const char*> decodeMscod = {
        {0x0,  "Non-APIC Error"},
        {0x10, "WBINVD Hitting Tag/Data Parity Error, ORxlat/ms Load Hitting "
               "Poisoned Data (coming from MLC)"},
        {0x11, "Stuffed Load Hitting Poisoned Data (coming from MLC)"},
        {0x20, "APIC Error"},
    };

    const std::map<uint16_t, const char*> decodeMcacod = {
        {0x114, "Cache Errors: RD.D.L0"},
        {0x124, "Cache Errors: WR.D.L0"},
        {0x134, "Cache Errors: DRD.D.L0"},
        {0x164, "Cache Errors: PREF.D.L0"},
        {0x174, "Cache Errors: EVICT.D.L0"},
        {0x184, "Cache Errors: SNOOP.D.L0"},
        {0x401, "WB Access to APIC Memory"},
        {0x404, "Tag/Data Parity Error on APIC load /store"},
    };

    MC_STATUS status_decoded;
    json entry;

    void decodeKnownFields()
    {
        auto mscodDecoded = getDecoded(decodeMscod,
            static_cast<uint32_t>(status_decoded.mscod));
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
                decodeMcacodGeneric(status_decoded.mcacod);
        }
    }

  public:
    IcxMcaBankDcu(const MCAData& mca) : IcxMcaDecoder(mca)
    {
        status_decoded.mc_status = mca.mc_status;
    }

    json decode_status() override
    {
        entry["MCACOD"] =
            int_to_hex(static_cast<uint16_t>(status_decoded.mcacod));
        entry["MSCOD"] =
            int_to_hex(static_cast<uint32_t>(status_decoded.mscod));
        entry["CORRECTED_ERROR_COUNT"] =
            int_to_hex(static_cast<uint16_t>(status_decoded.cecnt));
        entry["ENH_MCA_AVAIL"] =
            int_to_hex(static_cast<uint8_t>(status_decoded.enh_mca_avail));
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

class IcxMcaBankDtlb final : public IcxMcaDecoder
{
  private:
    union MC_STATUS
    {
        struct
        {
            uint64_t mcacod : 16, mscod : 16, enh_mca_avail0 : 6,
                corrected_error_count : 15, enh_mca_avail1 : 4, pcc : 1,
                addrv : 1, miscv : 1, en : 1, uc : 1, overflow : 1, valid : 1;
        };
        uint64_t mc_status;
    };

    union MCACOD
    {
        struct
        {
            uint16_t tlb_err_ll : 2, tlb_err_tt : 2, tlb_err : 1,
                reserved0 : 11;
        };
        struct
        {
            uint16_t simple_error: 3, reserved1: 13;
        };
        uint16_t mcacod_15_0;
    };

    const std::map<uint16_t, const char*> decodeMscod = {
        {0x0, "Error Type - Tag-parity"},
        {0x1, "Error Type - Data-parity"},
        {0x3, "IEU"},
        {0x4, "AGU"},
    };

    const std::map<uint16_t, const char*> decodeMcacod = {
        {0x0005, "Internal Parity Error"},
        {0x0014, "TLB Errors:D.L0"},
        {0x0019, "TLB Errors:G.L1"},
        {0x0406, "Trusted Paths"},
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

    const std::map<uint8_t, const char*> decodeSimpleError = {
        {0x0, "No error"},
        {0x1, "Unclassified error"},
        {0x2, "Microcode ROM Parity Error"},
        {0x3, "External Error"},
        {0x4, "FRC Error (Unexpected)"},
        {0x5, "Internal Parity Error"},
        {0x6, "Simple Error"},
        {0x7, "Simple Error"},
    };

    std::string decodedMcacod(uint16_t mcacod)
    {
        std::stringstream ss;
        MCACOD mcacodDecoded;
        mcacodDecoded.mcacod_15_0 = mcacod;

        if (mcacodDecoded.tlb_err == 1)
        {
            auto tlbErrLl = getDecoded(decodeTlbErrLl,
                static_cast<uint8_t>(mcacodDecoded.tlb_err_ll));
            auto tlbErrTt = getDecoded(decodeTlbErrTt,
                static_cast<uint8_t>(mcacodDecoded.tlb_err_tt));
            ss << "TLB_ERR|" << *tlbErrTt << "|" << *tlbErrLl;
        }
        else
        {
            auto simpleError = getDecoded(decodeSimpleError,
                static_cast<uint8_t>(mcacodDecoded.simple_error));
            if (simpleError)
            {
                ss << *simpleError;
            }
        }
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
    IcxMcaBankDtlb(const MCAData& mca) : IcxMcaDecoder(mca)
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
            static_cast<uint16_t>(status_decoded.corrected_error_count));
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

class IcxMcaBankMlc final : public IcxMcaDecoder
{
  private:
    union MC_STATUS
    {
        struct
        {
            uint64_t mcacod : 16, mscod : 16, enh_mca_avail0 : 6,
                corrected_error_count : 15, green_tracking : 1,
                yellow_tracking : 1, enh_mca_avail1 : 2, pcc : 1, addrv : 1,
                miscv : 1, en : 1, uc : 1, overflow : 1, valid : 1;
        };
        struct
        {
            uint64_t error_type : 32, reserved : 32;
        };
        uint64_t mc_status;
    };

    const std::map<uint16_t, const char*> decodeMscod = {
        {0x3, "Tag / Other"},
        {0x6, "MISC"},
        {0xC, "MESI"},
        {0x18, "Data"},
        {0x60, "MISC"},
        {0x80, "Watchdog timer (3-strike) Error"},
        {0xC0, "SUNPASS"},
        {0xFF, "SUNPASS"},
    };

    const std::map<uint16_t, const char*> decodeMcacod = {
        {0x115, "Cache Errors: RD.D.L1"},
        {0x129, "Cache Errors: WR.G.L1"},
        {0x135, "Cache Errors: DRD.D.L1"},
        {0x145, "Cache Errors: DWR.D.L1"},
        {0x151, "Cache Errors: IRD.I.L1"},
        {0x165, "Cache Errors: PREF.D.L1"},
        {0x179, "Cache Errors: EVICT.G.L1"},
        {0x185, "Cache Errors: SNOOP.D.L1"},
        {0x400, "Internal Timer Error"},
        {0x405, "Internal / E2E Parity / ECC"},
        {0x406, "Trusted Paths"},
        {0x409, "Internal Unclassified Errors"},
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
                decodeMcacodGeneric(status_decoded.mcacod);
        }
    }

  public:
    IcxMcaBankMlc(const MCAData& mca) : IcxMcaDecoder(mca)
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
            static_cast<uint16_t>(status_decoded.corrected_error_count));
        entry["GREEN_TRACKING"] =
            int_to_hex(static_cast<bool>(status_decoded.green_tracking));
        entry["YELLOW_TRACKING"] =
            int_to_hex(static_cast<bool>(status_decoded.yellow_tracking));
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

class IcxMcaBankPcu final : public IcxMcaDecoder
{
  private:
    union MC_STATUS
    {
        struct
        {
            uint64_t mccod : 16, mscod_19_16 : 4, mscod_23_20 : 4,
                mscod_31_24 : 8, enh_mca_avail0 : 6, corrected_error_count : 15,
                corr_err_sts_ind : 2, enh_mca_avail1 : 2, pcc : 1, addrv : 1,
                miscv : 1, en : 1, uc : 1, overflow : 1, valid : 1;
        };
        struct
        {
            uint64_t error_type : 32, reserved : 32;
        };
        uint64_t mc_status;
    };

    const std::map<uint32_t, const char*> statusDecode = {
        {0xb0000402L, "mca_adr_signal_timeout"}
    };

    const std::map<uint8_t, const char*> decodeMscod3124 = {
        {0xd, "LLC_BIST_ACTIVE_TIMEOUT ; Got timeout on LLC-BIST active."},
        {0xe, "DMI_TRAINING_TIMEOUT ; During reset, DMI training failed to "
        "complete within time-out. Possible DMI related failure."},
        {0xf, "DMI_STRAP_SET_ARRIVAL_TIMEOUT ; STRAP SET Message form PCH was "
        "not received in time; Possible DMI Issue."},
        {0x10, "DMI_CPU_RESET_ACK_TIMEOUT ; During reset, DMI message for CPU "
        "Reset Ack did not arrive within timeout. Possible DMI related "
        "failure."},
        {0x11, "MORE_THAN_ONE_LT_AGENT ; During Boot Mode Processing, >1 Intel "
        "TXT Agent detected. Possible Intel UPI BIOS/BMC setting issue."},
        {0x14, "INCOMPATIBLE_PCH_TYPE ; PCH being used is not compatible with "
        "the CPU it is paired with; Possible Platform configuration issue."},
        {0x1e, "BIOS_RST_CPL_INVALID_SEQ ; BIOS violated BIOS Reset CPL "
        "sequencing requirements. Possible BIOS related issue."},
        {0x1f, "BIOS_INVALID_PKG_STATE_CONFIG ; BIOS Invalid PKG State "
        "Configuration. Possible BIOS issue."},
        {0x2d, "PCU_PMAX_CALIB_ERROR ; pmax calibration error"},
        {0x2e, "TSC100_SYNC_TIMEOUT ; Hang on TSC100 sync with xtal clock TSC"},
        {0x3a, "GPSB_TIMEOUT ; Sideband does not respond within timeout value"},
        {0x3b, "PMSB_TIMEOUT ; Sideband does not respond within timeout value"},
        {0x3e, "IOSFSB_PMREQ_CMP_TIMEOUT; Timeout waiting for PMReq.CMP to be "
        "set - IOSF-SB driver"},
        {0x40, "SVID_VCCIN_VR_ICC_MAX_FAILURE ; Fused CPU Icc-max exceeds "
        "Vccin VR's limit"},
        {0x42, "SVID_VCCIN_VR_VOUT_FAILURE ; Fused CPU boot voltage exceeds "
        "Vccin VR's limit"},
        {0x43, "SVID_CPU_VR_CAPABILITY_ERROR ; A CPU VR found that does not "
        "support IOUT (IMON polling)"},
        {0x44, "SVID_CRITICAL_VR_FAILED ; Failure of critical VR detected "
        "during reset"},
        {0x45, "SVID_SA_ITD_ERROR ; Failure updating SA VR VID for ITD"},
        {0x46, "SVID_READ_REG_FAILED ; SVID command to read a register "
        "failed."},
        {0x47, "SVID_WRITE_REG_FAILED ; SVID command to write a register "
        "failed"},
        {0x4a, "SVID_PKGC_REQUEST_FAILED ; SVID Pkgc request failed"},
        {0x4b, "SVID_IMON_REQUEST_FAILED ; SVID IMON request failed"},
        {0x4c, "SVID_ALERT_REQUEST_FAILED ; SVID ALERT request failed"},
        {0x4d, "SVID_MCP_VR_RAMP_ERROR ; MCP VR failed to ramp"},
        {0x56, "FIVR_PD_HARDERR ; PD_HARDERR in IO_PM_EVENT_LOG[17]"},
        {0x58, "WATCHDOG_TIMEOUT_PKGC_SLAVE ; PkgC slave timed-out waiting for "
        "PmRsp from Master"},
        {0x59, "WATCHDOG_TIMEOUT_PKGC_MASTER ; PkgC master timed-out waiting "
        "for PmRsp from PCH"},
        {0x5a, "WATCHDOG_TIMEOUT_PKGS_MASTER ; PkgS master timed-out waiting "
        "for PmRsp from slave"},
        {0x5b, "WATCHDOG_TIMEOUT_MSG_CH_FSM ; Timeout waiting for "
        "IO_MESSAGE_CHANNEL FSM to go idle"},
        {0x5c, "WATCHDOG_TIMEOUT_BULK_CR_FSM ; Timeout waiting for "
        "SA_BULK_CR_READ FSM to go idle"},
        {0x5d, "WATCHDOG_TIMEOUT_IOSFSB_FSM ; Timeout waiting for IOSF-SB FSM "
        "to go idle"},
        {0x60, "PKGS_SAFE_WP_TIMEOUT ; Got timeout to change VID / Freq for "
        "safe GV WP at PKGS."},
        {0x61, "PKGS_CPD_UNCPD_TIMEOUT ; Got timeout to CPD-S1 or to UNCPD-S1 "
        "direct GV."},
        {0x62, "PKGS_INVALID_REQ_PCH ; PkgS Master detected improper request "
        "from PCH"},
        {0x63, "PKGS_INVALID_REQ_INTERNAL ; PkgS Slave detected improper "
        "request from Master"},
        {0x64, "PKGS_INVALID_RSP_INTERNAL ; PkgS Master detected improper "
        "response from Slave"},
        {0x65, "PKGS_RESET_PREP_TIMEOUT ; IP failed to acknowledge a reset "
        "prep message"},
        {0x66, "PKGS_RESET_PREP_TIMEOUT ; IP failed to acknowledge a reset "
        "prep message"},
        {0x67, "PKGS_RESET_PREP_TIMEOUT ; IP failed to acknowledge a reset "
        "prep message"},
        {0x68, "PKGS_RESET_PREP_TIMEOUT ; IP failed to acknowledge a reset "
        "prep message"},
        {0x69, "PKGS_RESET_PREP_TIMEOUT ; IP failed to acknowledge a reset "
        "prep message"},
        {0x6a, "PKGS_RESET_PREP_TIMEOUT ; IP failed to acknowledge a reset "
        "prep message"},
        {0x6b, "PKGS_RESET_PREP_TIMEOUT ; IP failed to acknowledge a reset "
        "prep message"},
        {0x6c, "PKGS_RESET_PREP_TIMEOUT ; IP failed to acknowledge a reset "
        "prep message"},
        {0x6d, "PKGS_RESET_PREP_TIMEOUT ; IP failed to acknowledge a reset "
        "prep message"},
        {0x6e, "PKGS_RESET_PREP_TIMEOUT ; IP failed to acknowledge a reset "
        "prep message"},
        {0x6f, "PKGS_RESET_PREP_TIMEOUT ; IP failed to acknowledge a reset "
        "prep message"},
        {0x70, "PKGS_RESET_PREP_TIMEOUT ; IP failed to acknowledge a reset "
        "prep message"},
        {0x71, "PKGS_RESET_PREP_TIMEOUT ; IP failed to acknowledge a reset "
        "prep message"},
        {0x72, "PKGS_RESET_PREP_TIMEOUT ; IP failed to acknowledge a reset "
        "prep message"},
        {0x73, "PKGS_RESET_PREP_TIMEOUT ; IP failed to acknowledge a reset "
        "prep message"},
        {0x74, "PKGS_RESET_PREP_TIMEOUT ; IP failed to acknowledge a reset "
        "prep message"},
        {0x75, "PKGS_RESET_PREP_TIMEOUT ; IP failed to acknowledge a reset "
        "prep message"},
        {0x76, "PKGS_RESET_PREP_TIMEOUT ; IP failed to acknowledge a reset "
        "prep message"},
        {0x77, "PKGS_RESET_PREP_TIMEOUT ; IP failed to acknowledge a reset "
        "prep message"},
        {0x78, "PKGS_RESET_PREP_TIMEOUT ; IP failed to acknowledge a reset "
        "prep message"},
        {0x79, "PKGS_RESET_PREP_TIMEOUT ; IP failed to acknowledge a reset "
        "prep message"},
        {0x7a, "PKGS_RESET_PREP_TIMEOUT ; IP failed to acknowledge a reset "
        "prep message"},
        {0x7b, "PKGS_SMBUS_VPP_PAUSE_TIMEOUT ; SMBUS VPP Pause Timeout. "
        "Possible SMBUS issue."},
        {0x7c, "PKGS_SMBUS_MCP_PAUSE_TIMEOUT ; SMBUS MCP Pause Timeout. "
        "Possible SMBUS issue."},
        {0x7d, "PKGS_SMBUS_SPD_PAUSE_TIMEOUT ; SMBUS SPD Pause Timeout. "
        "Possible SMBUS issue."},
        {0x80, "PKGC_DISP_BUSY_TIMEOUT ; PKGC exit timed out waiting for "
        "dispatcher to be not busy"},
        {0x81, "PKGC_INVALID_RSP_PCH ; PkgC Master detected improper response "
        "from PCH for EA request"},
        {0x83, "PKGC_WATCHDOG_HANG_CBZ_DOWN ; Pkg C-state hung at transient "
        "state: In Buffer Zone, going Down (towards C2/C3), waiting for "
        "PM_RSP(EA=0)."},
        {0x84, "PKGC_WATCHDOG_HANG_CBZ_UP ; Pkg C-state hung at transient "
        "state: In Buffer Zone, going Up (towards C0), waiting for "
        "PM_RSP(EA=1)."},
        {0x87, "PKGC_WATCHDOG_HANG_C2_BLKMASTER ; Hung in transient state"},
        {0x88, "PKGC_WATCHDOG_HANG_C2_PSLIMIT ; Hung in transient state"},
        {0x89, "PKGC_WATCHDOG_HANG_SETDISP ; Pkg C-state hung at transient "
        "state: In C2, going down towards C3. Ring Off was sent, but not ack."},
        {0x8b, "PKGC_ALLOW_L1_ERROR ; Allow L1 did not respond back"},
        {0x90, "RECOVERABLE_DIE_THERMAL_TOO_HOT ; During slow loop, it was "
        "detected that internal die thermals is too hot."},
        {0xa0, "ADR_SIGNAL_TIMEOUT ; eADR: timeout waiting for ADR to be "
        "signaled"},
        {0xa1, "BCLK_FREQ_OC_ABOVE_THRESHOLD ; Bclk govenor detected Bclk "
        "frequency above fused threshold"},
        {0xb0, "DISPATCHER_RUN_BUSY_TIMEOUT ; Dispatcher timed out"}
    };

    const std::map<uint8_t, const char*> decodeMscod2320 = {
        {0x4, "Clock/power IP response timeout"},
        {0x5, "SMBus controller raised SMI"},
        {0x9, "PM controller received invalid transaction"},
    };

    const std::map<uint8_t, const char*> decodeMscod1916 = {
        {0x1, "Instruction address out of valid space"},
        {0x2, "Double bit RAM error on Instruction Fetch"},
        {0x3, "Invalid OpCode seen"},
        {0x4, "Stack Underflow"},
        {0x5, "Stack Overflow"},
        {0x6, "Data address out of valid space"},
        {0x7, "Double bit RAM error on Data Fetch"},
    };

    const std::map<uint16_t, const char*> decodeMcacod = {
        {0x402, "PCU"},
        {0x406, "Internal firmware errors"},
    };

    MC_STATUS status_decoded;
    json entry;

    void decodeKnownFields()
    {
        auto mscod3124 = getDecoded(decodeMscod3124,
            static_cast<uint8_t>(status_decoded.mscod_31_24));
        if (mscod3124)
        {
            entry["MSEC_FW_decoded"] = *mscod3124;
        }
        else
        {
            entry["MSEC_FW_decoded"] = "Internal Error";
        }

        auto mscod2320 = getDecoded(decodeMscod2320,
            static_cast<uint8_t>(status_decoded.mscod_23_20));
        if (mscod2320)
        {
            entry["MSEC_HW_decoded"] = *mscod2320;
        }

        auto mscod1916 = getDecoded(decodeMscod1916,
            static_cast<uint8_t>(status_decoded.mscod_19_16));
        if (mscod1916)
        {
            entry["MSEC_UC_decoded"] = *mscod1916;
        }

        auto mcacod = getDecoded(decodeMcacod,
            static_cast<uint16_t>(status_decoded.mccod));
        if (mcacod)
        {
            entry["MCACOD_decoded"] = *mcacod;
        }

        auto corrErrStsIndDecoded =getDecoded(CorrErrStsInd,
            static_cast<uint8_t>(status_decoded.corr_err_sts_ind));
        if (corrErrStsIndDecoded)
        {
            entry["CORR_ERR_STS_IND_decoded"] = *corrErrStsIndDecoded;
        }
    }

  public:
    IcxMcaBankPcu(const MCAData& mca) : IcxMcaDecoder(mca)
    {
        status_decoded.mc_status = mca.mc_status;
    }

    json decode_status() override
    {
        entry["MCACOD"] =
            int_to_hex(static_cast<uint16_t>(status_decoded.mccod));
        entry["MSEC_UC"] =
            int_to_hex(static_cast<uint8_t>(status_decoded.mscod_19_16));
        entry["MSEC_HW"] =
            int_to_hex(static_cast<uint8_t>(status_decoded.mscod_23_20));
        entry["MSEC_FW"] =
            int_to_hex(static_cast<uint8_t>(status_decoded.mscod_31_24));
        entry["ENH_MCA_AVAIL0"] =
            int_to_hex(static_cast<uint8_t>(status_decoded.enh_mca_avail0));
        entry["CORRECTED_ERROR_COUNT"] = int_to_hex(
            static_cast<uint16_t>(status_decoded.corrected_error_count));
        entry["CORR_ERR_STS_IND"] =
            int_to_hex(static_cast<uint8_t>(status_decoded.corr_err_sts_ind));
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

class IcxMcaBankUpi final : public IcxMcaDecoder
{
  private:
    union MC_STATUS
    {
        struct
        {
            uint64_t mcacod_ll : 2, mcacod_ii : 2, mcacod_rrrr : 4,
                mcacod_t : 1, mcacod_pp : 2, mcacod_int : 1, mcacod_rsvd : 4,
                mscod_code : 6, mscod_spare : 10, other_info : 6,
                corrected_error_count : 15, corr_err_sts_ind : 2, ar : 1, s : 1,
                pcc : 1, addrv : 1, miscv : 1, en : 1, uc : 1, overflow : 1,
                valid : 1;
        };
        uint64_t mc_status;
    };

    union MscodData
    {
        struct
        {
            uint16_t tx_parity_err : 1, ll_reset : 1, ll_reset_in_progress : 1,
                rx_header : 1, tx_flit : 1, table_err : 1, rf_parity_err : 1,
                retry_req_flit : 1, retry_ack_flit : 1, phy_ctl_err : 1,
                reserved0 : 6;
        };
        uint16_t mscod;
    };

    MC_STATUS status_decoded;
    json entry;

    const std::map<uint8_t, const char*> decodeMscod = {
        {0x0, "UC Phy Initialization Failure"},
        {0x1, "UC Phy Detected Drift Buffer Alarm"},
        {0x2, "UC Phy Detected Latency Buffer Rollover"},
        {0x10, "UC LL Rx detected CRC error"},
        {0x11, "UC LL Rx Unsupported/Undefined packet"},
        {0x12, "UC LL or Phy Control Error: unexpected Tx Protocol flit"},
        {0x13, "UC LL Rx Parameter Exception"},
        {0x1f, "UC LL Detected Control Error from M3UPI Correctable (COR)"},
        {0x20, "COR Phy Initialization Abort"},
        {0x21, "COR Phy Reset"},
        {0x22, "COR Phy Lane failure, recovery in x8 width "},
        {0x23, "COR Phy L0c error"},
        {0x24, "COR Phy L0c error triggering Phy Reset"},
        {0x25, "COR Phy L0p exit error triggering Phy Reset"},
        {0x30, "COR LL Rx detected CRC error: successful LLR without Phy "
               "Reinit"},
        {0x31, "COR LL Rx detected CRC error: successful LLR with Phy Reinit"},
    };

    const std::map<uint8_t, const char*> decodeMcacodPp = {
        {0x2, "Unsupported/Undefined Packet"},
        {0x3, "Other Errors"},
    };

    const std::map<uint8_t, const char*> decodeMcacodIi = {
        {0x0, "Memory"},
        {0x1, "Reserved"},
        {0x2, "I/O"},
        {0x3, "Other"},
    };

    const std::map<uint8_t, const char*> decodeMcacodLl = {
        {0x0, "L0"},
        {0x1, "L1"},
        {0x2, "L2"},
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
            static_cast<uint8_t>(status_decoded.corr_err_sts_ind));
        if (corrErrStsIndDecoded)
        {
            entry["CORR_ERR_STS_IND_decoded"] = *corrErrStsIndDecoded;
        }

        auto mcacodLl = getDecoded(decodeMcacodLl,
            static_cast<uint8_t>(status_decoded.mcacod_ll));
        if (mcacodLl)
        {
            entry["MCACOD_LL_decoded"] = *mcacodLl;
        }

        auto mcacodIi = getDecoded(decodeMcacodIi,
            static_cast<uint8_t>(status_decoded.mcacod_ii));
        if (mcacodIi)
        {
            entry["MCACOD_II_decoded"] = *mcacodIi;
        }

        auto mcacodPp = getDecoded(decodeMcacodPp,
            static_cast<uint8_t>(status_decoded.mcacod_pp));
        if (mcacodPp)
        {
            entry["MCACOD_PP_decoded"] = *mcacodPp;
        }

        auto mscod = getDecoded(decodeMscod,
            static_cast<uint8_t>(status_decoded.mscod_code));
        if (mscod)
        {
            entry["MSCOD_decoded"] = *mscod;
        }

        entry["MSCOD_SPARE_decoded"] =
            decodeMscodSpare(status_decoded.mscod_spare);
    }

  public:
    IcxMcaBankUpi(const MCAData& mca) : IcxMcaDecoder(mca)
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
            int_to_hex(static_cast<bool>(status_decoded.mcacod_t));
        entry["MCACOD_PP"] =
            int_to_hex(static_cast<uint8_t>(status_decoded.mcacod_pp));
        entry["MCACOD_INT"] =
            int_to_hex(static_cast<bool>(status_decoded.mcacod_int));
        entry["MCACOD_RSVD"] =
            int_to_hex(static_cast<uint8_t>(status_decoded.mcacod_rsvd));
        entry["MSCOD"] =
            int_to_hex(static_cast<uint8_t>(status_decoded.mscod_code));
        entry["MSCOD_SPARE"] =
            int_to_hex(static_cast<uint16_t>(status_decoded.mscod_spare));
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

class IcxMcaBankIio final : public IcxMcaDecoder
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
            uint64_t reserved0 : 13, segment_log : 3, function_log : 3,
                device_log : 5, bus_log : 8, reserved1 : 32;
        };
        uint64_t mc_misc;
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
        {0xE0B, "Generic I/O Error"},
        {0x406, "Internal Firmware Error"},
        {0x407, "General UBox Error"},
    };

    MC_STATUS status_decoded;
    MC_MISC misc_decoded;
    json entry;

    void decodeKnownFields()
    {
        auto mscod = getDecoded(decodeMscod,
                                  static_cast<uint16_t>(status_decoded.mscod));
        if (mscod)
        {
            entry["MSCOD_decoded"] = *mscod;
        }

        auto mcacod = getDecoded(
            decodeMcacod, static_cast<uint16_t>(status_decoded.mcacod));
        if (mcacod)
        {
            std::stringstream ss;
            ss << *mcacod;
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
    IcxMcaBankIio(const MCAData& mca) : IcxMcaDecoder(mca)
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

class IcxMcaBankCha final : public IcxMcaDecoder
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
    };

  public:
    IcxMcaBankCha(const MCAData& mca) : IcxMcaDecoder(mca)
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

class IcxMcaBankM2m final : public IcxMcaDecoder
{
  private:
    union MC_STATUS
    {
        struct
        {
            uint64_t mcacod : 16, mscoddatarderr : 1, reserved0 : 1,
                mscodptlwrerr: 1, mscodfullwrerr: 1, mscodbgferr: 1,
                mscodtimeout: 1, mscodparerr: 1, mscodbucket1err: 1,
                mscodddrtype: 2, mscodmiscerrs: 6, other_info : 6,
                corrected_error_count : 15, corr_err_sts_ind : 2, ar : 1, s : 1,
                pcc : 1, addrv : 1, miscv : 1, en : 1, uc : 1, overflow : 1,
                valid : 1;
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
        {0x0, "No error logged (DDR4 or NVMDIMM)"},
        {0x1, "Error specifically on DDR4"},
        {0x2, "Error specifically on DDRT"},
        {0x3, "Error for this transaction was detected on both DDR4 and "
        "DDRT"},
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
        {0x4, "Memory Scrubbing Error"}
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
            static_cast<uint8_t>(status_decoded.corr_err_sts_ind));
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
            entry["MSCODTIMEOUT_decoded"] = "M2M time out";
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
    IcxMcaBankM2m(const MCAData& mca) : IcxMcaDecoder(mca)
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
        entry["MSCODMISCERRS"] =
            int_to_hex(static_cast<uint8_t>(status_decoded.mscodmiscerrs));
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

class IcxMcaBankImc final : public IcxMcaDecoder
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

    MC_STATUS status_decoded;
    json entry;

    union MCACOD
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

    const std::map<uint8_t, const char*> decodeErrSpecific = {
        {0x5, "Parity error on structures"},
    };

    const std::map<uint8_t, const char*> decodeErr = {
        {0x0, "Generic undefined request"}, {0x1, "Memory Read Error"},
        {0x2, "Memory Write Error"},        {0x3, "Address/Command Error"},
        {0x4, "Memory Scrubbing Error"},
    };

    const std::map<uint8_t, const char*> decodeChannel = {
        {0x0, "Channel_0"},  {0x1, "Channel_1"},
        {0x2, "Channel_2"},  {0x3, "Channel_3"},
        {0x4, "Channel_4"},  {0x5, "Channel_5"},
        {0x6, "Channel_6"},  {0x7, "Channel_7"},
        {0x8, "Channel_8"},  {0x9, "Channel_9"},
        {0xa, "Channel_10"}, {0xb, "Channel_11"},
        {0xc, "Channel_12"}, {0xd, "Channel_13"},
        {0xe, "Channel_14"}, {0xf, "Channel_not_specified"},
    };

    const std::map<uint16_t, const char*> decodeMscod = {
        {0x0001, "Address parity error"},
        {0x0002, "Data parity error"},
        {0x0003, "Data ECC error"},
        {0x0004, "Data byte enable parity error"},
        {0x0005, "Received uncorrectable data"},
        {0x0006, "Received Uncorrectable MetaData"},
        {0x0007, "Transaction ID parity error"},
        {0x0008, "Correctable patrol scrub error"},
        {0x0010, "Uncorrectable patrol scrub error"},
        {0x0020, "Correctable spare error"},
        {0x0040, "Uncorrectable spare error"},
        {0x0080, "Correctable read error"},
        {0x00A0, "Uncorrectable read error"},
        {0x00C0, "Uncorrectable MetaData"},
        {0x0100, "WDB read parity error"},
        {0x0103, "RPA parity error"},
        {0x0104, "RPA parity error"},
        {0x0105, "WPA parity error"},
        {0x0106, "DDRT_DPPP data BE error"},
        {0x0107, "DDRT_DPPP data error"},
        {0x0108, "DDR link failure"},
        {0x0111, "PCLS CAM error"},
        {0x0112, "PCLS data error"},
        {0x0200, "DDR4 Command / Address parity error"},
        {0x0400, "RPQ parity (primary) error"},
        {0x0401, "RPQ parity (buddy) error"},
        {0x0404, "WPQ parity (primary) error"},
        {0x0405, "WPQ parity (buddy) error"},
        {0x0408, "RPB parity (primary) error"},
        {0x0409, "RPB parity (buddy) error"},
        {0x0800, "DDRT bad request"},
        {0x0801, "DDR Data response to an invalid entry"},
        {0x0802, "DDR data response to an entry not expecting data"},
        {0x0803, "DDR4 completion to an invalid entry"},
        {0x0804, "DDRT completion to an invalid entry"},
        {0x0805, "DDR data/completion FIFO overflow"},
        {0x0806, "DDRT ERID correctable parity error"},
        {0x0807, "DDRT ERID uncorrectable error"},
        {0x0808, "DDRT interrupt received while outstanding interrupt was not "
                 "ACKed"},
        {0x0809, "ERID FIFO overflow"},
        {0x080A, "DDRT error on FNV write credits"},
        {0x080B, "DDRT error on FNV read credits"},
        {0x080C, "DDRT scheduler error"},
        {0x080D, "DDRT FNV error event"},
        {0x080E, "DDRT FNV thermal error"},
        {0x080F, "CMI packet while idle"},
    };

    std::string decodedMcacod(uint16_t mcacod)
    {
        std::stringstream ss;
        MCACOD mcacodDecoded;
        mcacodDecoded.mcacod_15_0 = mcacod;

        auto errorSpecific = getDecoded(decodeErrSpecific,
            static_cast<uint8_t>(mcacodDecoded.err_specific));

        if (errorSpecific)
        {
            ss << *errorSpecific;
        }
        else
        {
            auto channel = getDecoded(decodeChannel,
                static_cast<uint8_t>(mcacodDecoded.channel));
            auto error = getDecoded(decodeErr,
                static_cast<uint8_t>(mcacodDecoded.err));
            if (mcacodDecoded.imc_err == 1)
            {
                ss << "IMC_Error|";
                if (error)
                {
                    ss << *error << "|";
                }
                else
                {
                    ss << "Reserved|";
                }
                if (channel)
                {
                    ss << *channel;
                }
            }
            else
            {
                ss << "Reserved";
            }
        }
        return ss.str();
    }

    void decodeKnownFields()
    {
        auto corrErrStsIndDecoded = getDecoded(CorrErrStsInd,
            static_cast<uint8_t>(status_decoded.corr_err_sts_ind));
        if (corrErrStsIndDecoded)
        {
            entry["CORR_ERR_STS_IND_decoded"] = *corrErrStsIndDecoded;
        }

        auto mscod = getDecoded(decodeMscod,
            static_cast<uint16_t>(status_decoded.mscod));
        if (mscod)
        {
            entry["MSCOD_decoded"] = *mscod;
        }

        entry["MCACOD_decoded"] =
            decodedMcacod(static_cast<uint16_t>(status_decoded.mcacod));
    }

  public:
    IcxMcaBankImc(const MCAData& mca) : IcxMcaDecoder(mca)
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

class IcxMcaBankCbo final : public IcxMcaDecoder
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

    const std::map<uint16_t, const char*> decodeMcacod = {
        {0x0000, "No Error"},
        {0x010A, "Cache Errors: ERR.G.L2"},
        {0x0136, "Cache Errors: DRD.D.L2"},
        {0x0146, "Cache Errors: DWR.D.L2"},
        {0x0152, "Cache Errors: IRD.I.L2"},
        {0x0166, "Cache Errors: PREF.D.L2"},
        {0x017A, "Cache Errors: EVICT.G.L2"},
        {0x0182, "Cache Errors: SNOOP.I.L2"},
        {0x0186, "Cache Errors: SNOOP.D.L2"},
        {0x0408, "SAD Errors"},
        {0x110A, "Cache Errors (Filtered): ERR.G.L2"},
        {0x1136, "Cache Errors (Filtered): DRD.D.L2"},
        {0x1146, "Cache Errors (Filtered): DWR.D.L2"},
        {0x1152, "Cache Errors (Filtered): IRD.I.L2"},
        {0x1166, "Cache Errors (Filtered): PREF.D.L2"},
        {0x117A, "Cache Errors (Filtered): EVICT.G.L2"},
        {0x1182, "Cache Errors (Filtered): SNOOP.I.L2"},
        {0x1186, "Cache Errors (Filtered): SNOOP.D.L2"},
    };

    MC_STATUS status_decoded;
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
            entry["MSCOD_decoded"] = *mscod;
        }

        auto mcacod = getDecoded(
            decodeMcacod, static_cast<uint16_t>(status_decoded.mcacod));
        if (mcacod)
        {
            entry["MCACOD_decoded"] = *mcacod;
        }
        else
        {
            entry["MCACOD_decoded"] = decodeMcacodGeneric(status_decoded.mcacod);
        }
    }

  public:
    IcxMcaBankCbo(const MCAData& mca) : IcxMcaDecoder(mca)
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