/*
 ****************************************************************
 **                                                            **
 **    (C)Copyright 2009-2015, American Megatrends Inc.        **
 **                                                            **
 **            All Rights Reserved.                            **
 **                                                            **
 **        5555 Oakbrook Pkwy Suite 200, Norcross,             **
 **                                                            **
 **        Georgia - 30093, USA. Phone-(770)-246-8600.         **
 **                                                            **
 ****************************************************************
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/wait.h>
#include <linux/dma-mapping.h>
#include <mach/platform.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <linux/device.h>

#include "i2c-data.h"
#include "i2c-hardware.h"



struct i2c_as_data as_data_ptr[BUS_COUNT];
int i2c_dma_mode_select[ BUS_COUNT ] =
{
#if (BUS_COUNT>=1)
	I2C0_DMA_MODE, 
#endif
#if (BUS_COUNT>=2)
	I2C1_DMA_MODE, 
#endif
#if (BUS_COUNT>=3)
	I2C2_DMA_MODE, 
#endif
#if (BUS_COUNT>=4)
	I2C3_DMA_MODE, 
#endif
#if (BUS_COUNT>=5)
	I2C4_DMA_MODE, 
#endif
#if (BUS_COUNT>=6)
	I2C5_DMA_MODE, 
#endif
#if (BUS_COUNT>=7)
	I2C6_DMA_MODE, 
#endif
#if (BUS_COUNT>=8)
	I2C7_DMA_MODE, 
#endif
#if (BUS_COUNT>=9)
	I2C8_DMA_MODE, 
#endif
#if (BUS_COUNT>=10)
	I2C9_DMA_MODE, 
#endif
#if (BUS_COUNT>=11)
	I2C10_DMA_MODE, 
#endif
#if (BUS_COUNT>=12)
	I2C11_DMA_MODE, 
#endif
#if (BUS_COUNT>=13)
	I2C12_DMA_MODE, 
#endif
#if (BUS_COUNT>=14)
	I2C13_DMA_MODE,
#endif
#if (BUS_COUNT>=15)
	I2C14_DMA_MODE,
#endif
#if (BUS_COUNT>=16)
	I2C15_DMA_MODE,
#endif
#if (BUS_COUNT>=17)
	I2C16_DMA_MODE,
#endif
#if (BUS_COUNT>=18)
	I2C17_DMA_MODE,
#endif
#if (BUS_COUNT>=19)
	I2C18_DMA_MODE,
#endif
#if (BUS_COUNT>=20)
	I2C19_DMA_MODE,
#endif
#if (BUS_COUNT>=21)
	I2C20_DMA_MODE,
#endif
#if (BUS_COUNT>=22)
	I2C21_DMA_MODE,
#endif
#if (BUS_COUNT>=23)
	I2C22_DMA_MODE,
#endif
#if (BUS_COUNT>=24)
	I2C23_DMA_MODE,
#endif
#if (BUS_COUNT>=25)
	I2C24_DMA_MODE,
#endif
#if (BUS_COUNT>=26)
	I2C25_DMA_MODE,
#endif
#if (BUS_COUNT>=27)
	I2C26_DMA_MODE,
#endif
#if (BUS_COUNT>=28)
	I2C27_DMA_MODE,
#endif
#if (BUS_COUNT>=29)
	I2C28_DMA_MODE,
#endif
#if (BUS_COUNT>=30)
	I2C29_DMA_MODE,
#endif
};

void i2c_init_internal_data(int bus)
{
    int i;
	
    /* Initialize locks, queues and variables */
    spin_lock_init( &as_data_ptr[bus].data_lock );
    init_waitqueue_head( &(as_data_ptr[bus].as_wait)); 
    init_waitqueue_head( &(as_data_ptr[bus].as_slave_data_wait)); 
    init_waitqueue_head( &(as_data_ptr[bus].as_mctp_data_wait));
	as_data_ptr[bus].op_status = 0;
	as_data_ptr[bus].abort_status = 0;

#if defined(GROUP_AST1070_COMPANION)
	if (bus < AST_BMC_BUS_COUNT)
	{
		as_data_ptr[bus].i2c_dma_mode = I2C_BYTE_MODE;
	}
	else
	{
		#if defined(CONFIG_SPX_FEATURE_AST1070_I2C_DMA_MODE)
		as_data_ptr[bus].i2c_dma_mode = I2C_DMA_MODE;//I2C_DMA_MODE;I2C_BYTE_MODE
		#else
		as_data_ptr[bus].i2c_dma_mode = I2C_BYTE_MODE;
		#endif
	}
