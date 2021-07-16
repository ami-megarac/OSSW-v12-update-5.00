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

#ifndef _CPU_H_
#define _CPU_H_

//***************************************************************************
// Public Structure Definitions
//***************************************************************************
//
// CPU ID definitions
#define CPU0_ID 0
#define CPU1_ID 1
#define MAX_CPU 2

// CPU requests messages, that needs to be implemented by OEM CPU Code
typedef enum {
    OEM_CPUREQ_ENABLE = 0,
    OEM_CPUREQ_DISABLE,
    OEM_CPUREQ_MAX
} EOEMCPUReq;

// CPU States
typedef enum {
    CPU_STATE_ENABLE = 0,
    CPU_STATE_DISABLE
} ECPUState;

//
// This enumeration defines the offsets for the processor status sensor and
// history information. These map directly to the IPMI CPU sensor offsets.
//
typedef enum {
    CPU_IERR = 0,
    CPU_THERMAL_TRIP = 1,
    CPU_FRB1_BISTERR = 2,       // FRB 1 error Bist Failure
    CPU_FRB2_POSTERR = 3,       // FRB 2 Hang in Post failure
    CPU_FRB3_STARTERR = 4,      // FRB 3 Processor Startup/Initialization Failure
    CPU_CONFIGERR = 5,          // Configuration error DMI
    CPU_SMBIOSERR = 6,          // SM Bios error
    CPU_PRESENT = 7,            // Processor detected as present
    CPU_DISABLED = 8,           // Processor disabled - do not boot
    CPU_TERMINATOR_PRESENT = 9  // Terminator presence detected
} ECPUStateItem;

//
// This enumeration defines the offsets for the processor ERR2 Timeout sensor.
// These map directly to the IPMI CPU sensor offsets.
//
typedef enum {
    CPU_ERR2_OFFSET = 1,  // ERR2 assert
} ECPUERR2TimeoutStateItem;

//
// These defines provide the bitmask versions of the offsets
//
#define BIT_MASK(x) (1 << (x))

#define CPU_IERR_MASK BIT_MASK(CPU_IERR)
#define CPU_THERMAL_TRIP_MASK BIT_MASK(CPU_THERMAL_TRIP)
#define CPU_FRB1_BISTERR_MASK BIT_MASK(CPU_FRB1_BISTERR)
#define CPU_FRB2_POSTERR_MASK BIT_MASK(CPU_FRB2_POSTERR)
#define CPU_FRB3_STARTERR_MASK BIT_MASK(CPU_FRB3_STARTERR)
#define CPU_CONFIGERR_MASK BIT_MASK(CPU_CONFIGERR)
#define CPU_SMBIOSERR_MASK BIT_MASK(CPU_SMBIOSERR)
#define CPU_PRESENT_MASK BIT_MASK(CPU_PRESENT)
#define CPU_DISABLED_MASK BIT_MASK(CPU_DISABLED)
#define CPU_TERMINATOR_PRESENT_MASK BIT_MASK(CPU_TERMINATOR_PRESENT)

#define CPU_ALL_MASK (       \
    CPU_IERR_MASK |          \
    CPU_THERMAL_TRIP_MASK |  \
    CPU_FRB1_BISTERR_MASK |  \
    CPU_FRB2_POSTERR_MASK |  \
    CPU_FRB3_STARTERR_MASK | \
    CPU_CONFIGERR_MASK |     \
    CPU_SMBIOSERR_MASK |     \
    CPU_PRESENT_MASK |       \
    CPU_DISABLED_MASK |      \
    CPU_TERMINATOR_PRESENT_MASK)

//*****************************************************************************
// Public Members -- intramodule declarations
//*****************************************************************************

// MCA Error Source Log -Doc Ref# 443553 Sec 4.5.4.17
#define MCA_ERR_SRC_LOG_BUS_NUM (1)      // MCA Error Source Log Bus#
#define MCA_ERR_SRC_LOG_DEVICE_NUM (30)  // MCA Error Source Log Device#
#define MCA_ERR_SRC_LOG_FCN_NUM (2)      // MCA Error Source Log Function#
#define MCA_ERR_SRC_LOG_OFFSET (0xEC)    // MCA Error Source Log Offset

#define MCA_ERR_SRC_LOG_CATERR (1 << 31)         // CATERR bit
#define MCA_ERR_SRC_LOG_IERR (1 << 30)           // IERR bit
#define MCA_ERR_SRC_LOG_MCERR (1 << 29)          // MCERR bit
#define MCA_ERR_SRC_LOG_IERR_INTERNAL (1 << 27)  // IERR_INTERNAL bit
#define MCA_ERR_SRC_LOG_MSMI_INTERNAL (1 << 20)  // MSMI_INTERNAL bit
#define MCA_ERR_SRC_LOG_CORE_ERRS (0xFF)         // Core Error Mask

// MSEC [31:24] - Model Specific Error Code - Doc Ref# 546832 Sec 12.1.197
#define MCA_SVID_VCCIN_VR_ICC_MAX_FAILURE (0x40 << 24)
#define MCA_SVID_VCCIN_VR_VOUT_FAILURE (0x42 << 24)
#define MCA_SVID_CPU_VR_CAPABILITY_ERROR (0x43 << 24)

// MSCOD [31:24] - MSEC_FW Generated Error Encoding
#define MCA_FIVR_CATAS_OVERCUR_FAULT (0x52 << 24)
#define MCA_FIVR_CATAS_OVERVOL_FAULT (0x51 << 24)

// GSYSST - Global System Event Status
#define GSYSST_BUS_NUM (0)
#define GSYSST_DEVICE_NUM (8)
#define GSYSST_FCN_NUM (0)
#define GSYSST_OFFSET (0x204)

#define IIO_ERR0 (1 << 0)
#define IIO_ERR1 (1 << 1)
#define IIO_ERR2 (1 << 2)

// GFERRST - Global Fatal Error Status
#define GFERRST_BUS_NUM (0)
#define GFERRST_DEVICE_NUM (8)
#define GFERRST_FCN_NUM (0)
#define GFERRST_OFFSET (0x218)

// IIO_ERRPINSTS - IIO Error Pin Status
#define ERRPINSTS_BUS_NUM (0)
#define ERRPINSTS_DEVICE_NUM (8)
#define ERRPINSTS_FCN_NUM (0)
#define ERRPINSTS_OFFSET (0x210)

extern ESTATUS CPU_OEMRegister(STATUS (*pHandlerFunc)(EOEMCPUReq, UINT8));
extern ESTATUS CPU_SetState(UINT8 u8CPUID, ECPUState eCPUState);
extern ESTATUS CPU_GetSlotCount(UINT8* pu8SlotCount);
extern ESTATUS CPU_Is_FPGA_Platform(UINT8* pu8SupportsOnchipFPGAEnable);
extern ESTATUS CPU_GetStepping(UINT8 u8CPUID, UINT8* pu8Stepping);
#endif  //  ifndef _CPU_H_
