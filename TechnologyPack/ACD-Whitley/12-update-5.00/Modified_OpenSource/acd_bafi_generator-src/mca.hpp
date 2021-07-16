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
#include <memory>
#include <string>

#include <mca_cpx.hpp>
#include <mca_icx.hpp>
#include <mca_skx.hpp>
#include <mca_spr.hpp>

std::shared_ptr<McaDecoder> mcaDecoderFactory(const MCAData& mca,
                                              const std::string& cpuType)
{
    if (cpuType == "CPX" || cpuType == "CLX")
    {
        if (mca.cbo)
        {
            return std::make_shared<CpxMcaBankCbo>(mca);
        }
        switch (mca.bank)
        {
            case 0:
                return std::make_shared<CpxMcaBank0>(mca);
            case 1:
                return std::make_shared<CpxMcaBank1>(mca);
            case 2:
                return std::make_shared<CpxMcaBank2>(mca);
            case 3:
                return std::make_shared<CpxMcaBank3>(mca);
            case 4:
                return std::make_shared<CpxMcaBank4>(mca);
            case 5:
                return std::make_shared<CpxMcaBank5>(mca);
            case 6:
                return std::make_shared<CpxMcaBank6>(mca);
            case 7:
                return std::make_shared<CpxMcaBank7>(mca);
            case 8:
                return std::make_shared<CpxMcaBank8>(mca);
            case 9:
                return std::make_shared<CpxMcaBank9>(mca);
            case 10:
                return std::make_shared<CpxMcaBank10>(mca);
            case 11:
                return std::make_shared<CpxMcaBank11>(mca);
            case 12:
                return std::make_shared<CpxMcaBank12>(mca);
            case 13:
                return std::make_shared<CpxMcaBank13>(mca);
            case 14:
                return std::make_shared<CpxMcaBank14>(mca);
            case 15:
                return std::make_shared<CpxMcaBank15>(mca);
            case 16:
                return std::make_shared<CpxMcaBank16>(mca);
            case 17:
                return std::make_shared<CpxMcaBank17>(mca);
            case 18:
                return std::make_shared<CpxMcaBank18>(mca);
            case 19:
                return std::make_shared<CpxMcaBank19>(mca);
            case 20:
                return std::make_shared<CpxMcaBank20>(mca);
            case 21:
                return std::make_shared<CpxMcaBank21>(mca);
            case 22:
                return std::make_shared<CpxMcaBank22>(mca);
        }
    }
    else if (cpuType == "ICX")
    {
        if (mca.cbo)
        {
            return std::make_shared<IcxMcaBankCbo>(mca);
        }
        switch (mca.bank)
        {
            case 0:
                return std::make_shared<IcxMcaBankIfu>(mca);
            case 1:
                return std::make_shared<IcxMcaBankDcu>(mca);
            case 2:
                return std::make_shared<IcxMcaBankDtlb>(mca);
            case 3:
                return std::make_shared<IcxMcaBankMlc>(mca);
            case 4:
                return std::make_shared<IcxMcaBankPcu>(mca);
            case 6:
                return std::make_shared<IcxMcaBankIio>(mca);
            case 5:
            case 7:
            case 8:
                return std::make_shared<IcxMcaBankUpi>(mca);
            case 9:
            case 10:
            case 11:
                return std::make_shared<IcxMcaBankCha>(mca);
            case 12:
            case 16:
            case 20:
            case 24:
                return std::make_shared<IcxMcaBankM2m>(mca);
            case 13:
            case 14:
            case 17:
            case 18:
            case 21:
            case 22:
            case 25:
            case 26:
                return std::make_shared<IcxMcaBankImc>(mca);
        }
    }
    else if (cpuType == "SKX")
    {
        if (mca.cbo)
        {
            return std::make_shared<SkxMcaBankCbo>(mca);
        }
        switch (mca.bank)
        {
            case 0:
                return std::make_shared<SkxMcaBankIfu>(mca);
            case 1:
                return std::make_shared<SkxMcaBankDcu>(mca);
            case 3:
                return std::make_shared<SkxMcaBankMlc>(mca);
            case 4:
                return std::make_shared<SkxMcaBankPcu>(mca);
            case 5:
                return std::make_shared<SkxMcaBankUpi>(mca);
            case 6:
                return std::make_shared<SkxMcaBankIio>(mca);
            case 7:
                return std::make_shared<SkxMcaBankM2m>(mca);
            case 8:
                return std::make_shared<SkxMcaBankM2m>(mca);
            case 9:
                return std::make_shared<SkxMcaBankCha>(mca);
            case 10:
                return std::make_shared<SkxMcaBankCha>(mca);
            case 11:
                return std::make_shared<SkxMcaBankCha>(mca);
            case 12:
                return std::make_shared<SkxMcaBankUpi>(mca);
            case 13:
                return std::make_shared<SkxMcaBankImc>(mca);
            case 14:
                return std::make_shared<SkxMcaBankImc>(mca);
            case 15:
                return std::make_shared<SkxMcaBankImc>(mca);
            case 16:
                return std::make_shared<SkxMcaBankImc>(mca);
            case 17:
                return std::make_shared<SkxMcaBankImc>(mca);
            case 18:
                return std::make_shared<SkxMcaBankImc>(mca);
            case 19:
                return std::make_shared<SkxMcaBankUpi>(mca);
        }
    }
    else if (cpuType == "SPR")
    {
        if (mca.cbo)
        {
            return std::make_shared<SprMcaBankCbo>(mca);
        }
        switch (mca.bank)
        {
            case 0:
                return std::make_shared<SprMcaBankIfu>(mca);
            case 1:
                return std::make_shared<SprMcaBankDcu>(mca);
            case 2:
                return std::make_shared<SprMcaBankDtlb>(mca);
            case 3:
                return std::make_shared<SprMcaBankMlc>(mca);
            case 4:
                return std::make_shared<SprMcaBankPcu>(mca);
            case 5:
                return std::make_shared<SprMcaBankUpi>(mca);
            case 6:
                return std::make_shared<SprMcaBankIio>(mca);
            case 7:
                return std::make_shared<SprMcaBankUpi>(mca);
            case 8:
                return std::make_shared<SprMcaBankUpi>(mca);
            case 9:
                return std::make_shared<SprMcaBankCha>(mca);
            case 10:
                return std::make_shared<SprMcaBankCha>(mca);
            case 11:
                return std::make_shared<SprMcaBankCha>(mca);
            case 12:
                return std::make_shared<SprMcaBankM2m>(mca);
            case 13:
                return std::make_shared<SprMcaBankImc>(mca);
            case 14:
                return std::make_shared<SprMcaBankImc>(mca);
            case 15:
                return std::make_shared<SprMcaBankImc>(mca);
            case 16:
                return std::make_shared<SprMcaBankM2m>(mca);
            case 17:
                return std::make_shared<SprMcaBankImc>(mca);
            case 18:
                return std::make_shared<SprMcaBankImc>(mca);
            case 19:
                return std::make_shared<SprMcaBankImc>(mca);
            case 20:
                return std::make_shared<SprMcaBankM2m>(mca);
            case 21:
                return std::make_shared<SprMcaBankImc>(mca);
            case 22:
                return std::make_shared<SprMcaBankImc>(mca);
            case 23:
                return std::make_shared<SprMcaBankImc>(mca);
            case 24:
                return std::make_shared<SprMcaBankM2m>(mca);
            case 25:
                return std::make_shared<SprMcaBankImc>(mca);
            case 26:
                return std::make_shared<SprMcaBankImc>(mca);
            case 27:
                return std::make_shared<SprMcaBankImc>(mca);
        }
    }
    return nullptr;
};