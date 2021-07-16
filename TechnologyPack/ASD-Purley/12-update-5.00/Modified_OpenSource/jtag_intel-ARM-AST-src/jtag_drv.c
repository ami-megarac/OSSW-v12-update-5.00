/********************************************************************************
* File Name     : driver/char/asped/ast_jtag.c
* Author         : Ryan Chen
* Description   : AST JTAG driver
*
* Copyright (c) 2018 Intel Corporation
* Copyright (C) 2012-2020  ASPEED Technology Inc.
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by the Free Software Foundation;
* either version 2 of the License, or (at your option) any later version.
* This program is distributed in the hope that it will be useful,  but WITHOUT ANY WARRANTY;
* without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include <linux/sysfs.h>
#include <linux/fs.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#ifdef SPX_BMC
#include <linux/version.h>
#if (LINUX_VERSION_CODE > KERNEL_VERSION (3,14,17))
#include <linux/uaccess.h>
#endif
#endif

#include <asm/io.h>
#include <asm/uaccess.h>

//AMI_CHANGE_START
//platfrom registers in regs-jtag.h regs-bmc-scu.h adapted from ASPEED sdk
#ifndef SPX_BMC
#include <plat/ast-scu.h>
#include <plat/regs-scu.h>
#else
#include "regs-jtag.h"
#include "regs-bmc-scu.h"
#if (LINUX_VERSION_CODE > KERNEL_VERSION (3,14,17))
#include <mach/hwmap.h>
#endif
#endif
//AMI_CHANGE_END
#include <mach/hardware.h>

#include "jtag_drv.h"

// #define USE_INTERRUPTS
#define JTAG_DEV_MAJOR    252

//AMI_CHANGE_START
//declared IRQ number
#ifdef SPX_BMC
#define IRQ_JTAG                                             43
#endif
//AMI_CHANGE_END
static bool __read_mostly debug = 0;

//#define DBUG_LOG_ENABLE
#ifdef DBUG_LOG_ENABLE
    #define JTAG_DBUG(fmt, args...) \
        if (debug) { \
            printk(KERN_DEBUG "%s() " fmt, __FUNCTION__, ## args); \
        }
#endif

#define JTAG_ERR(fmt, args...) printk(KERN_ERR "%s() " fmt,__FUNCTION__, ## args)

static u32 ast_scu_base = IO_ADDRESS(AST_SCU_BASE);
static DEFINE_SPINLOCK(jtag_state_lock);

/*************************************************************************************/
struct ast_jtag_info {
    void __iomem        *reg_base;
    int                 irq;            //JTAG IRQ number
    u32                 flag;
    wait_queue_head_t   jtag_wq;
    bool                is_open;
    struct cdev         cdev;
};

// this structure represents a TMS cycle, as expressed in a set of bits and a count of bits (note: there are no start->end state transitions that require more than 1 byte of TMS cycles)
typedef struct {
    unsigned char tmsbits;
    unsigned char count;
} TmsCycle;

// these are the string representations of the TAP states corresponding to the enums literals in JtagStateEncode
const char* const c_statestr[] = {"TLR", "RTI", "SelDR", "CapDR", "ShfDR", "Ex1DR", "PauDR", "Ex2DR", "UpdDR", "SelIR", "CapIR", "ShfIR", "Ex1IR", "PauIR", "Ex2IR", "UpdIR"};

struct ast_jtag_info *ast_jtag;

