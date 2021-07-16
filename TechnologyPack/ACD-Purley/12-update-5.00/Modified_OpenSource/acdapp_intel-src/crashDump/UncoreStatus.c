/*
// Copyright (C) 2018 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions
// and limitations under the License.
//
//
// SPDX-License-Identifier: Apache-2.0
*/

#include "common.h"
#ifndef SPX_BMC_ACD
#include <cJSON.h>
#endif
#include "UncoreStatus.h"

extern BOOL IsCpuPresent(UINT8 CpuNum);

static const SUncoreStatusRegPci sUncoreStatusPci[] =
{
    // Register,                                                Bus,    Dev,    Func,   Offset, Size
    { "MMCFG_BASE",                                             0,      5,      0,      0x0090, US_REG_QWORD },
    { "MMCFG_LIMIT",                                            0,      5,      0,      0x0098, US_REG_QWORD },
    { "TSEG",                                                   0,      5,      0,      0x00A8, US_REG_QWORD },
    { "TOLM",                                                   0,      5,      0,      0x00D0, US_REG_QWORD },
    { "TOHM",                                                   0,      5,      0,      0x00D8, US_REG_QWORD },
    { "NCMEM_BASE",                                             0,      5,      0,      0x00E0, US_REG_QWORD },
    { "NCMEM_LIMIT",                                            0,      5,      0,      0x00E8, US_REG_QWORD },
    { "MENCMEM_BASE",                                           0,      5,      0,      0x00F0, US_REG_QWORD },
    { "MENCMEM_LIMIT",                                          0,      5,      0,      0x00F8, US_REG_QWORD },
    { "iio_mmioh_non_interleave_b0d05f0",                       0,      5,      0,      0x0340, US_REG_QWORD },
    { "iio_mmioh_non_interleave_b1d05f0",                       1,      5,      0,      0x0340, US_REG_QWORD },
    { "iio_mmioh_non_interleave_b2d05f0",                       2,      5,      0,      0x0340, US_REG_QWORD },
    { "iio_mmioh_non_interleave_b3d05f0",                       3,      5,      0,      0x0340, US_REG_QWORD },
    { "iio_mmioh_interleave_b0d05f0",                           0,      5,      0,      0x0348, US_REG_DWORD },
    { "iio_mmioh_interleave_b1d05f0",                           1,      5,      0,      0x0348, US_REG_DWORD },
    { "iio_mmioh_interleave_b2d05f0",                           2,      5,      0,      0x0348, US_REG_DWORD },
    { "iio_mmioh_interleave_b3d05f0",                           3,      5,      0,      0x0348, US_REG_DWORD },
    { "imc0_c0_correction_debug_dev_vec_1",                     2,      10,     3,      0x0310, US_REG_DWORD },
    { "imc0_c0_correction_debug_dev_vec_2",                     2,      10,     3,      0x0314, US_REG_DWORD },
    { "imc0_c0_correction_debug_log",                           2,      10,     3,      0x0130, US_REG_DWORD },
    { "imc0_c0_correction_debug_plus1_log",                     2,      10,     3,      0x013c, US_REG_DWORD },
    { "imc0_c0_correrrcnt_0",                                   2,      10,     3,      0x0104, US_REG_DWORD },
    { "imc0_c0_correrrcnt_1",                                   2,      10,     3,      0x0108, US_REG_DWORD },
    { "imc0_c0_correrrcnt_2",                                   2,      10,     3,      0x010c, US_REG_DWORD },
    { "imc0_c0_correrrcnt_3",                                   2,      10,     3,      0x0110, US_REG_DWORD },
    { "imc0_c0_correrrorstatus",                                2,      10,     3,      0x0134, US_REG_DWORD },
    { "imc0_c0_correrrthrshld_0",                               2,      10,     3,      0x011c, US_REG_DWORD },
    { "imc0_c0_correrrthrshld_1",                               2,      10,     3,      0x0120, US_REG_DWORD },
    { "imc0_c0_correrrthrshld_2",                               2,      10,     3,      0x0124, US_REG_DWORD },
    { "imc0_c0_correrrthrshld_3",                               2,      10,     3,      0x0128, US_REG_DWORD },
    { "imc0_c0_ddrt_err_log_1st",                               2,      10,     1,      0x0d80, US_REG_DWORD },
    { "imc0_c0_ddrt_err_log_next",                              2,      10,     1,      0x0d84, US_REG_DWORD },
    { "imc0_c0_ddrt_error",                                     2,      10,     1,      0x0d24, US_REG_DWORD },
    { "imc0_c0_ddrt_fnv0_event0",                               2,      10,     1,      0x0a60, US_REG_DWORD },
    { "imc0_c0_ddrt_fnv0_event1",                               2,      10,     1,      0x0a64, US_REG_DWORD },
    { "imc0_c0_ddrt_fnv1_event0",                               2,      10,     1,      0x0a70, US_REG_DWORD },
    { "imc0_c0_ddrt_fnv1_event1",                               2,      10,     1,      0x0a74, US_REG_DWORD },
    { "imc0_c0_ddrt_retry_fsm_state",                           2,      10,     1,      0x0904, US_REG_DWORD },
    { "imc0_c0_ddrt_retry_status",                              2,      10,     1,      0x0a98, US_REG_DWORD },
    { "imc0_c0_detection_debug_log",                            2,      10,     3,      0x014c, US_REG_DWORD },

    { "imc0_c0_detection_debug_log_address2",                   2,      10,     3,      0x0118, US_REG_DWORD },
    { "imc0_c0_detection_debug_log_address1",                   2,      10,     3,      0x012c, US_REG_DWORD },
    { "imc0_c0_detection_debug_log_parity",                     2,      10,     3,      0x016c, US_REG_DWORD },
    { "imc0_c0_detection_debug_log_locator",                    2,      10,     3,      0x019c, US_REG_DWORD },

    { "imc0_c0_devtag_cntl_0",                                  2,      10,     3,      0x0140,  US_REG_BYTE },
    { "imc0_c0_devtag_cntl_1",                                  2,      10,     3,      0x0141,  US_REG_BYTE },
    { "imc0_c0_devtag_cntl_2",                                  2,      10,     3,      0x0142,  US_REG_BYTE },
    { "imc0_c0_devtag_cntl_3",                                  2,      10,     3,      0x0143,  US_REG_BYTE },
    { "imc0_c0_devtag_cntl_4",                                  2,      10,     3,      0x0144,  US_REG_BYTE },
    { "imc0_c0_devtag_cntl_5",                                  2,      10,     3,      0x0145,  US_REG_BYTE },
    { "imc0_c0_devtag_cntl_6",                                  2,      10,     3,      0x0146,  US_REG_BYTE },
    { "imc0_c0_devtag_cntl_7",                                  2,      10,     3,      0x0147,  US_REG_BYTE },
    { "imc0_c0_link_err_fsm_state",                             2,      10,     3,      0x0420, US_REG_DWORD },
    { "imc0_c0_link_error",                                     2,      10,     3,      0x0308, US_REG_DWORD },
    { "imc0_c0_link_retry_err_limits",                          2,      10,     3,      0x040c, US_REG_DWORD },
    { "imc0_c0_link_retry_sb_err_count",                        2,      10,     3,      0x0400, US_REG_DWORD },
    { "imc0_c0_retry_rd_err_log",                               2,      10,     3,      0x0154, US_REG_DWORD },
    { "imc0_c0_retry_rd_err_log_address1",                      2,      10,     3,      0x015c, US_REG_DWORD },
    { "imc0_c0_retry_rd_err_log_address2",                      2,      10,     3,      0x0114, US_REG_DWORD },
    { "imc0_c0_retry_rd_err_log_misc",                          2,      10,     3,      0x0148, US_REG_DWORD },
    { "imc0_c0_retry_rd_err_log_parity",                        2,      10,     3,      0x0150, US_REG_DWORD },
    { "imc0_c1_correction_debug_dev_vec_1",                     2,      10,     7,      0x0310, US_REG_DWORD },
    { "imc0_c1_correction_debug_dev_vec_2",                     2,      10,     7,      0x0314, US_REG_DWORD },
    { "imc0_c1_correction_debug_log",                           2,      10,     7,      0x0130, US_REG_DWORD },
    { "imc0_c1_correction_debug_plus1_log",                     2,      10,     7,      0x013c, US_REG_DWORD },
    { "imc0_c1_correrrcnt_0",                                   2,      10,     7,      0x0104, US_REG_DWORD },
    { "imc0_c1_correrrcnt_1",                                   2,      10,     7,      0x0108, US_REG_DWORD },
    { "imc0_c1_correrrcnt_2",                                   2,      10,     7,      0x010c, US_REG_DWORD },
    { "imc0_c1_correrrcnt_3",                                   2,      10,     7,      0x0110, US_REG_DWORD },
    { "imc0_c1_correrrorstatus",                                2,      10,     7,      0x0134, US_REG_DWORD },
    { "imc0_c1_correrrthrshld_0",                               2,      10,     7,      0x011c, US_REG_DWORD },
    { "imc0_c1_correrrthrshld_1",                               2,      10,     7,      0x0120, US_REG_DWORD },
    { "imc0_c1_correrrthrshld_2",                               2,      10,     7,      0x0124, US_REG_DWORD },
    { "imc0_c1_correrrthrshld_3",                               2,      10,     7,      0x0128, US_REG_DWORD },
    { "imc0_c1_ddrt_err_log_1st",                               2,      10,     7,      0x0d80, US_REG_DWORD },
    { "imc0_c1_ddrt_err_log_next",                              2,      10,     7,      0x0d84, US_REG_DWORD },
    { "imc0_c1_ddrt_error",                                     2,      10,     7,      0x0d24, US_REG_DWORD },
    { "imc0_c1_ddrt_fnv0_event0",                               2,      10,     7,      0x0a60, US_REG_DWORD },
    { "imc0_c1_ddrt_fnv0_event1",                               2,      10,     7,      0x0a64, US_REG_DWORD },
    { "imc0_c1_ddrt_fnv1_event0",                               2,      10,     7,      0x0a70, US_REG_DWORD },
    { "imc0_c1_ddrt_fnv1_event1",                               2,      10,     7,      0x0a74, US_REG_DWORD },
    { "imc0_c1_ddrt_retry_fsm_state",                           2,      10,     7,      0x0904, US_REG_DWORD },
    { "imc0_c1_ddrt_retry_status",                              2,      10,     7,      0x0a98, US_REG_DWORD },
    { "imc0_c1_detection_debug_log",                            2,      10,     7,      0x014c, US_REG_DWORD },

    { "imc0_c1_detection_debug_log_address2",                   2,      10,     7,      0x0118, US_REG_DWORD },
    { "imc0_c1_detection_debug_log_address1",                   2,      10,     7,      0x012c, US_REG_DWORD },
    { "imc0_c1_detection_debug_log_parity",                     2,      10,     7,      0x016c, US_REG_DWORD },
    { "imc0_c1_detection_debug_log_locator",                    2,      10,     7,      0x019c, US_REG_DWORD },

    { "imc0_c1_devtag_cntl_0",                                  2,      10,     7,      0x0140,  US_REG_BYTE },
    { "imc0_c1_devtag_cntl_1",                                  2,      10,     7,      0x0141,  US_REG_BYTE },
    { "imc0_c1_devtag_cntl_2",                                  2,      10,     7,      0x0142,  US_REG_BYTE },
    { "imc0_c1_devtag_cntl_3",                                  2,      10,     7,      0x0143,  US_REG_BYTE },
    { "imc0_c1_devtag_cntl_4",                                  2,      10,     7,      0x0144,  US_REG_BYTE },
    { "imc0_c1_devtag_cntl_5",                                  2,      10,     7,      0x0145,  US_REG_BYTE },
    { "imc0_c1_devtag_cntl_6",                                  2,      10,     7,      0x0146,  US_REG_BYTE },
    { "imc0_c1_devtag_cntl_7",                                  2,      10,     7,      0x0147,  US_REG_BYTE },
    { "imc0_c1_link_err_fsm_state",                             2,      10,     7,      0x0420, US_REG_DWORD },
    { "imc0_c1_link_error",                                     2,      10,     7,      0x0308, US_REG_DWORD },
    { "imc0_c1_link_retry_err_limits",                          2,      10,     7,      0x040c, US_REG_DWORD },
    { "imc0_c1_link_retry_sb_err_count",                        2,      10,     7,      0x0400, US_REG_DWORD },
    { "imc0_c1_retry_rd_err_log",                               2,      10,     7,      0x0154, US_REG_DWORD },
    { "imc0_c1_retry_rd_err_log_address1",                      2,      10,     7,      0x015c, US_REG_DWORD },
    { "imc0_c1_retry_rd_err_log_address2",                      2,      10,     7,      0x0114, US_REG_DWORD },
    { "imc0_c1_retry_rd_err_log_misc",                          2,      10,     7,      0x0148, US_REG_DWORD },
    { "imc0_c1_retry_rd_err_log_parity",                        2,      10,     7,      0x0150, US_REG_DWORD },
    { "imc0_c2_correction_debug_dev_vec_1",                     2,      11,     3,      0x0310, US_REG_DWORD },
    { "imc0_c2_correction_debug_dev_vec_2",                     2,      11,     3,      0x0314, US_REG_DWORD },
    { "imc0_c2_correction_debug_log",                           2,      11,     3,      0x0130, US_REG_DWORD },
    { "imc0_c2_correction_debug_plus1_log",                     2,      11,     3,      0x013c, US_REG_DWORD },
    { "imc0_c2_correrrcnt_0",                                   2,      11,     3,      0x0104, US_REG_DWORD },
    { "imc0_c2_correrrcnt_1",                                   2,      11,     3,      0x0108, US_REG_DWORD },
    { "imc0_c2_correrrcnt_2",                                   2,      11,     3,      0x010c, US_REG_DWORD },
    { "imc0_c2_correrrcnt_3",                                   2,      11,     3,      0x0110, US_REG_DWORD },
    { "imc0_c2_correrrorstatus",                                2,      11,     3,      0x0134, US_REG_DWORD },
    { "imc0_c2_correrrthrshld_0",                               2,      11,     3,      0x011c, US_REG_DWORD },
    { "imc0_c2_correrrthrshld_1",                               2,      11,     3,      0x0120, US_REG_DWORD },
    { "imc0_c2_correrrthrshld_2",                               2,      11,     3,      0x0124, US_REG_DWORD },
    { "imc0_c2_correrrthrshld_3",                               2,      11,     3,      0x0128, US_REG_DWORD },
    { "imc0_c2_ddrt_err_log_1st",                               2,      11,     1,      0x0d80, US_REG_DWORD },
    { "imc0_c2_ddrt_err_log_next",                              2,      11,     1,      0x0d84, US_REG_DWORD },
    { "imc0_c2_ddrt_error",                                     2,      11,     1,      0x0d24, US_REG_DWORD },
    { "imc0_c2_ddrt_fnv0_event0",                               2,      11,     1,      0x0a60, US_REG_DWORD },
    { "imc0_c2_ddrt_fnv0_event1",                               2,      11,     1,      0x0a64, US_REG_DWORD },
    { "imc0_c2_ddrt_fnv1_event0",                               2,      11,     1,      0x0a70, US_REG_DWORD },
    { "imc0_c2_ddrt_fnv1_event1",                               2,      11,     1,      0x0a74, US_REG_DWORD },
    { "imc0_c2_ddrt_retry_fsm_state",                           2,      11,     1,      0x0904, US_REG_DWORD },
    { "imc0_c2_ddrt_retry_status",                              2,      11,     1,      0x0a98, US_REG_DWORD },
    { "imc0_c2_detection_debug_log",                            2,      11,     3,      0x014c, US_REG_DWORD },

    { "imc0_c2_detection_debug_log_address2",                   2,      11,     3,      0x0118, US_REG_DWORD },
    { "imc0_c2_detection_debug_log_address1",                   2,      11,     3,      0x012c, US_REG_DWORD },
    { "imc0_c2_detection_debug_log_parity",                     2,      11,     3,      0x016c, US_REG_DWORD },
    { "imc0_c2_detection_debug_log_locator",                    2,      11,     3,      0x019c, US_REG_DWORD },

    { "imc0_c2_devtag_cntl_0",                                  2,      11,     3,      0x0140,  US_REG_BYTE },
    { "imc0_c2_devtag_cntl_1",                                  2,      11,     3,      0x0141,  US_REG_BYTE },
    { "imc0_c2_devtag_cntl_2",                                  2,      11,     3,      0x0142,  US_REG_BYTE },
    { "imc0_c2_devtag_cntl_3",                                  2,      11,     3,      0x0143,  US_REG_BYTE },
    { "imc0_c2_devtag_cntl_4",                                  2,      11,     3,      0x0144,  US_REG_BYTE },
    { "imc0_c2_devtag_cntl_5",                                  2,      11,     3,      0x0145,  US_REG_BYTE },
    { "imc0_c2_devtag_cntl_6",                                  2,      11,     3,      0x0146,  US_REG_BYTE },
    { "imc0_c2_devtag_cntl_7",                                  2,      11,     3,      0x0147,  US_REG_BYTE },
    { "imc0_c2_link_err_fsm_state",                             2,      11,     3,      0x0420, US_REG_DWORD },
    { "imc0_c2_link_error",                                     2,      11,     3,      0x0308, US_REG_DWORD },
    { "imc0_c2_link_retry_err_limits",                          2,      11,     3,      0x040c, US_REG_DWORD },
    { "imc0_c2_link_retry_sb_err_count",                        2,      11,     3,      0x0400, US_REG_DWORD },
    { "imc0_c2_retry_rd_err_log",                               2,      11,     3,      0x0154, US_REG_DWORD },
    { "imc0_c2_retry_rd_err_log_address1",                      2,      11,     3,      0x015c, US_REG_DWORD },
    { "imc0_c2_retry_rd_err_log_address2",                      2,      11,     3,      0x0114, US_REG_DWORD },
    { "imc0_c2_retry_rd_err_log_misc",                          2,      11,     3,      0x0148, US_REG_DWORD },
    { "imc0_c2_retry_rd_err_log_parity",                        2,      11,     3,      0x0150, US_REG_DWORD },
    { "imc0_imc0_poison_source",                                2,      10,     0,      0x0980, US_REG_QWORD },
    { "imc0_imc1_poison_source",                                2,      10,     0,      0x0988, US_REG_QWORD },
    { "imc0_imc2_poison_source",                                2,      10,     0,      0x0990, US_REG_QWORD },
    { "imc0_m2mem_err_cntr",                                    2,      8,      0,      0x0144, US_REG_DWORD },
    { "imc0_m2mem_err_cntr_ctl",                                2,      8,      0,      0x0140, US_REG_DWORD },

    { "imc0_dimmmtr_0",                                         2,      10,     0,      0x0080, US_REG_DWORD },
    { "imc0_dimmmtr_1",                                         2,      10,     0,      0x0084, US_REG_DWORD },
    { "imc0_dimmmtr_2",                                         2,      10,     0,      0x0088, US_REG_DWORD },
    { "imc0_dimmmtr_0",                                         2,      10,     4,      0x0080, US_REG_DWORD },
    { "imc0_dimmmtr_1",                                         2,      10,     4,      0x0084, US_REG_DWORD },
    { "imc0_dimmmtr_2",                                         2,      10,     4,      0x0088, US_REG_DWORD },
    { "imc0_dimmmtr_0",                                         2,      11,     0,      0x0080, US_REG_DWORD },
    { "imc0_dimmmtr_1",                                         2,      11,     0,      0x0084, US_REG_DWORD },
    { "imc0_dimmmtr_2",                                         2,      11,     0,      0x0088, US_REG_DWORD },

    { "imc1_c0_correction_debug_dev_vec_1",                     2,      12,     3,      0x0310, US_REG_DWORD },
    { "imc1_c0_correction_debug_dev_vec_2",                     2,      12,     3,      0x0314, US_REG_DWORD },
    { "imc1_c0_correction_debug_log",                           2,      12,     3,      0x0130, US_REG_DWORD },
    { "imc1_c0_correction_debug_plus1_log",                     2,      12,     3,      0x013c, US_REG_DWORD },
    { "imc1_c0_correrrcnt_0",                                   2,      12,     3,      0x0104, US_REG_DWORD },
    { "imc1_c0_correrrcnt_1",                                   2,      12,     3,      0x0108, US_REG_DWORD },
    { "imc1_c0_correrrcnt_2",                                   2,      12,     3,      0x010c, US_REG_DWORD },
    { "imc1_c0_correrrcnt_3",                                   2,      12,     3,      0x0110, US_REG_DWORD },
    { "imc1_c0_correrrorstatus",                                2,      12,     3,      0x0134, US_REG_DWORD },
    { "imc1_c0_correrrthrshld_0",                               2,      12,     3,      0x011c, US_REG_DWORD },
    { "imc1_c0_correrrthrshld_1",                               2,      12,     3,      0x0120, US_REG_DWORD },
    { "imc1_c0_correrrthrshld_2",                               2,      12,     3,      0x0124, US_REG_DWORD },
    { "imc1_c0_correrrthrshld_3",                               2,      12,     3,      0x0128, US_REG_DWORD },
    { "imc1_c0_ddrt_err_log_1st",                               2,      12,     1,      0x0d80, US_REG_DWORD },
    { "imc1_c0_ddrt_err_log_next",                              2,      12,     1,      0x0d84, US_REG_DWORD },
    { "imc1_c0_ddrt_error",                                     2,      12,     1,      0x0d24, US_REG_DWORD },
    { "imc1_c0_ddrt_fnv0_event0",                               2,      12,     1,      0x0a60, US_REG_DWORD },
    { "imc1_c0_ddrt_fnv0_event1",                               2,      12,     1,      0x0a64, US_REG_DWORD },
    { "imc1_c0_ddrt_fnv1_event0",                               2,      12,     1,      0x0a70, US_REG_DWORD },
    { "imc1_c0_ddrt_fnv1_event1",                               2,      12,     1,      0x0a74, US_REG_DWORD },
    { "imc1_c0_ddrt_retry_fsm_state",                           2,      12,     1,      0x0904, US_REG_DWORD },
    { "imc1_c0_ddrt_retry_status",                              2,      12,     1,      0x0a98, US_REG_DWORD },
    { "imc1_c0_detection_debug_log",                            2,      12,     3,      0x014c, US_REG_DWORD },

    { "imc0_c0_detection_debug_log_address2",                   2,      12,     3,      0x0118, US_REG_DWORD },
    { "imc0_c0_detection_debug_log_address1",                   2,      12,     3,      0x012c, US_REG_DWORD },
    { "imc0_c0_detection_debug_log_parity",                     2,      12,     3,      0x016c, US_REG_DWORD },
    { "imc0_c0_detection_debug_log_locator",                    2,      12,     3,      0x019c, US_REG_DWORD },

    { "imc1_c0_devtag_cntl_0",                                  2,      12,     3,      0x0140,  US_REG_BYTE },
    { "imc1_c0_devtag_cntl_1",                                  2,      12,     3,      0x0141,  US_REG_BYTE },
    { "imc1_c0_devtag_cntl_2",                                  2,      12,     3,      0x0142,  US_REG_BYTE },
    { "imc1_c0_devtag_cntl_3",                                  2,      12,     3,      0x0143,  US_REG_BYTE },
    { "imc1_c0_devtag_cntl_4",                                  2,      12,     3,      0x0144,  US_REG_BYTE },
    { "imc1_c0_devtag_cntl_5",                                  2,      12,     3,      0x0145,  US_REG_BYTE },
    { "imc1_c0_devtag_cntl_6",                                  2,      12,     3,      0x0146,  US_REG_BYTE },
    { "imc1_c0_devtag_cntl_7",                                  2,      12,     3,      0x0147,  US_REG_BYTE },
    { "imc1_c0_link_err_fsm_state",                             2,      12,     3,      0x0420, US_REG_DWORD },
    { "imc1_c0_link_error",                                     2,      12,     3,      0x0308, US_REG_DWORD },
    { "imc1_c0_link_retry_err_limits",                          2,      12,     3,      0x040c, US_REG_DWORD },
    { "imc1_c0_link_retry_sb_err_count",                        2,      12,     3,      0x0400, US_REG_DWORD },
    { "imc1_c0_retry_rd_err_log",                               2,      12,     3,      0x0154, US_REG_DWORD },
    { "imc1_c0_retry_rd_err_log_address1",                      2,      12,     3,      0x015c, US_REG_DWORD },
    { "imc1_c0_retry_rd_err_log_address2",                      2,      12,     3,      0x0114, US_REG_DWORD },
    { "imc1_c0_retry_rd_err_log_misc",                          2,      12,     3,      0x0148, US_REG_DWORD },
    { "imc1_c0_retry_rd_err_log_parity",                        2,      12,     3,      0x0150, US_REG_DWORD },
    { "imc1_c1_correction_debug_dev_vec_1",                     2,      12,     7,      0x0310, US_REG_DWORD },
    { "imc1_c1_correction_debug_dev_vec_2",                     2,      12,     7,      0x0314, US_REG_DWORD },
    { "imc1_c1_correction_debug_log",                           2,      12,     7,      0x0130, US_REG_DWORD },
    { "imc1_c1_correction_debug_plus1_log",                     2,      12,     7,      0x013c, US_REG_DWORD },
    { "imc1_c1_correrrcnt_0",                                   2,      12,     7,      0x0104, US_REG_DWORD },
    { "imc1_c1_correrrcnt_1",                                   2,      12,     7,      0x0108, US_REG_DWORD },
    { "imc1_c1_correrrcnt_2",                                   2,      12,     7,      0x010c, US_REG_DWORD },
    { "imc1_c1_correrrcnt_3",                                   2,      12,     7,      0x0110, US_REG_DWORD },
    { "imc1_c1_correrrorstatus",                                2,      12,     7,      0x0134, US_REG_DWORD },
    { "imc1_c1_correrrthrshld_0",                               2,      12,     7,      0x011c, US_REG_DWORD },
    { "imc1_c1_correrrthrshld_1",                               2,      12,     7,      0x0120, US_REG_DWORD },
    { "imc1_c1_correrrthrshld_2",                               2,      12,     7,      0x0124, US_REG_DWORD },
    { "imc1_c1_correrrthrshld_3",                               2,      12,     7,      0x0128, US_REG_DWORD },
    { "imc1_c1_ddrt_err_log_1st",                               2,      12,     7,      0x0d80, US_REG_DWORD },
    { "imc1_c1_ddrt_err_log_next",                              2,      12,     7,      0x0d84, US_REG_DWORD },
    { "imc1_c1_ddrt_error",                                     2,      12,     7,      0x0d24, US_REG_DWORD },
    { "imc1_c1_ddrt_fnv0_event0",                               2,      12,     7,      0x0a60, US_REG_DWORD },
    { "imc1_c1_ddrt_fnv0_event1",                               2,      12,     7,      0x0a64, US_REG_DWORD },
    { "imc1_c1_ddrt_fnv1_event0",                               2,      12,     7,      0x0a70, US_REG_DWORD },
    { "imc1_c1_ddrt_fnv1_event1",                               2,      12,     7,      0x0a74, US_REG_DWORD },
    { "imc1_c1_ddrt_retry_fsm_state",                           2,      12,     7,      0x0904, US_REG_DWORD },
    { "imc1_c1_ddrt_retry_status",                              2,      12,     7,      0x0a98, US_REG_DWORD },
    { "imc1_c1_detection_debug_log",                            2,      12,     7,      0x014c, US_REG_DWORD },

    { "imc1_c1_detection_debug_log_address2",                   2,      12,     7,      0x0118, US_REG_DWORD },
    { "imc1_c1_detection_debug_log_address1",                   2,      12,     7,      0x012c, US_REG_DWORD },
    { "imc1_c1_detection_debug_log_parity",                     2,      12,     7,      0x016c, US_REG_DWORD },
    { "imc1_c1_detection_debug_log_locator",                    2,      12,     7,      0x019c, US_REG_DWORD },

    { "imc1_c1_devtag_cntl_0",                                  2,      12,     7,      0x0140,  US_REG_BYTE },
    { "imc1_c1_devtag_cntl_1",                                  2,      12,     7,      0x0141,  US_REG_BYTE },
    { "imc1_c1_devtag_cntl_2",                                  2,      12,     7,      0x0142,  US_REG_BYTE },
    { "imc1_c1_devtag_cntl_3",                                  2,      12,     7,      0x0143,  US_REG_BYTE },
    { "imc1_c1_devtag_cntl_4",                                  2,      12,     7,      0x0144,  US_REG_BYTE },
    { "imc1_c1_devtag_cntl_5",                                  2,      12,     7,      0x0145,  US_REG_BYTE },
    { "imc1_c1_devtag_cntl_6",                                  2,      12,     7,      0x0146,  US_REG_BYTE },
    { "imc1_c1_devtag_cntl_7",                                  2,      12,     7,      0x0147,  US_REG_BYTE },
    { "imc1_c1_link_err_fsm_state",                             2,      12,     7,      0x0420, US_REG_DWORD },
    { "imc1_c1_link_error",                                     2,      12,     7,      0x0308, US_REG_DWORD },
    { "imc1_c1_link_retry_err_limits",                          2,      12,     7,      0x040c, US_REG_DWORD },
    { "imc1_c1_link_retry_sb_err_count",                        2,      12,     7,      0x0400, US_REG_DWORD },
    { "imc1_c1_retry_rd_err_log",                               2,      12,     7,      0x0154, US_REG_DWORD },
    { "imc1_c1_retry_rd_err_log_address1",                      2,      12,     7,      0x015c, US_REG_DWORD },
    { "imc1_c1_retry_rd_err_log_address2",                      2,      12,     7,      0x0114, US_REG_DWORD },
    { "imc1_c1_retry_rd_err_log_misc",                          2,      12,     7,      0x0148, US_REG_DWORD },
    { "imc1_c1_retry_rd_err_log_parity",                        2,      12,     7,      0x0150, US_REG_DWORD },
    { "imc1_c2_correction_debug_dev_vec_1",                     2,      13,     3,      0x0310, US_REG_DWORD },
    { "imc1_c2_correction_debug_dev_vec_2",                     2,      13,     3,      0x0314, US_REG_DWORD },
    { "imc1_c2_correction_debug_log",                           2,      13,     3,      0x0130, US_REG_DWORD },
    { "imc1_c2_correction_debug_plus1_log",                     2,      13,     3,      0x013c, US_REG_DWORD },
    { "imc1_c2_correrrcnt_0",                                   2,      13,     3,      0x0104, US_REG_DWORD },
    { "imc1_c2_correrrcnt_1",                                   2,      13,     3,      0x0108, US_REG_DWORD },
    { "imc1_c2_correrrcnt_2",                                   2,      13,     3,      0x010c, US_REG_DWORD },
    { "imc1_c2_correrrcnt_3",                                   2,      13,     3,      0x0110, US_REG_DWORD },
    { "imc1_c2_correrrorstatus",                                2,      13,     3,      0x0134, US_REG_DWORD },
    { "imc1_c2_correrrthrshld_0",                               2,      13,     3,      0x011c, US_REG_DWORD },
    { "imc1_c2_correrrthrshld_1",                               2,      13,     3,      0x0120, US_REG_DWORD },
    { "imc1_c2_correrrthrshld_2",                               2,      13,     3,      0x0124, US_REG_DWORD },
    { "imc1_c2_correrrthrshld_3",                               2,      13,     3,      0x0128, US_REG_DWORD },
    { "imc1_c2_ddrt_err_log_1st",                               2,      13,     1,      0x0d80, US_REG_DWORD },
    { "imc1_c2_ddrt_err_log_next",                              2,      13,     1,      0x0d84, US_REG_DWORD },
    { "imc1_c2_ddrt_error",                                     2,      13,     1,      0x0d24, US_REG_DWORD },
    { "imc1_c2_ddrt_fnv0_event0",                               2,      13,     1,      0x0a60, US_REG_DWORD },
    { "imc1_c2_ddrt_fnv0_event1",                               2,      13,     1,      0x0a64, US_REG_DWORD },
    { "imc1_c2_ddrt_fnv1_event0",                               2,      13,     1,      0x0a70, US_REG_DWORD },
    { "imc1_c2_ddrt_fnv1_event1",                               2,      13,     1,      0x0a74, US_REG_DWORD },
    { "imc1_c2_ddrt_retry_fsm_state",                           2,      13,     1,      0x0904, US_REG_DWORD },
    { "imc1_c2_ddrt_retry_status",                              2,      13,     1,      0x0a98, US_REG_DWORD },
    { "imc1_c2_detection_debug_log",                            2,      13,     3,      0x014c, US_REG_DWORD },

    { "imc1_c2_detection_debug_log_address2",                   2,      13,     3,      0x0118, US_REG_DWORD },
    { "imc1_c2_detection_debug_log_address1",                   2,      13,     3,      0x012c, US_REG_DWORD },
    { "imc1_c2_detection_debug_log_parity",                     2,      13,     3,      0x016c, US_REG_DWORD },
    { "imc1_c2_detection_debug_log_locator",                    2,      13,     3,      0x019c, US_REG_DWORD },

    { "imc1_c2_devtag_cntl_0",                                  2,      13,     3,      0x0140,  US_REG_BYTE },
    { "imc1_c2_devtag_cntl_1",                                  2,      13,     3,      0x0141,  US_REG_BYTE },
    { "imc1_c2_devtag_cntl_2",                                  2,      13,     3,      0x0142,  US_REG_BYTE },
    { "imc1_c2_devtag_cntl_3",                                  2,      13,     3,      0x0143,  US_REG_BYTE },
    { "imc1_c2_devtag_cntl_4",                                  2,      13,     3,      0x0144,  US_REG_BYTE },
    { "imc1_c2_devtag_cntl_5",                                  2,      13,     3,      0x0145,  US_REG_BYTE },
    { "imc1_c2_devtag_cntl_6",                                  2,      13,     3,      0x0146,  US_REG_BYTE },
    { "imc1_c2_devtag_cntl_7",                                  2,      13,     3,      0x0147,  US_REG_BYTE },
    { "imc1_c2_link_err_fsm_state",                             2,      13,     3,      0x0420, US_REG_DWORD },
    { "imc1_c2_link_error",                                     2,      13,     3,      0x0308, US_REG_DWORD },
    { "imc1_c2_link_retry_err_limits",                          2,      13,     3,      0x040c, US_REG_DWORD },
    { "imc1_c2_link_retry_sb_err_count",                        2,      13,     3,      0x0400, US_REG_DWORD },
    { "imc1_c2_retry_rd_err_log",                               2,      13,     3,      0x0154, US_REG_DWORD },
    { "imc1_c2_retry_rd_err_log_address1",                      2,      13,     3,      0x015c, US_REG_DWORD },
    { "imc1_c2_retry_rd_err_log_address2",                      2,      13,     3,      0x0114, US_REG_DWORD },
    { "imc1_c2_retry_rd_err_log_misc",                          2,      13,     3,      0x0148, US_REG_DWORD },
    { "imc1_c2_retry_rd_err_log_parity",                        2,      13,     3,      0x0150, US_REG_DWORD },
    { "imc1_imc0_poison_source",                                2,      12,     0,      0x0980, US_REG_QWORD },
    { "imc1_imc1_poison_source",                                2,      12,     0,      0x0988, US_REG_QWORD },
    { "imc1_imc2_poison_source",                                2,      12,     0,      0x0990, US_REG_QWORD },
    { "imc1_m2mem_err_cntr",                                    2,      9,      0,      0x0144, US_REG_DWORD },
    { "imc1_m2mem_err_cntr_ctl",                                2,      9,      0,      0x0140, US_REG_DWORD },

    { "imc1_dimmmtr_0",                                         2,     12,      0,      0x0080, US_REG_DWORD },
    { "imc1_dimmmtr_1",                                         2,     12,      0,      0x0084, US_REG_DWORD },
    { "imc1_dimmmtr_2",                                         2,     12,      0,      0x0088, US_REG_DWORD },
    { "imc1_dimmmtr_0",                                         2,     12,      4,      0x0080, US_REG_DWORD },
    { "imc1_dimmmtr_1",                                         2,     12,      4,      0x0084, US_REG_DWORD },
    { "imc1_dimmmtr_2",                                         2,     12,      4,      0x0088, US_REG_DWORD },
    { "imc1_dimmmtr_0",                                         2,     13,      0,      0x0080, US_REG_DWORD },
    { "imc1_dimmmtr_1",                                         2,     13,      0,      0x0084, US_REG_DWORD },
    { "imc1_dimmmtr_2",                                         2,     13,      0,      0x0088, US_REG_DWORD },

    { "m2_mpc1_rpegrctrlconfig4_r2egrerrlog",                   3,     22,      4,      0x00b0, US_REG_DWORD },
    { "m2_mpc1_rpegrctrlconfig4_r2egrisoerrlog0",               3,     22,      4,      0x0060, US_REG_DWORD },
    { "m2_mpc1_rpegrctrlconfig4_r2egrprqerrlog0",               3,     22,      4,      0x0068, US_REG_DWORD },
    { "m2_mpc1_rputlctrlconfig4_r2glerrcfg",                    3,     22,      4,      0x00a8, US_REG_DWORD },
    { "m2d_pcie1_rpegrctrlconfig1_r2egrerrlog",                 3,     22,      0,      0x00b0, US_REG_DWORD },
    { "m2d_pcie1_rpegrctrlconfig1_r2egrprqerrlog0",             3,     22,      0,      0x0068, US_REG_DWORD },
    { "m2u_pcie0_rpegrctrlconfig0_r2egrerrlog",                 3,     21,      0,      0x00b0, US_REG_DWORD },
    { "m2u_pcie0_rpegrctrlconfig0_r2egrisoerrlog0",             3,     21,      0,      0x0060, US_REG_DWORD },
    { "m2u_pcie0_rpegrctrlconfig0_r2egrprqerrlog0",             3,     21,      0,      0x0068, US_REG_DWORD },
    { "b0d5f1_sltsts0",                                         0,      5,      1,      0x010e,  US_REG_WORD },
    { "b0d5f1_sltsts1",                                         0,      5,      1,      0x011e,  US_REG_WORD },
    { "b0d5f1_sltsts2",                                         0,      5,      1,      0x012e,  US_REG_WORD },
    { "b0d5f1_sltsts3",                                         0,      5,      1,      0x013e,  US_REG_WORD },
    { "cb0_devsts",                                             0,      4,      0,      0x009a,  US_REG_WORD },
    { "cb0_dmauncerrsts",                                       0,      4,      0,      0x0148, US_REG_DWORD },
    { "cb1_devsts",                                             0,      4,      1,      0x009a,  US_REG_WORD },
    { "cb2_devsts",                                             0,      4,      2,      0x009a,  US_REG_WORD },
    { "cb3_devsts",                                             0,      4,      3,      0x009a,  US_REG_WORD },
    { "cb4_devsts",                                             0,      4,      4,      0x009a,  US_REG_WORD },
    { "cb5_devsts",                                             0,      4,      5,      0x009a,  US_REG_WORD },
    { "cb6_devsts",                                             0,      4,      6,      0x009a,  US_REG_WORD },
    { "cb7_devsts",                                             0,      4,      7,      0x009a,  US_REG_WORD },
    { "pxp_b0d00f0_corerrsts",                                  0,      0,      0,      0x0158, US_REG_DWORD },
    { "pxp_b0d00f0_devsts",                                     0,      0,      0,      0x009a,  US_REG_WORD },
    { "pxp_b0d00f0_errsid",                                     0,      0,      0,      0x017c, US_REG_DWORD },
    { "pxp_b0d00f0_ler_ctrlsts",                                0,      0,      0,      0x0288, US_REG_DWORD },
    { "pxp_b0d00f0_lnerrsts",                                   0,      0,      0,      0x0258, US_REG_DWORD },
    { "pxp_b0d00f0_lnksts",                                     0,      0,      0,      0x00a2,  US_REG_WORD },
    { "pxp_b0d00f0_lnksts2",                                    0,      0,      0,      0x00c2,  US_REG_WORD },
    { "pxp_b0d00f0_miscctrlsts_0",                              0,      0,      0,      0x0188, US_REG_DWORD },
    { "pxp_b0d00f0_miscctrlsts_1",                              0,      0,      0,      0x018c, US_REG_DWORD },
    { "pxp_b0d00f0_pcists",                                     0,      0,      0,      0x0006,  US_REG_WORD },
    { "pxp_b0d00f0_rperrsts",                                   0,      0,      0,      0x0178, US_REG_DWORD },
    { "pxp_b0d00f0_rppioerr_cap",                               0,      0,      0,      0x0298, US_REG_DWORD },
    { "pxp_b0d00f0_rppioerr_status",                            0,      0,      0,      0x02a4, US_REG_DWORD },
    { "pxp_b0d00f0_secsts",                                     0,      0,      0,      0x001e,  US_REG_WORD },
    { "pxp_b0d00f0_sltsts",                                     0,      0,      0,      0x00aa,  US_REG_WORD },
    { "pxp_b0d00f0_uncerrsts",                                  0,      0,      0,      0x014c, US_REG_DWORD },
    { "pxp_b0d00f0_xpcorerrsts",                                0,      0,      0,      0x0200, US_REG_DWORD },
    { "pxp_b0d00f0_xpglberrptr",                                0,      0,      0,      0x0232,  US_REG_WORD },
    { "pxp_b0d00f0_xpglberrsts",                                0,      0,      0,      0x0230,  US_REG_WORD },
    { "pxp_b0d00f0_xpuncerrsts",                                0,      0,      0,      0x0208, US_REG_DWORD },
    { "pxp_b0d07f0_devsts",                                     0,      7,      0,      0x004a,  US_REG_WORD },
    { "pxp_b0d07f0_lnksts",                                     0,      7,      0,      0x0052,  US_REG_WORD },
    { "pxp_b0d07f0_lnksts2",                                    0,      7,      0,      0x0072,  US_REG_WORD },
    { "pxp_b0d07f0_pcists",                                     0,      7,      0,      0x0006,  US_REG_WORD },
    { "pxp_b0d07f0_sltsts2",                                    0,      7,      0,      0x007a,  US_REG_WORD },
    { "pxp_b0d07f4_devsts",                                     0,      7,      4,      0x004a,  US_REG_WORD },
    { "pxp_b0d07f4_lnksts",                                     0,      7,      4,      0x0052,  US_REG_WORD },
    { "pxp_b0d07f4_lnksts2",                                    0,      7,      4,      0x0072,  US_REG_WORD },
    { "pxp_b0d07f4_pcists",                                     0,      7,      4,      0x0006,  US_REG_WORD },
    { "pxp_b0d07f4_sltsts2",                                    0,      7,      4,      0x007a,  US_REG_WORD },
    { "pxp_b0d07f7_devsts",                                     0,      7,      7,      0x004a,  US_REG_WORD },
    { "pxp_b0d07f7_lnksts",                                     0,      7,      7,      0x0052,  US_REG_WORD },
    { "pxp_b0d07f7_lnksts2",                                    0,      7,      7,      0x0072,  US_REG_WORD },
    { "pxp_b0d07f7_pcists",                                     0,      7,      7,      0x0006,  US_REG_WORD },
    { "pxp_b0d07f7_sltsts2",                                    0,      7,      7,      0x007a,  US_REG_WORD },
    { "pxp_b0d07f7_tswdbgerrstdis1",                            0,      7,      7,      0x0358, US_REG_DWORD },
    { "pxp_b1d00f0_corerrsts",                                  1,      0,      0,      0x0158, US_REG_DWORD },
    { "pxp_b1d00f0_devsts",                                     1,      0,      0,      0x009a,  US_REG_WORD },
    { "pxp_b1d00f0_errsid",                                     1,      0,      0,      0x017c, US_REG_DWORD },
    { "pxp_b1d00f0_ler_ctrlsts",                                1,      0,      0,      0x0288, US_REG_DWORD },
    { "pxp_b1d00f0_lnerrsts",                                   1,      0,      0,      0x0258, US_REG_DWORD },
    { "pxp_b1d00f0_lnksts",                                     1,      0,      0,      0x00a2,  US_REG_WORD },
    { "pxp_b1d00f0_lnksts2",                                    1,      0,      0,      0x00c2,  US_REG_WORD },
    { "pxp_b1d00f0_miscctrlsts_0",                              1,      0,      0,      0x0188, US_REG_DWORD },
    { "pxp_b1d00f0_miscctrlsts_1",                              1,      0,      0,      0x018c, US_REG_DWORD },
    { "pxp_b1d00f0_pcists",                                     1,      0,      0,      0x0006,  US_REG_WORD },
    { "pxp_b1d00f0_rperrsts",                                   1,      0,      0,      0x0178, US_REG_DWORD },
    { "pxp_b1d00f0_rppioerr_cap",                               1,      0,      0,      0x0298, US_REG_DWORD },
    { "pxp_b1d00f0_rppioerr_status",                            1,      0,      0,      0x02a4, US_REG_DWORD },
    { "pxp_b1d00f0_secsts",                                     1,      0,      0,      0x001e,  US_REG_WORD },
    { "pxp_b1d00f0_sltsts",                                     1,      0,      0,      0x00aa,  US_REG_WORD },
    { "pxp_b1d00f0_uncerrsts",                                  1,      0,      0,      0x014c, US_REG_DWORD },

    { "xpcorerrsts",                                            1,      0,      0,      0x0200, US_REG_DWORD },
    { "xpcorerrmsk",                                            1,      0,      0,      0x0204, US_REG_DWORD },
    { "xpuncerrsts",                                            1,      0,      0,      0x0208, US_REG_DWORD },
    { "xpuncerrmsk",                                            1,      0,      0,      0x020c, US_REG_DWORD },
    { "xpuncerrsev",                                            1,      0,      0,      0x0210, US_REG_DWORD },
    { "xpuncerrptr",                                            1,      0,      0,      0x0214,  US_REG_BYTE },
    { "uncedmask",                                              1,      0,      0,      0x0218, US_REG_DWORD },
    { "coredmask",                                              1,      0,      0,      0x021c, US_REG_DWORD },
    { "rpedmask",                                               1,      0,      0,      0x0220, US_REG_DWORD },
    { "xpuncedmask",                                            1,      0,      0,      0x0224, US_REG_DWORD },
    { "xpcoredmask",                                            1,      0,      0,      0x0228, US_REG_DWORD },
    { "xpglberrsts",                                            1,      0,      0,      0x0230,  US_REG_WORD },
    { "xpglberrptr",                                            1,      0,      0,      0x0232,  US_REG_WORD },

    { "pbus",                                                   1,      0,      0,      0x0018,  US_REG_BYTE },
    { "secbus",                                                 1,      0,      0,      0x0019,  US_REG_BYTE },
    { "subbus",                                                 1,      0,      0,      0x001a,  US_REG_BYTE },
    { "iobas",                                                  1,      0,      0,      0x001c,  US_REG_BYTE },
    { "iolim",                                                  1,      0,      0,      0x001d,  US_REG_BYTE },
//    { "secsts",                                                 1,      0,      0,      0x001e,  US_REG_WORD },
    { "mbas",                                                   1,      0,      0,      0x0020,  US_REG_WORD },
    { "mlim",                                                   1,      0,      0,      0x0022,  US_REG_WORD },
    { "pbas",                                                   1,      0,      0,      0x0024,  US_REG_WORD },
    { "plim",                                                   1,      0,      0,      0x0026,  US_REG_WORD },
    { "pbasu",                                                  1,      0,      0,      0x0028, US_REG_DWORD },
    { "plimu",                                                  1,      0,      0,      0x002c, US_REG_DWORD },

    { "pxp_b1d01f0_corerrsts",                                  1,      1,      0,      0x0158, US_REG_DWORD },
    { "pxp_b1d01f0_devsts",                                     1,      1,      0,      0x009a,  US_REG_WORD },
    { "pxp_b1d01f0_errsid",                                     1,      1,      0,      0x017c, US_REG_DWORD },
    { "pxp_b1d01f0_ler_ctrlsts",                                1,      1,      0,      0x0288, US_REG_DWORD },
    { "pxp_b1d01f0_lnerrsts",                                   1,      1,      0,      0x0258, US_REG_DWORD },
    { "pxp_b1d01f0_lnksts",                                     1,      1,      0,      0x00a2,  US_REG_WORD },
    { "pxp_b1d01f0_lnksts2",                                    1,      1,      0,      0x00c2,  US_REG_WORD },
    { "pxp_b1d01f0_miscctrlsts_0",                              1,      1,      0,      0x0188, US_REG_DWORD },
    { "pxp_b1d01f0_miscctrlsts_1",                              1,      1,      0,      0x018c, US_REG_DWORD },
    { "pxp_b1d01f0_pcists",                                     1,      1,      0,      0x0006,  US_REG_WORD },
    { "pxp_b1d01f0_rperrsts",                                   1,      1,      0,      0x0178, US_REG_DWORD },
    { "pxp_b1d01f0_rppioerr_cap",                               1,      1,      0,      0x0298, US_REG_DWORD },
    { "pxp_b1d01f0_rppioerr_status",                            1,      1,      0,      0x02a4, US_REG_DWORD },
    { "pxp_b1d01f0_secsts",                                     1,      1,      0,      0x001e,  US_REG_WORD },
    { "pxp_b1d01f0_sltsts",                                     1,      1,      0,      0x00aa,  US_REG_WORD },

    { "xpcorerrsts",                                            1,      1,      0,      0x0200, US_REG_DWORD },
    { "xpcorerrmsk",                                            1,      1,      0,      0x0204, US_REG_DWORD },
    { "xpuncerrsts",                                            1,      1,      0,      0x0208, US_REG_DWORD },
    { "xpuncerrmsk",                                            1,      1,      0,      0x020c, US_REG_DWORD },
    { "xpuncerrsev",                                            1,      1,      0,      0x0210, US_REG_DWORD },
    { "xpuncerrptr",                                            1,      1,      0,      0x0214,  US_REG_BYTE },
    { "uncedmask",                                              1,      1,      0,      0x0218, US_REG_DWORD },
    { "coredmask",                                              1,      1,      0,      0x021c, US_REG_DWORD },
    { "rpedmask",                                               1,      1,      0,      0x0220, US_REG_DWORD },
    { "xpuncedmask",                                            1,      1,      0,      0x0224, US_REG_DWORD },
    { "xpcoredmask",                                            1,      1,      0,      0x0228, US_REG_DWORD },
    { "xpglberrsts",                                            1,      1,      0,      0x0230,  US_REG_WORD },
    { "xpglberrptr",                                            1,      1,      0,      0x0232,  US_REG_WORD },

    { "pbus",                                                   1,      1,      0,      0x0018,  US_REG_BYTE },
    { "secbus",                                                 1,      1,      0,      0x0019,  US_REG_BYTE },
    { "subbus",                                                 1,      1,      0,      0x001a,  US_REG_BYTE },
    { "iobas",                                                  1,      1,      0,      0x001c,  US_REG_BYTE },
    { "iolim",                                                  1,      1,      0,      0x001d,  US_REG_BYTE },
//    { "secsts",                                                 1,      1,      0,      0x001e,  US_REG_WORD },
    { "mbas",                                                   1,      1,      0,      0x0020,  US_REG_WORD },
    { "mlim",                                                   1,      1,      0,      0x0022,  US_REG_WORD },
    { "pbas",                                                   1,      1,      0,      0x0024,  US_REG_WORD },
    { "plim",                                                   1,      1,      0,      0x0026,  US_REG_WORD },
    { "pbasu",                                                  1,      1,      0,      0x0028, US_REG_DWORD },
    { "plimu",                                                  1,      1,      0,      0x002c, US_REG_DWORD },

    { "pxp_b1d02f0_corerrsts",                                  1,      2,      0,      0x0158, US_REG_DWORD },
    { "pxp_b1d02f0_devsts",                                     1,      2,      0,      0x009a,  US_REG_WORD },
    { "pxp_b1d02f0_errsid",                                     1,      2,      0,      0x017c, US_REG_DWORD },
    { "pxp_b1d02f0_ler_ctrlsts",                                1,      2,      0,      0x0288, US_REG_DWORD },
    { "pxp_b1d02f0_lnerrsts",                                   1,      2,      0,      0x0258, US_REG_DWORD },
    { "pxp_b1d02f0_lnksts",                                     1,      2,      0,      0x00a2,  US_REG_WORD },
    { "pxp_b1d02f0_lnksts2",                                    1,      2,      0,      0x00c2,  US_REG_WORD },
    { "pxp_b1d02f0_miscctrlsts_0",                              1,      2,      0,      0x0188, US_REG_DWORD },
    { "pxp_b1d02f0_miscctrlsts_1",                              1,      2,      0,      0x018c, US_REG_DWORD },
    { "pxp_b1d02f0_pcists",                                     1,      2,      0,      0x0006,  US_REG_WORD },
    { "pxp_b1d02f0_rperrsts",                                   1,      2,      0,      0x0178, US_REG_DWORD },
    { "pxp_b1d02f0_rppioerr_cap",                               1,      2,      0,      0x0298, US_REG_DWORD },
    { "pxp_b1d02f0_rppioerr_status",                            1,      2,      0,      0x02a4, US_REG_DWORD },
    { "pxp_b1d02f0_secsts",                                     1,      2,      0,      0x001e,  US_REG_WORD },
    { "pxp_b1d02f0_sltsts",                                     1,      2,      0,      0x00aa,  US_REG_WORD },
    { "pxp_b1d02f0_uncerrsts",                                  1,      2,      0,      0x014c, US_REG_DWORD },

    { "xpcorerrsts",                                            1,      2,      0,      0x0200, US_REG_DWORD },
    { "xpcorerrmsk",                                            1,      2,      0,      0x0204, US_REG_DWORD },
    { "xpuncerrsts",                                            1,      2,      0,      0x0208, US_REG_DWORD },
    { "xpuncerrmsk",                                            1,      2,      0,      0x020c, US_REG_DWORD },
    { "xpuncerrsev",                                            1,      2,      0,      0x0210, US_REG_DWORD },
    { "xpuncerrptr",                                            1,      2,      0,      0x0214,  US_REG_BYTE },
    { "uncedmask",                                              1,      2,      0,      0x0218, US_REG_DWORD },
    { "coredmask",                                              1,      2,      0,      0x021c, US_REG_DWORD },
    { "rpedmask",                                               1,      2,      0,      0x0220, US_REG_DWORD },
    { "xpuncedmask",                                            1,      2,      0,      0x0224, US_REG_DWORD },
    { "xpcoredmask",                                            1,      2,      0,      0x0228, US_REG_DWORD },
    { "xpglberrsts",                                            1,      2,      0,      0x0230,  US_REG_WORD },
    { "xpglberrptr",                                            1,      2,      0,      0x0232,  US_REG_WORD },

    { "pbus",                                                   1,      2,      0,      0x0018,  US_REG_BYTE },
    { "secbus",                                                 1,      2,      0,      0x0019,  US_REG_BYTE },
    { "subbus",                                                 1,      2,      0,      0x001a,  US_REG_BYTE },
    { "iobas",                                                  1,      2,      0,      0x001c,  US_REG_BYTE },
    { "iolim",                                                  1,      2,      0,      0x001d,  US_REG_BYTE },
//    { "secsts",                                                 1,      2,      0,      0x001e,  US_REG_WORD },
    { "mbas",                                                   1,      2,      0,      0x0020,  US_REG_WORD },
    { "mlim",                                                   1,      2,      0,      0x0022,  US_REG_WORD },
    { "pbas",                                                   1,      2,      0,      0x0024,  US_REG_WORD },
    { "plim",                                                   1,      2,      0,      0x0026,  US_REG_WORD },
    { "pbasu",                                                  1,      2,      0,      0x0028, US_REG_DWORD },
    { "plimu",                                                  1,      2,      0,      0x002c, US_REG_DWORD },

    { "pxp_b1d03f0_corerrsts",                                  1,      3,      0,      0x0158, US_REG_DWORD },
    { "pxp_b1d03f0_devsts",                                     1,      3,      0,      0x009a,  US_REG_WORD },
    { "pxp_b1d03f0_errsid",                                     1,      3,      0,      0x017c, US_REG_DWORD },
    { "pxp_b1d03f0_ler_ctrlsts",                                1,      3,      0,      0x0288, US_REG_DWORD },
    { "pxp_b1d03f0_lnerrsts",                                   1,      3,      0,      0x0258, US_REG_DWORD },
    { "pxp_b1d03f0_lnksts",                                     1,      3,      0,      0x00a2,  US_REG_WORD },
    { "pxp_b1d03f0_lnksts2",                                    1,      3,      0,      0x00c2,  US_REG_WORD },
    { "pxp_b1d03f0_miscctrlsts_0",                              1,      3,      0,      0x0188, US_REG_DWORD },
    { "pxp_b1d03f0_miscctrlsts_1",                              1,      3,      0,      0x018c, US_REG_DWORD },
    { "pxp_b1d03f0_pcists",                                     1,      3,      0,      0x0006,  US_REG_WORD },
    { "pxp_b1d03f0_rperrsts",                                   1,      3,      0,      0x0178, US_REG_DWORD },
    { "pxp_b1d03f0_rppioerr_cap",                               1,      3,      0,      0x0298, US_REG_DWORD },
    { "pxp_b1d03f0_rppioerr_status",                            1,      3,      0,      0x02a4, US_REG_DWORD },
    { "pxp_b1d03f0_secsts",                                     1,      3,      0,      0x001e,  US_REG_WORD },
    { "pxp_b1d03f0_sltsts",                                     1,      3,      0,      0x00aa,  US_REG_WORD },

    { "xpcorerrsts",                                            1,      3,      0,      0x0200, US_REG_DWORD },
    { "xpcorerrmsk",                                            1,      3,      0,      0x0204, US_REG_DWORD },
    { "xpuncerrsts",                                            1,      3,      0,      0x0208, US_REG_DWORD },
    { "xpuncerrmsk",                                            1,      3,      0,      0x020c, US_REG_DWORD },
    { "xpuncerrsev",                                            1,      3,      0,      0x0210, US_REG_DWORD },
    { "xpuncerrptr",                                            1,      3,      0,      0x0214,  US_REG_BYTE },
    { "uncedmask",                                              1,      3,      0,      0x0218, US_REG_DWORD },
    { "coredmask",                                              1,      3,      0,      0x021c, US_REG_DWORD },
    { "rpedmask",                                               1,      3,      0,      0x0220, US_REG_DWORD },
    { "xpuncedmask",                                            1,      3,      0,      0x0224, US_REG_DWORD },
    { "xpcoredmask",                                            1,      3,      0,      0x0228, US_REG_DWORD },
    { "xpglberrsts",                                            1,      3,      0,      0x0230,  US_REG_WORD },
    { "xpglberrptr",                                            1,      3,      0,      0x0232,  US_REG_WORD },

    { "pbus",                                                   1,      3,      0,      0x0018,  US_REG_BYTE },
    { "secbus",                                                 1,      3,      0,      0x0019,  US_REG_BYTE },
    { "subbus",                                                 1,      3,      0,      0x001a,  US_REG_BYTE },
    { "iobas",                                                  1,      3,      0,      0x001c,  US_REG_BYTE },
    { "iolim",                                                  1,      3,      0,      0x001d,  US_REG_BYTE },
//    { "secsts",                                                 1,      3,      0,      0x001e,  US_REG_WORD },
    { "mbas",                                                   1,      3,      0,      0x0020,  US_REG_WORD },
    { "mlim",                                                   1,      3,      0,      0x0022,  US_REG_WORD },
    { "pbas",                                                   1,      3,      0,      0x0024,  US_REG_WORD },
    { "plim",                                                   1,      3,      0,      0x0026,  US_REG_WORD },
    { "pbasu",                                                  1,      3,      0,      0x0028, US_REG_DWORD },
    { "plimu",                                                  1,      3,      0,      0x002c, US_REG_DWORD },

    { "pxp_b1d07f0_devsts",                                     1,      7,      0,      0x004a,  US_REG_WORD },
    { "pxp_b1d07f0_lnksts",                                     1,      7,      0,      0x0052,  US_REG_WORD },
    { "pxp_b1d07f0_lnksts2",                                    1,      7,      0,      0x0072,  US_REG_WORD },
    { "pxp_b1d07f0_pcists",                                     1,      7,      0,      0x0006,  US_REG_WORD },
    { "pxp_b1d07f0_sltsts2",                                    1,      7,      0,      0x007a,  US_REG_WORD },
    { "pxp_b1d07f1_devsts",                                     1,      7,      1,      0x004a,  US_REG_WORD },
    { "pxp_b1d07f1_lnksts",                                     1,      7,      1,      0x0052,  US_REG_WORD },
    { "pxp_b1d07f1_lnksts2",                                    1,      7,      1,      0x0072,  US_REG_WORD },
    { "pxp_b1d07f1_pcists",                                     1,      7,      1,      0x0006,  US_REG_WORD },
    { "pxp_b1d07f1_sltsts2",                                    1,      7,      1,      0x007a,  US_REG_WORD },
    { "pxp_b1d07f2_devsts",                                     1,      7,      2,      0x004a,  US_REG_WORD },
    { "pxp_b1d07f2_lnksts",                                     1,      7,      2,      0x0052,  US_REG_WORD },
    { "pxp_b1d07f2_lnksts2",                                    1,      7,      2,      0x0072,  US_REG_WORD },
    { "pxp_b1d07f2_pcists",                                     1,      7,      2,      0x0006,  US_REG_WORD },
    { "pxp_b1d07f2_sltsts2",                                    1,      7,      2,      0x007a,  US_REG_WORD },
    { "pxp_b1d07f3_devsts",                                     1,      7,      3,      0x004a,  US_REG_WORD },
    { "pxp_b1d07f3_lnksts",                                     1,      7,      3,      0x0052,  US_REG_WORD },
    { "pxp_b1d07f3_lnksts2",                                    1,      7,      3,      0x0072,  US_REG_WORD },
    { "pxp_b1d07f3_pcists",                                     1,      7,      3,      0x0006,  US_REG_WORD },
    { "pxp_b1d07f3_sltsts2",                                    1,      7,      3,      0x007a,  US_REG_WORD },
    { "pxp_b1d07f4_devsts",                                     1,      7,      4,      0x004a,  US_REG_WORD },
    { "pxp_b1d07f4_lnksts",                                     1,      7,      4,      0x0052,  US_REG_WORD },
    { "pxp_b1d07f4_lnksts2",                                    1,      7,      4,      0x0072,  US_REG_WORD },
    { "pxp_b1d07f4_pcists",                                     1,      7,      4,      0x0006,  US_REG_WORD },
    { "pxp_b1d07f4_sltsts2",                                    1,      7,      4,      0x007a,  US_REG_WORD },
    { "pxp_b1d07f7_devsts",                                     1,      7,      7,      0x004a,  US_REG_WORD },
    { "pxp_b1d07f7_lnksts",                                     1,      7,      7,      0x0052,  US_REG_WORD },
    { "pxp_b1d07f7_lnksts2",                                    1,      7,      7,      0x0072,  US_REG_WORD },
    { "pxp_b1d07f7_pcists",                                     1,      7,      7,      0x0006,  US_REG_WORD },
    { "pxp_b1d07f7_sltsts2",                                    1,      7,      7,      0x007a,  US_REG_WORD },
    { "pxp_b1d07f7_tswdbgerrstdis0",                            1,      7,      7,      0x0358, US_REG_DWORD },
    { "pxp_b2d00f0_corerrsts",                                  2,      0,      0,      0x0158, US_REG_DWORD },
    { "pxp_b2d00f0_devsts",                                     2,      0,      0,      0x009a,  US_REG_WORD },
    { "pxp_b2d00f0_errsid",                                     2,      0,      0,      0x017c, US_REG_DWORD },
    { "pxp_b2d00f0_ler_ctrlsts",                                2,      0,      0,      0x0288, US_REG_DWORD },
    { "pxp_b2d00f0_lnerrsts",                                   2,      0,      0,      0x0258, US_REG_DWORD },
    { "pxp_b2d00f0_lnksts",                                     2,      0,      0,      0x00a2,  US_REG_WORD },
    { "pxp_b2d00f0_lnksts2",                                    2,      0,      0,      0x00c2,  US_REG_WORD },
    { "pxp_b2d00f0_miscctrlsts_0",                              2,      0,      0,      0x0188, US_REG_DWORD },
    { "pxp_b2d00f0_miscctrlsts_1",                              2,      0,      0,      0x018c, US_REG_DWORD },
    { "pxp_b2d00f0_pcists",                                     2,      0,      0,      0x0006,  US_REG_WORD },
    { "pxp_b2d00f0_rperrsts",                                   2,      0,      0,      0x0178, US_REG_DWORD },
    { "pxp_b2d00f0_rppioerr_cap",                               2,      0,      0,      0x0298, US_REG_DWORD },
    { "pxp_b2d00f0_rppioerr_status",                            2,      0,      0,      0x02a4, US_REG_DWORD },
    { "pxp_b2d00f0_secsts",                                     2,      0,      0,      0x001e,  US_REG_WORD },
    { "pxp_b2d00f0_sltsts",                                     2,      0,      0,      0x00aa,  US_REG_WORD },
    { "pxp_b2d00f0_uncerrsts",                                  2,      0,      0,      0x014c, US_REG_DWORD },

    { "xpcorerrsts",                                            2,      0,      0,      0x0200, US_REG_DWORD },
    { "xpcorerrmsk",                                            2,      0,      0,      0x0204, US_REG_DWORD },
    { "xpuncerrsts",                                            2,      0,      0,      0x0208, US_REG_DWORD },
    { "xpuncerrmsk",                                            2,      0,      0,      0x020c, US_REG_DWORD },
    { "xpuncerrsev",                                            2,      0,      0,      0x0210, US_REG_DWORD },
    { "xpuncerrptr",                                            2,      0,      0,      0x0214,  US_REG_BYTE },
    { "uncedmask",                                              2,      0,      0,      0x0218, US_REG_DWORD },
    { "coredmask",                                              2,      0,      0,      0x021c, US_REG_DWORD },
    { "rpedmask",                                               2,      0,      0,      0x0220, US_REG_DWORD },
    { "xpuncedmask",                                            2,      0,      0,      0x0224, US_REG_DWORD },
    { "xpcoredmask",                                            2,      0,      0,      0x0228, US_REG_DWORD },
    { "xpglberrsts",                                            2,      0,      0,      0x0230,  US_REG_WORD },
    { "xpglberrptr",                                            2,      0,      0,      0x0232,  US_REG_WORD },

    { "pbus",                                                   2,      0,      0,      0x0018,  US_REG_BYTE },
    { "secbus",                                                 2,      0,      0,      0x0019,  US_REG_BYTE },
    { "subbus",                                                 2,      0,      0,      0x001a,  US_REG_BYTE },
    { "iobas",                                                  2,      0,      0,      0x001c,  US_REG_BYTE },
    { "iolim",                                                  2,      0,      0,      0x001d,  US_REG_BYTE },
//    { "secsts",                                                 2,      0,      0,      0x001e,  US_REG_WORD },
    { "mbas",                                                   2,      0,      0,      0x0020,  US_REG_WORD },
    { "mlim",                                                   2,      0,      0,      0x0022,  US_REG_WORD },
    { "pbas",                                                   2,      0,      0,      0x0024,  US_REG_WORD },
    { "plim",                                                   2,      0,      0,      0x0026,  US_REG_WORD },
    { "pbasu",                                                  2,      0,      0,      0x0028, US_REG_DWORD },
    { "plimu",                                                  2,      0,      0,      0x002c, US_REG_DWORD },

//    { "pxp_b2d00f0_pbas",                                       2,      0,      0,      0x0024,  US_REG_WORD },
//    { "pxp_b2d00f0_plim",                                       2,      0,      0,      0x0026,  US_REG_WORD },
    { "pxp_b2d01f0_corerrsts",                                  2,      1,      0,      0x0158, US_REG_DWORD },
    { "pxp_b2d01f0_devsts",                                     2,      1,      0,      0x009a,  US_REG_WORD },
    { "pxp_b2d01f0_errsid",                                     2,      1,      0,      0x017c, US_REG_DWORD },
    { "pxp_b2d01f0_ler_ctrlsts",                                2,      1,      0,      0x0288, US_REG_DWORD },
    { "pxp_b2d01f0_lnerrsts",                                   2,      1,      0,      0x0258, US_REG_DWORD },
    { "pxp_b2d01f0_lnksts",                                     2,      1,      0,      0x00a2,  US_REG_WORD },
    { "pxp_b2d01f0_lnksts2",                                    2,      1,      0,      0x00c2,  US_REG_WORD },
    { "pxp_b2d01f0_miscctrlsts_0",                              2,      1,      0,      0x0188, US_REG_DWORD },
    { "pxp_b2d01f0_miscctrlsts_1",                              2,      1,      0,      0x018c, US_REG_DWORD },
    { "pxp_b2d01f0_pcists",                                     2,      1,      0,      0x0006,  US_REG_WORD },
    { "pxp_b2d01f0_rperrsts",                                   2,      1,      0,      0x0178, US_REG_DWORD },
    { "pxp_b2d01f0_rppioerr_cap",                               2,      1,      0,      0x0298, US_REG_DWORD },
    { "pxp_b2d01f0_rppioerr_status",                            2,      1,      0,      0x02a4, US_REG_DWORD },
    { "pxp_b2d01f0_secsts",                                     2,      1,      0,      0x001e,  US_REG_WORD },
    { "pxp_b2d01f0_sltsts",                                     2,      1,      0,      0x00aa,  US_REG_WORD },
    { "pxp_b2d01f0_uncerrsts",                                  2,      1,      0,      0x014c, US_REG_DWORD },

    { "xpcorerrsts",                                            2,      1,      0,      0x0200, US_REG_DWORD },
    { "xpcorerrmsk",                                            2,      1,      0,      0x0204, US_REG_DWORD },
    { "xpuncerrsts",                                            2,      1,      0,      0x0208, US_REG_DWORD },
    { "xpuncerrmsk",                                            2,      1,      0,      0x020c, US_REG_DWORD },
    { "xpuncerrsev",                                            2,      1,      0,      0x0210, US_REG_DWORD },
    { "xpuncerrptr",                                            2,      1,      0,      0x0214,  US_REG_BYTE },
    { "uncedmask",                                              2,      1,      0,      0x0218, US_REG_DWORD },
    { "coredmask",                                              2,      1,      0,      0x021c, US_REG_DWORD },
    { "rpedmask",                                               2,      1,      0,      0x0220, US_REG_DWORD },
    { "xpuncedmask",                                            2,      1,      0,      0x0224, US_REG_DWORD },
    { "xpcoredmask",                                            2,      1,      0,      0x0228, US_REG_DWORD },
    { "xpglberrsts",                                            2,      1,      0,      0x0230,  US_REG_WORD },
    { "xpglberrptr",                                            2,      1,      0,      0x0232,  US_REG_WORD },

    { "pbus",                                                   2,      1,      0,      0x0018,  US_REG_BYTE },
    { "secbus",                                                 2,      1,      0,      0x0019,  US_REG_BYTE },
    { "subbus",                                                 2,      1,      0,      0x001a,  US_REG_BYTE },
    { "iobas",                                                  2,      1,      0,      0x001c,  US_REG_BYTE },
    { "iolim",                                                  2,      1,      0,      0x001d,  US_REG_BYTE },
//    { "secsts",                                                 2,      1,      0,      0x001e,  US_REG_WORD },
    { "mbas",                                                   2,      1,      0,      0x0020,  US_REG_WORD },
    { "mlim",                                                   2,      1,      0,      0x0022,  US_REG_WORD },
    { "pbas",                                                   2,      1,      0,      0x0024,  US_REG_WORD },
    { "plim",                                                   2,      1,      0,      0x0026,  US_REG_WORD },
    { "pbasu",                                                  2,      1,      0,      0x0028, US_REG_DWORD },
    { "plimu",                                                  2,      1,      0,      0x002c, US_REG_DWORD },

    { "pxp_b2d02f0_corerrsts",                                  2,      2,      0,      0x0158, US_REG_DWORD },
    { "pxp_b2d02f0_devsts",                                     2,      2,      0,      0x009a,  US_REG_WORD },
    { "pxp_b2d02f0_errsid",                                     2,      2,      0,      0x017c, US_REG_DWORD },
    { "pxp_b2d02f0_ler_ctrlsts",                                2,      2,      0,      0x0288, US_REG_DWORD },
    { "pxp_b2d02f0_lnerrsts",                                   2,      2,      0,      0x0258, US_REG_DWORD },
    { "pxp_b2d02f0_lnksts",                                     2,      2,      0,      0x00a2,  US_REG_WORD },
    { "pxp_b2d02f0_lnksts2",                                    2,      2,      0,      0x00c2,  US_REG_WORD },
    { "pxp_b2d02f0_miscctrlsts_0",                              2,      2,      0,      0x0188, US_REG_DWORD },
    { "pxp_b2d02f0_miscctrlsts_1",                              2,      2,      0,      0x018c, US_REG_DWORD },
    { "pxp_b2d02f0_pcists",                                     2,      2,      0,      0x0006,  US_REG_WORD },
    { "pxp_b2d02f0_rperrsts",                                   2,      2,      0,      0x0178, US_REG_DWORD },
    { "pxp_b2d02f0_rppioerr_cap",                               2,      2,      0,      0x0298, US_REG_DWORD },
    { "pxp_b2d02f0_rppioerr_status",                            2,      2,      0,      0x02a4, US_REG_DWORD },
    { "pxp_b2d02f0_secsts",                                     2,      2,      0,      0x001e,  US_REG_WORD },
    { "pxp_b2d02f0_sltsts",                                     2,      2,      0,      0x00aa,  US_REG_WORD },
    { "pxp_b2d02f0_uncerrsts",                                  2,      2,      0,      0x014c, US_REG_DWORD },

    { "xpcorerrsts",                                            2,      2,      0,      0x0200, US_REG_DWORD },
    { "xpcorerrmsk",                                            2,      2,      0,      0x0204, US_REG_DWORD },
    { "xpuncerrsts",                                            2,      2,      0,      0x0208, US_REG_DWORD },
    { "xpuncerrmsk",                                            2,      2,      0,      0x020c, US_REG_DWORD },
    { "xpuncerrsev",                                            2,      2,      0,      0x0210, US_REG_DWORD },
    { "xpuncerrptr",                                            2,      2,      0,      0x0214,  US_REG_BYTE },
    { "uncedmask",                                              2,      2,      0,      0x0218, US_REG_DWORD },
    { "coredmask",                                              2,      2,      0,      0x021c, US_REG_DWORD },
    { "rpedmask",                                               2,      2,      0,      0x0220, US_REG_DWORD },
    { "xpuncedmask",                                            2,      2,      0,      0x0224, US_REG_DWORD },
    { "xpcoredmask",                                            2,      2,      0,      0x0228, US_REG_DWORD },
    { "xpglberrsts",                                            2,      2,      0,      0x0230,  US_REG_WORD },
    { "xpglberrptr",                                            2,      2,      0,      0x0232,  US_REG_WORD },

    { "pbus",                                                   2,      2,      0,      0x0018,  US_REG_BYTE },
    { "secbus",                                                 2,      2,      0,      0x0019,  US_REG_BYTE },
    { "subbus",                                                 2,      2,      0,      0x001a,  US_REG_BYTE },
    { "iobas",                                                  2,      2,      0,      0x001c,  US_REG_BYTE },
    { "iolim",                                                  2,      2,      0,      0x001d,  US_REG_BYTE },
//    { "secsts",                                                 2,      2,      0,      0x001e,  US_REG_WORD },
    { "mbas",                                                   2,      2,      0,      0x0020,  US_REG_WORD },
    { "mlim",                                                   2,      2,      0,      0x0022,  US_REG_WORD },
    { "pbas",                                                   2,      2,      0,      0x0024,  US_REG_WORD },
    { "plim",                                                   2,      2,      0,      0x0026,  US_REG_WORD },
    { "pbasu",                                                  2,      2,      0,      0x0028, US_REG_DWORD },
    { "plimu",                                                  2,      2,      0,      0x002c, US_REG_DWORD },

    { "pxp_b2d03f0_corerrsts",                                  2,      3,      0,      0x0158, US_REG_DWORD },
    { "pxp_b2d03f0_devsts",                                     2,      3,      0,      0x009a,  US_REG_WORD },
    { "pxp_b2d03f0_errsid",                                     2,      3,      0,      0x017c, US_REG_DWORD },
    { "pxp_b2d03f0_ler_ctrlsts",                                2,      3,      0,      0x0288, US_REG_DWORD },
    { "pxp_b2d03f0_lnerrsts",                                   2,      3,      0,      0x0258, US_REG_DWORD },
    { "pxp_b2d03f0_lnksts",                                     2,      3,      0,      0x00a2,  US_REG_WORD },
    { "pxp_b2d03f0_lnksts2",                                    2,      3,      0,      0x00c2,  US_REG_WORD },
    { "pxp_b2d03f0_miscctrlsts_0",                              2,      3,      0,      0x0188, US_REG_DWORD },
    { "pxp_b2d03f0_miscctrlsts_1",                              2,      3,      0,      0x018c, US_REG_DWORD },
    { "pxp_b2d03f0_pcists",                                     2,      3,      0,      0x0006,  US_REG_WORD },
    { "pxp_b2d03f0_rperrsts",                                   2,      3,      0,      0x0178, US_REG_DWORD },
    { "pxp_b2d03f0_rppioerr_cap",                               2,      3,      0,      0x0298, US_REG_DWORD },
    { "pxp_b2d03f0_rppioerr_status",                            2,      3,      0,      0x02a4, US_REG_DWORD },
    { "pxp_b2d03f0_secsts",                                     2,      3,      0,      0x001e,  US_REG_WORD },
    { "pxp_b2d03f0_sltsts",                                     2,      3,      0,      0x00aa,  US_REG_WORD },
    { "pxp_b2d03f0_uncerrsts",                                  2,      3,      0,      0x014c, US_REG_DWORD },

    { "xpcorerrsts",                                            2,      3,      0,      0x0200, US_REG_DWORD },
    { "xpcorerrmsk",                                            2,      3,      0,      0x0204, US_REG_DWORD },
    { "xpuncerrsts",                                            2,      3,      0,      0x0208, US_REG_DWORD },
    { "xpuncerrmsk",                                            2,      3,      0,      0x020c, US_REG_DWORD },
    { "xpuncerrsev",                                            2,      3,      0,      0x0210, US_REG_DWORD },
    { "xpuncerrptr",                                            2,      3,      0,      0x0214,  US_REG_BYTE },
    { "uncedmask",                                              2,      3,      0,      0x0218, US_REG_DWORD },
    { "coredmask",                                              2,      3,      0,      0x021c, US_REG_DWORD },
    { "rpedmask",                                               2,      3,      0,      0x0220, US_REG_DWORD },
    { "xpuncedmask",                                            2,      3,      0,      0x0224, US_REG_DWORD },
    { "xpcoredmask",                                            2,      3,      0,      0x0228, US_REG_DWORD },
    { "xpglberrsts",                                            2,      3,      0,      0x0230,  US_REG_WORD },
    { "xpglberrptr",                                            2,      3,      0,      0x0232,  US_REG_WORD },

    { "pbus",                                                   2,      3,      0,      0x0018,  US_REG_BYTE },
    { "secbus",                                                 2,      3,      0,      0x0019,  US_REG_BYTE },
    { "subbus",                                                 2,      3,      0,      0x001a,  US_REG_BYTE },
    { "iobas",                                                  2,      3,      0,      0x001c,  US_REG_BYTE },
    { "iolim",                                                  2,      3,      0,      0x001d,  US_REG_BYTE },
//    { "secsts",                                                 2,      3,      0,      0x001e,  US_REG_WORD },
    { "mbas",                                                   2,      3,      0,      0x0020,  US_REG_WORD },
    { "mlim",                                                   2,      3,      0,      0x0022,  US_REG_WORD },
    { "pbas",                                                   2,      3,      0,      0x0024,  US_REG_WORD },
    { "plim",                                                   2,      3,      0,      0x0026,  US_REG_WORD },
    { "pbasu",                                                  2,      3,      0,      0x0028, US_REG_DWORD },
    { "plimu",                                                  2,      3,      0,      0x002c, US_REG_DWORD },

    { "pxp_b2d07f0_devsts",                                     2,      7,      0,      0x004a,  US_REG_WORD },
    { "pxp_b2d07f0_lnksts",                                     2,      7,      0,      0x0052,  US_REG_WORD },
    { "pxp_b2d07f0_lnksts2",                                    2,      7,      0,      0x0072,  US_REG_WORD },
    { "pxp_b2d07f0_pcists",                                     2,      7,      0,      0x0006,  US_REG_WORD },
    { "pxp_b2d07f0_sltsts2",                                    2,      7,      0,      0x007a,  US_REG_WORD },
    { "pxp_b2d07f1_devsts",                                     2,      7,      1,      0x004a,  US_REG_WORD },
    { "pxp_b2d07f1_lnksts",                                     2,      7,      1,      0x0052,  US_REG_WORD },
    { "pxp_b2d07f1_lnksts2",                                    2,      7,      1,      0x0072,  US_REG_WORD },
    { "pxp_b2d07f1_pcists",                                     2,      7,      1,      0x0006,  US_REG_WORD },
    { "pxp_b2d07f1_sltsts2",                                    2,      7,      1,      0x007a,  US_REG_WORD },
    { "pxp_b2d07f2_devsts",                                     2,      7,      2,      0x004a,  US_REG_WORD },
    { "pxp_b2d07f2_lnksts",                                     2,      7,      2,      0x0052,  US_REG_WORD },
    { "pxp_b2d07f2_lnksts2",                                    2,      7,      2,      0x0072,  US_REG_WORD },
    { "pxp_b2d07f2_pcists",                                     2,      7,      2,      0x0006,  US_REG_WORD },
    { "pxp_b2d07f2_sltsts2",                                    2,      7,      2,      0x007a,  US_REG_WORD },
    { "pxp_b2d07f3_devsts",                                     2,      7,      3,      0x004a,  US_REG_WORD },
    { "pxp_b2d07f3_lnksts",                                     2,      7,      3,      0x0052,  US_REG_WORD },
    { "pxp_b2d07f3_lnksts2",                                    2,      7,      3,      0x0072,  US_REG_WORD },
    { "pxp_b2d07f3_pcists",                                     2,      7,      3,      0x0006,  US_REG_WORD },
    { "pxp_b2d07f3_sltsts2",                                    2,      7,      3,      0x007a,  US_REG_WORD },
    { "pxp_b2d07f4_devsts",                                     2,      7,      4,      0x004a,  US_REG_WORD },
    { "pxp_b2d07f4_lnksts",                                     2,      7,      4,      0x0052,  US_REG_WORD },
    { "pxp_b2d07f4_lnksts2",                                    2,      7,      4,      0x0072,  US_REG_WORD },
    { "pxp_b2d07f4_pcists",                                     2,      7,      4,      0x0006,  US_REG_WORD },
    { "pxp_b2d07f4_sltsts2",                                    2,      7,      4,      0x007a,  US_REG_WORD },
    { "pxp_b2d07f7_devsts",                                     2,      7,      7,      0x004a,  US_REG_WORD },
    { "pxp_b2d07f7_lnksts",                                     2,      7,      7,      0x0052,  US_REG_WORD },
    { "pxp_b2d07f7_lnksts2",                                    2,      7,      7,      0x0072,  US_REG_WORD },
    { "pxp_b2d07f7_pcists",                                     2,      7,      7,      0x0006,  US_REG_WORD },
    { "pxp_b2d07f7_sltsts2",                                    2,      7,      7,      0x007a,  US_REG_WORD },
    { "pxp_b2d07f7_tswdbgerrstdis0",                            2,      7,      7,      0x0358, US_REG_DWORD },
    { "pxp_b3d00f0_corerrsts",                                  3,      0,      0,      0x0158, US_REG_DWORD },
    { "pxp_b3d00f0_devsts",                                     3,      0,      0,      0x009a,  US_REG_WORD },
    { "pxp_b3d00f0_errsid",                                     3,      0,      0,      0x017c, US_REG_DWORD },
    { "pxp_b3d00f0_ler_ctrlsts",                                3,      0,      0,      0x0288, US_REG_DWORD },
    { "pxp_b3d00f0_lnerrsts",                                   3,      0,      0,      0x0258, US_REG_DWORD },
    { "pxp_b3d00f0_lnksts",                                     3,      0,      0,      0x00a2,  US_REG_WORD },
    { "pxp_b3d00f0_lnksts2",                                    3,      0,      0,      0x00c2,  US_REG_WORD },
    { "pxp_b3d00f0_miscctrlsts_0",                              3,      0,      0,      0x0188, US_REG_DWORD },
    { "pxp_b3d00f0_miscctrlsts_1",                              3,      0,      0,      0x018c, US_REG_DWORD },
    { "pxp_b3d00f0_pcists",                                     3,      0,      0,      0x0006,  US_REG_WORD },
    { "pxp_b3d00f0_rperrsts",                                   3,      0,      0,      0x0178, US_REG_DWORD },
    { "pxp_b3d00f0_rppioerr_cap",                               3,      0,      0,      0x0298, US_REG_DWORD },
    { "pxp_b3d00f0_rppioerr_status",                            3,      0,      0,      0x02a4, US_REG_DWORD },
    { "pxp_b3d00f0_secsts",                                     3,      0,      0,      0x001e,  US_REG_WORD },
    { "pxp_b3d00f0_sltsts",                                     3,      0,      0,      0x00aa,  US_REG_WORD },
    { "pxp_b3d00f0_uncerrsts",                                  3,      0,      0,      0x014c, US_REG_DWORD },

    { "xpcorerrsts",                                            3,      0,      0,      0x0200, US_REG_DWORD },
    { "xpcorerrmsk",                                            3,      0,      0,      0x0204, US_REG_DWORD },
    { "xpuncerrsts",                                            3,      0,      0,      0x0208, US_REG_DWORD },
    { "xpuncerrmsk",                                            3,      0,      0,      0x020c, US_REG_DWORD },
    { "xpuncerrsev",                                            3,      0,      0,      0x0210, US_REG_DWORD },
    { "xpuncerrptr",                                            3,      0,      0,      0x0214,  US_REG_BYTE },
    { "uncedmask",                                              3,      0,      0,      0x0218, US_REG_DWORD },
    { "coredmask",                                              3,      0,      0,      0x021c, US_REG_DWORD },
    { "rpedmask",                                               3,      0,      0,      0x0220, US_REG_DWORD },
    { "xpuncedmask",                                            3,      0,      0,      0x0224, US_REG_DWORD },
    { "xpcoredmask",                                            3,      0,      0,      0x0228, US_REG_DWORD },
    { "xpglberrsts",                                            3,      0,      0,      0x0230,  US_REG_WORD },
    { "xpglberrptr",                                            3,      0,      0,      0x0232,  US_REG_WORD },

    { "pbus",                                                   3,      0,      0,      0x0018,  US_REG_BYTE },
    { "secbus",                                                 3,      0,      0,      0x0019,  US_REG_BYTE },
    { "subbus",                                                 3,      0,      0,      0x001a,  US_REG_BYTE },
    { "iobas",                                                  3,      0,      0,      0x001c,  US_REG_BYTE },
    { "iolim",                                                  3,      0,      0,      0x001d,  US_REG_BYTE },
//    { "secsts",                                                 3,      0,      0,      0x001e,  US_REG_WORD },
    { "mbas",                                                   3,      0,      0,      0x0020,  US_REG_WORD },
    { "mlim",                                                   3,      0,      0,      0x0022,  US_REG_WORD },
    { "pbas",                                                   3,      0,      0,      0x0024,  US_REG_WORD },
    { "plim",                                                   3,      0,      0,      0x0026,  US_REG_WORD },
    { "pbasu",                                                  3,      0,      0,      0x0028, US_REG_DWORD },
    { "plimu",                                                  3,      0,      0,      0x002c, US_REG_DWORD },

//    { "pxp_b3d00f0_pbas",                                       3,      0,      0,      0x0024,  US_REG_WORD },
//    { "pxp_b3d00f0_plim",                                       3,      0,      0,      0x0026,  US_REG_WORD },
    { "pxp_b3d01f0_corerrsts",                                  3,      1,      0,      0x0158, US_REG_DWORD },
    { "pxp_b3d01f0_devsts",                                     3,      1,      0,      0x009a,  US_REG_WORD },
    { "pxp_b3d01f0_errsid",                                     3,      1,      0,      0x017c, US_REG_DWORD },
    { "pxp_b3d01f0_ler_ctrlsts",                                3,      1,      0,      0x0288, US_REG_DWORD },
    { "pxp_b3d01f0_lnerrsts",                                   3,      1,      0,      0x0258, US_REG_DWORD },
    { "pxp_b3d01f0_lnksts",                                     3,      1,      0,      0x00a2,  US_REG_WORD },
    { "pxp_b3d01f0_lnksts2",                                    3,      1,      0,      0x00c2,  US_REG_WORD },
    { "pxp_b3d01f0_miscctrlsts_0",                              3,      1,      0,      0x0188, US_REG_DWORD },
    { "pxp_b3d01f0_miscctrlsts_1",                              3,      1,      0,      0x018c, US_REG_DWORD },
    { "pxp_b3d01f0_pcists",                                     3,      1,      0,      0x0006,  US_REG_WORD },
    { "pxp_b3d01f0_rperrsts",                                   3,      1,      0,      0x0178, US_REG_DWORD },
    { "pxp_b3d01f0_rppioerr_cap",                               3,      1,      0,      0x0298, US_REG_DWORD },
    { "pxp_b3d01f0_rppioerr_status",                            3,      1,      0,      0x02a4, US_REG_DWORD },
    { "pxp_b3d01f0_secsts",                                     3,      1,      0,      0x001e,  US_REG_WORD },
    { "pxp_b3d01f0_sltsts",                                     3,      1,      0,      0x00aa,  US_REG_WORD },
    { "pxp_b3d01f0_uncerrsts",                                  3,      1,      0,      0x014c, US_REG_DWORD },

    { "xpcorerrsts",                                            3,      1,      0,      0x0200, US_REG_DWORD },
    { "xpcorerrmsk",                                            3,      1,      0,      0x0204, US_REG_DWORD },
    { "xpuncerrsts",                                            3,      1,      0,      0x0208, US_REG_DWORD },
    { "xpuncerrmsk",                                            3,      1,      0,      0x020c, US_REG_DWORD },
    { "xpuncerrsev",                                            3,      1,      0,      0x0210, US_REG_DWORD },
    { "xpuncerrptr",                                            3,      1,      0,      0x0214,  US_REG_BYTE },
    { "uncedmask",                                              3,      1,      0,      0x0218, US_REG_DWORD },
    { "coredmask",                                              3,      1,      0,      0x021c, US_REG_DWORD },
    { "rpedmask",                                               3,      1,      0,      0x0220, US_REG_DWORD },
    { "xpuncedmask",                                            3,      1,      0,      0x0224, US_REG_DWORD },
    { "xpcoredmask",                                            3,      1,      0,      0x0228, US_REG_DWORD },
    { "xpglberrsts",                                            3,      1,      0,      0x0230,  US_REG_WORD },
    { "xpglberrptr",                                            3,      1,      0,      0x0232,  US_REG_WORD },

    { "pbus",                                                   3,      1,      0,      0x0018,  US_REG_BYTE },
    { "secbus",                                                 3,      1,      0,      0x0019,  US_REG_BYTE },
    { "subbus",                                                 3,      1,      0,      0x001a,  US_REG_BYTE },
    { "iobas",                                                  3,      1,      0,      0x001c,  US_REG_BYTE },
    { "iolim",                                                  3,      1,      0,      0x001d,  US_REG_BYTE },
//    { "secsts",                                                 3,      1,      0,      0x001e,  US_REG_WORD },
    { "mbas",                                                   3,      1,      0,      0x0020,  US_REG_WORD },
    { "mlim",                                                   3,      1,      0,      0x0022,  US_REG_WORD },
    { "pbas",                                                   3,      1,      0,      0x0024,  US_REG_WORD },
    { "plim",                                                   3,      1,      0,      0x0026,  US_REG_WORD },
    { "pbasu",                                                  3,      1,      0,      0x0028, US_REG_DWORD },
    { "plimu",                                                  3,      1,      0,      0x002c, US_REG_DWORD },

    { "pxp_b3d02f0_corerrsts",                                  3,      2,      0,      0x0158, US_REG_DWORD },
    { "pxp_b3d02f0_devsts",                                     3,      2,      0,      0x009a,  US_REG_WORD },
    { "pxp_b3d02f0_errsid",                                     3,      2,      0,      0x017c, US_REG_DWORD },
    { "pxp_b3d02f0_ler_ctrlsts",                                3,      2,      0,      0x0288, US_REG_DWORD },
    { "pxp_b3d02f0_lnerrsts",                                   3,      2,      0,      0x0258, US_REG_DWORD },
    { "pxp_b3d02f0_lnksts",                                     3,      2,      0,      0x00a2,  US_REG_WORD },
    { "pxp_b3d02f0_lnksts2",                                    3,      2,      0,      0x00c2,  US_REG_WORD },
    { "pxp_b3d02f0_miscctrlsts_0",                              3,      2,      0,      0x0188, US_REG_DWORD },
    { "pxp_b3d02f0_miscctrlsts_1",                              3,      2,      0,      0x018c, US_REG_DWORD },
    { "pxp_b3d02f0_pcists",                                     3,      2,      0,      0x0006,  US_REG_WORD },
    { "pxp_b3d02f0_rperrsts",                                   3,      2,      0,      0x0178, US_REG_DWORD },
    { "pxp_b3d02f0_rppioerr_cap",                               3,      2,      0,      0x0298, US_REG_DWORD },
    { "pxp_b3d02f0_rppioerr_status",                            3,      2,      0,      0x02a4, US_REG_DWORD },
    { "pxp_b3d02f0_secsts",                                     3,      2,      0,      0x001e,  US_REG_WORD },
    { "pxp_b3d02f0_sltsts",                                     3,      2,      0,      0x00aa,  US_REG_WORD },
    { "pxp_b3d02f0_uncerrsts",                                  3,      2,      0,      0x014c, US_REG_DWORD },

    { "xpcorerrsts",                                            3,      2,      0,      0x0200, US_REG_DWORD },
    { "xpcorerrmsk",                                            3,      2,      0,      0x0204, US_REG_DWORD },
    { "xpuncerrsts",                                            3,      2,      0,      0x0208, US_REG_DWORD },
    { "xpuncerrmsk",                                            3,      2,      0,      0x020c, US_REG_DWORD },
    { "xpuncerrsev",                                            3,      2,      0,      0x0210, US_REG_DWORD },
    { "xpuncerrptr",                                            3,      2,      0,      0x0214,  US_REG_BYTE },
    { "uncedmask",                                              3,      2,      0,      0x0218, US_REG_DWORD },
    { "coredmask",                                              3,      2,      0,      0x021c, US_REG_DWORD },
    { "rpedmask",                                               3,      2,      0,      0x0220, US_REG_DWORD },
    { "xpuncedmask",                                            3,      2,      0,      0x0224, US_REG_DWORD },
    { "xpcoredmask",                                            3,      2,      0,      0x0228, US_REG_DWORD },
    { "xpglberrsts",                                            3,      2,      0,      0x0230,  US_REG_WORD },
    { "xpglberrptr",                                            3,      2,      0,      0x0232,  US_REG_WORD },

    { "pbus",                                                   3,      2,      0,      0x0018,  US_REG_BYTE },
    { "secbus",                                                 3,      2,      0,      0x0019,  US_REG_BYTE },
    { "subbus",                                                 3,      2,      0,      0x001a,  US_REG_BYTE },
    { "iobas",                                                  3,      2,      0,      0x001c,  US_REG_BYTE },
    { "iolim",                                                  3,      2,      0,      0x001d,  US_REG_BYTE },
//    { "secsts",                                                 3,      2,      0,      0x001e,  US_REG_WORD },
    { "mbas",                                                   3,      2,      0,      0x0020,  US_REG_WORD },
    { "mlim",                                                   3,      2,      0,      0x0022,  US_REG_WORD },
    { "pbas",                                                   3,      2,      0,      0x0024,  US_REG_WORD },
    { "plim",                                                   3,      2,      0,      0x0026,  US_REG_WORD },
    { "pbasu",                                                  3,      2,      0,      0x0028, US_REG_DWORD },
    { "plimu",                                                  3,      2,      0,      0x002c, US_REG_DWORD },

    { "pxp_b3d03f0_corerrsts",                                  3,      3,      0,      0x0158, US_REG_DWORD },
    { "pxp_b3d03f0_devsts",                                     3,      3,      0,      0x009a,  US_REG_WORD },
    { "pxp_b3d03f0_errsid",                                     3,      3,      0,      0x017c, US_REG_DWORD },
    { "pxp_b3d03f0_ler_ctrlsts",                                3,      3,      0,      0x0288, US_REG_DWORD },
    { "pxp_b3d03f0_lnerrsts",                                   3,      3,      0,      0x0258, US_REG_DWORD },
    { "pxp_b3d03f0_lnksts",                                     3,      3,      0,      0x00a2,  US_REG_WORD },
    { "pxp_b3d03f0_lnksts2",                                    3,      3,      0,      0x00c2,  US_REG_WORD },
    { "pxp_b3d03f0_miscctrlsts_0",                              3,      3,      0,      0x0188, US_REG_DWORD },
    { "pxp_b3d03f0_miscctrlsts_1",                              3,      3,      0,      0x018c, US_REG_DWORD },
    { "pxp_b3d03f0_pcists",                                     3,      3,      0,      0x0006,  US_REG_WORD },
    { "pxp_b3d03f0_rperrsts",                                   3,      3,      0,      0x0178, US_REG_DWORD },
    { "pxp_b3d03f0_rppioerr_cap",                               3,      3,      0,      0x0298, US_REG_DWORD },
    { "pxp_b3d03f0_rppioerr_status",                            3,      3,      0,      0x02a4, US_REG_DWORD },
    { "pxp_b3d03f0_secsts",                                     3,      3,      0,      0x001e,  US_REG_WORD },
    { "pxp_b3d03f0_sltsts",                                     3,      3,      0,      0x00aa,  US_REG_WORD },
    { "pxp_b3d03f0_uncerrsts",                                  3,      3,      0,      0x014c, US_REG_DWORD },

    { "xpcorerrsts",                                            3,      3,      0,      0x0200, US_REG_DWORD },
    { "xpcorerrmsk",                                            3,      3,      0,      0x0204, US_REG_DWORD },
    { "xpuncerrsts",                                            3,      3,      0,      0x0208, US_REG_DWORD },
    { "xpuncerrmsk",                                            3,      3,      0,      0x020c, US_REG_DWORD },
    { "xpuncerrsev",                                            3,      3,      0,      0x0210, US_REG_DWORD },
    { "xpuncerrptr",                                            3,      3,      0,      0x0214,  US_REG_BYTE },
    { "uncedmask",                                              3,      3,      0,      0x0218, US_REG_DWORD },
    { "coredmask",                                              3,      3,      0,      0x021c, US_REG_DWORD },
    { "rpedmask",                                               3,      3,      0,      0x0220, US_REG_DWORD },
    { "xpuncedmask",                                            3,      3,      0,      0x0224, US_REG_DWORD },
    { "xpcoredmask",                                            3,      3,      0,      0x0228, US_REG_DWORD },
    { "xpglberrsts",                                            3,      3,      0,      0x0230,  US_REG_WORD },
    { "xpglberrptr",                                            3,      3,      0,      0x0232,  US_REG_WORD },

    { "pbus",                                                   3,      3,      0,      0x0018,  US_REG_BYTE },
    { "secbus",                                                 3,      3,      0,      0x0019,  US_REG_BYTE },
    { "subbus",                                                 3,      3,      0,      0x001a,  US_REG_BYTE },
    { "iobas",                                                  3,      3,      0,      0x001c,  US_REG_BYTE },
    { "iolim",                                                  3,      3,      0,      0x001d,  US_REG_BYTE },
//    { "secsts",                                                 3,      3,      0,      0x001e,  US_REG_WORD },
    { "mbas",                                                   3,      3,      0,      0x0020,  US_REG_WORD },
    { "mlim",                                                   3,      3,      0,      0x0022,  US_REG_WORD },
    { "pbas",                                                   3,      3,      0,      0x0024,  US_REG_WORD },
    { "plim",                                                   3,      3,      0,      0x0026,  US_REG_WORD },
    { "pbasu",                                                  3,      3,      0,      0x0028, US_REG_DWORD },
    { "plimu",                                                  3,      3,      0,      0x002c, US_REG_DWORD },

    { "pxp_b3d07f0_devsts",                                     3,      7,      0,      0x004a,  US_REG_WORD },
    { "pxp_b3d07f0_lnksts",                                     3,      7,      0,      0x0052,  US_REG_WORD },
    { "pxp_b3d07f0_lnksts2",                                    3,      7,      0,      0x0072,  US_REG_WORD },
    { "pxp_b3d07f0_pcists",                                     3,      7,      0,      0x0006,  US_REG_WORD },
    { "pxp_b3d07f0_sltsts2",                                    3,      7,      0,      0x007a,  US_REG_WORD },
    { "pxp_b3d07f1_devsts",                                     3,      7,      1,      0x004a,  US_REG_WORD },
    { "pxp_b3d07f1_lnksts",                                     3,      7,      1,      0x0052,  US_REG_WORD },
    { "pxp_b3d07f1_lnksts2",                                    3,      7,      1,      0x0072,  US_REG_WORD },
    { "pxp_b3d07f1_pcists",                                     3,      7,      1,      0x0006,  US_REG_WORD },
    { "pxp_b3d07f1_sltsts2",                                    3,      7,      1,      0x007a,  US_REG_WORD },
    { "pxp_b3d07f2_devsts",                                     3,      7,      2,      0x004a,  US_REG_WORD },
    { "pxp_b3d07f2_lnksts",                                     3,      7,      2,      0x0052,  US_REG_WORD },
    { "pxp_b3d07f2_lnksts2",                                    3,      7,      2,      0x0072,  US_REG_WORD },
    { "pxp_b3d07f2_pcists",                                     3,      7,      2,      0x0006,  US_REG_WORD },
    { "pxp_b3d07f2_sltsts2",                                    3,      7,      2,      0x007a,  US_REG_WORD },
    { "pxp_b3d07f3_devsts",                                     3,      7,      3,      0x004a,  US_REG_WORD },
    { "pxp_b3d07f3_lnksts",                                     3,      7,      3,      0x0052,  US_REG_WORD },
    { "pxp_b3d07f3_lnksts2",                                    3,      7,      3,      0x0072,  US_REG_WORD },
    { "pxp_b3d07f3_pcists",                                     3,      7,      3,      0x0006,  US_REG_WORD },
    { "pxp_b3d07f3_sltsts2",                                    3,      7,      3,      0x007a,  US_REG_WORD },
    { "pxp_b3d07f4_devsts",                                     3,      7,      4,      0x004a,  US_REG_WORD },
    { "pxp_b3d07f4_lnksts",                                     3,      7,      4,      0x0052,  US_REG_WORD },
    { "pxp_b3d07f4_lnksts2",                                    3,      7,      4,      0x0072,  US_REG_WORD },
    { "pxp_b3d07f4_pcists",                                     3,      7,      4,      0x0006,  US_REG_WORD },
    { "pxp_b3d07f4_sltsts2",                                    3,      7,      4,      0x007a,  US_REG_WORD },
    { "pxp_b3d07f7_devsts",                                     3,      7,      7,      0x004a,  US_REG_WORD },
    { "pxp_b3d07f7_lnksts",                                     3,      7,      7,      0x0052,  US_REG_WORD },
    { "pxp_b3d07f7_lnksts2",                                    3,      7,      7,      0x0072,  US_REG_WORD },
    { "pxp_b3d07f7_pcists",                                     3,      7,      7,      0x0006,  US_REG_WORD },
    { "pxp_b3d07f7_sltsts2",                                    3,      7,      7,      0x007a,  US_REG_WORD },
    { "pxp_b3d07f7_tswdbgerrstdis0",                            3,      7,      7,      0x0358, US_REG_DWORD },
    { "pxp_b4d07f7_tswdbgerrstdis0",                            4,      7,      7,      0x0358, US_REG_DWORD },
//    { "pxp_dmi_corerrsts",                                      0,      0,      0,      0x0158, US_REG_DWORD },
    { "pxp_dmi_devsts",                                         0,      0,      0,      0x00f2,  US_REG_WORD },
//    { "pxp_dmi_errsid",                                         0,      0,      0,      0x017c, US_REG_DWORD },
//    { "pxp_dmi_ler_ctrlsts",                                    0,      0,      0,      0x0288, US_REG_DWORD },
//    { "pxp_dmi_lnerrsts",                                       0,      0,      0,      0x0258, US_REG_DWORD },
    { "pxp_dmi_lnksts",                                         0,      0,      0,      0x01b2,  US_REG_WORD },
    { "pxp_dmi_lnksts2",                                        0,      0,      0,      0x01c2,  US_REG_WORD },
//    { "pxp_dmi_miscctrlsts_0",                                  0,      0,      0,      0x0188, US_REG_DWORD },
//    { "pxp_dmi_miscctrlsts_1",                                  0,      0,      0,      0x018c, US_REG_DWORD },
//    { "pxp_dmi_pcists",                                         0,      0,      0,      0x0006,  US_REG_WORD },
//    { "pxp_dmi_rperrsts",                                       0,      0,      0,      0x0178, US_REG_DWORD },
//    { "pxp_dmi_rppioerr_cap",                                   0,      0,      0,      0x0298, US_REG_DWORD },
//    { "pxp_dmi_rppioerr_status",                                0,      0,      0,      0x02a4, US_REG_DWORD },
//    { "pxp_dmi_sltsts",                                         0,      0,      0,      0x00aa,  US_REG_WORD },
//    { "pxp_dmi_uncerrsts",                                      0,      0,      0,      0x014c, US_REG_DWORD },
//    { "pxp_dmi_xpcorerrsts",                                    0,      0,      0,      0x0200, US_REG_DWORD },
//    { "pxp_dmi_xpglberrptr",                                    0,      0,      0,      0x0232,  US_REG_WORD },
//    { "pxp_dmi_xpglberrsts",                                    0,      0,      0,      0x0230,  US_REG_WORD },
//    { "pxp_dmi_xpuncerrsts",                                    0,      0,      0,      0x0208, US_REG_DWORD },
    { "pcu_cr_mca_err_src_log",                                 1,     30,      2,      0x00ec, US_REG_DWORD },
    { "pcu_cr_capid6_cfg",                                      1,     30,      3,      0x009c, US_REG_DWORD },
    { "pcu_cr_smb_status_cfg_0",                                1,     30,      5,      0x00a8, US_REG_DWORD },
    { "pcu_cr_smb_status_cfg_1",                                1,     30,      5,      0x00ac, US_REG_DWORD },
    { "pcu_cr_smb_status_cfg_2",                                1,     30,      5,      0x00b0, US_REG_DWORD },
    { "ubox_ncevents_emca_core_csmi_log_cfg_b0d08f0",           0,      8,      0,      0x00b0, US_REG_DWORD },
    { "ubox_ncevents_emca_core_msmi_log_cfg_b0d08f0",           0,      8,      0,      0x00b8, US_REG_DWORD },
    { "ubox_ncevents_emca_en_core_ierr_to_msmi_cfg_b0d08f0",    0,      8,      0,      0x00d8, US_REG_DWORD },
    { "ubox_ncevents_ierrloggingreg_b0d08f0",                   0,      8,      0,      0x00a4, US_REG_DWORD },
    { "ubox_ncevents_iio_errpinctl_cfg_b0d08f0",                0,      8,      0,      0x020c, US_REG_DWORD },
    { "ubox_ncevents_iio_errpindat_cfg_b0d08f0",                0,      8,      0,      0x0214, US_REG_DWORD },
    { "ubox_ncevents_iio_errpinsts_cfg_b0d08f0",                0,      8,      0,      0x0210, US_REG_DWORD },
    { "ubox_ncevents_iio_gc_1st_errst_cfg_b0d08f0",             0,      8,      0,      0x0234, US_REG_DWORD },
    { "ubox_ncevents_iio_gc_errst_cfg_b0d08f0",                 0,      8,      0,      0x0230, US_REG_DWORD },
    { "ubox_ncevents_iio_gc_nxt_errst_cfg_b0d08f0",             0,      8,      0,      0x0238, US_REG_DWORD },
    { "ubox_ncevents_iio_gf_1st_errst_cfg_b0d08f0",             0,      8,      0,      0x021c, US_REG_DWORD },
    { "ubox_ncevents_iio_gf_errst_cfg_b0d08f0",                 0,      8,      0,      0x0218, US_REG_DWORD },
    { "ubox_ncevents_iio_gf_nxt_errst_cfg_b0d08f0",             0,      8,      0,      0x0220, US_REG_DWORD },
    { "ubox_ncevents_iio_gnf_1st_errst_cfg_b0d08f0",            0,      8,      0,      0x0228, US_REG_DWORD },
    { "ubox_ncevents_iio_gnf_errst_cfg_b0d08f0",                0,      8,      0,      0x0224, US_REG_DWORD },
    { "ubox_ncevents_iio_gnf_nxt_errst_cfg_b0d08f0",            0,      8,      0,      0x022C, US_REG_DWORD },
    { "ubox_ncevents_iio_gsysctl_cfg_b0d08f0",                  0,      8,      0,      0x0208, US_REG_DWORD },
    { "ubox_ncevents_iio_gsysst_cfg_b0d08f0",                   0,      8,      0,      0x0204, US_REG_DWORD },
    { "ubox_ncevents_mcerrloggingreg_b0d08f0",                  0,      8,      0,      0x00a8, US_REG_DWORD },
    { "ubox_ncevents_ubox_cts_err_mask_cfg_b0d08f0",            0,      8,      0,      0x0094, US_REG_DWORD },
    { "ubox_ncevents_ubox_gl_err_cfg_cfg_b0d08f0",              0,      8,      0,      0x0090, US_REG_DWORD },
    { "ubox_ncevents_uboxerrctl_cfg_b0d08f0",                   0,      8,      0,      0x00e8, US_REG_DWORD },
    { "ubox_ncevents_uboxerrmisc_cfg_b0d08f0",                  0,      8,      0,      0x00ec, US_REG_DWORD },
    { "ubox_ncevents_uboxerrmisc2_cfg_b0d08f0",                 0,      8,      0,      0x00f0, US_REG_DWORD },
    { "ubox_ncevents_uboxerrsts_cfg_b0d08f0",                   0,      8,      0,      0x00e4, US_REG_DWORD },
    { "biosnonstickyscratchpad12",                              0,      8,      2,      0x0090, US_REG_DWORD },
    { "iio_irpp0errcnt_b0d05f2",                                0,      5,      2,      0x026c, US_REG_DWORD },
    { "iio_irpp0errcnt_b1d05f2",                                1,      5,      2,      0x026c, US_REG_DWORD },
    { "iio_irpp0errcnt_b2d05f2",                                2,      5,      2,      0x026c, US_REG_DWORD },
    { "iio_irpp0errcnt_b3d05f2",                                3,      5,      2,      0x026c, US_REG_DWORD },
    { "iio_irpp0errcnt_b4d05f2",                                4,      5,      2,      0x026c, US_REG_DWORD },
    { "iio_irpp0errst_b0d05f2",                                 0,      5,      2,      0x023c, US_REG_DWORD },
    { "iio_irpp0errst_b1d05f2",                                 1,      5,      2,      0x023c, US_REG_DWORD },
    { "iio_irpp0errst_b2d05f2",                                 2,      5,      2,      0x023c, US_REG_DWORD },
    { "iio_irpp0errst_b3d05f2",                                 3,      5,      2,      0x023c, US_REG_DWORD },
    { "iio_irpp0errst_b4d05f2",                                 4,      5,      2,      0x023c, US_REG_DWORD },
    { "iio_irpp0fferrst_b0d05f2",                               0,      5,      2,      0x0238, US_REG_DWORD },
    { "iio_irpp0fferrst_b1d05f2",                               1,      5,      2,      0x0238, US_REG_DWORD },
    { "iio_irpp0fferrst_b2d05f2",                               2,      5,      2,      0x0238, US_REG_DWORD },
    { "iio_irpp0fferrst_b3d05f2",                               3,      5,      2,      0x0238, US_REG_DWORD },
    { "iio_irpp0fferrst_b4d05f2",                               4,      5,      2,      0x0238, US_REG_DWORD },
    { "iio_irpp0fnerrst_b0d05f2",                               0,      5,      2,      0x0230, US_REG_DWORD },
    { "iio_irpp0fnerrst_b1d05f2",                               1,      5,      2,      0x0230, US_REG_DWORD },
    { "iio_irpp0fnerrst_b2d05f2",                               2,      5,      2,      0x0230, US_REG_DWORD },
    { "iio_irpp0fnerrst_b3d05f2",                               3,      5,      2,      0x0230, US_REG_DWORD },
    { "iio_irpringerrst_b1d05f2",                               1,      5,      2,      0x0208, US_REG_DWORD },
    { "iio_irpringerrst_b2d05f2",                               2,      5,      2,      0x0208, US_REG_DWORD },
    { "iio_irpringerrst_b3d05f2",                               3,      5,      2,      0x0208, US_REG_DWORD },
    { "iio_irpringfferrst_b1d05f2",                             1,      5,      2,      0x0210, US_REG_DWORD },
    { "iio_irpringfferrst_b2d05f2",                             2,      5,      2,      0x0210, US_REG_DWORD },
    { "iio_irpringfferrst_b3d05f2",                             3,      5,      2,      0x0210, US_REG_DWORD },
    { "iio_irpringfnerrst_b1d05f2",                             1,      5,      2,      0x0214, US_REG_DWORD },
    { "iio_irpringfnerrst_b2d05f2",                             2,      5,      2,      0x0214, US_REG_DWORD },
    { "iio_irpringfnerrst_b3d05f2",                             3,      5,      2,      0x0214, US_REG_DWORD },
    { "iio_irpringnferrst_b1d05f2",                             1,      5,      2,      0x0218, US_REG_DWORD },
    { "iio_irpringnferrst_b2d05f2",                             2,      5,      2,      0x0218, US_REG_DWORD },
    { "iio_irpringnferrst_b3d05f2",                             3,      5,      2,      0x0218, US_REG_DWORD },
    { "iio_itcfferrst_b0d05f2",                                 0,      5,      2,      0x02A0, US_REG_DWORD },
    { "iio_itcfferrst_b1d05f2",                                 1,      5,      2,      0x02A0, US_REG_DWORD },
    { "iio_itcfferrst_b2d05f2",                                 2,      5,      2,      0x02A0, US_REG_DWORD },
    { "iio_itcfferrst_b3d05f2",                                 3,      5,      2,      0x02A0, US_REG_DWORD },
    { "iio_itcfferrst_b4d05f2",                                 4,      5,      2,      0x02A0, US_REG_DWORD },
    { "iio_itcnnerrst_b0d05f2",                                 0,      5,      2,      0x02cc, US_REG_DWORD },
    { "iio_itcnnerrst_b1d05f2",                                 1,      5,      2,      0x02cc, US_REG_DWORD },
    { "iio_itcnnerrst_b2d05f2",                                 2,      5,      2,      0x02cc, US_REG_DWORD },
    { "iio_itcnnerrst_b3d05f2",                                 3,      5,      2,      0x02cc, US_REG_DWORD },
    { "iio_itcnnerrst_b4d05f2",                                 4,      5,      2,      0x02cc, US_REG_DWORD },
    { "iio_lcferrst_b0d05f2",                                   0,      5,      2,      0x01ac, US_REG_DWORD },
    { "iio_lcferrst_b1d05f2",                                   1,      5,      2,      0x01ac, US_REG_DWORD },
    { "iio_lcferrst_b2d05f2",                                   2,      5,      2,      0x01ac, US_REG_DWORD },
    { "iio_lcferrst_b3d05f2",                                   3,      5,      2,      0x01ac, US_REG_DWORD },
    { "iio_lferrst_b0d05f2",                                    0,      5,      2,      0x01c4, US_REG_DWORD },
    { "iio_lferrst_b1d05f2",                                    1,      5,      2,      0x01c4, US_REG_DWORD },
    { "iio_lferrst_b2d05f2",                                    2,      5,      2,      0x01c4, US_REG_DWORD },
    { "iio_lferrst_b3d05f2",                                    3,      5,      2,      0x01c4, US_REG_DWORD },
    { "iio_lfferrst_b0d05f2",                                   0,      5,      2,      0x01dc, US_REG_DWORD },
    { "iio_lfferrst_b1d05f2",                                   1,      5,      2,      0x01dc, US_REG_DWORD },
    { "iio_lfferrst_b2d05f2",                                   2,      5,      2,      0x01dc, US_REG_DWORD },
    { "iio_lfferrst_b3d05f2",                                   3,      5,      2,      0x01dc, US_REG_DWORD },
    { "iio_lnferrst_b0d05f2",                                   0,      5,      2,      0x01ec, US_REG_DWORD },
    { "iio_lnferrst_b1d05f2",                                   1,      5,      2,      0x01ec, US_REG_DWORD },
    { "iio_lnferrst_b2d05f2",                                   2,      5,      2,      0x01ec, US_REG_DWORD },
    { "iio_lnferrst_b3d05f2",                                   3,      5,      2,      0x01ec, US_REG_DWORD },
    { "iio_otcfferrst_b0d05f2",                                 0,      5,      2,      0x02D0, US_REG_DWORD },
    { "iio_otcfferrst_b1d05f2",                                 1,      5,      2,      0x02D0, US_REG_DWORD },
    { "iio_otcfferrst_b2d05f2",                                 2,      5,      2,      0x02D0, US_REG_DWORD },
    { "iio_otcfferrst_b3d05f2",                                 3,      5,      2,      0x02D0, US_REG_DWORD },
    { "iio_otcfferrst_b4d05f2",                                 4,      5,      2,      0x02D0, US_REG_DWORD },
    { "iio_tcerrst_b0d05f2",                                    0,      5,      2,      0x0288, US_REG_DWORD },
    { "iio_tcerrst_b1d05f2",                                    1,      5,      2,      0x0288, US_REG_DWORD },
    { "iio_tcerrst_b2d05f2",                                    2,      5,      2,      0x0288, US_REG_DWORD },
    { "iio_tcerrst_b3d05f2",                                    3,      5,      2,      0x0288, US_REG_DWORD },
    { "iio_tcerrst_b4d05f2",                                    4,      5,      2,      0x0288, US_REG_DWORD },
    { "iio_vtuncerrptr_b0d05f0",                                0,      5,      0,      0x01b4, US_REG_DWORD },
    { "iio_vtuncerrptr_b1d05f0",                                1,      5,      0,      0x01b4, US_REG_DWORD },
    { "iio_vtuncerrptr_b2d05f0",                                2,      5,      0,      0x01b4, US_REG_DWORD },
    { "iio_vtuncerrptr_b3d05f0",                                3,      5,      0,      0x01b4, US_REG_DWORD },
    { "iio_vtuncerrsts_b0d05f0",                                0,      5,      0,      0x01a8, US_REG_DWORD },
    { "iio_vtuncerrsts_b1d05f0",                                1,      5,      0,      0x01a8, US_REG_DWORD },
    { "iio_vtuncerrsts_b2d05f0",                                2,      5,      0,      0x01a8, US_REG_DWORD },
    { "iio_vtuncerrsts_b3d05f0",                                3,      5,      0,      0x01a8, US_REG_DWORD },
    { "kti0_bios_err_ad",                                       3,     14,      0,      0x00b8, US_REG_DWORD },
    { "kti0_bios_err_misc",                                     3,     14,      0,      0x00b0, US_REG_DWORD },
    { "kti0_bios_err_st",                                       3,     14,      0,      0x00a8, US_REG_DWORD },
    { "kti0_crcerrcnt",                                         3,     14,      0,      0x00c0, US_REG_DWORD },
    { "kti0_dbgerrst0",                                         3,     14,      2,      0x03a0, US_REG_DWORD },
    { "kti0_dbgerrst1",                                         3,     14,      2,      0x03a8, US_REG_DWORD },
    { "kti0_dbgerrstdis0",                                      3,     14,      2,      0x03a4, US_REG_DWORD },
    { "kti0_dbgerrstdis1",                                      3,     14,      2,      0x03ac, US_REG_DWORD },
    { "kti0_dfx_lck_ctl_cfg",                                   3,     14,      2,      0x0800, US_REG_DWORD },
    { "kti0_errcnt0_cntr",                                      3,     14,      0,      0x00d4, US_REG_DWORD },
    { "kti0_errcnt1_cntr",                                      3,     14,      0,      0x00dc, US_REG_DWORD },
    { "kti0_errcnt2_cntr",                                      3,     14,      0,      0x00e4, US_REG_DWORD },
    { "kti0_lcl",                                               3,     14,      0,      0x0084, US_REG_DWORD },
    { "kti0_les",                                               3,     14,      0,      0x0090, US_REG_DWORD },
    { "kti0_ls",                                                3,     14,      0,      0x008c, US_REG_DWORD },
    { "kti0_ph_rxl0c_err_log0",                                 3,     14,      2,      0x09f4, US_REG_DWORD },
    { "kti0_ph_rxl0c_err_log1",                                 3,     14,      2,      0x09f8, US_REG_DWORD },
    { "kti0_ph_rxl0c_err_sts",                                  3,     14,      2,      0x09f0, US_REG_DWORD },
    { "kti0_pq_csr_mcsr0",                                      3,     14,      2,      0x0820, US_REG_DWORD },
    { "kti0_pq_csr_mcsr1",                                      3,     14,      2,      0x0824, US_REG_DWORD },
    { "kti0_pq_csr_mcsr3",                                      3,     14,      2,      0x082c, US_REG_DWORD },
    { "kti0_reut_ph_cls",                                       3,     14,      1,      0x0160, US_REG_DWORD },
    { "kti0_reut_ph_ctr1",                                      3,     14,      1,      0x012c, US_REG_DWORD },
    { "kti0_reut_ph_ctr2",                                      3,     14,      1,      0x0130, US_REG_DWORD },
    { "kti0_reut_ph_das",                                       3,     14,      1,      0x0170, US_REG_DWORD },
    { "kti0_reut_ph_kes",                                       3,     14,      1,      0x016c, US_REG_DWORD },
    { "kti0_reut_ph_ldc",                                       3,     14,      1,      0x0178, US_REG_DWORD },
    { "kti0_reut_ph_pis",                                       3,     14,      1,      0x014c, US_REG_DWORD },
    { "kti0_reut_ph_pls",                                       3,     14,      1,      0x0164, US_REG_DWORD },
    { "kti0_reut_ph_pss",                                       3,     14,      1,      0x0154, US_REG_DWORD },
    { "kti0_reut_ph_rds",                                       3,     14,      1,      0x0140, US_REG_DWORD },
    { "kti0_reut_ph_tds",                                       3,     14,      1,      0x013c, US_REG_DWORD },
    { "kti0_reut_ph_ttlecr",                                    3,     14,      1,      0x01b4, US_REG_DWORD },
    { "kti0_reut_ph_ttres",                                     3,     14,      1,      0x01c0, US_REG_DWORD },
    { "kti0_reut_ph_wes",                                       3,     14,      1,      0x0168, US_REG_DWORD },

    { "kti0_viral",                                             3,     14,      0,      0x009c, US_REG_DWORD },
    { "kti0_csmithres",                                         3,     14,      0,      0x00a0, US_REG_DWORD },
    { "kti0_cerrlogctrl",                                       3,     14,      0,      0x00a4, US_REG_DWORD },
    { "kti0_errdis0",                                           3,     14,      0,      0x00c4, US_REG_DWORD },
    { "kti0_errdis1",                                           3,     14,      0,      0x00c8, US_REG_DWORD },

    { "kti1_bios_err_ad",                                       3,     15,      0,      0x00b8, US_REG_DWORD },
    { "kti1_bios_err_misc",                                     3,     15,      0,      0x00b0, US_REG_DWORD },
    { "kti1_bios_err_st",                                       3,     15,      0,      0x00a8, US_REG_DWORD },
    { "kti1_crcerrcnt",                                         3,     15,      0,      0x00c0, US_REG_DWORD },
    { "kti1_dbgerrst0",                                         3,     15,      2,      0x03a0, US_REG_DWORD },
    { "kti1_dbgerrst1",                                         3,     15,      2,      0x03a8, US_REG_DWORD },
    { "kti1_dbgerrstdis0",                                      3,     15,      2,      0x03a4, US_REG_DWORD },
    { "kti1_dbgerrstdis1",                                      3,     15,      2,      0x03ac, US_REG_DWORD },
    { "kti1_dfx_lck_ctl_cfg",                                   3,     15,      2,      0x0800, US_REG_DWORD },
    { "kti1_errcnt0_cntr",                                      3,     15,      0,      0x00d4, US_REG_DWORD },
    { "kti1_errcnt1_cntr",                                      3,     15,      0,      0x00dc, US_REG_DWORD },
    { "kti1_errcnt2_cntr",                                      3,     15,      0,      0x00e4, US_REG_DWORD },
    { "kti1_lcl",                                               3,     15,      0,      0x0084, US_REG_DWORD },
    { "kti1_les",                                               3,     15,      0,      0x0090, US_REG_DWORD },
    { "kti1_ls",                                                3,     15,      0,      0x008c, US_REG_DWORD },
    { "kti1_ph_rxl0c_err_log0",                                 3,     15,      2,      0x09f4, US_REG_DWORD },
    { "kti1_ph_rxl0c_err_log1",                                 3,     15,      2,      0x09f8, US_REG_DWORD },
    { "kti1_ph_rxl0c_err_sts",                                  3,     15,      2,      0x09f0, US_REG_DWORD },
    { "kti1_pq_csr_mcsr0",                                      3,     15,      2,      0x0820, US_REG_DWORD },
    { "kti1_pq_csr_mcsr1",                                      3,     15,      2,      0x0824, US_REG_DWORD },
    { "kti1_pq_csr_mcsr3",                                      3,     15,      2,      0x082c, US_REG_DWORD },
    { "kti1_reut_ph_cls",                                       3,     15,      1,      0x0160, US_REG_DWORD },
    { "kti1_reut_ph_ctr1",                                      3,     15,      1,      0x012c, US_REG_DWORD },
    { "kti1_reut_ph_ctr2",                                      3,     15,      1,      0x0130, US_REG_DWORD },
    { "kti1_reut_ph_das",                                       3,     15,      1,      0x0170, US_REG_DWORD },
    { "kti1_reut_ph_kes",                                       3,     15,      1,      0x016c, US_REG_DWORD },
    { "kti1_reut_ph_ldc",                                       3,     15,      1,      0x0178, US_REG_DWORD },
    { "kti1_reut_ph_pis",                                       3,     15,      1,      0x014c, US_REG_DWORD },
    { "kti1_reut_ph_pls",                                       3,     15,      1,      0x0164, US_REG_DWORD },
    { "kti1_reut_ph_pss",                                       3,     15,      1,      0x0154, US_REG_DWORD },
    { "kti1_reut_ph_rds",                                       3,     15,      1,      0x0140, US_REG_DWORD },
    { "kti1_reut_ph_tds",                                       3,     15,      1,      0x013c, US_REG_DWORD },
    { "kti1_reut_ph_ttlecr",                                    3,     15,      1,      0x01b4, US_REG_DWORD },
    { "kti1_reut_ph_ttres",                                     3,     15,      1,      0x01c0, US_REG_DWORD },
    { "kti1_reut_ph_wes",                                       3,     15,      1,      0x0168, US_REG_DWORD },

    { "kti1_viral",                                             3,     15,      0,      0x009c, US_REG_DWORD },
    { "kti1_csmithres",                                         3,     15,      0,      0x00a0, US_REG_DWORD },
    { "kti1_cerrlogctrl",                                       3,     15,      0,      0x00a4, US_REG_DWORD },
    { "kti1_errdis0",                                           3,     15,      0,      0x00c4, US_REG_DWORD },
    { "kti1_errdis1",                                           3,     15,      0,      0x00c8, US_REG_DWORD },

    { "kti2_bios_err_ad",                                       3,     16,      0,      0x00b8, US_REG_DWORD },
    { "kti2_bios_err_misc",                                     3,     16,      0,      0x00b0, US_REG_DWORD },
    { "kti2_bios_err_st",                                       3,     16,      0,      0x00a8, US_REG_DWORD },
    { "kti2_crcerrcnt",                                         3,     16,      0,      0x00c0, US_REG_DWORD },
    { "kti2_dbgerrst0",                                         3,     16,      2,      0x03a0, US_REG_DWORD },
    { "kti2_dbgerrst1",                                         3,     16,      2,      0x03a8, US_REG_DWORD },
    { "kti2_dbgerrstdis0",                                      3,     16,      2,      0x03a4, US_REG_DWORD },
    { "kti2_dbgerrstdis1",                                      3,     16,      2,      0x03ac, US_REG_DWORD },
    { "kti2_dfx_lck_ctl_cfg",                                   3,     16,      2,      0x0800, US_REG_DWORD },
    { "kti2_errcnt0_cntr",                                      3,     16,      0,      0x00d4, US_REG_DWORD },
    { "kti2_errcnt1_cntr",                                      3,     16,      0,      0x00dc, US_REG_DWORD },
    { "kti2_errcnt2_cntr",                                      3,     16,      0,      0x00e4, US_REG_DWORD },
    { "kti2_lcl",                                               3,     16,      0,      0x0084, US_REG_DWORD },
    { "kti2_les",                                               3,     16,      0,      0x0090, US_REG_DWORD },
    { "kti2_ls",                                                3,     16,      0,      0x008c, US_REG_DWORD },
    { "kti2_ph_rxl0c_err_log0",                                 3,     16,      2,      0x09f4, US_REG_DWORD },
    { "kti2_ph_rxl0c_err_log1",                                 3,     16,      2,      0x09f8, US_REG_DWORD },
    { "kti2_ph_rxl0c_err_sts",                                  3,     16,      2,      0x09f0, US_REG_DWORD },
    { "kti2_pq_csr_mcsr0",                                      3,     16,      2,      0x0820, US_REG_DWORD },
    { "kti2_pq_csr_mcsr1",                                      3,     16,      2,      0x0824, US_REG_DWORD },
    { "kti2_pq_csr_mcsr3",                                      3,     16,      2,      0x082c, US_REG_DWORD },
    { "kti2_reut_ph_cls",                                       3,     16,      1,      0x0160, US_REG_DWORD },
    { "kti2_reut_ph_ctr1",                                      3,     16,      1,      0x012c, US_REG_DWORD },
    { "kti2_reut_ph_ctr2",                                      3,     16,      1,      0x0130, US_REG_DWORD },
    { "kti2_reut_ph_das",                                       3,     16,      1,      0x0170, US_REG_DWORD },
    { "kti2_reut_ph_kes",                                       3,     16,      1,      0x016c, US_REG_DWORD },
    { "kti2_reut_ph_ldc",                                       3,     16,      1,      0x0178, US_REG_DWORD },
    { "kti2_reut_ph_pis",                                       3,     16,      1,      0x014c, US_REG_DWORD },
    { "kti2_reut_ph_pls",                                       3,     16,      1,      0x0164, US_REG_DWORD },
    { "kti2_reut_ph_pss",                                       3,     16,      1,      0x0154, US_REG_DWORD },
    { "kti2_reut_ph_rds",                                       3,     16,      1,      0x0140, US_REG_DWORD },
    { "kti2_reut_ph_tds",                                       3,     16,      1,      0x013c, US_REG_DWORD },
    { "kti2_reut_ph_ttlecr",                                    3,     16,      1,      0x01b4, US_REG_DWORD },
    { "kti2_reut_ph_ttres",                                     3,     16,      1,      0x01c0, US_REG_DWORD },
    { "kti2_reut_ph_wes",                                       3,     16,      1,      0x0168, US_REG_DWORD },

    { "kti2_viral",                                             3,     16,      0,      0x009c, US_REG_DWORD },
    { "kti2_csmithres",                                         3,     16,      0,      0x00a0, US_REG_DWORD },
    { "kti2_cerrlogctrl",                                       3,     16,      0,      0x00a4, US_REG_DWORD },
    { "kti2_errdis0",                                           3,     16,      0,      0x00c4, US_REG_DWORD },
    { "kti2_errdis1",                                           3,     16,      0,      0x00c8, US_REG_DWORD },

    { "m3kti0_m3egrerrlog0",                                    3,     18,      0,      0x0120, US_REG_DWORD },
    { "m3kti0_m3egrerrlog1",                                    3,     18,      0,      0x0124, US_REG_DWORD },
    { "m3kti0_m3egrerrlog2",                                    3,     18,      0,      0x0128, US_REG_DWORD },
    { "m3kti0_m3egrerrlog3",                                    3,     18,      0,      0x012c, US_REG_DWORD },
    { "m3kti0_m3egrerrlog4",                                    3,     18,      0,      0x0130, US_REG_DWORD },
    { "m3kti0_m3egrerrlog5",                                    3,     18,      0,      0x0134, US_REG_DWORD },
    { "m3kti0_m3egrerrlog6",                                    3,     18,      0,      0x0138, US_REG_DWORD },
    { "m3kti0_m3egrerrlog7",                                    3,     18,      0,      0x013c, US_REG_DWORD },
    { "m3kti0_m3egrerrmsk0123",                                 3,     18,      0,      0x0118, US_REG_DWORD },
    { "m3kti0_m3egrerrmsk4567",                                 3,     18,      0,      0x011c, US_REG_DWORD },
    { "m3kti0_m3egrerrmsk8",                                    3,     18,      0,      0x0148, US_REG_DWORD },
    { "m3kti0_m3ingerrlog0",                                    3,     18,      0,      0x0160, US_REG_DWORD },
    { "m3kti0_m3ingerrlog1",                                    3,     18,      0,      0x0164, US_REG_DWORD },
    { "m3kti0_m3ingerrlog2",                                    3,     18,      0,      0x0168, US_REG_DWORD },
    { "m3kti0_m3ingerrlog3",                                    3,     18,      0,      0x016c, US_REG_DWORD },
    { "m3kti0_m3ingerrmask0",                                   3,     18,      0,      0x0158, US_REG_DWORD },
    { "m3kti0_m3ingerrmask1",                                   3,     18,      0,      0x015c, US_REG_DWORD },
    { "m3kti1_m3egrerrlog0",                                    3,     18,      4,      0x0120, US_REG_DWORD },
    { "m3kti1_m3egrerrlog1",                                    3,     18,      4,      0x0124, US_REG_DWORD },
    { "m3kti1_m3egrerrlog2",                                    3,     18,      4,      0x0128, US_REG_DWORD },
    { "m3kti1_m3egrerrlog3",                                    3,     18,      4,      0x012c, US_REG_DWORD },
    { "m3kti1_m3egrerrlog4",                                    3,     18,      4,      0x0130, US_REG_DWORD },
    { "m3kti1_m3egrerrlog5",                                    3,     18,      4,      0x0134, US_REG_DWORD },
    { "m3kti1_m3egrerrlog6",                                    3,     18,      4,      0x0138, US_REG_DWORD },
    { "m3kti1_m3egrerrlog7",                                    3,     18,      4,      0x013c, US_REG_DWORD },
    { "m3kti1_m3egrerrmsk0123",                                 3,     18,      4,      0x0118, US_REG_DWORD },
    { "m3kti1_m3egrerrmsk4567",                                 3,     18,      4,      0x011c, US_REG_DWORD },
    { "m3kti1_m3egrerrmsk8",                                    3,     18,      4,      0x0148, US_REG_DWORD },
    { "m3kti1_m3ingerrlog0",                                    3,     18,      4,      0x0160, US_REG_DWORD },
    { "m3kti1_m3ingerrlog1",                                    3,     18,      4,      0x0164, US_REG_DWORD },
    { "m3kti1_m3ingerrlog2",                                    3,     18,      4,      0x0168, US_REG_DWORD },
    { "m3kti1_m3ingerrlog3",                                    3,     18,      4,      0x016c, US_REG_DWORD },
    { "m3kti1_m3ingerrmask0",                                   3,     18,      4,      0x0158, US_REG_DWORD },
    { "m3kti1_m3ingerrmask1",                                   3,     18,      4,      0x015c, US_REG_DWORD },
//    { "ntb_b1d00f0_corerrsts",                                  1,      0,      0,      0x0158, US_REG_DWORD },
//    { "ntb_b1d00f0_devsts",                                     1,      0,      0,      0x009a,  US_REG_WORD },
//    { "ntb_b1d00f0_errsid",                                     1,      0,      0,      0x017c, US_REG_DWORD },
//    { "ntb_b1d00f0_ler_ctrlsts",                                1,      0,      0,      0x0288, US_REG_DWORD },
//    { "ntb_b1d00f0_lnerrsts",                                   1,      0,      0,      0x0258, US_REG_DWORD },
    { "ntb_b1d00f0_lnksts",                                     1,      0,      0,      0x01a2,  US_REG_WORD },
    { "ntb_b1d00f0_lnksts2",                                    1,      0,      0,      0x01c2,  US_REG_WORD },
//    { "ntb_b1d00f0_miscctrlsts_0",                              1,      0,      0,      0x0188, US_REG_DWORD },
//    { "ntb_b1d00f0_miscctrlsts_1",                              1,      0,      0,      0x018c, US_REG_DWORD },
//    { "ntb_b1d00f0_rperrsts",                                   1,      0,      0,      0x0178, US_REG_DWORD },
//    { "ntb_b1d00f0_rppioerr_cap",                               1,      0,      0,      0x0298, US_REG_DWORD },
//    { "ntb_b1d00f0_rppioerr_status",                            1,      0,      0,      0x02a4, US_REG_DWORD },
    { "ntb_b1d00f0_sltsts",                                     1,      0,      0,      0x01aa,  US_REG_WORD },
//    { "ntb_b1d00f0_uncerrsts",                                  1,      0,      0,      0x014c, US_REG_DWORD },
//    { "ntb_b1d00f0_xpcorerrsts",                                1,      0,      0,      0x0200, US_REG_DWORD },
//    { "ntb_b1d00f0_xpglberrptr",                                1,      0,      0,      0x0232,  US_REG_WORD },
//    { "ntb_b1d00f0_xpglberrsts",                                1,      0,      0,      0x0230,  US_REG_WORD },
//    { "ntb_b1d00f0_xpuncerrsts",                                1,      0,      0,      0x0208, US_REG_DWORD },
//    { "ntb_b2d00f0_corerrsts",                                  2,      0,      0,      0x0158, US_REG_DWORD },
//    { "ntb_b2d00f0_devsts",                                     2,      0,      0,      0x009a,  US_REG_WORD },
//    { "ntb_b2d00f0_errsid",                                     2,      0,      0,      0x017c, US_REG_DWORD },
//    { "ntb_b2d00f0_ler_ctrlsts",                                2,      0,      0,      0x0288, US_REG_DWORD },
//    { "ntb_b2d00f0_lnerrsts",                                   2,      0,      0,      0x0258, US_REG_DWORD },
    { "ntb_b2d00f0_lnksts",                                     2,      0,      0,      0x01a2,  US_REG_WORD },
    { "ntb_b2d00f0_lnksts2",                                    2,      0,      0,      0x01c2,  US_REG_WORD },
//    { "ntb_b2d00f0_miscctrlsts_0",                              2,      0,      0,      0x0188, US_REG_DWORD },
//    { "ntb_b2d00f0_miscctrlsts_1",                              2,      0,      0,      0x018c, US_REG_DWORD },
//    { "ntb_b2d00f0_rperrsts",                                   2,      0,      0,      0x0178, US_REG_DWORD },
//    { "ntb_b2d00f0_rppioerr_cap",                               2,      0,      0,      0x0298, US_REG_DWORD },
//    { "ntb_b2d00f0_rppioerr_status",                            2,      0,      0,      0x02a4, US_REG_DWORD },
    { "ntb_b2d00f0_sltsts",                                     2,      0,      0,      0x01aa,  US_REG_WORD },
//    { "ntb_b2d00f0_uncerrsts",                                  2,      0,      0,      0x014c, US_REG_DWORD },
//    { "ntb_b2d00f0_xpcorerrsts",                                2,      0,      0,      0x0200, US_REG_DWORD },
//    { "ntb_b2d00f0_xpglberrptr",                                2,      0,      0,      0x0232,  US_REG_WORD },
//    { "ntb_b2d00f0_xpglberrsts",                                2,      0,      0,      0x0230,  US_REG_WORD },
//    { "ntb_b2d00f0_xpuncerrsts",                                2,      0,      0,      0x0208, US_REG_DWORD },
//    { "ntb_b3d00f0_corerrsts",                                  3,      0,      0,      0x0158, US_REG_DWORD },
//    { "ntb_b3d00f0_devsts",                                     3,      0,      0,      0x009a,  US_REG_WORD },
//    { "ntb_b3d00f0_errsid",                                     3,      0,      0,      0x017c, US_REG_DWORD },
//    { "ntb_b3d00f0_ler_ctrlsts",                                3,      0,      0,      0x0288, US_REG_DWORD },
//    { "ntb_b3d00f0_lnerrsts",                                   3,      0,      0,      0x0258, US_REG_DWORD },
    { "ntb_b3d00f0_lnksts",                                     3,      0,      0,      0x01a2,  US_REG_WORD },
    { "ntb_b3d00f0_lnksts2",                                    3,      0,      0,      0x01c2,  US_REG_WORD },
//    { "ntb_b3d00f0_miscctrlsts_0",                              3,      0,      0,      0x0188, US_REG_DWORD },
//    { "ntb_b3d00f0_miscctrlsts_1",                              3,      0,      0,      0x018c, US_REG_DWORD },
//    { "ntb_b3d00f0_rperrsts",                                   3,      0,      0,      0x0178, US_REG_DWORD },
//    { "ntb_b3d00f0_rppioerr_cap",                               3,      0,      0,      0x0298, US_REG_DWORD },
//    { "ntb_b3d00f0_rppioerr_status",                            3,      0,      0,      0x02a4, US_REG_DWORD },
    { "ntb_b3d00f0_sltsts",                                     3,      0,      0,      0x01aa,  US_REG_WORD },
//    { "ntb_b3d00f0_uncerrsts",                                  3,      0,      0,      0x014c, US_REG_DWORD },
//    { "ntb_b3d00f0_xpcorerrsts",                                3,      0,      0,      0x0200, US_REG_DWORD },
//    { "ntb_b3d00f0_xpglberrptr",                                3,      0,      0,      0x0232,  US_REG_WORD },
//    { "ntb_b3d00f0_xpglberrsts",                                3,      0,      0,      0x0230,  US_REG_WORD },
//    { "ntb_b3d00f0_xpuncerrsts",                                3,      0,      0,      0x0208, US_REG_DWORD },
};