#else
	as_data_ptr[bus].i2c_dma_mode = i2c_dma_mode_select[bus];
#endif

    if (as_data_ptr[bus].i2c_dma_mode == I2C_DMA_MODE)
    {
        as_data_ptr[bus].dma_buff = dma_alloc_coherent(NULL, AST_I2C_DMA_SIZE, &as_data_ptr[bus].dma_addr, GFP_KERNEL);
        if (!as_data_ptr[bus].dma_buff) {
            printk( KERN_ERR "unable to allocate tx Buffer memory\n");
        }
        if(as_data_ptr[bus].dma_addr%4 !=0) {
            printk( KERN_ERR "not 4 byte boundary \n");
        }
        memset (as_data_ptr[bus].dma_buff, 0, AST_I2C_DMA_SIZE);
    }
    else
    {
        as_data_ptr[bus].dma_buff = NULL;
        as_data_ptr[bus].dma_addr = 0;
    }

	as_data_ptr[bus].TX_len = 0;
	as_data_ptr[bus].TX_index = 0;

	as_data_ptr[bus].MasterRX_len = 0;
	as_data_ptr[bus].MasterRX_index = 0;

	as_data_ptr[bus].Linear_SlaveRX_len = 0;
	as_data_ptr[bus].Linear_SlaveRX_index = 0;

	for(i=0;i<MAX_FIFO_LEN;i++)
	{
		as_data_ptr[bus].SlaveRX_len[i] = 0;
		as_data_ptr[bus].SlaveRX_index[i] = 0;
	}
	as_data_ptr[bus].SlaveRX_Writer = 0;
	as_data_ptr[bus].SlaveRX_Reader = 0;
	as_data_ptr[bus].SlaveRX_Entries = 0;

	as_data_ptr[bus].SlaveTX_Enable = 0;
	as_data_ptr[bus].SlaveTX_READ_DATA = 0;
	as_data_ptr[bus].SlaveTX_REQ_cmd = 0;
	as_data_ptr[bus].SlaveTX_RES_len = 0;
	as_data_ptr[bus].SlaveTX_index = 0;
	as_data_ptr[bus].SlaveTX_count = 0;
	
	for(i=0;i<MAX_FIFO_LEN;i++)
	{
		as_data_ptr[bus].MCTPRX_Len[i] = 0;
	}
	as_data_ptr[bus].MCTPRX_Writer = 0;
	as_data_ptr[bus].MCTPRX_Reader = 0;
	as_data_ptr[bus].MCTPRX_Entries = 0;

	as_data_ptr[bus].master_xmit_recv_mode_flag = SLAVE;

	for(i=0;i<SLAVETX_MAX_RES_SIZE;i++) 
	{ 
		as_data_ptr[bus].SlaveTX_RES_data[i] = 0xFF; 
	}

	as_data_ptr[bus].address_sent= 0;
	as_data_ptr[bus].target_address = 0;
	as_data_ptr[bus].block_read = 0;
	as_data_ptr[bus].block_proc_call= 0;
	as_data_ptr[bus].host_notify_flag = 0;


	as_data_ptr[bus].SMB_Linear_SlaveRX_len = 0;
	as_data_ptr[bus].SMB_Linear_SlaveRX_index = 0;

	for(i=0;i<MAX_FIFO_LEN;i++)
	{
		as_data_ptr[bus].SMB_SlaveRX_len[i] = 0;
		as_data_ptr[bus].SMB_SlaveRX_index[i] = 0;
	}
	as_data_ptr[bus].SMB_SlaveRX_Writer = 0;
	as_data_ptr[bus].SMB_SlaveRX_Reader = 0;
	as_data_ptr[bus].SMB_SlaveRX_Entries = 0;
	as_data_ptr[bus].i2c_link_state = 0;

#ifdef JM_RECOVERY 
	for (i=0; i<128; i++) {
		as_data_ptr[bus].dev_fail_cnt[i] = 0;
	}
	as_data_ptr[bus].bus_busy_cnt = 0;
#endif
	
}