// this is the complete set TMS cycles for going from any TAP state to any other TAP state, following a “shortest path” rule
const TmsCycle _tmsCycleLookup[][16] = {
/*   start*/ /*TLR      RTI      SelDR    CapDR    SDR      Ex1DR    PDR      Ex2DR    UpdDR    SelIR    CapIR    SIR      Ex1IR    PIR      Ex2IR    UpdIR    destination*/
/*     TLR*/{ {0x00,0},{0x00,1},{0x02,2},{0x02,3},{0x02,4},{0x0a,4},{0x0a,5},{0x2a,6},{0x1a,5},{0x06,3},{0x06,4},{0x06,5},{0x16,5},{0x16,6},{0x56,7},{0x36,6} },
/*     RTI*/{ {0x07,3},{0x00,0},{0x01,1},{0x01,2},{0x01,3},{0x05,3},{0x05,4},{0x15,5},{0x0d,4},{0x03,2},{0x03,3},{0x03,4},{0x0b,4},{0x0b,5},{0x2b,6},{0x1b,5} },
/*   SelDR*/{ {0x03,2},{0x03,3},{0x00,0},{0x00,1},{0x00,2},{0x02,2},{0x02,3},{0x0a,4},{0x06,3},{0x01,1},{0x01,2},{0x01,3},{0x05,3},{0x05,4},{0x15,5},{0x0d,4} },
/*   CapDR*/{ {0x1f,5},{0x03,3},{0x07,3},{0x00,0},{0x00,1},{0x01,1},{0x01,2},{0x05,3},{0x03,2},{0x0f,4},{0x0f,5},{0x0f,6},{0x2f,6},{0x2f,7},{0xaf,8},{0x6f,7} },
/*     SDR*/{ {0x1f,5},{0x03,3},{0x07,3},{0x07,4},{0x00,0},{0x01,1},{0x01,2},{0x05,3},{0x03,2},{0x0f,4},{0x0f,5},{0x0f,6},{0x2f,6},{0x2f,7},{0xaf,8},{0x6f,7} },
/*   Ex1DR*/{ {0x0f,4},{0x01,2},{0x03,2},{0x03,3},{0x02,3},{0x00,0},{0x00,1},{0x02,2},{0x01,1},{0x07,3},{0x07,4},{0x07,5},{0x17,5},{0x17,6},{0x57,7},{0x37,6} },
/*     PDR*/{ {0x1f,5},{0x03,3},{0x07,3},{0x07,4},{0x01,2},{0x05,3},{0x00,0},{0x01,1},{0x03,2},{0x0f,4},{0x0f,5},{0x0f,6},{0x2f,6},{0x2f,7},{0xaf,8},{0x6f,7} },
/*   Ex2DR*/{ {0x0f,4},{0x01,2},{0x03,2},{0x03,3},{0x00,1},{0x02,2},{0x02,3},{0x00,0},{0x01,1},{0x07,3},{0x07,4},{0x07,5},{0x17,5},{0x17,6},{0x57,7},{0x37,6} },
/*   UpdDR*/{ {0x07,3},{0x00,1},{0x01,1},{0x01,2},{0x01,3},{0x05,3},{0x05,4},{0x15,5},{0x00,0},{0x03,2},{0x03,3},{0x03,4},{0x0b,4},{0x0b,5},{0x2b,6},{0x1b,5} },
/*   SelIR*/{ {0x01,1},{0x01,2},{0x05,3},{0x05,4},{0x05,5},{0x15,5},{0x15,6},{0x55,7},{0x35,6},{0x00,0},{0x00,1},{0x00,2},{0x02,2},{0x02,3},{0x0a,4},{0x06,3} },
/*   CapIR*/{ {0x1f,5},{0x03,3},{0x07,3},{0x07,4},{0x07,5},{0x17,5},{0x17,6},{0x57,7},{0x37,6},{0x0f,4},{0x00,0},{0x00,1},{0x01,1},{0x01,2},{0x05,3},{0x03,2} },
/*     SIR*/{ {0x1f,5},{0x03,3},{0x07,3},{0x07,4},{0x07,5},{0x17,5},{0x17,6},{0x57,7},{0x37,6},{0x0f,4},{0x0f,5},{0x00,0},{0x01,1},{0x01,2},{0x05,3},{0x03,2} },
/*   Ex1IR*/{ {0x0f,4},{0x01,2},{0x03,2},{0x03,3},{0x03,4},{0x0b,4},{0x0b,5},{0x2b,6},{0x1b,5},{0x07,3},{0x07,4},{0x02,3},{0x00,0},{0x00,1},{0x02,2},{0x01,1} },
/*     PIR*/{ {0x1f,5},{0x03,3},{0x07,3},{0x07,4},{0x07,5},{0x17,5},{0x17,6},{0x57,7},{0x37,6},{0x0f,4},{0x0f,5},{0x01,2},{0x05,3},{0x00,0},{0x01,1},{0x03,2} },
/*   Ex2IR*/{ {0x0f,4},{0x01,2},{0x03,2},{0x03,3},{0x03,4},{0x0b,4},{0x0b,5},{0x2b,6},{0x1b,5},{0x07,3},{0x07,4},{0x00,1},{0x02,2},{0x02,3},{0x00,0},{0x01,1} },
/*   UpdIR*/{ {0x07,3},{0x00,1},{0x01,1},{0x01,2},{0x01,3},{0x05,3},{0x05,4},{0x15,5},{0x0d,4},{0x03,2},{0x03,3},{0x03,4},{0x0b,4},{0x0b,5},{0x2b,6},{0x00,0} },
};

char *regnames[] = {
        [AST_JTAG_DATA] = "AST_JTAG_DATA",
        [AST_JTAG_INST] = "AST_JTAG_INST",
        [AST_JTAG_CTRL] = "AST_JTAG_CTRL",
        [AST_JTAG_ISR]  = "AST_JTAG_ISR",
        [AST_JTAG_SW]   = "AST_JTAG_SW",
        [AST_JTAG_TCK]  = "AST_JTAG_TCK",
        [AST_JTAG_IDLE] = "AST_JTAG_IDLE",
};

static inline u32 ast_jtag_read(struct ast_jtag_info *ast_jtag, u32 reg) {
    u32 val;
    val = readl(ast_jtag->reg_base + reg);
#ifdef DBUG_LOG_ENABLE
    JTAG_DBUG("read:%s val = 0x%08x\n", regnames[reg], val);
#endif
    return val;
}

static inline void ast_jtag_write(struct ast_jtag_info *ast_jtag, u32 val, u32 reg) {
#ifdef DBUG_LOG_ENABLE
    JTAG_DBUG("write:%s val = 0x%08x\n", regnames[reg], val);
#endif
    writel(val, ast_jtag->reg_base + reg);
}

void ast_jtag_set_tck(struct ast_jtag_info *ast_jtag, xfer_mode mode, unsigned int tck) {
    if (tck == 0)
        tck = 1;
    else if (tck > JTAG_TCK_DIVISOR_MASK)
        tck = JTAG_TCK_DIVISOR_MASK;
    ast_jtag_write(ast_jtag, ((ast_jtag_read(ast_jtag, AST_JTAG_TCK) & ~JTAG_TCK_DIVISOR_MASK) | tck),  AST_JTAG_TCK);
}

void ast_jtag_get_tck(struct ast_jtag_info *ast_jtag, xfer_mode mode, unsigned int *tck) {
    *tck = JTAG_GET_TCK_DIVISOR(ast_jtag_read(ast_jtag, AST_JTAG_TCK));
}

// Used only in SW mode to walk the JTAG state machine.
static u8 TCK_Cycle(struct ast_jtag_info *ast_jtag, u8 TMS, u8 TDI, bool do_read) {
    u8 result = 0;
    u32 regwriteval = JTAG_SW_MODE_EN | (TMS * JTAG_SW_MODE_TMS) | (TDI * JTAG_SW_MODE_TDIO);

    // TCK = 0
    ast_jtag_write(ast_jtag, regwriteval, AST_JTAG_SW);

    ast_jtag_read(ast_jtag, AST_JTAG_SW);

    // TCK = 1
    ast_jtag_write(ast_jtag, JTAG_SW_MODE_TCK | regwriteval, AST_JTAG_SW);

    if (do_read)
        result = (ast_jtag_read(ast_jtag, AST_JTAG_SW) & JTAG_SW_MODE_TDIO) ? 1 : 0;
    return result;
}

