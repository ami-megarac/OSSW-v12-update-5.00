/*
 * JTAG driver for the Aspeed SoC
 *
 * Copyright (C) ASPEED Technology Inc.
 * Ryan Chen <ryan_chen@aspeedtech.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
#include <linux/sysfs.h>
#include <linux/fs.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/vmalloc.h>
#include <linux/nmi.h>
#include <linux/version.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include "helper.h"
#include "jtag_ioctl.h"
#include "jtag.h"

/******************************************************************************/
#define ASPEED_JTAG_DATA        0x00
#define ASPEED_JTAG_INST        0x04
#define ASPEED_JTAG_CTRL        0x08
#define ASPEED_JTAG_ISR         0x0C
#define ASPEED_JTAG_SW          0x10
#define ASPEED_JTAG_TCK         0x14
#define ASPEED_JTAG_IDLE        0x18

/* ASPEED_JTAG_CTRL - 0x08 : Engine Control */
#define JTAG_ENG_EN             (0x1 << 31)
#define JTAG_ENG_OUT_EN         (0x1 << 30)
#define JTAG_FORCE_TMS          (0x1 << 29)

#define JTAG_IR_UPDATE          (0x1 << 26)    //AST2500 only
#define JTAG_INST_LEN_MASK      (0x3f << 20)
#define JTAG_SET_INST_LEN(x)    (x << 20)
#define JTAG_SET_INST_MSB       (0x1 << 19)
#define JTAG_TERMINATE_INST     (0x1 << 18)
#define JTAG_LAST_INST          (0x1 << 17)
#define JTAG_INST_EN            (0x1 << 16)
#define JTAG_DATA_LEN_MASK      (0x3f << 4)

#define JTAG_DR_UPDATE          (0x1 << 10)    //AST2500 only
#define JTAG_DATA_LEN(x)        (x << 4)
#define JTAG_SET_DATA_MSB       (0x1 << 3)
#define JTAG_TERMINATE_DATA     (0x1 << 2)
#define JTAG_LAST_DATA          (0x1 << 1)
#define JTAG_DATA_EN            (0x1)

/* ASPEED_JTAG_ISR    - 0x0C : INterrupt status and enable */
#define JTAG_INST_PAUSE         (0x1 << 19)
#define JTAG_INST_COMPLETE      (0x1 << 18)
#define JTAG_DATA_PAUSE         (0x1 << 17)
#define JTAG_DATA_COMPLETE      (0x1 << 16)

#define JTAG_INST_PAUSE_EN      (0x1 << 3)
#define JTAG_INST_COMPLETE_EN   (0x1 << 2)
#define JTAG_DATA_PAUSE_EN      (0x1 << 1)
#define JTAG_DATA_COMPLETE_EN   (0x1)

/* ASPEED_JTAG_SW    - 0x10 : Software Mode and Status */
#define JTAG_SW_MODE_EN         (0x1 << 19)
#define JTAG_SW_MODE_TCK        (0x1 << 18)
#define JTAG_SW_MODE_TMS        (0x1 << 17)
#define JTAG_SW_MODE_TDIO       (0x1 << 16)
//
#define JTAG_STS_INST_PAUSE     (0x1 << 2)
#define JTAG_STS_DATA_PAUSE     (0x1 << 1)
#define JTAG_STS_ENG_IDLE       (0x1)

/* ASPEED_JTAG_TCK    - 0x14 : TCK Control */
#define JTAG_TCK_INVERSE        (0x1 << 31)
#define JTAG_TCK_DIVISOR_MASK   (0x7ff)
#define JTAG_GET_TCK_DIVISOR(x) (x & 0x7ff)

/*  ASPEED_JTAG_IDLE - 0x18 : Ctroller set for go to IDLE */
#define JTAG_CTRL_TRSTn_HIGH    (0x1 << 31)
#define JTAG_GO_IDLE            (0x1)

#if (LINUX_VERSION_CODE >=  KERNEL_VERSION(5,0,0))
#define IO_BASE			0xF0000000                 // VA of IO
#define IO_ADDRESS(x)   ((x&0x0fffffff)+IO_BASE)
#define IRQF_DISABLED		0x00000020
#define BW_CMP_NR_IRQS		         32 //This is the number of IRQS backward-compatible to ast2050/2150
#define IRQ_JTAG_MASTER        43
#define IRQMASK_JTAG_MASTER    (1 << (IRQ_JTAG_MASTER - BW_CMP_NR_IRQS))
#endif

int jtag_debug_init(void);
/*************************************************************************************/
int printx(const char *msg,unsigned char *x, int len)
{    
    int i=0,j=0;
    printk("\n%s %d bytes\n",msg,len);
    for(i=0;i<len;i=i+16)
    {
        if((len - i) > 15)
        printk("%02x %02x  %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x \n", x[i+0],x[i+1],x[i+2],x[i+3],x[i+4],x[i+5],x[i+6],x[i+7],x[i+8],x[i+9],x[i+10],x[i+11],x[i+12],x[i+13],x[i+14],x[i+15]);
        else {
          for(j=0;j<(len-i);j++)
                 printk("%02x ",x[i+j]);

          printk(" \n");
        }
    }
    return 0;
}

#ifdef ASPEED_JTAG_DEBUG
#define JTAG_DBUG(fmt, args...) printk(KERN_DEBUG "%s() " fmt,__FUNCTION__, ## args)
#else
#define JTAG_DBUG(fmt, args...)
#endif
#define JTAG_MSG(fmt, args...) printk(fmt, ## args)

struct aspeed_jtag_info {
    void __iomem        *reg_base;    
    u8                  sts;            //0: idle, 1:irpause 2:drpause
    int                 irq;                //JTAG IRQ number     
    struct reset_control    *reset;
    struct clk          *clk;
    u32                 apb_clk;
    u32                 flag;    
    wait_queue_head_t   jtag_wq;    
    bool                is_open;
};