static const SUncoreStatusRegPciMmio sUncoreStatusPciMmio[] =
{
    // Register,                            Bar,        Bus,        Dev,        Func,       Offset,         SizeCode
    { "cb_bar0_chanerr",                {{  .bar = 0,   .bus = 0,   .dev = 4,   .func = 0,  .reg = 0x00a8,  .lenCode = US_MMIO_DWORD }}},
    { "cb_bar1_chanerr",                {{  .bar = 0,   .bus = 0,   .dev = 4,   .func = 1,  .reg = 0x00a8,  .lenCode = US_MMIO_DWORD }}},
    { "cb_bar2_chanerr",                {{  .bar = 0,   .bus = 0,   .dev = 4,   .func = 2,  .reg = 0x00a8,  .lenCode = US_MMIO_DWORD }}},
    { "cb_bar3_chanerr",                {{  .bar = 0,   .bus = 0,   .dev = 4,   .func = 3,  .reg = 0x00a8,  .lenCode = US_MMIO_DWORD }}},
    { "cb_bar4_chanerr",                {{  .bar = 0,   .bus = 0,   .dev = 4,   .func = 4,  .reg = 0x00a8,  .lenCode = US_MMIO_DWORD }}},
    { "cb_bar5_chanerr",                {{  .bar = 0,   .bus = 0,   .dev = 4,   .func = 5,  .reg = 0x00a8,  .lenCode = US_MMIO_DWORD }}},
    { "cb_bar6_chanerr",                {{  .bar = 0,   .bus = 0,   .dev = 4,   .func = 6,  .reg = 0x00a8,  .lenCode = US_MMIO_DWORD }}},
    { "cb_bar7_chanerr",                {{  .bar = 0,   .bus = 0,   .dev = 4,   .func = 7,  .reg = 0x00a8,  .lenCode = US_MMIO_DWORD }}},
    { "iio_vtd0_fltsts_b0d05f0",        {{  .bar = 0,   .bus = 0,   .dev = 5,   .func = 0,  .reg = 0x0034,  .lenCode = US_MMIO_DWORD }}},
    { "iio_vtd0_fltsts_b1d05f0",        {{  .bar = 0,   .bus = 1,   .dev = 5,   .func = 0,  .reg = 0x0034,  .lenCode = US_MMIO_DWORD }}},
    { "iio_vtd0_fltsts_b2d05f0",        {{  .bar = 0,   .bus = 2,   .dev = 5,   .func = 0,  .reg = 0x0034,  .lenCode = US_MMIO_DWORD }}},
    { "iio_vtd0_fltsts_b3d05f0",        {{  .bar = 0,   .bus = 3,   .dev = 5,   .func = 0,  .reg = 0x0034,  .lenCode = US_MMIO_DWORD }}},
    { "iio_vtd0_glbsts_b0d05f0",        {{  .bar = 0,   .bus = 0,   .dev = 5,   .func = 0,  .reg = 0x001c,  .lenCode = US_MMIO_DWORD }}},
    { "iio_vtd0_glbsts_b1d05f0",        {{  .bar = 0,   .bus = 1,   .dev = 5,   .func = 0,  .reg = 0x001c,  .lenCode = US_MMIO_DWORD }}},
    { "iio_vtd0_glbsts_b2d05f0",        {{  .bar = 0,   .bus = 2,   .dev = 5,   .func = 0,  .reg = 0x001c,  .lenCode = US_MMIO_DWORD }}},
    { "iio_vtd0_glbsts_b3d05f0",        {{  .bar = 0,   .bus = 3,   .dev = 5,   .func = 0,  .reg = 0x001c,  .lenCode = US_MMIO_DWORD }}},
    { "iio_vtd1_fltsts_b0d05f0",        {{  .bar = 0,   .bus = 0,   .dev = 5,   .func = 0,  .reg = 0x1034,  .lenCode = US_MMIO_DWORD }}},
    { "iio_vtd1_fltsts_b1d05f0",        {{  .bar = 0,   .bus = 1,   .dev = 5,   .func = 0,  .reg = 0x1034,  .lenCode = US_MMIO_DWORD }}},
    { "iio_vtd1_fltsts_b2d05f0",        {{  .bar = 0,   .bus = 2,   .dev = 5,   .func = 0,  .reg = 0x1034,  .lenCode = US_MMIO_DWORD }}},
    { "iio_vtd1_fltsts_b3d05f0",        {{  .bar = 0,   .bus = 3,   .dev = 5,   .func = 0,  .reg = 0x1034,  .lenCode = US_MMIO_DWORD }}},
    { "iio_vtd1_glbsts_b0d05f0",        {{  .bar = 0,   .bus = 0,   .dev = 5,   .func = 0,  .reg = 0x101c,  .lenCode = US_MMIO_DWORD }}},
    { "iio_vtd1_glbsts_b1d05f0",        {{  .bar = 0,   .bus = 1,   .dev = 5,   .func = 0,  .reg = 0x101c,  .lenCode = US_MMIO_DWORD }}},
    { "iio_vtd1_glbsts_b2d05f0",        {{  .bar = 0,   .bus = 2,   .dev = 5,   .func = 0,  .reg = 0x101c,  .lenCode = US_MMIO_DWORD }}},
    { "iio_vtd1_glbsts_b3d05f0",        {{  .bar = 0,   .bus = 3,   .dev = 5,   .func = 0,  .reg = 0x101c,  .lenCode = US_MMIO_DWORD }}},
    { "ntb_b1d00f0_devsts_pb01base",    {{  .bar = 0,   .bus = 1,   .dev = 0,   .func = 0,  .reg = 0x459a,  .lenCode =  US_MMIO_WORD }}},
    { "ntb_b1d00f0_devsts_sb01base",    {{  .bar = 2,   .bus = 1,   .dev = 0,   .func = 0,  .reg = 0x059a,  .lenCode =  US_MMIO_WORD }}},
    { "ntb_b1d00f0_lnksts_pb01base",    {{  .bar = 0,   .bus = 1,   .dev = 0,   .func = 0,  .reg = 0x45a2,  .lenCode =  US_MMIO_WORD }}},
    { "ntb_b1d00f0_lnksts_sb01base",    {{  .bar = 2,   .bus = 1,   .dev = 0,   .func = 0,  .reg = 0x05a2,  .lenCode =  US_MMIO_WORD }}},
    { "ntb_b2d00f0_devsts_pb01base",    {{  .bar = 0,   .bus = 2,   .dev = 0,   .func = 0,  .reg = 0x459a,  .lenCode =  US_MMIO_WORD }}},
    { "ntb_b2d00f0_devsts_sb01base",    {{  .bar = 2,   .bus = 2,   .dev = 0,   .func = 0,  .reg = 0x059a,  .lenCode =  US_MMIO_WORD }}},
    { "ntb_b2d00f0_lnksts_pb01base",    {{  .bar = 0,   .bus = 2,   .dev = 0,   .func = 0,  .reg = 0x45a2,  .lenCode =  US_MMIO_WORD }}},
    { "ntb_b2d00f0_lnksts_sb01base",    {{  .bar = 2,   .bus = 2,   .dev = 0,   .func = 0,  .reg = 0x05a2,  .lenCode =  US_MMIO_WORD }}},
    { "ntb_b3d00f0_devsts_pb01base",    {{  .bar = 0,   .bus = 3,   .dev = 0,   .func = 0,  .reg = 0x459a,  .lenCode =  US_MMIO_WORD }}},
    { "ntb_b3d00f0_devsts_sb01base",    {{  .bar = 2,   .bus = 3,   .dev = 0,   .func = 0,  .reg = 0x059a,  .lenCode =  US_MMIO_WORD }}},
    { "ntb_b3d00f0_lnksts_pb01base",    {{  .bar = 0,   .bus = 3,   .dev = 0,   .func = 0,  .reg = 0x45a2,  .lenCode =  US_MMIO_WORD }}},
    { "ntb_b3d00f0_lnksts_sb01base",    {{  .bar = 2,   .bus = 3,   .dev = 0,   .func = 0,  .reg = 0x05a2,  .lenCode =  US_MMIO_WORD }}},
};