#define WAIT_ITERATIONS 75

void ast_jtag_wait_instruction_pause_complete(struct ast_jtag_info *ast_jtag) {
#ifdef USE_INTERRUPTS
    wait_event_interruptible(ast_jtag->jtag_wq, (ast_jtag->flag == JTAG_INST_PAUSE));
    ast_jtag->flag = 0;
#else
    u32 status = 0;
    u32 iterations = 0;
    while ((status & JTAG_INST_PAUSE) == 0) {
        status = ast_jtag_read(ast_jtag, AST_JTAG_ISR);
#ifdef DBUG_LOG_ENABLE
        JTAG_DBUG("ast_jtag_wait_instruction_pause_complete = 0x%08x\n", status);
#endif
        iterations++;
        if (iterations > WAIT_ITERATIONS) {
            JTAG_ERR("ast_jtag driver timed out waiting for instruction pause complete\n");
            return;
        }
        if ((status & JTAG_DATA_COMPLETE) == 0) {
            if(iterations % 25 == 0)
                usleep_range(1 , 5);
            else
                udelay(1);
        }
    }
    // clear the JTAG_INST_PAUSE bit by writing to it.
    ast_jtag_write(ast_jtag, JTAG_INST_PAUSE | (status & 0xf), AST_JTAG_ISR);
#endif
}

void ast_jtag_wait_instruction_complete(struct ast_jtag_info *ast_jtag) {
#ifdef USE_INTERRUPTS
    wait_event_interruptible(ast_jtag->jtag_wq, (ast_jtag->flag == JTAG_INST_COMPLETE));
    ast_jtag->flag = 0;
#else
    u32 status = 0;
    u32 iterations = 0;
    while ((status & JTAG_INST_COMPLETE) == 0) {
        status = ast_jtag_read(ast_jtag, AST_JTAG_ISR);
#ifdef DBUG_LOG_ENABLE
        JTAG_DBUG("ast_jtag_wait_instruction_complete = 0x%08x\n", status);
#endif
        iterations++;
        if (iterations > WAIT_ITERATIONS) {
            JTAG_ERR("ast_jtag driver timed out waiting for instruction complete\n");
            return;
        }
        if ((status & JTAG_DATA_COMPLETE) == 0) {
            if(iterations % 25 == 0)
                usleep_range(1 , 5);
            else
                udelay(1);
        }
    }
    // clear the JTAG_INST_COMPLETE bit by writing to it.
    ast_jtag_write(ast_jtag, JTAG_INST_COMPLETE | (status & 0xf), AST_JTAG_ISR);
#endif
}

void ast_jtag_wait_data_pause_complete(struct ast_jtag_info *ast_jtag) {
#ifdef USE_INTERRUPTS
    wait_event_interruptible(ast_jtag->jtag_wq, (ast_jtag->flag == JTAG_DATA_PAUSE));
    ast_jtag->flag = 0;
#else
    u32 status = 0;
    u32 iterations = 0;
    while ((status & JTAG_DATA_PAUSE) == 0) {
        status = ast_jtag_read(ast_jtag, AST_JTAG_ISR);
#ifdef DBUG_LOG_ENABLE
        JTAG_DBUG("ast_jtag_wait_data_pause_complete = 0x%08x\n", status);
#endif
        iterations++;
        if (iterations > WAIT_ITERATIONS) {
            JTAG_ERR("ast_jtag driver timed out waiting for data pause complete\n");
            return;
        }
        if ((status & JTAG_DATA_COMPLETE) == 0) {
            if(iterations % 25 == 0)
                usleep_range(1 , 5);
            else
                udelay(1);
        }
    }
    // clear the JTAG_DATA_PAUSE bit by writing to it.
    ast_jtag_write(ast_jtag, JTAG_DATA_PAUSE | (status & 0xf), AST_JTAG_ISR);
#endif
}

void ast_jtag_wait_data_complete(struct ast_jtag_info *ast_jtag) {
#ifdef USE_INTERRUPTS
    wait_event_interruptible(ast_jtag->jtag_wq, (ast_jtag->flag == JTAG_DATA_COMPLETE));
    ast_jtag->flag = 0;
#else
    u32 status = 0;
    u32 iterations = 0;
    while ((status & JTAG_DATA_COMPLETE) == 0) {
        status = ast_jtag_read(ast_jtag, AST_JTAG_ISR);
#ifdef DBUG_LOG_ENABLE
        JTAG_DBUG("ast_jtag_wait_data_complete = 0x%08x\n", status);
#endif
        iterations++;
        if (iterations > WAIT_ITERATIONS) {
            JTAG_ERR("ast_jtag driver timed out waiting for data complete\n");
            return;
        }
        if ((status & JTAG_DATA_COMPLETE) == 0) {
            if(iterations % 25 == 0)
                usleep_range(1 , 5);
            else
                udelay(1);
        }
    }
    // clear the JTAG_DATA_COMPLETE bit by writing to it.
    ast_jtag_write(ast_jtag, JTAG_DATA_COMPLETE | (status & 0xf), AST_JTAG_ISR);
#endif
}

void ast_jtag_bitbang(struct ast_jtag_info *ast_jtag, struct tck_bitbang *bitbang) {
    bitbang->tdo = TCK_Cycle(ast_jtag, bitbang->tms, bitbang->tdi, true);
}