static struct aspeed_jtag_info g_aspeed_jtag_info;

/*************************************************************************************/
//static DEFINE_SPINLOCK(jtag_state_lock);
spinlock_t jtag_irq_lock;
/******************************************************************************/
static inline u32
aspeed_jtag_read(struct aspeed_jtag_info *aspeed_jtag, u32 reg)
{
#if 0
	u32 val;

	val = readl(aspeed_jtag->reg_base + reg);
	JTAG_DBUG("reg = 0x%08x, val = 0x%08x\n", reg, val);
	return val;
#else
	return readl(aspeed_jtag->reg_base + reg);
#endif
}

static inline void
aspeed_jtag_write(struct aspeed_jtag_info *aspeed_jtag, u32 val, u32 reg)
{
	JTAG_DBUG("reg = 0x%08x, val = 0x%08x\n", reg, val);
	writel(val, aspeed_jtag->reg_base + reg);
}


/*************************************************************************************/
// Calculate PCLK Value
/******************************************************************************/
#define ASPEED_SCU_BASE                     0x1E6E2000
#define ASPEED_SCU_H_PLL                    0x24        /*  H-PLL Parameter register        */
#define SCU_H_PLL_OFF                       (0x1 << 19)
#define SCU_H_PLL_BYPASS_EN                 (0x1 << 20)
#define ASPEED_SCU_CLK_SEL                  0x08        /*  clock selection register    */
#define ASPEED_SCU_HW_STRAP1                0x70        /*  hardware strapping register */
#define CLK_25M_IN                          (0x1 << 23)
#define ASPEED_PLL_25MHZ                    25000000
#define ASPEED_PLL_24MHZ                    24000000


#define SCU_GET_PCLK_DIV(x)             ((x >> 23) & 0x7)
#define SCU_H_PLL_GET_MNUM(x)           ((x >> 5) & 0xff)
#define SCU_H_PLL_GET_NNUM(x)           (x & 0xf)
#define SCU_H_PLL_GET_PNUM(x)           ((x >> 13) & 0x3f)
#define SCU_GET_PCLK_DIV(x)             ((x >> 23) & 0x7)

static u32 aspeed_scu_base = IO_ADDRESS(ASPEED_SCU_BASE);

spinlock_t aspeed_scu_lock;

static inline u32 aspeed_scu_read(u32 reg)
{
    u32 val;

    val = readl((void *)(aspeed_scu_base + reg));

    JTAG_DBUG("aspeed_scu_read : reg = 0x%08x, val = 0x%08x\n", reg, val);

    return val;
}

static u32 aspeed_get_clk_source(void)
{
    if(aspeed_scu_read(ASPEED_SCU_HW_STRAP1) & CLK_25M_IN)
        return ASPEED_PLL_25MHZ;
    else
        return ASPEED_PLL_24MHZ;
}

static u32 aspeed_get_h_pll_clk(void)
{
    u32 clk=0;
    u32 h_pll_set = aspeed_scu_read(ASPEED_SCU_H_PLL);
    
    if(h_pll_set & SCU_H_PLL_OFF)
    return 0;
    
    // Programming
    clk = aspeed_get_clk_source();
    if(h_pll_set & SCU_H_PLL_BYPASS_EN) {
        return clk;
    } else {
        //P = SCU24[18:13]
        //M = SCU24[12:5]
        //N = SCU24[4:0]
        //hpll = 24MHz * [(M+1) /(N+1)] / (P+1)
        clk = ((clk * (SCU_H_PLL_GET_MNUM(h_pll_set) + 1)) / (SCU_H_PLL_GET_NNUM(h_pll_set) + 1)) /(SCU_H_PLL_GET_PNUM(h_pll_set) + 1);
    }
    JTAG_DBUG("h_pll = %d\n",clk);
    return clk;
}

static unsigned int aspeed_get_pclk(void)
{
    unsigned int div, hpll;
    
    hpll = aspeed_get_h_pll_clk();
    div = SCU_GET_PCLK_DIV(aspeed_scu_read(ASPEED_SCU_CLK_SEL));
    if((div >> 2) == 1) {
            JTAG_DBUG("div=%d , return 24000000\n", div);
            return 24000000;
        } else {
            JTAG_DBUG("hpll=%d, Div=%d, PCLK=%d\n", hpll, div, hpll/div);
            div = (div+1) << 1;
            return (hpll/div);
    }
    
    JTAG_DBUG("HPLL=%d, Div=%d, PCLK=%d\n", hpll, div, hpll/div);
    return (hpll/div);
}

/******************************************************************************/
void aspeed_jtag_set_freq(struct aspeed_jtag_info *aspeed_jtag, unsigned int freq)
{
	u16 i;

	for (i = 0; i < 0x7ff; i++) {
//		JTAG_DBUG("[%d] : freq : %d , target : %d \n", i, aspeed_get_pclk()/(i + 1), freq);
		if ((aspeed_get_pclk() / (i + 1)) <= freq)
			break;
	}
//	printk("div = %x \n", i);
	aspeed_jtag_write(aspeed_jtag, ((aspeed_jtag_read(aspeed_jtag, ASPEED_JTAG_TCK) & ~JTAG_TCK_DIVISOR_MASK) | i),  ASPEED_JTAG_TCK);

}

unsigned int aspeed_jtag_get_freq(struct aspeed_jtag_info *aspeed_jtag)
{
	return aspeed_get_pclk() / (JTAG_GET_TCK_DIVISOR(aspeed_jtag_read(aspeed_jtag, ASPEED_JTAG_TCK)) + 1);
}
/******************************************************************************/
void dummy(struct aspeed_jtag_info *aspeed_jtag, unsigned int cnt)
{
	int i = 0;

	for (i = 0; i < cnt; i++)
		aspeed_jtag_read(aspeed_jtag, ASPEED_JTAG_SW);
}

