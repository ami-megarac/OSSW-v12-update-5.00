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
#include <vector>

struct UncAerData
{
    union
    {
        struct
        {
            uint32_t reserved0 : 4, data_link_protocol_error : 1,
                surprise_down_error : 1, reserved1 : 6, poisoned_tlp : 1,
                flow_control_protocol_error : 1, completition_timeout : 1,
                completer_abort : 1, unexpected_completition : 1,
                receiver_buffer_overflow : 1, malformed_tlp : 1, ecrc_error : 1,
                received_an_unsupported_request : 1, acs_violation : 1,
                uncorrectable_internal_error : 1, mc_blocked_tlp : 1,
                atomic_egress_blocked : 1, tlp_prefix_blocked : 1,
                poisoned_tlp_egress_blocked : 1, reserved2 : 5;
        };
        uint32_t error_status;
    };
    std::string address;
};

struct CorAerData
{
    union
    {
        struct
        {
            uint32_t receiver_error : 1, reserved0 : 5, bad_tlp : 1,
                bad_dllp : 1, replay_num_rollover : 1, reserved1 : 3,
                replay_timer_timeout : 1, advisory_non_fatal_error : 1,
                correctable_internal_error : 1, header_log_overflow_error : 1,
                reserved2 : 16;
        };
        uint32_t error_status;
    };
    std::string address;
};

using UncAer = std::map<uint32_t, std::vector<UncAerData>>;
using CorAer = std::map<uint32_t, std::vector<CorAerData>>;