void reset_tap(struct ast_jtag_info *ast_jtag, xfer_mode mode) {
    unsigned char i;
    if (mode == SW_MODE) {
        // clear tap state and go back to TLR
        for (i = 0; i < 9; i++) {
            TCK_Cycle(ast_jtag, 1, 0, false);
        }
    } else {
        ast_jtag_write(ast_jtag, 0, AST_JTAG_SW);  // disable sw mode
        mdelay(1);
        ast_jtag_write(ast_jtag, JTAG_ENG_EN | JTAG_ENG_OUT_EN | JTAG_FORCE_TMS, AST_JTAG_CTRL);
        mdelay(1);
        ast_jtag_write(ast_jtag, JTAG_SW_MODE_EN | JTAG_SW_MODE_TDIO, AST_JTAG_SW);
    }
}

static int ast_jtag_set_tapstate(struct ast_jtag_info *ast_jtag, xfer_mode mode,
                                 unsigned int from_state, unsigned int to_state) {
    unsigned char i;
    unsigned char tmsbits;
    unsigned char count;

    // ensure that the requested and current tap states are within 0 to 15.
    if ((from_state >= sizeof(_tmsCycleLookup[0]) / sizeof(_tmsCycleLookup[0][0])) ||  // Column
        (to_state >= sizeof(_tmsCycleLookup) / sizeof _tmsCycleLookup[0])) {  // row
        return -1;
    }

#ifdef DBUG_LOG_ENABLE
    JTAG_DBUG("Set TAP state: %s\n", c_statestr[to_state]);
#endif

    if (mode == SW_MODE) {
        ast_jtag_write(ast_jtag, JTAG_SW_MODE_EN | JTAG_SW_MODE_TDIO, AST_JTAG_SW);  // Eanble Clock

        if (to_state == JtagTLR) {
            reset_tap(ast_jtag, mode);
        } else {
            tmsbits = _tmsCycleLookup[from_state][to_state].tmsbits;
            count = _tmsCycleLookup[from_state][to_state].count;

            if (count == 0) return 0;

            for (i = 0; i < count; i++) {
                TCK_Cycle(ast_jtag, (tmsbits & 1), 0, false);
                tmsbits >>= 1;
            }
        }
    } else if (to_state == JtagTLR) {  // HW Mode and TLR
        reset_tap(ast_jtag, mode);
    }

    return 0;
}