static u8 TCK_Cycle(struct aspeed_jtag_info *aspeed_jtag, u8 TMS, u8 TDI)
{
	u8 tdo;

	// TCK = 0
	aspeed_jtag_write(aspeed_jtag, JTAG_SW_MODE_EN | (TMS * JTAG_SW_MODE_TMS) | (TDI * JTAG_SW_MODE_TDIO), ASPEED_JTAG_SW);

	dummy(aspeed_jtag, 1);

	// TCK = 1
	aspeed_jtag_write(aspeed_jtag, JTAG_SW_MODE_EN | JTAG_SW_MODE_TCK | (TMS * JTAG_SW_MODE_TMS) | (TDI * JTAG_SW_MODE_TDIO), ASPEED_JTAG_SW);

	if (aspeed_jtag_read(aspeed_jtag, ASPEED_JTAG_SW) & JTAG_SW_MODE_TDIO)
		tdo = 1;
	else
		tdo = 0;

	dummy(aspeed_jtag, 1);

	// TCK = 0
	aspeed_jtag_write(aspeed_jtag, JTAG_SW_MODE_EN | (TMS * JTAG_SW_MODE_TMS) | (TDI * JTAG_SW_MODE_TDIO), ASPEED_JTAG_SW);

	return tdo;
}

/******************************************************************************/
void aspeed_jtag_wait_instruction_pause_complete(struct aspeed_jtag_info *aspeed_jtag)
{
	wait_event_interruptible(aspeed_jtag->jtag_wq, (aspeed_jtag->flag == JTAG_INST_PAUSE));
	JTAG_DBUG("\n");
	aspeed_jtag->flag = 0;
}

void aspeed_jtag_wait_instruction_complete(struct aspeed_jtag_info *aspeed_jtag)
{
	wait_event_interruptible(aspeed_jtag->jtag_wq, (aspeed_jtag->flag == JTAG_INST_COMPLETE));
	JTAG_DBUG("\n");
	aspeed_jtag->flag = 0;
}

void aspeed_jtag_wait_data_pause_complete(struct aspeed_jtag_info *aspeed_jtag)
{
	wait_event_interruptible(aspeed_jtag->jtag_wq, (aspeed_jtag->flag == JTAG_DATA_PAUSE));
	JTAG_DBUG("\n");
	aspeed_jtag->flag = 0;
}

void aspeed_jtag_wait_data_complete(struct aspeed_jtag_info *aspeed_jtag)
{
	wait_event_interruptible(aspeed_jtag->jtag_wq, (aspeed_jtag->flag == JTAG_DATA_COMPLETE));
	JTAG_DBUG("\n");
	aspeed_jtag->flag = 0;
}
/******************************************************************************/
/* JTAG_reset() is to generate at least 9 TMS high and 
 * 1 TMS low to force devices into Run-Test/Idle State 
 */
void aspeed_jtag_run_test_idle(struct aspeed_jtag_info *aspeed_jtag, struct runtest_idle *runtest)
{
	int i = 0;

	JTAG_DBUG(":%s mode\n", runtest->mode ? "SW" : "HW");

	if (runtest->mode) {
		//SW mode
		//from idle , from pause,  -- > to pause, to idle

		if (runtest->reset) {
			for (i = 0; i < 10; i++)
				TCK_Cycle(aspeed_jtag, 1, 0);
		}

		switch (aspeed_jtag->sts) {
		case 0:
			if (runtest->end == 1) {
				TCK_Cycle(aspeed_jtag, 1, 0);	// go to DRSCan
				TCK_Cycle(aspeed_jtag, 1, 0);	// go to IRSCan
				TCK_Cycle(aspeed_jtag, 0, 0);	// go to IRCap
				TCK_Cycle(aspeed_jtag, 1, 0);	// go to IRExit1
				TCK_Cycle(aspeed_jtag, 0, 0);	// go to IRPause
				aspeed_jtag->sts = 1;
			} else if (runtest->end == 2) {
				TCK_Cycle(aspeed_jtag, 1, 0);	// go to DRSCan
				TCK_Cycle(aspeed_jtag, 0, 0);	// go to DRCap
				TCK_Cycle(aspeed_jtag, 1, 0);	// go to DRExit1
				TCK_Cycle(aspeed_jtag, 0, 0);	// go to DRPause
				aspeed_jtag->sts = 1;
			} else {
				TCK_Cycle(aspeed_jtag, 0, 0);	// go to IDLE
				aspeed_jtag->sts = 0;
			}
			break;
		case 1:
			//from IR/DR Pause
			if (runtest->end == 1) {
				TCK_Cycle(aspeed_jtag, 1, 0);	// go to Exit2 IR / DR
				TCK_Cycle(aspeed_jtag, 1, 0);	// go to Update IR /DR
				TCK_Cycle(aspeed_jtag, 1, 0);	// go to DRSCan
				TCK_Cycle(aspeed_jtag, 1, 0);	// go to IRSCan
				TCK_Cycle(aspeed_jtag, 0, 0);	// go to IRCap
				TCK_Cycle(aspeed_jtag, 1, 0);	// go to IRExit1
				TCK_Cycle(aspeed_jtag, 0, 0);	// go to IRPause
				aspeed_jtag->sts = 1;
			} else if (runtest->end == 2) {
				TCK_Cycle(aspeed_jtag, 1, 0);	// go to Exit2 IR / DR
				TCK_Cycle(aspeed_jtag, 1, 0);	// go to Update IR /DR
				TCK_Cycle(aspeed_jtag, 1, 0);	// go to DRSCan
				TCK_Cycle(aspeed_jtag, 0, 0);	// go to DRCap
				TCK_Cycle(aspeed_jtag, 1, 0);	// go to DRExit1
				TCK_Cycle(aspeed_jtag, 0, 0);	// go to DRPause
				aspeed_jtag->sts = 1;
			} else {
				TCK_Cycle(aspeed_jtag, 1, 0);	// go to Exit2 IR / DR
				TCK_Cycle(aspeed_jtag, 1, 0);	// go to Update IR /DR
				TCK_Cycle(aspeed_jtag, 0, 0);	// go to IDLE
				aspeed_jtag->sts = 0;
			}
			break;
		default:
			printk("TODO check ERROR \n");
			break;
		}

		for (i = 0; i < runtest->tck; i++)
			TCK_Cycle(aspeed_jtag, 0, 0);	// stay on IDLE for at lease  TCK cycle

	} else {
		aspeed_jtag_write(aspeed_jtag, 0, ASPEED_JTAG_SW);  //dis sw mode
		mdelay(1);
		if (runtest->reset)
			aspeed_jtag_write(aspeed_jtag, JTAG_ENG_EN | JTAG_ENG_OUT_EN | JTAG_FORCE_TMS, ASPEED_JTAG_CTRL);	// x TMS high + 1 TMS low
		else
			aspeed_jtag_write(aspeed_jtag, JTAG_GO_IDLE, ASPEED_JTAG_IDLE);
		mdelay(1);
		aspeed_jtag_write(aspeed_jtag, JTAG_SW_MODE_EN | JTAG_SW_MODE_TDIO, ASPEED_JTAG_SW);
		aspeed_jtag->sts = 0;
	}
}