static const SUncoreStatusRegIio sUncoreStatusIio[] =
{
    { "iio_cstack_mc_%s", 0 },
    { "iio_pstack0_mc_%s", 1 },
    { "iio_pstack1_mc_%s", 2 },
    { "iio_pstack2_mc_%s", 3 },
    { "iio_pstack3_mc_%s", 4 },
};

/******************************************************************************
*
*   uncoreStatusPciRaw
*
*   This function writes the Uncore Status PCI registers to raw file
*
******************************************************************************/
#ifdef BUILD_RAW
static void uncoreStatusPciRaw(UINT8 u8Cpu, const char * regName, SUncoreStatusRegRawData * sRegData, FILE * fpRaw)
{
#ifdef SPX_BMC_ACD
    UN_USED(u8Cpu);
    UN_USED(regName);
#endif
    // Add the Uncore Status register data to the Uncore Status dump
    fwrite(sRegData, sizeof(SUncoreStatusRegRawData), 1, fpRaw);
}
#endif //BUILD_RAW

/******************************************************************************
*
*   uncoreStatusPciJson
*
*   This function formats the Uncore Status PCI registers into a JSON object
*
******************************************************************************/
#ifdef BUILD_JSON
static void uncoreStatusPciJson(UINT8 u8Cpu, const char * regName, SUncoreStatusRegRawData * sRegData, cJSON * pJsonChild)
{
    cJSON * socket;
    char jsonItemString[US_JSON_STRING_LEN];

    // Add the socket number item to the Uncore Status JSON structure only if it doesn't already exist
    snprintf(jsonItemString, US_JSON_STRING_LEN, US_JSON_SOCKET_NAME, u8Cpu);
    if ((socket = cJSON_GetObjectItemCaseSensitive(pJsonChild, jsonItemString)) == NULL) {
        cJSON_AddItemToObject(pJsonChild, jsonItemString, socket = cJSON_CreateObject());
    }

    // Format the Uncore Status register data out to the .json debug file
    if (sRegData->bInvalid) {
        snprintf(jsonItemString, US_JSON_STRING_LEN, "%s0x%02x", US_FAILED, sRegData->uValue.u32[0]);
        cJSON_AddStringToObject(socket, regName, jsonItemString);
    } else {
        snprintf(jsonItemString, US_JSON_STRING_LEN, "0x%08x%08x", sRegData->uValue.u32[1], sRegData->uValue.u32[0]);
        cJSON_AddStringToObject(socket, regName, jsonItemString);
    }
}
#endif //BUILD_JSON