/******************************************************************************/
void ast_jtag_readwrite_scan(struct ast_jtag_info *ast_jtag, struct scan_xfer *scan_xfer) {
    unsigned int chunk_len = 0;
    unsigned int bit_index = 0;
    unsigned char* tdi_p = scan_xfer->tdi;
    unsigned char* tdo_p = scan_xfer->tdo;
    u32* hw_tdi_p = (u32*) scan_xfer->tdi;
    u32* hw_tdo_p = (u32*) scan_xfer->tdo;
    unsigned int length = 0;
    u32 bits_to_send=0;
    u32 bits_received=0;
    int scan_end = 0;
    bool is_IR = (scan_xfer->tap_state == JtagShfIR);

    if ((scan_xfer->tap_state != JtagShfDR) && (scan_xfer->tap_state != JtagShfIR)) {
        if (scan_xfer->tap_state < sizeof(c_statestr)/sizeof(c_statestr[0])) {
            JTAG_ERR("readwrite_scan bad current tapstate = %s\n", c_statestr[scan_xfer->tap_state]);
        }
        return;
    }
    if (scan_xfer->length == 0) {
        JTAG_ERR("readwrite_scan bad length 0\n");
        return;
    } else {
        length = scan_xfer->length;
    }

    if (scan_xfer->tdi == NULL && scan_xfer->tdi_bytes != 0) {
        JTAG_ERR("readwrite_scan null tdi with nonzero length %u!\n", scan_xfer->tdi_bytes);
        return;
    }

    if (scan_xfer->tdo == NULL && scan_xfer->tdo_bytes != 0) {
        JTAG_ERR("readwrite_scan null tdo with nonzero length %u!\n", scan_xfer->tdo_bytes);
        return;
    }

#ifdef DBUG_LOG_ENABLE
    JTAG_DBUG("In %s SHIFT %s, length = %d, scan_end = %d\n",
              scan_xfer->mode == SW_MODE ? "SW" : "HW", is_IR ? "IR" : "DR", length, scan_end);
#endif

    if(scan_xfer->mode == SW_MODE) {
        ast_jtag_write(ast_jtag, JTAG_SW_MODE_EN | JTAG_SW_MODE_TDIO, AST_JTAG_SW);  // Eanble Clock
        while (bit_index < scan_xfer->length) {
            int bit_offset = (bit_index % 8);
            int this_input_bit = 0;
            int tms_high_or_low;
            int this_output_bit;
            if (bit_index / 8 < scan_xfer->tdi_bytes) {
                // If we are on a byte boundary, increment the byte pointers
                // Don't increment on 0, pointer is already on the first byte
                if (bit_index % 8 == 0 && bit_index != 0) {
                    tdi_p++;
                }
                this_input_bit = (*tdi_p >> bit_offset) & 1;
            }
            // If this is the last bit, leave TMS high
            tms_high_or_low = (bit_index == scan_xfer->length - 1) && (scan_xfer->end_tap_state != JtagShfDR) &&
                              (scan_xfer->end_tap_state != JtagShfIR);

            this_output_bit = TCK_Cycle(ast_jtag, tms_high_or_low, this_input_bit, (tdo_p != NULL));
            // If it was the last bit in the scan and the end_tap_state is something other than shiftDR or shiftIR then go to Exit1.
            // IMPORTANT Note: if the end_tap_state is ShiftIR/DR and the next call to this function is a shiftDR/IR then the driver will not change state!
            if (tms_high_or_low) {
                scan_xfer->tap_state = (scan_xfer->tap_state == JtagShfDR) ? JtagEx1DR : JtagEx1IR;
            }
            if (tdo_p && bit_index / 8 < scan_xfer->tdo_bytes) {
                if (bit_index % 8 == 0) {
                    if (bit_index != 0) {
                        tdo_p++;
                    }
                    // Zero the output buffer before we write data to it
                    *tdo_p = 0;
                }
                *tdo_p |= this_output_bit << bit_offset;
            }
            bit_index++;
        }
        ast_jtag_set_tapstate(ast_jtag, scan_xfer->mode, scan_xfer->tap_state, scan_xfer->end_tap_state);
    } else {  // HW_MODE
        ast_jtag_write(ast_jtag, 0, AST_JTAG_SW);  // disable sw mode

        if (scan_xfer->end_tap_state == JtagPauDR || scan_xfer->end_tap_state == JtagPauIR ||
            scan_xfer->end_tap_state == JtagShfDR || scan_xfer->end_tap_state == JtagShfIR) {
            scan_end = 0;
        } else {
            scan_end = 1;
        }

        while (length > 0) {
            int is_last;

            if (length > 32) {
                chunk_len = 32;
                is_last = 0;
            } else {
                chunk_len = length;
                if (scan_end == 1) {
                    is_last = is_IR ? JTAG_LAST_INST : JTAG_LAST_DATA;
                } else {
                    is_last = 0;
                }
            }
#ifdef DBUG_LOG_ENABLE
            JTAG_DBUG("In SHIFT %s, length = %d, scan_end = %d, chunk_len=%d, is_last=%d\n", is_IR ? "IR" : "DR", length, scan_end, chunk_len, is_last);
#endif

            if (hw_tdi_p && ((scan_xfer->length - length)/8 < scan_xfer->tdi_bytes)) {
                bits_to_send = *hw_tdi_p++;
                ast_jtag_write(ast_jtag, bits_to_send, is_IR ? AST_JTAG_INST : AST_JTAG_DATA);
            } else {
                bits_to_send = 0;
                ast_jtag_write(ast_jtag, 0, is_IR ? AST_JTAG_INST : AST_JTAG_DATA);
            }

#ifdef DBUG_LOG_ENABLE
            JTAG_DBUG("In SHIFT %s, len: %d chunk_len: %d is_last: %x bits_to_send: %x\n", is_IR ? "IR" : "DR",
                   length, chunk_len, is_last, bits_to_send);
#endif

            if (is_last && scan_end) {
                ast_jtag_write(ast_jtag, JTAG_ENG_EN | JTAG_ENG_OUT_EN
                                         | is_last
                                         | (is_IR ? JTAG_SET_INST_LEN(chunk_len) : JTAG_DATA_LEN(chunk_len)),
                               AST_JTAG_CTRL);
                ast_jtag_write(ast_jtag, JTAG_ENG_EN | JTAG_ENG_OUT_EN
                                         | is_last
                                         | (is_IR ? JTAG_SET_INST_LEN(chunk_len) : JTAG_DATA_LEN(chunk_len))
                                         | (is_IR ? JTAG_INST_EN : JTAG_DATA_EN),
                               AST_JTAG_CTRL);
                if (is_IR)
                    ast_jtag_wait_instruction_complete(ast_jtag);
                else
                    ast_jtag_wait_data_complete(ast_jtag);
            } else {
                ast_jtag_write(ast_jtag, JTAG_ENG_EN | JTAG_ENG_OUT_EN
                                         | (is_IR ? JTAG_IR_UPDATE : JTAG_DR_UPDATE)
                                         | (is_IR ? JTAG_SET_INST_LEN(chunk_len) : JTAG_DATA_LEN(chunk_len)),
                               AST_JTAG_CTRL);
                ast_jtag_write(ast_jtag, JTAG_ENG_EN | JTAG_ENG_OUT_EN
                                         | (is_IR ? JTAG_IR_UPDATE : JTAG_DR_UPDATE)
                                         | (is_IR ? JTAG_SET_INST_LEN(chunk_len) : JTAG_DATA_LEN(chunk_len))
                                         | (is_IR ? JTAG_INST_EN : JTAG_DATA_EN),
                               AST_JTAG_CTRL);
                if (is_IR)
                    ast_jtag_wait_instruction_pause_complete(ast_jtag);
                else
                    ast_jtag_wait_data_pause_complete(ast_jtag);
            }

            if (hw_tdo_p) {
                bits_received = ast_jtag_read(ast_jtag, is_IR ? AST_JTAG_INST : AST_JTAG_DATA);
                bits_received >>= (32 - chunk_len);
                *hw_tdo_p++ = bits_received;
            }
#ifdef DBUG_LOG_ENABLE
            JTAG_DBUG("In SHIFT %s, len: %d chunk_len: %d is_last %x  bits_received %x\n", is_IR ? "IR" : "DR",
                      length, chunk_len, is_last, bits_received);
#endif
            length -= chunk_len;
        }
    }
}

/*************************************************************************************/
#ifdef USE_INTERRUPTS
static irqreturn_t ast_jtag_interrupt (int this_irq, void *dev_id) {
    u32 status;
    struct ast_jtag_info *ast_jtag = dev_id;

    status = ast_jtag_read(ast_jtag, AST_JTAG_ISR);

    if (status & JTAG_INST_PAUSE) {
        ast_jtag_write(ast_jtag, JTAG_INST_PAUSE | (status & 0xf), AST_JTAG_ISR);
        ast_jtag->flag = JTAG_INST_PAUSE;
    }

    if (status & JTAG_INST_COMPLETE) {
        ast_jtag_write(ast_jtag, JTAG_INST_COMPLETE | (status & 0xf), AST_JTAG_ISR);
        ast_jtag->flag = JTAG_INST_COMPLETE;
    }

    if (status & JTAG_DATA_PAUSE) {
        ast_jtag_write(ast_jtag, JTAG_DATA_PAUSE | (status & 0xf), AST_JTAG_ISR);
        ast_jtag->flag = JTAG_DATA_PAUSE;
    }

    if (status & JTAG_DATA_COMPLETE) {
        ast_jtag_write(ast_jtag, JTAG_DATA_COMPLETE | (status & 0xf),AST_JTAG_ISR);
        ast_jtag->flag = JTAG_DATA_COMPLETE;
    }

    if (ast_jtag->flag) {
        wake_up_interruptible(&ast_jtag->jtag_wq);
        return IRQ_HANDLED;
    }
    else {
        return IRQ_NONE;
    }

}
#endif