int aspeed_jtag_sir_xfer(struct aspeed_jtag_info *aspeed_jtag, struct sir_xfer *sir)
{
	unsigned int i = 0;
	u32 remainbits = 0;
	u32 index = 0;

	// IRQ_SAVE fixes
	unsigned long jtag_flags = 0;
	u32 irq_save_flag;

	irq_save_flag = 0;

	JTAG_DBUG("%s mode, ENDIR : %d, len : %d \n", sir->mode? "SW":"HW", sir->endir, sir->length);

	remainbits = sir->length;
	if(sir->mode) {
		if(aspeed_jtag->sts) {
			//from IR/DR Pause 
			TCK_Cycle(aspeed_jtag, 1, 0);		// go to Exit2 IR / DR
			//TCK_Cycle(aspeed_jtag, 1, 0);		// go to Update IR /DR
		}
		
		if(aspeed_jtag->sts == 0)
		{
			TCK_Cycle(aspeed_jtag, 1, 0);		// go to DRSCan		
			TCK_Cycle(aspeed_jtag, 1, 0);		// go to IRSCan
			TCK_Cycle(aspeed_jtag, 0, 0);		// go to CapIR
		}
		TCK_Cycle(aspeed_jtag, 0, 0);		// go to ShiftIR

		while (remainbits) {
			while(remainbits > 32) {
				JTAG_DBUG("W ir->tdi[%d]: %x\n", index, sir->tdi[index]);
				sir->tdo[index]=0;
				for(i=0; i<32; i++) {
					sir->tdo[index] |= (TCK_Cycle(aspeed_jtag, 0, sir->tdi[index] & 0x1)<<i); // go to ShiftIR
					sir->tdi[index] >>= 1;
				}
				JTAG_DBUG("R ir->tdo: %x\n",sir->tdo[index]);
				remainbits = remainbits - 32;
				index++;
			}
		
			if(remainbits != 0) {
				JTAG_DBUG("W ir->tdi[%d]: %x\n", index, sir->tdi[index]);
				sir->tdo[index] = 0;
				for(i=0; i<remainbits; i++) {
					if(i == (remainbits - 1)) {
						sir->tdo[index] |= (TCK_Cycle(aspeed_jtag, 1, sir->tdi[index] & 0x1)<<i); // go to IRExit1
					} else {
						sir->tdo[index] |= (TCK_Cycle(aspeed_jtag, 0, sir->tdi[index] & 0x1)<<i); // go to ShiftIR
						sir->tdi[index] >>= 1;
					}
				}
				JTAG_DBUG("R ir->tdo: %x\n",sir->tdo[index]);
				remainbits = remainbits - remainbits;
			}
		}

		TCK_Cycle(aspeed_jtag, 0, 0);		// go to IRPause

		//stop pause
		if(sir->endir == 0) {
			//go to idle
			TCK_Cycle(aspeed_jtag, 1, 0);		// go to IRExit2
			TCK_Cycle(aspeed_jtag, 1, 0);		// go to IRUpdate
			TCK_Cycle(aspeed_jtag, 0, 0);		// go to IDLE
		}
	}else {
		//HW MODE
#if 0
		//ast2300 , ast2400 not support end pause

		if(sir->endir) 
			return 1;
#endif
		if(!aspeed_jtag->sts) {
			aspeed_jtag_write(aspeed_jtag, 0 , ASPEED_JTAG_SW); //dis sw mode 
		}	   
		while (remainbits) {
			while(remainbits > 32) {
				aspeed_jtag_write(aspeed_jtag, sir->tdi[index], ASPEED_JTAG_INST);
				JTAG_DBUG("W ir->tdi[%d]: %x\n", index, sir->tdi[index]);
				aspeed_jtag_write(aspeed_jtag, JTAG_ENG_EN | JTAG_ENG_OUT_EN | JTAG_SET_INST_LEN(32), ASPEED_JTAG_CTRL);
				aspeed_jtag_write(aspeed_jtag, JTAG_ENG_EN | JTAG_ENG_OUT_EN | JTAG_SET_INST_LEN(32) | JTAG_INST_EN, ASPEED_JTAG_CTRL);
				aspeed_jtag_wait_instruction_pause_complete(aspeed_jtag);
				//aspeed_jtag_wait_partial_instruction_complete(aspeed_jtag);
				// IRQ_SAVE fixes
				if (irq_save_flag == 0) {
					spin_lock_irqsave(&jtag_irq_lock, jtag_flags);	
					irq_save_flag = 1;
				}
				udelay(2);
				sir->tdo[index] = aspeed_jtag_read(aspeed_jtag, ASPEED_JTAG_INST);
				JTAG_DBUG("R ir->tdo: %x\n",sir->tdo[index]);
				remainbits = remainbits - 32;
				index++;
			}

			if(remainbits != 0)
			{
			   if(sir->endir) {
					aspeed_jtag_write(aspeed_jtag, sir->tdi[index], ASPEED_JTAG_INST);
					JTAG_DBUG("W ir->tdi[%d]: %x\n", index, sir->tdi[index]);
					aspeed_jtag_write(aspeed_jtag, JTAG_ENG_EN | JTAG_ENG_OUT_EN | JTAG_SET_INST_LEN(remainbits), ASPEED_JTAG_CTRL);
					aspeed_jtag_write(aspeed_jtag, JTAG_ENG_EN | JTAG_ENG_OUT_EN | JTAG_SET_INST_LEN(remainbits) | JTAG_INST_EN, ASPEED_JTAG_CTRL);
					aspeed_jtag_wait_instruction_pause_complete(aspeed_jtag);
					//aspeed_jtag_wait_partial_instruction_complete(aspeed_jtag);
					// IRQ_SAVE fixes
					if (irq_save_flag == 0) {
						spin_lock_irqsave(&jtag_irq_lock, jtag_flags);	
						irq_save_flag = 1;
					}
					udelay(2);
					sir->tdo[index] = (aspeed_jtag_read(aspeed_jtag, ASPEED_JTAG_INST)>> (32 - remainbits));
					JTAG_DBUG("R ir->tdo: %x\n",sir->tdo[index]);
					remainbits = 0;
				} else {
					aspeed_jtag_write(aspeed_jtag, sir->tdi[index], ASPEED_JTAG_INST);
					JTAG_DBUG("W ir->tdi[%d]: %x\n", index, sir->tdi[index]);
					aspeed_jtag_write(aspeed_jtag, JTAG_ENG_EN | JTAG_ENG_OUT_EN | JTAG_LAST_INST | JTAG_SET_INST_LEN(remainbits), ASPEED_JTAG_CTRL);
					aspeed_jtag_write(aspeed_jtag, JTAG_ENG_EN | JTAG_ENG_OUT_EN | JTAG_LAST_INST | JTAG_SET_INST_LEN(remainbits) | JTAG_INST_EN, ASPEED_JTAG_CTRL);
					aspeed_jtag_wait_instruction_complete(aspeed_jtag);
					// IRQ_SAVE fixes
					if (irq_save_flag == 0) {
						spin_lock_irqsave(&jtag_irq_lock, jtag_flags);	
						irq_save_flag = 1;
					}
					udelay(2);
					sir->tdo[index] = (aspeed_jtag_read(aspeed_jtag, ASPEED_JTAG_INST)>> (32 - remainbits));
					JTAG_DBUG("R ir->tdo: %x\n",sir->tdo[index]);
					remainbits = 0;
				}	
			}
		}
		if(sir->endir == 0) {
			aspeed_jtag_write(aspeed_jtag, JTAG_SW_MODE_EN | JTAG_SW_MODE_TDIO, ASPEED_JTAG_SW);
		}
	}
	aspeed_jtag->sts = sir->endir;
	// IRQ_SAVE fixes
        if (irq_save_flag == 1)
                spin_unlock_irqrestore(&jtag_irq_lock, jtag_flags);
	return 0;
}