/******************************************************************************
*
*   uncoreStatusPci
*
*   This function gathers the Uncore Status PCI registers
*
******************************************************************************/
static ESTATUS uncoreStatusPci(FILE * fpRaw, cJSON * pJsonChild)
{
    UINT32 i;
    UINT8 u8Cpu;
    UINT8 u8Dword;
    unsigned int work, bus, device, func, reg;
    SRdPCIConfigLocalReq sRdPCIConfigLocalReq;
    SRdPCIConfigLocalRes sRdPCIConfigLocalRes;
#ifdef BUILD_JSON
    char regName[US_JSON_STRING_LEN];
#endif //BUILD_JSON

    for (u8Cpu = CPU0_ID; u8Cpu < MAX_CPU; u8Cpu++) {
        if (FALSE == IsCpuPresent(u8Cpu)) {
            continue;
        }

        PRINT(PRINT_DBG, PRINT_INFO, "Platform Debug - Uncore Status PCI Registers %d\n", u8Cpu);

        // Get the Uncore Status PCI Registers
        for (i = 0; i < (sizeof(sUncoreStatusPci) / sizeof(SUncoreStatusRegPci)); i++) {
#ifndef SPX_BMC_ACD
            SUncoreStatusRegRawData sRegData = {};
#else
			SUncoreStatusRegRawData sRegData;
			memset(&sRegData, 0x0, sizeof(SUncoreStatusRegRawData));
#endif
            PRINT(PRINT_DBG, PRINT_DEBUG2, "%s (CPU %d BDF %x:%x:%x reg %x) Local PCI Read\n", sUncoreStatusPci[i].regName, u8Cpu, sUncoreStatusPci[i].u8Bus, sUncoreStatusPci[i].u8Dev, sUncoreStatusPci[i].u8Func, sUncoreStatusPci[i].u16Reg);
            switch (sUncoreStatusPci[i].u8Size) {
                case US_REG_BYTE:
                case US_REG_WORD:
                case US_REG_DWORD:
                    memset(&sRdPCIConfigLocalReq, 0 , sizeof(SRdPCIConfigLocalReq));
                    memset(&sRdPCIConfigLocalRes, 0 , sizeof(SRdPCIConfigLocalRes));
                    sRdPCIConfigLocalReq.sHeader.u8ClientAddr = PECI_BASE_ADDR + u8Cpu;
                    sRdPCIConfigLocalReq.sHeader.u8WriteLength = 0x05;
                    sRdPCIConfigLocalReq.sHeader.u8ReadLength = sUncoreStatusPci[i].u8Size + 1;
                    sRdPCIConfigLocalReq.u8CmdCode = 0xE1;
                    sRdPCIConfigLocalReq.u8HostID_Retry = 0x02;
                    bus = sUncoreStatusPci[i].u8Bus;
                    device = sUncoreStatusPci[i].u8Dev;
                    func = sUncoreStatusPci[i].u8Func;
                    reg = sUncoreStatusPci[i].u16Reg;
                    work = 0x00;
                    work |= (bus << 20) & 0x00F00000;
                    work |= (device << 15) & 0x000f8000;
                    work |= (func << 12) & 0x00007000;
                    work |= reg & 0x00000FFF;
                    sRdPCIConfigLocalReq.u8PCIConfigAddr[0] = work & 0x000000FF;
                    sRdPCIConfigLocalReq.u8PCIConfigAddr[1] = (work >> 8) & 0x000000FF;
                    sRdPCIConfigLocalReq.u8PCIConfigAddr[2] = (work >> 16) & 0x000000FF;
                    if (0 == PECI_RdPCIConfigLocal (&sRdPCIConfigLocalReq, &sRdPCIConfigLocalRes))
                    {
                        memcpy(&sRegData.uValue.u64, sRdPCIConfigLocalRes.u8Data, sUncoreStatusPci[i].u8Size);
                    }
                    else
                    {
                        sRdPCIConfigLocalRes.u8CompletionCode = 0x00;
                    }
                    if (sRdPCIConfigLocalRes.u8CompletionCode != PECI_CC_SUCCESS) {
                        PRINT(PRINT_DBG, PRINT_ERROR, "%s (CPU %d BDF %x:%x:%x reg %x) Local PCI Failed\n", sUncoreStatusPci[i].regName, u8Cpu, sUncoreStatusPci[i].u8Bus, sUncoreStatusPci[i].u8Dev, sUncoreStatusPci[i].u8Func, sUncoreStatusPci[i].u16Reg);
                        sRegData.uValue.u32[0] = sRdPCIConfigLocalRes.u8CompletionCode;
                        sRegData.bInvalid = TRUE;
                    }
                    break;
                case US_REG_QWORD:
                    for (u8Dword = 0; u8Dword < 2; u8Dword++) {
                        memset(&sRdPCIConfigLocalReq, 0 , sizeof(SRdPCIConfigLocalReq));
                        memset(&sRdPCIConfigLocalRes, 0 , sizeof(SRdPCIConfigLocalRes));
                        sRdPCIConfigLocalReq.sHeader.u8ClientAddr = PECI_BASE_ADDR + u8Cpu;
                        sRdPCIConfigLocalReq.sHeader.u8WriteLength = 0x05;
                        sRdPCIConfigLocalReq.sHeader.u8ReadLength = sizeof(UINT32) + 1;
                        sRdPCIConfigLocalReq.u8CmdCode = 0xE1;
                        sRdPCIConfigLocalReq.u8HostID_Retry = 0x02;
                        bus = sUncoreStatusPci[i].u8Bus;
                        device = sUncoreStatusPci[i].u8Dev;
                        func = sUncoreStatusPci[i].u8Func;
                        reg = sUncoreStatusPci[i].u16Reg + (u8Dword * 4);
                        work = 0x00;
                        work |= (bus << 20) & 0x00F00000;
                        work |= (device << 15) & 0x000f8000;
                        work |= (func << 12) & 0x00007000;
                        work |= reg & 0x00000FFF;
                        sRdPCIConfigLocalReq.u8PCIConfigAddr[0] = work & 0x000000FF;
                        sRdPCIConfigLocalReq.u8PCIConfigAddr[1] = (work >> 8) & 0x000000FF;
                        sRdPCIConfigLocalReq.u8PCIConfigAddr[2] = (work >> 16) & 0x000000FF;
                        if (0 == PECI_RdPCIConfigLocal (&sRdPCIConfigLocalReq, &sRdPCIConfigLocalRes))
                        {
                            memcpy(&sRegData.uValue.u32[u8Dword], sRdPCIConfigLocalRes.u8Data, sizeof(UINT32));
                        }
                        else
                        {
                            sRdPCIConfigLocalRes.u8CompletionCode = 0x00;
                        }
                        if (sRdPCIConfigLocalRes.u8CompletionCode != PECI_CC_SUCCESS) {
                            PRINT(PRINT_DBG, PRINT_ERROR, "%s (CPU %d BDF %x:%x:%x reg %x) Local PCI Failed\n", sUncoreStatusPci[i].regName, u8Cpu, sUncoreStatusPci[i].u8Bus, sUncoreStatusPci[i].u8Dev, sUncoreStatusPci[i].u8Func, sUncoreStatusPci[i].u16Reg + (u8Dword * 4));
                            sRegData.uValue.u32[0] = sRdPCIConfigLocalRes.u8CompletionCode;
                            sRegData.bInvalid = TRUE;
                            break;
                        }
                    }
                    break;
#ifdef SPX_BMC_ACD
                default:
				    break;
#endif
            }
            // Log this Uncore Status PCI Register
#ifdef BUILD_RAW
            uncoreStatusPciRaw(u8Cpu, sUncoreStatusPci[i].regName, &sRegData, fpRaw);
#endif //BUILD_RAW
#ifdef BUILD_JSON
            snprintf(regName, US_JSON_STRING_LEN, "B%02x_D%02x_F%x_0x%X",
                        sUncoreStatusPci[i].u8Bus,
                        sUncoreStatusPci[i].u8Dev,
                        sUncoreStatusPci[i].u8Func,
                        sUncoreStatusPci[i].u16Reg
                    );
            uncoreStatusPciJson(u8Cpu, regName, &sRegData, pJsonChild);
#endif //BUILD_JSON
        }
    }
    return ST_OK;
}