/*************************************************************************************/
static inline void ast_jtag_slave(void) {
    u32 currReg = readl((void *)(ast_scu_base + AST_SCU_RESET));
    // unlock scu
    writel(SCU_PROTECT_UNLOCK, (void *)ast_scu_base);
    writel(currReg | SCU_RESET_JTAG, (void *)(ast_scu_base + AST_SCU_RESET));
    // lock scu
    writel(0xaa,(void *)ast_scu_base);
}
//AMI_CHANGE_START
//Function adapted from ASPEED sdk
#ifdef SPX_BMC
static inline u32 
ast_scu_read(u32 reg)
{
	u32 val;
		
	val = readl((void *)(ast_scu_base + reg));
	
	//printk("ast_scu_read : ast_reg_base 0x%08lx, reg = 0x%08x, val = 0x%08x\n",(unsigned long)ast_jtag->reg_base, reg, val);
	
	return val;
}
static inline void
ast_scu_write(u32 val, u32 reg) 
{
	//printk("ast_scu_write : ast_reg_base 0x%08lx, reg = 0x%08x, val = 0x%08x\n",(unsigned long)ast_jtag->reg_base, reg, val);
#ifdef CONFIG_AST_SCU_LOCK
	//unlock 
	writel(SCU_PROTECT_UNLOCK, (void *)ast_scu_base);
	writel(val, (void *) (ast_scu_base + reg));
	//lock
	writel(0xaa,(void *)ast_scu_base);	
#else
	writel(SCU_PROTECT_UNLOCK, (void *) ast_scu_base);
	writel(val, (void *) (ast_scu_base + reg));
#endif
}

void
ast_scu_init_jtag(void)
{
	ast_scu_write(ast_scu_read(AST_SCU_RESET) | SCU_RESET_JTAG, AST_SCU_RESET);
	udelay(3);
	ast_scu_write(ast_scu_read(AST_SCU_RESET) & ~SCU_RESET_JTAG, AST_SCU_RESET);
}
#endif
//AMI_CHANGE_END
static inline void ast_jtag_master(xfer_mode mode) {
    ast_scu_init_jtag();
    ast_jtag_write(ast_jtag, JTAG_ENG_EN | JTAG_ENG_OUT_EN, AST_JTAG_CTRL);  // Eanble Clock
    ast_jtag_write(ast_jtag, JTAG_SW_MODE_EN | JTAG_SW_MODE_TDIO, AST_JTAG_SW);  // Enable SW Mode
    ast_jtag_write(ast_jtag, JTAG_INST_PAUSE | JTAG_INST_COMPLETE |
        JTAG_DATA_PAUSE | JTAG_DATA_COMPLETE |
        JTAG_INST_PAUSE_EN | JTAG_INST_COMPLETE_EN |
        JTAG_DATA_PAUSE_EN | JTAG_DATA_COMPLETE_EN,
        AST_JTAG_ISR);        // Enable Interrupt

    // When leaving Slave mode, we do not know what state the TAP is in, so
    // reset the state to TLR, regardless of the previous unknown state.
    reset_tap(ast_jtag, mode);
}

static long jtag_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    int ret = 0;
    struct ast_jtag_info *ast_jtag = file->private_data;
    void __user *argp = (void __user *)arg;
    struct tck_bitbang bitbang;
    struct scan_xfer scan_xfer;
    struct set_tck_param set_tck_param;
    struct get_tck_param get_tck_param;
    struct tap_state_param tap_state_param;
    struct controller_mode_param controller_mode_param;

    switch (cmd) {
        case AST_JTAG_SET_TCK:
            if (copy_from_user(&set_tck_param, argp, sizeof(struct set_tck_param)))
                ret = -EFAULT;
            else {
                ast_jtag_set_tck(ast_jtag, set_tck_param.mode, set_tck_param.tck);
            }
            break;
        case AST_JTAG_GET_TCK:
            if (copy_from_user(&get_tck_param, argp, sizeof(struct get_tck_param)))
                ret = -EFAULT;
            else
                ast_jtag_get_tck(ast_jtag, get_tck_param.mode, &get_tck_param.tck);
            if (copy_to_user(argp, &get_tck_param, sizeof(struct get_tck_param)))
                ret = -EFAULT;
            break;
        case AST_JTAG_BITBANG:
            if (copy_from_user(&bitbang, argp, sizeof(struct tck_bitbang)))
                ret = -EFAULT;
            else {
                if (bitbang.tms > 1 || bitbang.tdi > 1)
                    ret = -EFAULT;
                else
                    ast_jtag_bitbang(ast_jtag, &bitbang);
            }
            if (copy_to_user(argp, &bitbang, sizeof(struct tck_bitbang)))
                ret = -EFAULT;
            break;
        case AST_JTAG_SET_TAPSTATE:
            if (copy_from_user(&tap_state_param, argp, sizeof(struct tap_state_param)))
                ret = -EFAULT;
            else
                ast_jtag_set_tapstate(ast_jtag, tap_state_param.mode, tap_state_param.from_state, tap_state_param.to_state);
            break;
        case AST_JTAG_READWRITESCAN:
            if (copy_from_user(&scan_xfer, argp, sizeof(struct scan_xfer)))
                ret = -EFAULT;
            else
                ast_jtag_readwrite_scan(ast_jtag, &scan_xfer);

            // no need to copy to user if only doing a write scan
            if(scan_xfer.tdo != NULL) {
                if (copy_to_user(argp, &scan_xfer, sizeof(struct scan_xfer)))
                    ret = -EFAULT;
            }
            break;
        case AST_JTAG_SLAVECONTLR:
            if (copy_from_user(&controller_mode_param, argp, sizeof(struct controller_mode_param)))
                ret = -EFAULT;
            else {
                if (controller_mode_param.controller_mode)
                    ast_jtag_slave();
                else
                    ast_jtag_master(controller_mode_param.mode);
            }
            break;
        default:
            return -ENOTTY;
    }

    return ret;
}