int aspeed_jtag_sdr_xfer(struct aspeed_jtag_info *aspeed_jtag, struct sdr_xfer *sdr)
{
	unsigned int index = 0;
	u32 shift_bits =0;
	u32 tdo = 0;
	u32 tdi = 0;
	u32 tdio = 0;
	u32 remain_xfer = sdr->length;
	u32 xferbits = 0;

	// IRQ_SAVE fixes
	unsigned long jtag_flags = 0;
	u32 irq_save_flag;

	irq_save_flag = 0;
	JTAG_DBUG("%s mode, ENDDR : %d, len : %d \n", sdr->mode? "SW":"HW", sdr->enddr, sdr->length);
	if(sdr->mode) {
		//SW mode 
		if(aspeed_jtag->sts) {
			//from IR/DR Pause 
			TCK_Cycle(aspeed_jtag, 1, 0);		// go to Exit2 IR / DR
		}

		if(aspeed_jtag->sts == 0) { 
			TCK_Cycle(aspeed_jtag, 1, 0);		// go to DRScan
			TCK_Cycle(aspeed_jtag, 0, 0);		// go to DRCap
		}

		TCK_Cycle(aspeed_jtag, 0, 0);		// go to DRShift
		
		while (remain_xfer) {
			if((shift_bits % 32) == 0) {
				JTAG_DBUG("W dr->dr_data(in)[%d]: %x\n",index, sdr->tdio[index]);
			}
				
			tdo = (sdr->tdio[index] >> (shift_bits % 32)) & (0x1);
			if(remain_xfer == 1) {
				tdi = TCK_Cycle(aspeed_jtag, 1, tdo);	// go to DRExit1
			} else {
				tdi = TCK_Cycle(aspeed_jtag, 0, tdo);	// go to DRShit
			}
			
			tdio |= (tdi << (shift_bits % 32));

			shift_bits++;
			xferbits++;
			remain_xfer--;
			if(((shift_bits % 32) == 0)||(remain_xfer == 0)) {
				sdr->tdio[index] = tdio;
				barrier();
				JTAG_DBUG("W dr->dr_data(out)[%d]: %x\n",index, sdr->tdio[index]);
				tdio = 0;
				index++;
			}
			
			if( (remain_xfer > 1) && (xferbits >= 2048000) )
			{
				xferbits = 0;
				touch_softlockup_watchdog();
			}
		}

		TCK_Cycle(aspeed_jtag, 0, 0);		// go to DRPause
		
		if(sdr->enddr == 0) {
			TCK_Cycle(aspeed_jtag, 1, 0);		// go to DRExit2
			TCK_Cycle(aspeed_jtag, 1, 0);		// go to DRUpdate
			TCK_Cycle(aspeed_jtag, 0, 0);		// go to IDLE
		}		
	} else {
		//HW MODE
#if 0
		//ast2300 , ast2400 not support end pause
		if(sdr->enddr) 
			return 1;
#endif
		if(!aspeed_jtag->sts) {
			aspeed_jtag_write(aspeed_jtag, 0 , ASPEED_JTAG_SW); //disable sw mode
		}	   
		
		while (remain_xfer)
		{
			while (remain_xfer > 32) 
			{	
				shift_bits = 32;
				JTAG_DBUG("shift bits %d \n", shift_bits);
				aspeed_jtag_write(aspeed_jtag, sdr->tdio[index], ASPEED_JTAG_DATA);
				aspeed_jtag_write(aspeed_jtag,JTAG_ENG_EN | JTAG_ENG_OUT_EN | JTAG_DATA_LEN(shift_bits), ASPEED_JTAG_CTRL);
				aspeed_jtag_write(aspeed_jtag, JTAG_ENG_EN | JTAG_ENG_OUT_EN | JTAG_DATA_LEN(shift_bits) | JTAG_DATA_EN, ASPEED_JTAG_CTRL);
				aspeed_jtag_wait_data_pause_complete (aspeed_jtag);
				// IRQ_SAVE fixes
				if (irq_save_flag == 0) {
					spin_lock_irqsave(&jtag_irq_lock, jtag_flags);	
					irq_save_flag = 1;
				}
				udelay(2);
				sdr->tdio[index] = aspeed_jtag_read(aspeed_jtag, ASPEED_JTAG_DATA);
				JTAG_DBUG("W dr->dr_data[%d]: %x\n", index, sdr->tdio[index]);
				remain_xfer -= shift_bits;
				index ++;		
			}		
			if (remain_xfer != 0) 
			{
				shift_bits = remain_xfer;
				JTAG_DBUG("shift bits %d with last \n", shift_bits);
				aspeed_jtag_write(aspeed_jtag, sdr->tdio[index], ASPEED_JTAG_DATA);
				if(sdr->enddr)
				{
					JTAG_DBUG("DR Keep Pause \n");
					aspeed_jtag_write(aspeed_jtag, JTAG_ENG_EN | JTAG_ENG_OUT_EN | JTAG_DR_UPDATE |  JTAG_DATA_LEN(shift_bits),ASPEED_JTAG_CTRL);
					aspeed_jtag_write(aspeed_jtag, JTAG_ENG_EN | JTAG_ENG_OUT_EN | JTAG_DR_UPDATE | JTAG_DATA_LEN(shift_bits) | JTAG_DATA_EN, ASPEED_JTAG_CTRL);
					aspeed_jtag_wait_data_pause_complete(aspeed_jtag);
					if (irq_save_flag == 0) {
						spin_lock_irqsave(&jtag_irq_lock, jtag_flags);	
					irq_save_flag = 1;
					}
					udelay(2);
					sdr->tdio[index] = aspeed_jtag_read(aspeed_jtag, ASPEED_JTAG_DATA)>> (32 - shift_bits);
					JTAG_DBUG("W dr->dr_data[%d]: %x\n", index, sdr->tdio[index]);
					remain_xfer -= shift_bits;
					index++;
				}
				else
				{
					JTAG_DBUG("DR go IDLE \n");					
					aspeed_jtag_write(aspeed_jtag, JTAG_ENG_EN | JTAG_ENG_OUT_EN | JTAG_LAST_DATA | JTAG_DATA_LEN(shift_bits),ASPEED_JTAG_CTRL);
					aspeed_jtag_write(aspeed_jtag, JTAG_ENG_EN | JTAG_ENG_OUT_EN | JTAG_LAST_DATA | JTAG_DATA_LEN(shift_bits) | JTAG_DATA_EN, ASPEED_JTAG_CTRL);
					aspeed_jtag_wait_data_complete(aspeed_jtag);
					// IRQ_SAVE fixes
					if (irq_save_flag == 0) {
						spin_lock_irqsave(&jtag_irq_lock, jtag_flags);	
						irq_save_flag = 1;
					}
					udelay(2);
					sdr->tdio[index] = aspeed_jtag_read(aspeed_jtag, ASPEED_JTAG_DATA)>> (32 - shift_bits);
					JTAG_DBUG("W dr->dr_data[%d]: %x\n", index, sdr->tdio[index]);
					remain_xfer -= shift_bits;
					index++;
				}
			}
	
		}
		if(sdr->enddr == 0) {
			aspeed_jtag_write(aspeed_jtag, JTAG_SW_MODE_EN | JTAG_SW_MODE_TDIO,ASPEED_JTAG_SW);
		}
	}
	aspeed_jtag->sts = sdr->enddr;
	// IRQ_SAVE fixes
	if (irq_save_flag == 1)
		spin_unlock_irqrestore(&jtag_irq_lock, jtag_flags);
	return 0;
}