/******************************************************************************
*
*   uncoreStatusMmioRead
*
*   This function gathers the Uncore Status PCI MMIO registers
*
******************************************************************************/
static ESTATUS uncoreStatusMmioRead(UINT8 u8Cpu, UINT32 u32Param, UINT8 u8NumDwords, SUncoreStatusRegRawData * sUncoreStatusMmioRawData)
{
    SRdPkgConfigReq sRdPkgConfigReq;
    SRdPkgConfigRes sRdPkgConfigRes;
    SWrPkgConfigReq sWrPkgConfigReq;
    SWrPkgConfigRes sWrPkgConfigRes;
    UINT32 u32work;
    UINT8 u8Dword;

    // Open the MMIO dump sequence
    PRINT(PRINT_DBG, PRINT_INFO, "[DBG] - CPU #%d Uncore Status MMIO Param %x Sequence Opened\n", u8Cpu, u32Param);
    memset(&sWrPkgConfigReq, 0 , sizeof(SWrPkgConfigReq));
    memset(&sWrPkgConfigRes, 0 , sizeof(SWrPkgConfigRes));
    sWrPkgConfigReq.sHeader.u8ClientAddr = PECI_BASE_ADDR + u8Cpu;
    sWrPkgConfigReq.sHeader.u8WriteLength = sizeof(UINT32) + 6;
    sWrPkgConfigReq.sHeader.u8ReadLength = 0x01;
    sWrPkgConfigReq.u8CmdCode = 0xA5;
    sWrPkgConfigReq.u8HostID_Retry = 0x02;
    sWrPkgConfigReq.u8Index = MBX_INDEX_VCU;
    sWrPkgConfigReq.u16Parameter = VCU_OPEN_SEQ;
    u32work = VCU_READ_LOCAL_MMIO_SEQ;
    memcpy (sWrPkgConfigReq.u8Data, &u32work, sizeof(UINT32));
    if (0 != PECI_WrPkgConfig (&sWrPkgConfigReq, &sWrPkgConfigRes))
    {
        sWrPkgConfigRes.u8CompletionCode = 0x00;
    }
    if (sWrPkgConfigRes.u8CompletionCode != PECI_CC_SUCCESS) {
        // MMIO sequence failed, abort the sequence
        sUncoreStatusMmioRawData->uValue.u32[0] = sWrPkgConfigRes.u8CompletionCode;
        memset(&sWrPkgConfigReq, 0 , sizeof(SWrPkgConfigReq));
        memset(&sWrPkgConfigRes, 0 , sizeof(SWrPkgConfigRes));
        sWrPkgConfigReq.sHeader.u8ClientAddr = PECI_BASE_ADDR + u8Cpu;
        sWrPkgConfigReq.sHeader.u8WriteLength = sizeof(UINT32) + 6;
        sWrPkgConfigReq.sHeader.u8ReadLength = 0x01;
        sWrPkgConfigReq.u8CmdCode = 0xA5;
        sWrPkgConfigReq.u8HostID_Retry = 0x02;
        sWrPkgConfigReq.u8Index = MBX_INDEX_VCU;
        sWrPkgConfigReq.u16Parameter = VCU_ABORT_SEQ;
        u32work = VCU_READ_LOCAL_MMIO_SEQ;
        memcpy (sWrPkgConfigReq.u8Data, &u32work, sizeof(UINT32));
        if (0 != PECI_WrPkgConfig (&sWrPkgConfigReq, &sWrPkgConfigRes))
        {
            sWrPkgConfigRes.u8CompletionCode = 0x00;
        }
        PRINT(PRINT_DBG, PRINT_ERROR, "[DBG] - CPU #%d Uncore Status MMIO Param %x Sequence Failed\n", u8Cpu, u32Param);
        return ST_HW_FAILURE;
    }

    // Set MMIO address
    PRINT(PRINT_DBG, PRINT_INFO, "[DBG] - CPU #%d Set MMIO Parameter %x\n", u8Cpu, u32Param);
    memset(&sWrPkgConfigReq, 0 , sizeof(SWrPkgConfigReq));
    memset(&sWrPkgConfigRes, 0 , sizeof(SWrPkgConfigRes));
    sWrPkgConfigReq.sHeader.u8ClientAddr = PECI_BASE_ADDR + u8Cpu;
    sWrPkgConfigReq.sHeader.u8WriteLength = sizeof(UINT32) + 6;
    sWrPkgConfigReq.sHeader.u8ReadLength = 0x01;
    sWrPkgConfigReq.u8CmdCode = 0xA5;
    sWrPkgConfigReq.u8HostID_Retry = 0x02;
    sWrPkgConfigReq.u8Index = MBX_INDEX_VCU;
    sWrPkgConfigReq.u16Parameter = US_MMIO_PARAM;
    u32work = u32Param;
    memcpy (sWrPkgConfigReq.u8Data, &u32work, sizeof(UINT32));
    if (0 != PECI_WrPkgConfig (&sWrPkgConfigReq, &sWrPkgConfigRes))
    {
        sWrPkgConfigRes.u8CompletionCode = 0x00;
    }
    if (sWrPkgConfigRes.u8CompletionCode != PECI_CC_SUCCESS) {
        // MMIO sequence failed, abort the sequence
        sUncoreStatusMmioRawData->uValue.u32[0] = sWrPkgConfigRes.u8CompletionCode;
        memset(&sWrPkgConfigReq, 0 , sizeof(SWrPkgConfigReq));
        memset(&sWrPkgConfigRes, 0 , sizeof(SWrPkgConfigRes));
        sWrPkgConfigReq.sHeader.u8ClientAddr = PECI_BASE_ADDR + u8Cpu;
        sWrPkgConfigReq.sHeader.u8WriteLength = sizeof(UINT32) + 6;
        sWrPkgConfigReq.sHeader.u8ReadLength = 0x01;
        sWrPkgConfigReq.u8CmdCode = 0xA5;
        sWrPkgConfigReq.u8HostID_Retry = 0x02;
        sWrPkgConfigReq.u8Index = MBX_INDEX_VCU;
        sWrPkgConfigReq.u16Parameter = VCU_ABORT_SEQ;
        u32work = VCU_READ_LOCAL_MMIO_SEQ;
        memcpy (sWrPkgConfigReq.u8Data, &u32work, sizeof(UINT32));
        if (0 != PECI_WrPkgConfig (&sWrPkgConfigReq, &sWrPkgConfigRes))
        {
            sWrPkgConfigRes.u8CompletionCode = 0x00;
        }
        PRINT(PRINT_DBG, PRINT_ERROR, "[DBG] - CPU #%d Uncore Status MMIO Param %x Sequence Failed\n", u8Cpu, u32Param);
        return ST_HW_FAILURE;
    }

    // Get the MMIO data
    for (u8Dword = 0; u8Dword < u8NumDwords; u8Dword++) {
        memset(&sRdPkgConfigReq, 0 , sizeof(SRdPkgConfigReq));
        memset(&sRdPkgConfigRes, 0 , sizeof(SRdPkgConfigRes));
        sRdPkgConfigReq.sHeader.u8ClientAddr = PECI_BASE_ADDR + u8Cpu;
        sRdPkgConfigReq.sHeader.u8WriteLength = 0x05;
        sRdPkgConfigReq.sHeader.u8ReadLength = sizeof(UINT32) + 1;
        sRdPkgConfigReq.u8CmdCode = 0xA1;
        sRdPkgConfigReq.u8HostID_Retry = 0x02;
        sRdPkgConfigReq.u8Index = MBX_INDEX_VCU;
        sRdPkgConfigReq.u16Parameter = VCU_READ;
        if (0 == PECI_RdPkgConfig (&sRdPkgConfigReq, &sRdPkgConfigRes))
        {
            memcpy(&sUncoreStatusMmioRawData->uValue.u32[u8Dword], sRdPkgConfigRes.u8Data, sizeof(UINT32));
        }
        else
        {
            sRdPkgConfigRes.u8CompletionCode = 0x00;
        }
        if (sRdPkgConfigRes.u8CompletionCode != PECI_CC_SUCCESS) {
            // MMIO sequence failed, abort the sequence
            sUncoreStatusMmioRawData->uValue.u32[0] = sRdPkgConfigRes.u8CompletionCode;
            memset(&sWrPkgConfigReq, 0 , sizeof(SWrPkgConfigReq));
            memset(&sWrPkgConfigRes, 0 , sizeof(SWrPkgConfigRes));
            sWrPkgConfigReq.sHeader.u8ClientAddr = PECI_BASE_ADDR + u8Cpu;
            sWrPkgConfigReq.sHeader.u8WriteLength = sizeof(UINT32) + 6;
            sWrPkgConfigReq.sHeader.u8ReadLength = 0x01;
            sWrPkgConfigReq.u8CmdCode = 0xA5;
            sWrPkgConfigReq.u8HostID_Retry = 0x02;
            sWrPkgConfigReq.u8Index = MBX_INDEX_VCU;
            sWrPkgConfigReq.u16Parameter = VCU_ABORT_SEQ;
            u32work = VCU_READ_LOCAL_MMIO_SEQ;
            memcpy (sWrPkgConfigReq.u8Data, &u32work, sizeof(UINT32));
            if (0 != PECI_WrPkgConfig (&sWrPkgConfigReq, &sWrPkgConfigRes))
            {
                sWrPkgConfigRes.u8CompletionCode = 0x00;
            }
            PRINT(PRINT_DBG, PRINT_ERROR, "[DBG] - CPU #%d Uncore Status MMIO Param %x Sequence Failed\n", u8Cpu, u32Param);
            sUncoreStatusMmioRawData->bInvalid = TRUE;
            return ST_HW_FAILURE;
        }
    }

    // Close the MMIO sequence
    PRINT(PRINT_DBG, PRINT_INFO, "[DBG] - CPU #%d Uncore Status MMIO Param %x Sequence Closed\n", u8Cpu, u32Param);
    memset(&sWrPkgConfigReq, 0 , sizeof(SWrPkgConfigReq));
    memset(&sWrPkgConfigRes, 0 , sizeof(SWrPkgConfigRes));
    sWrPkgConfigReq.sHeader.u8ClientAddr = PECI_BASE_ADDR + u8Cpu;
    sWrPkgConfigReq.sHeader.u8WriteLength = sizeof(UINT32) + 6;
    sWrPkgConfigReq.sHeader.u8ReadLength = 0x01;
    sWrPkgConfigReq.u8CmdCode = 0xA5;
    sWrPkgConfigReq.u8HostID_Retry = 0x02;
    sWrPkgConfigReq.u8Index = MBX_INDEX_VCU;
    sWrPkgConfigReq.u16Parameter = VCU_CLOSE_SEQ;
    u32work = VCU_READ_LOCAL_MMIO_SEQ;
    memcpy (sWrPkgConfigReq.u8Data, &u32work, sizeof(UINT32));
    if (0 != PECI_WrPkgConfig (&sWrPkgConfigReq, &sWrPkgConfigRes))
    {
        sWrPkgConfigRes.u8CompletionCode = 0x00;
    }

    return ST_OK;
}