static int jtag_open(struct inode *inode, struct file *file) {
    spin_lock(&jtag_state_lock);
    if (ast_jtag->is_open) {
        spin_unlock(&jtag_state_lock);
        return -EBUSY;
    }

    ast_jtag->is_open = true;
    file->private_data = ast_jtag;

    spin_unlock(&jtag_state_lock);

    return 0;
}

static int jtag_release(struct inode *inode, struct file *file) {
    struct ast_jtag_info *drvdata = file->private_data;

    spin_lock(&jtag_state_lock);

    drvdata->is_open = false;

    spin_unlock(&jtag_state_lock);

    return 0;
}

static ssize_t show_tdo(struct device *dev,
    struct device_attribute *attr, char *buf) {
    struct ast_jtag_info *ast_jtag = dev_get_drvdata(dev);

    return sprintf(buf, "%s\n", ast_jtag_read(ast_jtag, AST_JTAG_SW) & JTAG_SW_MODE_TDIO? "1":"0");
}

static DEVICE_ATTR(tdo, S_IRUGO, show_tdo, NULL);

static ssize_t store_tdi(struct device *dev,
    struct device_attribute *attr, const char *buf, size_t count) {
    u32 tdi;
    struct ast_jtag_info *ast_jtag = dev_get_drvdata(dev);

    tdi = simple_strtoul(buf, NULL, 1);

    ast_jtag_write(ast_jtag, ast_jtag_read(ast_jtag, AST_JTAG_SW) | JTAG_SW_MODE_EN | (tdi * JTAG_SW_MODE_TDIO), AST_JTAG_SW);

    return count;
}

static DEVICE_ATTR(tdi, S_IWUSR, NULL, store_tdi);

static ssize_t store_tms(struct device *dev,
    struct device_attribute *attr, const char *buf, size_t count) {
    u32 tms;
    struct ast_jtag_info *ast_jtag = dev_get_drvdata(dev);

    tms = simple_strtoul(buf, NULL, 1);

    ast_jtag_write(ast_jtag, ast_jtag_read(ast_jtag, AST_JTAG_SW) | JTAG_SW_MODE_EN | (tms * JTAG_SW_MODE_TMS), AST_JTAG_SW);

    return count;
}

static DEVICE_ATTR(tms, S_IWUSR, NULL, store_tms);

static ssize_t store_tck(struct device *dev,
    struct device_attribute *attr, const char *buf, size_t count) {
    u32 tck;
    struct ast_jtag_info *ast_jtag = dev_get_drvdata(dev);

    tck = simple_strtoul(buf, NULL, 1);

    ast_jtag_write(ast_jtag, ast_jtag_read(ast_jtag, AST_JTAG_SW) | JTAG_SW_MODE_EN | (tck * JTAG_SW_MODE_TDIO), AST_JTAG_SW);

    return count;
}

static DEVICE_ATTR(tck, S_IWUSR, NULL, store_tck);

//AMI_CHANGE_START
//ast_get_pclk function defintion is not found , to fix build error commented show_frequency definition.
#ifndef SPX_BMC
static ssize_t show_frequency(struct device *dev,
    struct device_attribute *attr, char *buf) {
    struct ast_jtag_info *ast_jtag = dev_get_drvdata(dev);
    return sprintf(buf, "Frequency : %d\n", ast_get_pclk() / (JTAG_GET_TCK_DIVISOR(ast_jtag_read(ast_jtag, AST_JTAG_TCK)) + 1));
}
#endif
//AMI_CHANGE_END

static ssize_t store_frequency(struct device *dev,
    struct device_attribute *attr, const char *buf, size_t count) {
    u32 val;
    struct ast_jtag_info *ast_jtag = dev_get_drvdata(dev);

    val = simple_strtoul(buf, NULL, 10);
    ast_jtag_set_tck(ast_jtag, HW_MODE, val);

    return count;
}

//AMI_CHANGE_START
//ast_get_pclk function defintion is not found , to fix build error commented show_frequency definition
#ifndef SPX_BMC
static DEVICE_ATTR(freq, S_IRUGO | S_IWUSR, show_frequency, store_frequency);
#else
static DEVICE_ATTR(freq, S_IRUGO | S_IWUSR, NULL, store_frequency);
#endif
//AMI_CHANGE_END

static struct attribute *jtag_sysfs_entries[] = {
    &dev_attr_freq.attr,
    &dev_attr_tck.attr,
    &dev_attr_tms.attr,
    &dev_attr_tdi.attr,
    &dev_attr_tdo.attr,
    NULL
};

static struct attribute_group jtag_attribute_group = {
    .attrs = jtag_sysfs_entries,
};

static const struct file_operations ast_jtag_fops = {
    .owner             = THIS_MODULE,
    .unlocked_ioctl    = jtag_ioctl,
    .open              = jtag_open,
    .release           = jtag_release,
};