/*************************************************************************************/
static irqreturn_t aspeed_jtag_interrupt(int this_irq, void *dev_id)
{
	u32 status;
	struct aspeed_jtag_info *aspeed_jtag = &g_aspeed_jtag_info;

	status = aspeed_jtag_read(aspeed_jtag, ASPEED_JTAG_ISR);
	JTAG_DBUG("sts %x \n", status);

	if (status & JTAG_INST_PAUSE) {
		aspeed_jtag_write(aspeed_jtag, JTAG_INST_PAUSE | (status & 0xf), ASPEED_JTAG_ISR);
		aspeed_jtag->flag = JTAG_INST_PAUSE;
	}

	if (status & JTAG_INST_COMPLETE) {
		aspeed_jtag_write(aspeed_jtag, JTAG_INST_COMPLETE | (status & 0xf), ASPEED_JTAG_ISR);
		aspeed_jtag->flag = JTAG_INST_COMPLETE;
	}

	if (status & JTAG_DATA_PAUSE) {
		aspeed_jtag_write(aspeed_jtag, JTAG_DATA_PAUSE | (status & 0xf), ASPEED_JTAG_ISR);
		aspeed_jtag->flag = JTAG_DATA_PAUSE;
	}

	if (status & JTAG_DATA_COMPLETE) {
		aspeed_jtag_write(aspeed_jtag, JTAG_DATA_COMPLETE | (status & 0xf), ASPEED_JTAG_ISR);
		aspeed_jtag->flag = JTAG_DATA_COMPLETE;
	}

	if (aspeed_jtag->flag) {
		wake_up_interruptible(&aspeed_jtag->jtag_wq);
		return IRQ_HANDLED;
	} else {
		printk("TODO Check JTAG's interrupt %x\n", status);
		return IRQ_NONE;
	}

}