/******************************************************************************
*
*   uncoreStatusPciMmio
*
*   This function gathers the Uncore Status PCI MMIO registers
*
******************************************************************************/
static ESTATUS uncoreStatusPciMmio(FILE * fpRaw, cJSON * pJsonChild)
{
    UINT32 i;
    UINT8 u8Cpu;
#ifdef BUILD_JSON
    char regName[US_JSON_STRING_LEN];
#endif //BUILD_JSON

    for (u8Cpu = CPU0_ID; u8Cpu < MAX_CPU; u8Cpu++) {
        if (FALSE == IsCpuPresent(u8Cpu)) {
            continue;
        }

        PRINT(PRINT_DBG, PRINT_INFO, "Platform Debug - Uncore Status PCI MMIO Registers %d\n", u8Cpu);

        // Get the Uncore Status PCI MMIO Registers
        for (i = 0; i < (sizeof(sUncoreStatusPciMmio) / sizeof(SUncoreStatusRegPciMmio)); i++) {
#ifndef SPX_BMC_ACD
            SUncoreStatusRegRawData sRegData = {};
#else
			SUncoreStatusRegRawData sRegData;
			memset(&sRegData, 0x0, sizeof(SUncoreStatusRegRawData));
#endif
            UINT32 u32MmioParam = sUncoreStatusPciMmio[i].uMmioReg.raw;
            UINT8 u8NumDwords = (UINT8)sUncoreStatusPciMmio[i].uMmioReg.fields.lenCode == US_MMIO_QWORD ? 2 : 1;

            // Get the MMIO data
            if (uncoreStatusMmioRead(u8Cpu, u32MmioParam, u8NumDwords, &sRegData) == ST_OK) {
                PRINT(PRINT_DBG, PRINT_INFO, "[DBG] - CPU #%d MMIO %x logged\n", u8Cpu, u32MmioParam);
            }

            // Log this Uncore Status PCI Register
#ifdef BUILD_RAW
            uncoreStatusPciRaw(u8Cpu, sUncoreStatusPci[i].regName, &sRegData, fpRaw);
#endif //BUILD_RAW
#ifdef BUILD_JSON
            snprintf(regName, US_JSON_STRING_LEN, "B%02x_D%02x_F%x_0x%X",
                        sUncoreStatusPciMmio[i].uMmioReg.fields.bus,
                        sUncoreStatusPciMmio[i].uMmioReg.fields.dev,
                        sUncoreStatusPciMmio[i].uMmioReg.fields.func,
                        sUncoreStatusPciMmio[i].uMmioReg.fields.reg
                    );
            uncoreStatusPciJson(u8Cpu, regName, &sRegData, pJsonChild);
#endif //BUILD_JSON
        }
    }
    return ST_OK;
}