static int ast_jtag_probe(struct platform_device *pdev) {
    struct resource *res;
    int ret=0;

#ifdef DBUG_LOG_ENABLE
    JTAG_DBUG("ast_jtag_probe load started\n");
#endif

    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if (NULL == res) {
        dev_err(&pdev->dev, "cannot get IORESOURCE_MEM\n");
        ret = -ENOENT;
        goto out;
    }

    if (!request_mem_region(res->start, resource_size(res), res->name)) {
        dev_err(&pdev->dev, "cannot reserved region\n");
        ret = -ENXIO;
        goto out;
    }

    if (!(ast_jtag = kzalloc(sizeof(struct ast_jtag_info), GFP_KERNEL))) {
        return -ENOMEM;
    }

    ast_jtag->reg_base = ioremap(res->start, resource_size(res));
    if (!ast_jtag->reg_base) {
        ret = -EIO;
        goto out_region;
    }

#ifdef USE_INTERRUPTS
    ast_jtag->irq = platform_get_irq(pdev, 0);
    if (ast_jtag->irq < 0) {
        dev_err(&pdev->dev, "no irq specified\n");
        ret = -ENOENT;
        goto out_region;
    }

    ret = request_irq(ast_jtag->irq, ast_jtag_interrupt, IRQF_SHARED, "ast-jtag", ast_jtag);
    if (ret) {
        JTAG_ERR("JTAG Unable to get IRQ");
        goto out_region;
    }
#endif

    ast_jtag->flag = 0;
    init_waitqueue_head(&ast_jtag->jtag_wq);

    cdev_init(&ast_jtag->cdev, &ast_jtag_fops);
    ast_jtag->cdev.owner = THIS_MODULE;
    ret = cdev_add(&ast_jtag->cdev, MKDEV(JTAG_DEV_MAJOR, 0), 1);
    if (ret){
        JTAG_ERR("JTAG : Can't register dev\n");
        goto out_irq;
    }

    platform_set_drvdata(pdev, ast_jtag);

    ret = sysfs_create_group(&pdev->dev.kobj, &jtag_attribute_group);
    if (ret) {
        JTAG_ERR("ast_jtag: failed to create sysfs device attributes.\n");
        return -1;
    }

    ast_jtag_slave();

    printk("JTAG driver loaded successfully!\n");

    return 0;

out_irq:
#ifdef USE_INTERRUPTS
    free_irq(ast_jtag->irq, NULL);
#endif
out_region:
    release_mem_region(res->start, res->end - res->start + 1);
out:
    printk(KERN_WARNING "ast_jtag: driver init failed (ret=%d)!\n", ret);
    return ret;
}

static int ast_jtag_remove(struct platform_device *pdev) {
    struct resource *res;
    struct ast_jtag_info *ast_jtag = platform_get_drvdata(pdev);

    if (ast_jtag == NULL)
        return 0;

#ifdef DBUG_LOG_ENABLE
    JTAG_DBUG("ast_jtag_remove\n");
#endif

    sysfs_remove_group(&pdev->dev.kobj, &jtag_attribute_group);
    cdev_del(&ast_jtag->cdev);

#ifdef USE_INTERRUPTS
    free_irq(ast_jtag->irq, ast_jtag);
#endif

    iounmap(ast_jtag->reg_base);
    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if (res != NULL)
        release_mem_region(res->start, res->end - res->start + 1);

    platform_set_drvdata(pdev, NULL);

    if (res != NULL)
        release_mem_region(res->start, res->end - res->start + 1);

    kfree(ast_jtag);

    ast_jtag_slave();

    printk("JTAG driver removed successfully!\n");

    return 0;
}

#ifdef CONFIG_PM
static int
ast_jtag_suspend(struct platform_device *pdev, pm_message_t state)
{
    return 0;
}

static int
ast_jtag_resume(struct platform_device *pdev)
{
    return 0;
}

#else
#define ast_jtag_suspend        NULL
#define ast_jtag_resume         NULL
#endif

//AMI_CHANGE_START
//Added init , exit and probe function defintion to load module.
#ifndef SPX_BMC
static struct platform_driver ast_jtag_driver = {
    .remove         = ast_jtag_remove,
    .suspend        = ast_jtag_suspend,
    .resume         = ast_jtag_resume,
    .driver         = {
        .name   = "ast-jtag",
        .owner  = THIS_MODULE,
    },
};

module_platform_driver_probe(ast_jtag_driver, ast_jtag_probe);

#else
static struct platform_driver ast_jtag_driver = {
	.probe          = ast_jtag_probe,
    .remove         = ast_jtag_remove,
    .suspend        = ast_jtag_suspend,
    .resume         = ast_jtag_resume,
    .driver         = {
        .name   = "ast-jtag",
        .owner  = THIS_MODULE,
    },
};
static struct platform_device *ast_jtag_device;

static int __init ast_jtag_init(void)
{
        int ret;
        static const struct resource ast_jtag_resources[] = {
                [0] = {
                        .start = AST_JTAG_BASE,
                        .end = AST_JTAG_BASE + (SZ_16*4) - 1,
                        .flags = IORESOURCE_MEM,
                },
                [1] = {
                        .start = IRQ_JTAG,
                        .end = IRQ_JTAG,
                        .flags = IORESOURCE_IRQ,
                },
        };

        ast_scu_init_jtag();

        ret = platform_driver_register(&ast_jtag_driver);

        if (!ret) {
                ast_jtag_device = platform_device_register_simple("ast-jtag", 0,
                                                                ast_jtag_resources, ARRAY_SIZE(ast_jtag_resources));
                if (IS_ERR(ast_jtag_device)) {
                        platform_driver_unregister(&ast_jtag_driver);
                        ret = PTR_ERR(ast_jtag_device);
                }
        }

        return ret;
}

static void __exit ast_jtag_exit(void)
{
        platform_device_unregister(ast_jtag_device);
        platform_driver_unregister(&ast_jtag_driver);
}

module_init(ast_jtag_init);
module_exit(ast_jtag_exit);

#endif
//AMI_CHANGE_END
module_param_named(debug, debug, bool, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(debug, "debug flag for tracing");

MODULE_AUTHOR("Ryan Chen <ryan_chen@aspeedtech.com>");
MODULE_DESCRIPTION("AST JTAG LIB Driver");
MODULE_LICENSE("GPL");