void JTAG_reset(struct aspeed_jtag_info *aspeed_jtag)
{
	aspeed_jtag_write(aspeed_jtag, 0, ASPEED_JTAG_SW);
	aspeed_jtag_write(aspeed_jtag, JTAG_ENG_EN | JTAG_ENG_OUT_EN, ASPEED_JTAG_CTRL);
	aspeed_jtag_write(aspeed_jtag, JTAG_ENG_EN | JTAG_ENG_OUT_EN | JTAG_FORCE_TMS, ASPEED_JTAG_CTRL);
	mdelay(5);
	while((aspeed_jtag_read(aspeed_jtag, ASPEED_JTAG_CTRL) & JTAG_FORCE_TMS)!=0);
	aspeed_jtag_write(aspeed_jtag, JTAG_SW_MODE_EN | JTAG_SW_MODE_TDIO, ASPEED_JTAG_SW);
}

/*************************************************************************************/
long jtag_debug_ioctl(unsigned int cmd,unsigned long arg)
{
    int ret = 0;
    struct aspeed_jtag_info *aspeed_jtag = &g_aspeed_jtag_info;//file->private_data;    
    void __user *argp = (void __user *)arg;
    struct sir_xfer sir;
    struct sdr_xfer sdr;
    struct io_xfer io;
    struct trst_reset trst_pin;
    struct runtest_idle run_idle;

	switch (cmd) {
	case ASPEED_JTAG_GIOCFREQ:
		ret = __put_user(aspeed_jtag_get_freq(aspeed_jtag), (unsigned int __user *)arg);
		break;
	case ASPEED_JTAG_SIOCFREQ:
//			printk("set freq = %d , pck %d \n",config.freq, aspeed_get_pclk());
		if ((unsigned int)arg > aspeed_get_pclk())
			ret = -EFAULT;
		else
			aspeed_jtag_set_freq(aspeed_jtag, (unsigned int)arg);

		break;
	case ASPEED_JTAG_IOCRUNTEST:
		if (copy_from_user(&run_idle, argp, sizeof(struct runtest_idle)))
			ret = -EFAULT;
		else
			aspeed_jtag_run_test_idle(aspeed_jtag, &run_idle);
		break;
	case ASPEED_JTAG_IOCSIR:
		if (copy_from_user(&sir, argp, sizeof(struct sir_xfer))) {
			ret = -EFAULT;
		} else {
			unsigned int *kerneltdo = 0;
			unsigned int *kerneltdi = 0;
			unsigned int usertdi = (unsigned int)sir.tdi;
			unsigned int usertdo = (unsigned int)sir.tdo;

			kerneltdi = vmalloc((((sir.length + 7) >> 3) + 4));
			// if (kerneltdi == NULL) {
			// 	printk("ASPEED_JTAG : Can't allocate memory\n");
			// 	return -ENOMEM;
			// }
			kerneltdo = vmalloc((((sir.length + 7) >> 3) + 4));
			// if (kerneltdo == NULL) {
			// 	printk("ASPEED_JTAG : Can't allocate memory\n");
			// 	vfree(kerneltdi);
			// 	return -ENOMEM;
			// }

			memset(kerneltdi, 0, (((sir.length + 7) >> 3) + 4));
			memset(kerneltdo, 0, (((sir.length + 7) >> 3) + 4));

			if (copy_from_user(kerneltdi, sir.tdi, ((sir.length + 7) >> 3))) {
				ret = -EFAULT;
			} else {
				sir.tdi = kerneltdi;
				sir.tdo = kerneltdo;
				aspeed_jtag_sir_xfer(aspeed_jtag, &sir);
				sir.tdo = (unsigned int *)usertdo;
				sir.tdi = (unsigned int *)usertdi;
				if (copy_to_user(sir.tdo, kerneltdo, ((sir.length + 7) >> 3)))
					ret = -EFAULT;
				if (copy_to_user(argp, &sir, sizeof(struct sir_xfer)))
					ret = -EFAULT;
			}
			vfree(kerneltdi);
			vfree(kerneltdo);
		}
		break;
	case ASPEED_JTAG_IOCSDR:
		if (copy_from_user(&sdr, argp, sizeof(struct sdr_xfer))) {
			ret = -EFAULT;
		} else {
			unsigned int useraddress = (unsigned int)sdr.tdio;
			unsigned int *kernelAddess = vmalloc((((sdr.length + 7) >> 3) + 4));

			if (kernelAddess == NULL) {
				printk("ASPEED_JTAG : Can't allocate memory\n");
				return -ENOMEM;
			}
			memset(kernelAddess, 0, (((sdr.length + 7) >> 3) + 4));
			if (copy_from_user(kernelAddess, sdr.tdio, ((sdr.length + 7) >> 3))) {
				ret = -EFAULT;
			} else {
				sdr.tdio = (unsigned int *)kernelAddess;
				aspeed_jtag_sdr_xfer(aspeed_jtag, &sdr);
				sdr.tdio = (unsigned int *)useraddress;
				if (copy_to_user(sdr.tdio, kernelAddess, ((sdr.length + 7) >> 3)))
					ret = -EFAULT;
				if (copy_to_user(argp, &sdr, sizeof(struct sdr_xfer)))
					ret = -EFAULT;
			}
			vfree(kernelAddess);
		}
		break;
	case ASPEED_JTAG_IOWRITE:
		if (copy_from_user(&io, argp, sizeof(struct io_xfer))) {
			ret = -EFAULT;
		} else {
			void __iomem	*reg_add;

			reg_add = ioremap(io.Address, 4);
			writel(io.Data, reg_add);
			iounmap(reg_add);
		}

		break;
	case ASPEED_JTAG_IOREAD:
		if (copy_from_user(&io, argp, sizeof(struct io_xfer))) {
			ret = -EFAULT;
		} else {
			void __iomem	*reg_add;

			reg_add = ioremap(io.Address, 4);
			io.Data = readl(reg_add);
			iounmap(reg_add);
		}
		if (copy_to_user(argp, &io, sizeof(struct io_xfer)))
			ret = -EFAULT;
		break;
	case ASPEED_JTAG_RESET:
		JTAG_reset(aspeed_jtag);
		break;
	case ASPEED_JTAG_RUNTCK:
		if (copy_from_user(&io, argp, sizeof(struct io_xfer))) {
			ret = -EFAULT;
		} else {
			int i;

			for (i = 0; i < io.Address; i++)
				TCK_Cycle(aspeed_jtag, io.mode, io.Data);
		}
		break;
	case ASPEED_JTAG_TRST_RESET:
		if (copy_from_user(&trst_pin, argp, sizeof(struct trst_reset))) {
			ret = -EFAULT;
		} else {
			unsigned int regs = aspeed_jtag_read(aspeed_jtag, ASPEED_JTAG_IDLE);

			if (trst_pin.operation == 1) {
				if (trst_pin.Data == 1)
					aspeed_jtag_write(aspeed_jtag, regs | (1 << 31), ASPEED_JTAG_IDLE);
				else
					aspeed_jtag_write(aspeed_jtag, regs & (~(1 << 31)), ASPEED_JTAG_IDLE);
			} else
				trst_pin.Data = (regs >> 31);

		}
		if (copy_to_user(argp, &trst_pin, sizeof(struct trst_reset)))
			ret = -EFAULT;
		break;
	default:
		return -ENOTTY;
	}

	return ret;
}