/******************************************************************************
*
*   uncoreStatusMcaRead
*
*   This function gathers the Uncore Status MCA registers
*
******************************************************************************/
static ESTATUS uncoreStatusMcaRead(UINT8 u8Cpu, UINT32 u32Param, SUncoreStatusMcaRawData * sUncoreStatusMcaRawData)
{
    SRdPkgConfigReq sRdPkgConfigReq;
    SRdPkgConfigRes sRdPkgConfigRes;
    SWrPkgConfigReq sWrPkgConfigReq;
    SWrPkgConfigRes sWrPkgConfigRes;
    UINT32 u32work;
    UINT8 u8Dword;

    // Open the MCA Bank dump sequence
    PRINT(PRINT_DBG, PRINT_INFO, "[DBG] - CPU #%d Uncore Status MCA Param %x Sequence Opened\n", u8Cpu, u32Param);
    memset(&sWrPkgConfigReq, 0 , sizeof(SWrPkgConfigReq));
    memset(&sWrPkgConfigRes, 0 , sizeof(SWrPkgConfigRes));
    sWrPkgConfigReq.sHeader.u8ClientAddr = PECI_BASE_ADDR + u8Cpu;
    sWrPkgConfigReq.sHeader.u8WriteLength = sizeof(UINT32) + 6;
    sWrPkgConfigReq.sHeader.u8ReadLength = 0x01;
    sWrPkgConfigReq.u8CmdCode = 0xA5;
    sWrPkgConfigReq.u8HostID_Retry = 0x02;
    sWrPkgConfigReq.u8Index = MBX_INDEX_VCU;
    sWrPkgConfigReq.u16Parameter = VCU_OPEN_SEQ;
    u32work = VCU_UNCORE_MCA_SEQ;
    memcpy (sWrPkgConfigReq.u8Data, &u32work, sizeof(UINT32));
    if (0 != PECI_WrPkgConfig (&sWrPkgConfigReq, &sWrPkgConfigRes))
    {
        sWrPkgConfigRes.u8CompletionCode = 0x00;
    }
    if (sWrPkgConfigRes.u8CompletionCode != PECI_CC_SUCCESS) {
        // MCA Bank sequence failed, abort the sequence
        sUncoreStatusMcaRawData->uRegData.u32Raw[0] = sWrPkgConfigRes.u8CompletionCode;
        memset(&sWrPkgConfigReq, 0 , sizeof(SWrPkgConfigReq));
        memset(&sWrPkgConfigRes, 0 , sizeof(SWrPkgConfigRes));
        sWrPkgConfigReq.sHeader.u8ClientAddr = PECI_BASE_ADDR + u8Cpu;
        sWrPkgConfigReq.sHeader.u8WriteLength = sizeof(UINT32) + 6;
        sWrPkgConfigReq.sHeader.u8ReadLength = 0x01;
        sWrPkgConfigReq.u8CmdCode = 0xA5;
        sWrPkgConfigReq.u8HostID_Retry = 0x02;
        sWrPkgConfigReq.u8Index = MBX_INDEX_VCU;
        sWrPkgConfigReq.u16Parameter = VCU_ABORT_SEQ;
        u32work = VCU_UNCORE_MCA_SEQ;
        memcpy (sWrPkgConfigReq.u8Data, &u32work, sizeof(UINT32));
        if (0 != PECI_WrPkgConfig (&sWrPkgConfigReq, &sWrPkgConfigRes))
        {
            sWrPkgConfigRes.u8CompletionCode = 0x00;
        }
        PRINT(PRINT_DBG, PRINT_ERROR, "[DBG] - CPU #%d Uncore Status MCA Param %x Sequence Failed\n", u8Cpu, u32Param);
        return ST_HW_FAILURE;
    }

    // Set MCA Bank number
    PRINT(PRINT_DBG, PRINT_INFO, "[DBG] - CPU #%d Set MCA Parameter %x\n", u8Cpu, u32Param);
    memset(&sWrPkgConfigReq, 0 , sizeof(SWrPkgConfigReq));
    memset(&sWrPkgConfigRes, 0 , sizeof(SWrPkgConfigRes));
    sWrPkgConfigReq.sHeader.u8ClientAddr = PECI_BASE_ADDR + u8Cpu;
    sWrPkgConfigReq.sHeader.u8WriteLength = sizeof(UINT32) + 6;
    sWrPkgConfigReq.sHeader.u8ReadLength = 0x01;
    sWrPkgConfigReq.u8CmdCode = 0xA5;
    sWrPkgConfigReq.u8HostID_Retry = 0x02;
    sWrPkgConfigReq.u8Index = MBX_INDEX_VCU;
    sWrPkgConfigReq.u16Parameter = US_MCA_PARAM;
    u32work = u32Param;
    memcpy (sWrPkgConfigReq.u8Data, &u32work, sizeof(UINT32));
    if (0 != PECI_WrPkgConfig (&sWrPkgConfigReq, &sWrPkgConfigRes))
    {
        sWrPkgConfigRes.u8CompletionCode = 0x00;
    }
    if (sWrPkgConfigRes.u8CompletionCode != PECI_CC_SUCCESS) {
        // MCA Bank sequence failed, abort the sequence
        sUncoreStatusMcaRawData->uRegData.u32Raw[0] = sWrPkgConfigRes.u8CompletionCode;
        memset(&sWrPkgConfigReq, 0 , sizeof(SWrPkgConfigReq));
        memset(&sWrPkgConfigRes, 0 , sizeof(SWrPkgConfigRes));
        sWrPkgConfigReq.sHeader.u8ClientAddr = PECI_BASE_ADDR + u8Cpu;
        sWrPkgConfigReq.sHeader.u8WriteLength = sizeof(UINT32) + 6;
        sWrPkgConfigReq.sHeader.u8ReadLength = 0x01;
        sWrPkgConfigReq.u8CmdCode = 0xA5;
        sWrPkgConfigReq.u8HostID_Retry = 0x02;
        sWrPkgConfigReq.u8Index = MBX_INDEX_VCU;
        sWrPkgConfigReq.u16Parameter = VCU_ABORT_SEQ;
        u32work = VCU_UNCORE_MCA_SEQ;
        memcpy (sWrPkgConfigReq.u8Data, &u32work, sizeof(UINT32));
        if (0 != PECI_WrPkgConfig (&sWrPkgConfigReq, &sWrPkgConfigRes))
        {
            sWrPkgConfigRes.u8CompletionCode = 0x00;
        }
        PRINT(PRINT_DBG, PRINT_ERROR, "[DBG] - CPU #%d Uncore Status MCA Param %x Sequence Failed\n", u8Cpu, u32Param);
        return ST_HW_FAILURE;
    }

    // Get the MCA Bank Registers
    for (u8Dword = 0; u8Dword < US_NUM_MCA_DWORDS; u8Dword++) {
        memset(&sRdPkgConfigReq, 0 , sizeof(SRdPkgConfigReq));
        memset(&sRdPkgConfigRes, 0 , sizeof(SRdPkgConfigRes));
        sRdPkgConfigReq.sHeader.u8ClientAddr = PECI_BASE_ADDR + u8Cpu;
        sRdPkgConfigReq.sHeader.u8WriteLength = 0x05;
        sRdPkgConfigReq.sHeader.u8ReadLength = sizeof(UINT32) + 1;
        sRdPkgConfigReq.u8CmdCode = 0xA1;
        sRdPkgConfigReq.u8HostID_Retry = 0x02;
        sRdPkgConfigReq.u8Index = MBX_INDEX_VCU;
        sRdPkgConfigReq.u16Parameter = VCU_READ;
        if (0 == PECI_RdPkgConfig (&sRdPkgConfigReq, &sRdPkgConfigRes))
        {
            memcpy(&sUncoreStatusMcaRawData->uRegData.u32Raw[u8Dword], sRdPkgConfigRes.u8Data, sizeof(UINT32));
        }
        else
        {
            sRdPkgConfigRes.u8CompletionCode = 0x00;
        }
        if (sRdPkgConfigRes.u8CompletionCode != PECI_CC_SUCCESS) {
            // MCA Bank sequence failed, abort the sequence
            sUncoreStatusMcaRawData->uRegData.u32Raw[0] = sRdPkgConfigRes.u8CompletionCode;
            memset(&sWrPkgConfigReq, 0 , sizeof(SWrPkgConfigReq));
            memset(&sWrPkgConfigRes, 0 , sizeof(SWrPkgConfigRes));
            sWrPkgConfigReq.sHeader.u8ClientAddr = PECI_BASE_ADDR + u8Cpu;
            sWrPkgConfigReq.sHeader.u8WriteLength = sizeof(UINT32) + 6;
            sWrPkgConfigReq.sHeader.u8ReadLength = 0x01;
            sWrPkgConfigReq.u8CmdCode = 0xA5;
            sWrPkgConfigReq.u8HostID_Retry = 0x02;
            sWrPkgConfigReq.u8Index = MBX_INDEX_VCU;
            sWrPkgConfigReq.u16Parameter = VCU_ABORT_SEQ;
            u32work = VCU_UNCORE_MCA_SEQ;
            memcpy (sWrPkgConfigReq.u8Data, &u32work, sizeof(UINT32));
            if (0 != PECI_WrPkgConfig (&sWrPkgConfigReq, &sWrPkgConfigRes))
            {
                sWrPkgConfigRes.u8CompletionCode = 0x00;
            }
            PRINT(PRINT_DBG, PRINT_ERROR, "[DBG] - CPU #%d Uncore Status MCA Param %x Sequence Failed\n", u8Cpu, u32Param);
            return ST_HW_FAILURE;
        }
    }

    // Close the MCA Bank sequence
    PRINT(PRINT_DBG, PRINT_INFO, "[DBG] - CPU #%d Uncore Status MCA Param %x Sequence Closed\n", u8Cpu, u32Param);
    memset(&sWrPkgConfigReq, 0 , sizeof(SWrPkgConfigReq));
    memset(&sWrPkgConfigRes, 0 , sizeof(SWrPkgConfigRes));
    sWrPkgConfigReq.sHeader.u8ClientAddr = PECI_BASE_ADDR + u8Cpu;
    sWrPkgConfigReq.sHeader.u8WriteLength = sizeof(UINT32) + 6;
    sWrPkgConfigReq.sHeader.u8ReadLength = 0x01;
    sWrPkgConfigReq.u8CmdCode = 0xA5;
    sWrPkgConfigReq.u8HostID_Retry = 0x02;
    sWrPkgConfigReq.u8Index = MBX_INDEX_VCU;
    sWrPkgConfigReq.u16Parameter = VCU_CLOSE_SEQ;
    u32work = VCU_UNCORE_MCA_SEQ;
    memcpy (sWrPkgConfigReq.u8Data, &u32work, sizeof(UINT32));
    if (0 != PECI_WrPkgConfig (&sWrPkgConfigReq, &sWrPkgConfigRes))
    {
        sWrPkgConfigRes.u8CompletionCode = 0x00;
    }

    return ST_OK;
}

/******************************************************************************
*
*   uncoreStatusCboRaw
*
*   This function writes the Uncore Status CBO MCA registers to raw file
*
******************************************************************************/
#ifdef BUILD_RAW
static void uncoreStatusCboRaw(UINT8 u8Cpu, UINT32 u32CboNum, SUncoreStatusMcaRawData * psUncoreStatusCboRawData, FILE * fpRaw)
{
#ifdef SPX_BMC_ACD
    UN_USED(u8Cpu);
    UN_USED(u32CboNum);
#endif
    // Add the Uncore Status CBO MCA data to the Uncore Status dump
    fwrite(psUncoreStatusCboRawData, sizeof(SUncoreStatusMcaRawData), 1, fpRaw);
}
#endif //BUILD_RAW

/******************************************************************************
*
*   uncoreStatusCboJson
*
*   This function formats the Uncore Status CBO MCA registers into a JSON object
*
******************************************************************************/
#ifdef BUILD_JSON
static void uncoreStatusCboJson(UINT8 u8Cpu, UINT32 u32CboNum, SUncoreStatusMcaRawData * psUncoreStatusCboRawData, cJSON * pJsonChild)
{
    cJSON * socket;
    char jsonItemString[US_JSON_STRING_LEN];
    char jsonNameString[US_JSON_STRING_LEN];
    UINT32 i;

    // Add the socket number item to the Uncore Status JSON structure only if it doesn't already exist
    snprintf(jsonItemString, US_JSON_STRING_LEN, US_JSON_SOCKET_NAME, u8Cpu);
    if ((socket = cJSON_GetObjectItemCaseSensitive(pJsonChild, jsonItemString)) == NULL) {
        cJSON_AddItemToObject(pJsonChild, jsonItemString, socket = cJSON_CreateObject());
    }

    // Format the Uncore Status CBO MCA data out to the .json debug file
    // Fill in NULL for this CBO MCA if it's not valid
    if (psUncoreStatusCboRawData->bInvalid) {
        for (i = 0; i < US_NUM_MCA_QWORDS; i++) {
            snprintf(jsonNameString, US_JSON_STRING_LEN, US_CBO_MCA_REG_NAME, u32CboNum, uncoreStatusMcaRegNames[i]);
            snprintf(jsonItemString, US_JSON_STRING_LEN, "%s0x%02x", US_FAILED, psUncoreStatusCboRawData->uRegData.u32Raw[0]);
            cJSON_AddStringToObject(socket, jsonNameString, jsonItemString);
        }
    // Otherwise fill in the register data
    } else {
        for (i = 0; i < US_NUM_MCA_QWORDS; i++) {
            snprintf(jsonNameString, US_JSON_STRING_LEN, US_CBO_MCA_REG_NAME, u32CboNum, uncoreStatusMcaRegNames[i]);
            snprintf(jsonItemString, US_JSON_STRING_LEN, "0x%08x%08x", psUncoreStatusCboRawData->uRegData.u32Raw[(i * 2) + 1], psUncoreStatusCboRawData->uRegData.u32Raw[(i * 2)]);
            cJSON_AddStringToObject(socket, jsonNameString, jsonItemString);
        }
    }
}
#endif //BUILD_JSON

/******************************************************************************
*
*   uncoreStatusCbo
*
*   This function gathers the Uncore Status CBO MCA registers
*
******************************************************************************/
static ESTATUS uncoreStatusCbo(FILE * fpRaw, cJSON * pJsonChild)
{
    SRdPkgConfigReq sRdPkgConfigReq;
    SRdPkgConfigRes sRdPkgConfigRes;
    UINT32 u32NumCbos;
    UINT32 i;
    UINT8 u8Cpu;

    for (u8Cpu = CPU0_ID; u8Cpu < MAX_CPU; u8Cpu++) {
        if (FALSE == IsCpuPresent(u8Cpu)) {
            continue;
        }

        PRINT(PRINT_DBG, PRINT_INFO, "Platform Debug - Uncore Status CBO MCA log %d\n", u8Cpu);

        // Get the maximum Thread ID of the processor
        memset(&sRdPkgConfigReq, 0 , sizeof(SRdPkgConfigReq));
        memset(&sRdPkgConfigRes, 0 , sizeof(SRdPkgConfigRes));
        sRdPkgConfigReq.sHeader.u8ClientAddr = PECI_BASE_ADDR + u8Cpu;
        sRdPkgConfigReq.sHeader.u8WriteLength = 0x05;
        sRdPkgConfigReq.sHeader.u8ReadLength = sizeof(u32NumCbos) + 1;
        sRdPkgConfigReq.u8CmdCode = 0xA1;
        sRdPkgConfigReq.u8HostID_Retry = 0x02;
        sRdPkgConfigReq.u8Index = MBX_INDEX_CPU_ID;
        sRdPkgConfigReq.u16Parameter = PKG_ID_MAX_THREAD_ID;
        if (0 == PECI_RdPkgConfig (&sRdPkgConfigReq, &sRdPkgConfigRes))
        {
            memcpy(&u32NumCbos, sRdPkgConfigRes.u8Data, sizeof(u32NumCbos));
        }
        else
        {
            sRdPkgConfigRes.u8CompletionCode = 0x00;
        }
        if (sRdPkgConfigRes.u8CompletionCode == PECI_CC_SUCCESS) {
            // Convert max thread ID to number of CBOs
            u32NumCbos = (u32NumCbos / 2) + 1;
            PRINT(PRINT_DBG, PRINT_INFO, "[DBG] - CPU #%d has %d CBOs\n", u8Cpu, u32NumCbos);
        } else {
            PRINT(PRINT_DBG, PRINT_ERROR, "[DBG] - Failed to get number of CBOs for CPU #%d\n", u8Cpu);
            continue;
        }

        // Go through each CBO in this CPU
        for (i = 0; i < u32NumCbos; i++) {
#ifndef SPX_BMC_ACD
            SUncoreStatusMcaRawData sMcaData = {};
#else
            SUncoreStatusMcaRawData sMcaData;
			memset(&sMcaData, 0x0, sizeof(SUncoreStatusMcaRawData));
#endif

            // Build the MCA parameter for this CBO
            UINT32 u32CboParam = ((i / US_NUM_CBO_BANKS) << 24) | US_MCA_UNMERGE | ((i % US_NUM_CBO_BANKS) + US_BASE_CBO_BANK);

            // Get the CBO MCA data
            if (uncoreStatusMcaRead(u8Cpu, u32CboParam, &sMcaData) != ST_OK) {
                sMcaData.bInvalid = TRUE;
            } else {
                PRINT(PRINT_DBG, PRINT_INFO, "[DBG] - CPU #%d CBO %d logged\n", u8Cpu, i);
            }

            // Log the MCA for this CBO
#ifdef BUILD_RAW
            uncoreStatusCboRaw(u8Cpu, i, &sMcaData, fpRaw);
#endif //BUILD_RAW
#ifdef BUILD_JSON
            uncoreStatusCboJson(u8Cpu, i, &sMcaData, pJsonChild);
#endif //BUILD_JSON
        }
    }

    return ST_OK;
}