int jtag_debug_init(void)
{

    int ret=0;
    
    struct aspeed_jtag_info *aspeed_jtag = &g_aspeed_jtag_info;

    memset(aspeed_jtag,0,sizeof(struct aspeed_jtag_info));

    JTAG_DBUG("aspeed_jtag_probe\n");    

    aspeed_jtag->reg_base= ioremap_nocache(0x1e6e4000, 0x20);
    if(aspeed_jtag->reg_base==NULL)
    {
        printk("ioremap fail\n");
    }

	aspeed_jtag_write(aspeed_jtag, JTAG_ENG_EN | JTAG_ENG_OUT_EN, ASPEED_JTAG_CTRL);  //Eanble Clock
	//Enable sw mode for disable clk
	aspeed_jtag_write(aspeed_jtag, JTAG_SW_MODE_EN | JTAG_SW_MODE_TDIO, ASPEED_JTAG_SW);

	aspeed_jtag_write(aspeed_jtag, JTAG_INST_PAUSE | JTAG_INST_COMPLETE |
		       JTAG_DATA_PAUSE | JTAG_DATA_COMPLETE |
		       JTAG_INST_PAUSE_EN | JTAG_INST_COMPLETE_EN |
		       JTAG_DATA_PAUSE_EN | JTAG_DATA_COMPLETE_EN,
		       ASPEED_JTAG_ISR);		//Eanble Interrupt

#if (LINUX_VERSION_CODE >=  KERNEL_VERSION(5,0,0))
    aspeed_jtag->irq = GetIrqFromDT("ami_jtag", IRQ_JTAG_MASTER);
#else
    aspeed_jtag->irq = IRQ_JTAG_MASTER;
#endif

    ret=request_irq(aspeed_jtag->irq, aspeed_jtag_interrupt, IRQF_DISABLED,"aspeed_jtag", NULL);
    if (aspeed_jtag->irq < 0||ret<0) 
    {
        printk("no irq specified\n");
        ret = -ENOENT;
        free_irq(aspeed_jtag->irq, NULL);
        iounmap(aspeed_jtag->reg_base);
        return ret;
    }

    aspeed_jtag->flag = 0;
    init_waitqueue_head(&aspeed_jtag->jtag_wq);
    return ret;
}

int jtag_debug_remove(void)
{
    struct aspeed_jtag_info *aspeed_jtag = &g_aspeed_jtag_info;
    free_irq(aspeed_jtag->irq, NULL);
    if(aspeed_jtag->reg_base!=0)
    {
        iounmap(aspeed_jtag->reg_base);
        aspeed_jtag->reg_base=0;
    }

    return 0;    
}

static jtag_debug_hw_operations_t jtag_debug_hw_ops = {
    .jtag_debug_ioctl = jtag_debug_ioctl,
};

int jtag_debug_hw_init(void)
{
    if (0 > register_jtag_debug_hw_device_ops(&jtag_debug_hw_ops))
        printk ("Failed to register JTAG Debug Hardware Driver\n");
    else
    {
        jtag_debug_init();
        printk ("The JTAG Debug Hardware Driver is loaded successfully\n");
    }

    return 0;
}


void jtag_debug_hw_exit(void)
{
    unregister_jtag_debug_hw_device_ops();
    jtag_debug_remove();
    printk ( "Exit the JTAG Debug Hardware Driver successfully\n");
    return;
}

module_init(jtag_debug_hw_init);
module_exit(jtag_debug_hw_exit);
MODULE_AUTHOR("American Megatrends Inc.");
MODULE_DESCRIPTION("JTAG Debug Hardware Driver");
MODULE_LICENSE ("GPL");