/******************************************************************************
*
*   uncoreStatusIioRaw
*
*   This function writes the Uncore Status IIO MCA registers to raw file
*
******************************************************************************/
#ifdef BUILD_RAW
static void uncoreStatusIioRaw(UINT8 u8Cpu, const char * regName, SUncoreStatusMcaRawData * sMcaData, FILE * fpRaw)
{
#ifdef SPX_BMC_ACD
    UN_USED(u8Cpu);
    UN_USED(regName);
#endif
    // Add the Uncore Status IIO MCA data to the Uncore Status dump
    fwrite(sMcaData, sizeof(SUncoreStatusMcaRawData), 1, fpRaw);
}
#endif //BUILD_RAW

/******************************************************************************
*
*   uncoreStatusIioJson
*
*   This function formats the Uncore Status IIO MCA registers into a JSON object
*
******************************************************************************/
#ifdef BUILD_JSON
static void uncoreStatusIioJson(UINT8 u8Cpu, const char * regName, SUncoreStatusMcaRawData * sMcaData, cJSON * pJsonChild)
{
    cJSON * socket;
    char jsonItemString[US_JSON_STRING_LEN];
    char jsonNameString[US_JSON_STRING_LEN];
    UINT32 i;

    // Add the socket number item to the Uncore Status JSON structure only if it doesn't already exist
    snprintf(jsonItemString, US_JSON_STRING_LEN, US_JSON_SOCKET_NAME, u8Cpu);
    if ((socket = cJSON_GetObjectItemCaseSensitive(pJsonChild, jsonItemString)) == NULL) {
        cJSON_AddItemToObject(pJsonChild, jsonItemString, socket = cJSON_CreateObject());
    }

    // Format the Uncore Status IIO MCA data out to the .json debug file
    // Fill in NULL for this IIO MCA if it's not valid
    if (sMcaData->bInvalid) {
        for (i = 0; i < US_NUM_MCA_QWORDS; i++) {
            snprintf(jsonNameString, US_JSON_STRING_LEN, regName, uncoreStatusMcaRegNames[i]);
            snprintf(jsonItemString, US_JSON_STRING_LEN, "%s0x%02x", US_FAILED, sMcaData->uRegData.u32Raw[0]);
            cJSON_AddStringToObject(socket, jsonNameString, jsonItemString);
        }
    // Otherwise fill in the register data
    } else {
        for (i = 0; i < US_NUM_MCA_QWORDS; i++) {
            snprintf(jsonNameString, US_JSON_STRING_LEN, regName, uncoreStatusMcaRegNames[i]);
            snprintf(jsonItemString, US_JSON_STRING_LEN, "0x%08x%08x", sMcaData->uRegData.u32Raw[(i * 2) + 1], sMcaData->uRegData.u32Raw[(i * 2)]);
            cJSON_AddStringToObject(socket, jsonNameString, jsonItemString);
        }
    }
}
#endif //BUILD_JSON

/******************************************************************************
*
*   uncoreStatusIio
*
*   This function gathers the Uncore Status IIO MCA registers
*
******************************************************************************/
static ESTATUS uncoreStatusIio(FILE * fpRaw, cJSON * pJsonChild)
{
    UINT32 i;
    UINT8 u8Cpu;

#ifdef SPX_BMC_ACD
    UN_USED(u8Cpu);
#endif

    for (u8Cpu = CPU0_ID; u8Cpu < MAX_CPU; u8Cpu++) {
        if (FALSE == IsCpuPresent(u8Cpu)) {
            continue;
        }

        PRINT(PRINT_DBG, PRINT_INFO, "Platform Debug - Uncore Status IIO MCA log %d\n", u8Cpu);

        // Go through each IIO in this CPU
        for (i = 0; i < (sizeof(sUncoreStatusIio) / sizeof(SUncoreStatusRegIio)); i++) {
#ifndef SPX_BMC_ACD
            SUncoreStatusMcaRawData sMcaData = {};
#else
            SUncoreStatusMcaRawData sMcaData;
			memset(&sMcaData, 0x0, sizeof(SUncoreStatusMcaRawData));
#endif
            // Build the MCA parameter for this IIO
            UINT32 u32IioParam = (sUncoreStatusIio[i].u8IioNum << 24 | US_MCA_UNMERGE | US_BASE_IIO_BANK);

            // Get the IIO MCA data
            if (uncoreStatusMcaRead(u8Cpu, u32IioParam, &sMcaData) != ST_OK) {
                sMcaData.bInvalid = TRUE;
            } else {
                PRINT(PRINT_DBG, PRINT_INFO, "[DBG] - CPU #%d IIO %d logged\n", u8Cpu, sUncoreStatusIio[i].u8IioNum);
            }

            // Log the MCA for this IIO
#ifdef BUILD_RAW
            uncoreStatusIioRaw(u8Cpu, sUncoreStatusIio[i].regName, &sMcaData, fpRaw);
#endif //BUILD_RAW
#ifdef BUILD_JSON
            uncoreStatusIioJson(u8Cpu, sUncoreStatusIio[i].regName, &sMcaData, pJsonChild);
#endif //BUILD_JSON
        }
    }

    return ST_OK;
}

/******************************************************************************
*
*   uncoreStatusCrashdumpRaw
*
*   This function writes the Uncore Status Crashdump registers to raw file
*
******************************************************************************/
#ifdef BUILD_RAW
static void uncoreStatusCrashdumpRaw(UINT8 u8Cpu, UINT32 u32NumReads, UINT32 * pu32UncoreCrashdump, FILE * fpRaw)
{
#ifdef SPX_BMC_ACD
    UN_USED(u8Cpu);
#endif
    // Add the u32NumReads to the Uncore Status dump
    fwrite(&u32NumReads, sizeof(u32NumReads), 1, fpRaw);

    // Add the Uncore Crashdump dump info to the Uncore Status dump
    fwrite(pu32UncoreCrashdump, (sizeof(UINT32) * u32NumReads), 1, fpRaw);
}
#endif //BUILD_RAW

/******************************************************************************
*
*   uncoreStatusCrashdumpJson
*
*   This function formats the Uncore Status Crashdump into a JSON object
*
******************************************************************************/
#ifdef BUILD_JSON
static void uncoreStatusCrashdumpJson(UINT8 u8Cpu, UINT32 u32NumReads, UINT32 * pu32UncoreCrashdump, cJSON * pJsonChild)
{
    UINT32 i;
    cJSON * socket;
    char jsonItemString[US_JSON_STRING_LEN];
    char jsonNameString[US_JSON_STRING_LEN];

    // Add the socket number item to the Uncore Status JSON structure only if it doesn't already exist
    snprintf(jsonItemString, US_JSON_STRING_LEN, US_JSON_SOCKET_NAME, u8Cpu);
    if ((socket = cJSON_GetObjectItemCaseSensitive(pJsonChild, jsonItemString)) == NULL) {
        cJSON_AddItemToObject(pJsonChild, jsonItemString, socket = cJSON_CreateObject());
    }

    // Add the Uncore Crashdump dump info to the Uncore Status dump JSON structure
    for (i = 0; i < u32NumReads; i++) {
        snprintf(jsonNameString, US_JSON_STRING_LEN, US_UNCORE_CRASH_DW_NAME, i);
        snprintf(jsonItemString, US_JSON_STRING_LEN, "0x%x", pu32UncoreCrashdump[i]);
        cJSON_AddStringToObject(socket, jsonNameString, jsonItemString);
    }
}
#endif //BUILD_JSON

/******************************************************************************
*
*   uncoreStatusCrashdump
*
*   This function gathers the Uncore Status Crashdump
*
******************************************************************************/
static ESTATUS uncoreStatusCrashdump(FILE * fpRaw, cJSON * pJsonChild)
{
    SRdPkgConfigReq sRdPkgConfigReq;
    SRdPkgConfigRes sRdPkgConfigRes;
    SWrPkgConfigReq sWrPkgConfigReq;
    SWrPkgConfigRes sWrPkgConfigRes;
    UINT32 u32work;
    ESTATUS eStatus = ST_OK;
    UINT32 u32NumReads = 0;
    UINT32 u32ApiVersion = 0;
    UINT32 i;
    UINT8 u8Cpu;

    // Go through all CPUs
    for (u8Cpu = CPU0_ID; u8Cpu < MAX_CPU; u8Cpu++) {
        if (!IsCpuPresent(u8Cpu)) {
            continue;
        }
        // Start the Uncore Crashdump dump log
        PRINT(PRINT_DBG, PRINT_INFO, "Platform Debug - Uncore Crashdump dump log %d\n", u8Cpu);

        // Open the Uncore Crashdump dump sequence
        PRINT(PRINT_DBG, PRINT_INFO, "[DBG] - CPU #%d Uncore Crashdump Dump Sequence Opened\n", u8Cpu);
        memset(&sWrPkgConfigReq, 0 , sizeof(SWrPkgConfigReq));
        memset(&sWrPkgConfigRes, 0 , sizeof(SWrPkgConfigRes));
        sWrPkgConfigReq.sHeader.u8ClientAddr = PECI_BASE_ADDR + u8Cpu;
        sWrPkgConfigReq.sHeader.u8WriteLength = sizeof(UINT32) + 6;
        sWrPkgConfigReq.sHeader.u8ReadLength = 0x01;
        sWrPkgConfigReq.u8CmdCode = 0xA5;
        sWrPkgConfigReq.u8HostID_Retry = 0x02;
        sWrPkgConfigReq.u8Index = MBX_INDEX_VCU;
        sWrPkgConfigReq.u16Parameter = VCU_OPEN_SEQ;
        u32work = VCU_UNCORE_CRASHDUMP_SEQ;
        memcpy (sWrPkgConfigReq.u8Data, &u32work, sizeof(UINT32));
        if (0 != PECI_WrPkgConfig (&sWrPkgConfigReq, &sWrPkgConfigRes))
        {
            sWrPkgConfigRes.u8CompletionCode = 0x00;
        }
        if (sWrPkgConfigRes.u8CompletionCode != PECI_CC_SUCCESS) {
            // Uncore Crashdump dump sequence failed, abort the sequence and go to the next CPU
            memset(&sWrPkgConfigReq, 0 , sizeof(SWrPkgConfigReq));
            memset(&sWrPkgConfigRes, 0 , sizeof(SWrPkgConfigRes));
            sWrPkgConfigReq.sHeader.u8ClientAddr = PECI_BASE_ADDR + u8Cpu;
            sWrPkgConfigReq.sHeader.u8WriteLength = sizeof(UINT32) + 6;
            sWrPkgConfigReq.sHeader.u8ReadLength = 0x01;
            sWrPkgConfigReq.u8CmdCode = 0xA5;
            sWrPkgConfigReq.u8HostID_Retry = 0x02;
            sWrPkgConfigReq.u8Index = MBX_INDEX_VCU;
            sWrPkgConfigReq.u16Parameter = VCU_ABORT_SEQ;
            u32work = VCU_UNCORE_CRASHDUMP_SEQ;
            memcpy (sWrPkgConfigReq.u8Data, &u32work, sizeof(UINT32));
            if (0 != PECI_WrPkgConfig (&sWrPkgConfigReq, &sWrPkgConfigRes))
            {
                sWrPkgConfigRes.u8CompletionCode = 0x00;
            }
            PRINT(PRINT_DBG, PRINT_ERROR, "[DBG] - CPU #%d Uncore Crashdump dump Sequence Failed\n", u8Cpu);
            eStatus = ST_HW_FAILURE;
            continue;
        }

        // Set Uncore Crashdump dump parameter
        PRINT(PRINT_DBG, PRINT_INFO, "[DBG] - CPU #%d Set Uncore Crashdump Dump Parameter\n", u8Cpu);
        memset(&sWrPkgConfigReq, 0 , sizeof(SWrPkgConfigReq));
        memset(&sWrPkgConfigRes, 0 , sizeof(SWrPkgConfigRes));
        sWrPkgConfigReq.sHeader.u8ClientAddr = PECI_BASE_ADDR + u8Cpu;
        sWrPkgConfigReq.sHeader.u8WriteLength = sizeof(UINT32) + 6;
        sWrPkgConfigReq.sHeader.u8ReadLength = 0x01;
        sWrPkgConfigReq.u8CmdCode = 0xA5;
        sWrPkgConfigReq.u8HostID_Retry = 0x02;
        sWrPkgConfigReq.u8Index = MBX_INDEX_VCU;
        sWrPkgConfigReq.u16Parameter = VCU_OPEN_SEQ;
        u32work = US_UCRASH_PARAM;
        memcpy (sWrPkgConfigReq.u8Data, &u32work, sizeof(UINT32));
        if (0 != PECI_WrPkgConfig (&sWrPkgConfigReq, &sWrPkgConfigRes))
        {
            sWrPkgConfigRes.u8CompletionCode = 0x00;
        }
        if (sWrPkgConfigRes.u8CompletionCode != PECI_CC_SUCCESS) {
            // Uncore Crashdump dump sequence failed, abort the sequence and go to the next CPU
            memset(&sWrPkgConfigReq, 0 , sizeof(SWrPkgConfigReq));
            memset(&sWrPkgConfigRes, 0 , sizeof(SWrPkgConfigRes));
            sWrPkgConfigReq.sHeader.u8ClientAddr = PECI_BASE_ADDR + u8Cpu;
            sWrPkgConfigReq.sHeader.u8WriteLength = sizeof(UINT32) + 6;
            sWrPkgConfigReq.sHeader.u8ReadLength = 0x01;
            sWrPkgConfigReq.u8CmdCode = 0xA5;
            sWrPkgConfigReq.u8HostID_Retry = 0x02;
            sWrPkgConfigReq.u8Index = MBX_INDEX_VCU;
            sWrPkgConfigReq.u16Parameter = VCU_ABORT_SEQ;
            u32work = VCU_UNCORE_CRASHDUMP_SEQ;
            memcpy (sWrPkgConfigReq.u8Data, &u32work, sizeof(UINT32));
            if (0 != PECI_WrPkgConfig (&sWrPkgConfigReq, &sWrPkgConfigRes))
            {
                sWrPkgConfigRes.u8CompletionCode = 0x00;
            }
            PRINT(PRINT_DBG, PRINT_ERROR, "[DBG] - CPU #%d Uncore Crashdump dump Sequence Failed\n", u8Cpu);
            eStatus = ST_HW_FAILURE;
            continue;
        }

        // Get the number of dwords to read
        memset(&sRdPkgConfigReq, 0 , sizeof(SRdPkgConfigReq));
        memset(&sRdPkgConfigRes, 0 , sizeof(SRdPkgConfigRes));
        sRdPkgConfigReq.sHeader.u8ClientAddr = PECI_BASE_ADDR + u8Cpu;
        sRdPkgConfigReq.sHeader.u8WriteLength = 0x05;
        sRdPkgConfigReq.sHeader.u8ReadLength = sizeof(UINT32) + 1;
        sRdPkgConfigReq.u8CmdCode = 0xA1;
        sRdPkgConfigReq.u8HostID_Retry = 0x02;
        sRdPkgConfigReq.u8Index = MBX_INDEX_VCU;
        sRdPkgConfigReq.u16Parameter = US_UCRASH_START;
        if (0 == PECI_RdPkgConfig (&sRdPkgConfigReq, &sRdPkgConfigRes))
        {
            memcpy(&u32NumReads, sRdPkgConfigRes.u8Data, sizeof(UINT32));
        }
        else
        {
            sRdPkgConfigRes.u8CompletionCode = 0x00;
        }
        if (sRdPkgConfigRes.u8CompletionCode != PECI_CC_SUCCESS) {
            // Uncore Crashdump dump sequence failed, abort the sequence and go to the next CPU
            memset(&sWrPkgConfigReq, 0 , sizeof(SWrPkgConfigReq));
            memset(&sWrPkgConfigRes, 0 , sizeof(SWrPkgConfigRes));
            sWrPkgConfigReq.sHeader.u8ClientAddr = PECI_BASE_ADDR + u8Cpu;
            sWrPkgConfigReq.sHeader.u8WriteLength = sizeof(UINT32) + 6;
            sWrPkgConfigReq.sHeader.u8ReadLength = 0x01;
            sWrPkgConfigReq.u8CmdCode = 0xA5;
            sWrPkgConfigReq.u8HostID_Retry = 0x02;
            sWrPkgConfigReq.u8Index = MBX_INDEX_VCU;
            sWrPkgConfigReq.u16Parameter = VCU_ABORT_SEQ;
            u32work = VCU_UNCORE_CRASHDUMP_SEQ;
            memcpy (sWrPkgConfigReq.u8Data, &u32work, sizeof(UINT32));
            if (0 != PECI_WrPkgConfig (&sWrPkgConfigReq, &sWrPkgConfigRes))
            {
                sWrPkgConfigRes.u8CompletionCode = 0x00;
            }
            PRINT(PRINT_DBG, PRINT_ERROR, "[DBG] - CPU #%d Uncore Crashdump dump Sequence Failed\n", u8Cpu);
            eStatus = ST_HW_FAILURE;
            continue;
        }

        // Get the API version number
        memset(&sRdPkgConfigReq, 0 , sizeof(SRdPkgConfigReq));
        memset(&sRdPkgConfigRes, 0 , sizeof(SRdPkgConfigRes));
        sRdPkgConfigReq.sHeader.u8ClientAddr = PECI_BASE_ADDR + u8Cpu;
        sRdPkgConfigReq.sHeader.u8WriteLength = 0x05;
        sRdPkgConfigReq.sHeader.u8ReadLength = sizeof(UINT32) + 1;
        sRdPkgConfigReq.u8CmdCode = 0xA1;
        sRdPkgConfigReq.u8HostID_Retry = 0x02;
        sRdPkgConfigReq.u8Index = MBX_INDEX_VCU;
        sRdPkgConfigReq.u16Parameter = VCU_READ;
        if (0 == PECI_RdPkgConfig (&sRdPkgConfigReq, &sRdPkgConfigRes))
        {
            memcpy(&u32ApiVersion, sRdPkgConfigRes.u8Data, sizeof(UINT32));
        }
        else
        {
            sRdPkgConfigRes.u8CompletionCode = 0x00;
        }
        if (sRdPkgConfigRes.u8CompletionCode != PECI_CC_SUCCESS) {
            // Uncore Crashdump dump sequence failed, abort the sequence and go to the next CPU
            memset(&sWrPkgConfigReq, 0 , sizeof(SWrPkgConfigReq));
            memset(&sWrPkgConfigRes, 0 , sizeof(SWrPkgConfigRes));
            sWrPkgConfigReq.sHeader.u8ClientAddr = PECI_BASE_ADDR + u8Cpu;
            sWrPkgConfigReq.sHeader.u8WriteLength = sizeof(UINT32) + 6;
            sWrPkgConfigReq.sHeader.u8ReadLength = 0x01;
            sWrPkgConfigReq.u8CmdCode = 0xA5;
            sWrPkgConfigReq.u8HostID_Retry = 0x02;
            sWrPkgConfigReq.u8Index = MBX_INDEX_VCU;
            sWrPkgConfigReq.u16Parameter = VCU_ABORT_SEQ;
            u32work = VCU_UNCORE_CRASHDUMP_SEQ;
            memcpy (sWrPkgConfigReq.u8Data, &u32work, sizeof(UINT32));
            if (0 != PECI_WrPkgConfig (&sWrPkgConfigReq, &sWrPkgConfigRes))
            {
                sWrPkgConfigRes.u8CompletionCode = 0x00;
            }
            PRINT(PRINT_DBG, PRINT_ERROR, "[DBG] - CPU #%d Uncore Crashdump dump Sequence Failed\n", u8Cpu);
            eStatus = ST_HW_FAILURE;
            continue;
        }
        // API version is included in the number of reads, so decrement by one
        u32NumReads--;

        // Get the raw data
        UINT32 * pu32UncoreCrashdump = (UINT32 *)calloc(u32NumReads, sizeof(UINT32));
        if (pu32UncoreCrashdump == NULL) {
            // calloc failed, abort the sequence and go to the next CPU
            memset(&sWrPkgConfigReq, 0 , sizeof(SWrPkgConfigReq));
            memset(&sWrPkgConfigRes, 0 , sizeof(SWrPkgConfigRes));
            sWrPkgConfigReq.sHeader.u8ClientAddr = PECI_BASE_ADDR + u8Cpu;
            sWrPkgConfigReq.sHeader.u8WriteLength = sizeof(UINT32) + 6;
            sWrPkgConfigReq.sHeader.u8ReadLength = 0x01;
            sWrPkgConfigReq.u8CmdCode = 0xA5;
            sWrPkgConfigReq.u8HostID_Retry = 0x02;
            sWrPkgConfigReq.u8Index = MBX_INDEX_VCU;
            sWrPkgConfigReq.u16Parameter = VCU_ABORT_SEQ;
            u32work = VCU_UNCORE_CRASHDUMP_SEQ;
            memcpy (sWrPkgConfigReq.u8Data, &u32work, sizeof(UINT32));
            if (0 != PECI_WrPkgConfig (&sWrPkgConfigReq, &sWrPkgConfigRes))
            {
                sWrPkgConfigRes.u8CompletionCode = 0x00;
            }
            PRINT(PRINT_DBG, PRINT_ERROR, "[DBG] - CPU #%d Uncore Crashdump dump Sequence Failed\n", u8Cpu);
            eStatus = ST_HW_FAILURE;
            continue;
        }
        for (i = 0; i < u32NumReads; i++) {
            memset(&sRdPkgConfigReq, 0 , sizeof(SRdPkgConfigReq));
            memset(&sRdPkgConfigRes, 0 , sizeof(SRdPkgConfigRes));
            sRdPkgConfigReq.sHeader.u8ClientAddr = PECI_BASE_ADDR + u8Cpu;
            sRdPkgConfigReq.sHeader.u8WriteLength = 0x05;
            sRdPkgConfigReq.sHeader.u8ReadLength = sizeof(UINT32) + 1;
            sRdPkgConfigReq.u8CmdCode = 0xA1;
            sRdPkgConfigReq.u8HostID_Retry = 0x02;
            sRdPkgConfigReq.u8Index = MBX_INDEX_VCU;
            sRdPkgConfigReq.u16Parameter = VCU_READ;
            if (0 == PECI_RdPkgConfig (&sRdPkgConfigReq, &sRdPkgConfigRes))
            {
                memcpy(&pu32UncoreCrashdump[i], sRdPkgConfigRes.u8Data, sizeof(UINT32));
            }
            else
            {
                sRdPkgConfigRes.u8CompletionCode = 0x00;
            }
            if (sRdPkgConfigRes.u8CompletionCode != PECI_CC_SUCCESS) {
                // Uncore Crashdump dump sequence failed, note the number of dwords read and abort the sequence
                memset(&sWrPkgConfigReq, 0 , sizeof(SWrPkgConfigReq));
                memset(&sWrPkgConfigRes, 0 , sizeof(SWrPkgConfigRes));
                sWrPkgConfigReq.sHeader.u8ClientAddr = PECI_BASE_ADDR + u8Cpu;
                sWrPkgConfigReq.sHeader.u8WriteLength = sizeof(UINT32) + 6;
                sWrPkgConfigReq.sHeader.u8ReadLength = 0x01;
                sWrPkgConfigReq.u8CmdCode = 0xA5;
                sWrPkgConfigReq.u8HostID_Retry = 0x02;
                sWrPkgConfigReq.u8Index = MBX_INDEX_VCU;
                sWrPkgConfigReq.u16Parameter = VCU_ABORT_SEQ;
                u32work = VCU_UNCORE_CRASHDUMP_SEQ;
                memcpy (sWrPkgConfigReq.u8Data, &u32work, sizeof(UINT32));
                if (0 != PECI_WrPkgConfig (&sWrPkgConfigReq, &sWrPkgConfigRes))
                {
                    sWrPkgConfigRes.u8CompletionCode = 0x00;
                }
                u32NumReads = i;
                break;
            }
        }

        // Close the Uncore Crashdump dump sequence
        PRINT(PRINT_DBG, PRINT_INFO, "[DBG] - CPU #%d Uncore Crashdump dump Sequence Closed\n", u8Cpu);
        memset(&sWrPkgConfigReq, 0 , sizeof(SWrPkgConfigReq));
        memset(&sWrPkgConfigRes, 0 , sizeof(SWrPkgConfigRes));
        sWrPkgConfigReq.sHeader.u8ClientAddr = PECI_BASE_ADDR + u8Cpu;
        sWrPkgConfigReq.sHeader.u8WriteLength = sizeof(UINT32) + 6;
        sWrPkgConfigReq.sHeader.u8ReadLength = 0x01;
        sWrPkgConfigReq.u8CmdCode = 0xA5;
        sWrPkgConfigReq.u8HostID_Retry = 0x02;
        sWrPkgConfigReq.u8Index = MBX_INDEX_VCU;
        sWrPkgConfigReq.u16Parameter = VCU_CLOSE_SEQ;
        u32work = VCU_UNCORE_CRASHDUMP_SEQ;
        memcpy (sWrPkgConfigReq.u8Data, &u32work, sizeof(UINT32));
        if (0 != PECI_WrPkgConfig (&sWrPkgConfigReq, &sWrPkgConfigRes))
        {
            sWrPkgConfigRes.u8CompletionCode = 0x00;
        }

        // Log the Uncore Crashdump dump for this CPU
#ifdef BUILD_RAW
        if (fpRaw!= NULL) {
            uncoreStatusCrashdumpRaw(u8Cpu, u32NumReads, pu32UncoreCrashdump, fpRaw);
        }
#endif //BUILD_RAW
#ifdef BUILD_JSON
        if (pJsonChild != NULL) {
            uncoreStatusCrashdumpJson(u8Cpu, u32NumReads, pu32UncoreCrashdump, pJsonChild);
        }
#endif //BUILD_JSON

        free(pu32UncoreCrashdump);
    }

    return eStatus;
}

static UncoreStatusRead UncoreStatusTypes[] = {
    uncoreStatusPci,
    uncoreStatusPciMmio,
    uncoreStatusCbo,
    uncoreStatusIio,
    uncoreStatusCrashdump,
};

/******************************************************************************
*
*   logUncoreStatus
*
*   This function gathers the Uncore Status register contents and adds them to the
*   debug log.
*
******************************************************************************/
ESTATUS logUncoreStatus(FILE * fpRaw, FILE * fp, cJSON * pJsonChild)
{
    ESTATUS eStatus = ST_OK;
    UINT32 i;

#ifdef SPX_BMC_ACD
    UN_USED(fp);
#endif

    for (i = 0; i < (sizeof(UncoreStatusTypes) / sizeof(UncoreStatusTypes[0])); i++) {
        if (UncoreStatusTypes[i](fpRaw, pJsonChild) != ST_OK) {
            eStatus = ST_HW_FAILURE;
        }
    }

    return eStatus;
}
