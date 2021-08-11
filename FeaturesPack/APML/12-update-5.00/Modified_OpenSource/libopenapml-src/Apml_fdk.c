/*
 * APML FDK Routines - V1.0
 *
 ******************************************************************************


============================================================
License Agreement

Copyright (c) 2008, 2009 Advanced Micro Devices, Inc.

All rights reserved.

Redistribution and use in any form of this material and any product thereof
including software in source or binary forms, along with any related
documentation, with or without modification ("this material"), is permitted
provided that the following conditions are met:

+ Redistributions of source code of any software must retain the above
copyright notice and all terms of this license as part of the code.

+ Redistributions in binary form of any software must reproduce the above
copyright notice and all terms of this license in any related documentation
and/or other materials.

+ Neither the names nor trademarks of Advanced Micro Devices, Inc. or any
copyright holders or contributors may be used to endorse or promote products
derived from this material without specific prior written permission.

+ Notice about U.S. Government restricted rights: This material is provided
with RESTRICTED RIGHTS. Use, duplication or disclosure by the U.S. Government
is subject to the full extent of restrictions set forth in FAR52.227 and
DFARS252.227 et seq., or any successor or applicable regulations. Use of this
material by the U.S. Government constitutes acknowledgment of the proprietary
rights of Advanced Micro Devices, Inc. and any copyright holders and
contributors.

+ In no event shall anyone redistributing or accessing or using this material
commence or participate in any arbitration or legal action relating to this
material against Advanced Micro Devices, Inc. or any copyright holders or
contributors. The foregoing shall survive any expiration or termination of this
license or any agreement or access or use related to this material.

+ ANY BREACH OF ANY TERM OF THIS LICENSE SHALL RESULT IN THE IMMEDIATE
REVOCATION OF ALL RIGHTS TO REDISTRIBUTE, ACCESS OR USE THIS MATERIAL.

THIS MATERIAL IS PROVIDED BY ADVANCED MICRO DEVICES, INC. AND ANY COPYRIGHT
HOLDERS AND CONTRIBUTORS "AS IS" IN ITS CURRENT CONDITION AND WITHOUT ANY
REPRESENTATIONS, GUARANTEE, OR WARRANTY OF ANY KIND OR IN ANY WAY RELATED TO
SUPPORT, INDEMNITY, ERROR FREE OR UNINTERRUPTED OPERATION, OR THAT IT IS FREE
FROM DEFECTS OR VIRUSES.  ALL OBLIGATIONS ARE HEREBY DISCLAIMED - WHETHER
EXPRESS, IMPLIED, OR STATUTORY - INCLUDING, BUT NOT LIMITED TO, ANY IMPLIED
WARRANTIES OF TITLE, MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE,
ACCURACY, COMPLETENESS, OPERABILITY, QUALITY OF SERVICE, OR NON-INFRINGEMENT.
IN NO EVENT SHALL ADVANCED MICRO DEVICES, INC. OR ANY COPYRIGHT HOLDERS OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, PUNITIVE,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, REVENUE, DATA, OR PROFITS; OR
BUSINESS INTERRUPTION) HOWEVER CAUSED OR BASED ON ANY THEORY OF LIABILITY
ARISING IN ANY WAY RELATED TO THIS MATERIAL, EVEN IF ADVISED OF THE POSSIBILITY
OF SUCH DAMAGE. THE ENTIRE AND AGGREGATE LIABILITY OF ADVANCED MICRO DEVICES,
INC. AND ANY COPYRIGHT HOLDERS AND CONTRIBUTORS SHALL NOT EXCEED TEN DOLLARS
(US $10.00). ANYONE REDISTRIBUTING OR ACCESSING OR USING THIS MATERIAL ACCEPTS
THIS ALLOCATION OF RISK AND AGREES TO RELEASE ADVANCED MICRO DEVICES, INC. AND
ANY COPYRIGHT HOLDERS AND CONTRIBUTORS FROM ANY AND ALL LIABILITIES,
OBLIGATIONS, CLAIMS, OR DEMANDS IN EXCESS OF TEN DOLLARS (US $10.00). THE
FOREGOING ARE ESSENTIAL TERMS OF THIS LICENSE AND, IF ANY OF THESE TERMS ARE
CONSTRUED AS UNENFORCEABLE, FAIL IN ESSENTIAL PURPOSE, OR BECOME VOID OR
DETRIMENTAL TO ADVANCED MICRO DEVICES, INC. OR ANY COPYRIGHT HOLDERS OR
CONTRIBUTORS FOR ANY REASON, THEN ALL RIGHTS TO REDISTRIBUTE, ACCESS OR USE
THIS MATERIAL SHALL TERMINATE IMMEDIATELY. MOREOVER, THE FOREGOING SHALL
SURVIVE ANY EXPIRATION OR TERMINATION OF THIS LICENSE OR ANY AGREEMENT OR
ACCESS OR USE RELATED TO THIS MATERIAL.

NOTICE IS HEREBY PROVIDED, AND BY REDISTRIBUTING OR ACCESSING OR USING THIS
MATERIAL SUCH NOTICE IS ACKNOWLEDGED, THAT THIS MATERIAL MAY BE SUBJECT TO
RESTRICTIONS UNDER THE LAWS AND REGULATIONS OF THE UNITED STATES OR OTHER
COUNTRIES, WHICH INCLUDE BUT ARE NOT LIMITED TO, U.S. EXPORT CONTROL LAWS SUCH
AS THE EXPORT ADMINISTRATION REGULATIONS AND NATIONAL SECURITY CONTROLS AS
DEFINED THEREUNDER, AS WELL AS STATE DEPARTMENT CONTROLS UNDER THE U.S.
MUNITIONS LIST. THIS MATERIAL MAY NOT BE USED, RELEASED, TRANSFERRED, IMPORTED,
EXPORTED AND/OR RE-EXPORTED IN ANY MANNER PROHIBITED UNDER ANY APPLICABLE LAWS,
INCLUDING U.S. EXPORT CONTROL LAWS REGARDING SPECIFICALLY DESIGNATED PERSONS,
COUNTRIES AND NATIONALS OF COUNTRIES SUBJECT TO NATIONAL SECURITY CONTROLS.
MOREOVER, THE FOREGOING SHALL SURVIVE ANY EXPIRATION OR TERMINATION OF ANY
LICENSE OR AGREEMENT OR ACCESS OR USE RELATED TO THIS MATERIAL.

This license forms the entire agreement regarding the subject matter hereof and
supersedes all proposals and prior discussions and writings between the parties
with respect thereto. This license does not affect any ownership, rights,
title, or interest in, or relating to, this material. No terms of this license
can be modified or waived, and no breach of this license can be excused, unless
done so in a writing signed by all affected parties. Each term of this license
is separately enforceable. If any term of this license is determined to be or
becomes unenforceable or illegal, such term shall be reformed to the minimum
extent necessary in order for this license to remain in effect in accordance
with its terms as modified by such reformation. This license shall be governed
by and construed in accordance with the laws of the State of Texas without
regard to rules on conflicts of law of any state or jurisdiction or the United
Nations Convention on the International Sale of Goods. All disputes arising out
of this license shall be subject to the jurisdiction of the federal and state
courts in Austin, Texas, and all defenses are hereby waived concerning personal
jurisdiction and venue of these courts.
============================================================


 ****************************************************************************** 
 */



#include <stdio.h>

#include "Apml_fdk.h"
#include "mmap.h"
#include "EINTR_wrappers.h"
#include "dbgout.h"

/* **************************************************************************/
/*                                                                          */
/*                         Private Static Defines                           */
/*                                                                          */
/* **************************************************************************/

const APML_ERR_DESC hal_err_desc[] = {
	{ APML_BAD_PROC_NDX,	"Processor Index is Out of Range" },
	{ APML_UNK_PROC_NUM,	"Processor At This Index Not Found" },
	{ APML_PEC_FAIL,	"I2C Transaction Received PEC Mismatch" },
	{ APML_NOT_SUPPORTED,	"Function Not Yet Implemented" },
	{ APML_RCV_LEN_MATCH,	"Message Receive Len Does Not Match Expected" },
	{ APML_ABORT_TMO,	"0x10 SMBus Timeout, Command Aborted" },
	{ APML_CMD_TMO,		"0x11 SMBus Timeout, Command Pending" },
	{ APML_RSP_TMO,		"0x12 SMBus Timeout, Command Executing" },
	{ APML_SBI_BUSY,	"0x20 Busy, Previous Command Still Executing" },
	{ APML_HDT_BUSY,	"0x21 Busy - HDT Owns the Interface" },
	{ APML_CORE_NA,		"0x23 Core Not Enabled or in APIC Spin Loop" },
	{ APML_UNK_CMD,		"0x40 Command Not Recognized" },
	{ APML_INV_RD_LEN,	"0x41 Invalid Read Length" },
	{ APML_XS_DATA_LEN,	"0x42 Excessive SMBus Command Length" },
	{ APML_UNK_THREAD,	"0x44 Selected Core Does Not Exist" },
	{ APML_CMD_NA,		"0x45 Command Not Supported" },
	{ APML_UNK_RESPONSE,	"Unknown Response Code" },
	{ APML_NO_RESPONSE,	"No Status Code Received" },
	{ APML_BAD_RMI_VERSION,	"Unsupported RMI version" },
	{ APML_ILL_BCAST,	"Command Not Allowed for Core Broadcast" }
};


const APML_MAP err_map[] = {
	{ 0x10,	APML_ABORT_TMO },
	{ 0x11,	APML_CMD_TMO },
	{ 0x12,	APML_RSP_TMO },
	{ 0x20,	APML_SBI_BUSY },
	{ 0x21,	APML_HDT_BUSY },
	{ 0x23,	APML_CORE_NA },
	{ 0x40,	APML_UNK_CMD },
	{ 0x41,	APML_INV_RD_LEN },
	{ 0x42,	APML_XS_DATA_LEN },
	{ 0x44,	APML_UNK_THREAD },
	{ 0x45,	APML_CMD_NA }
};


//PEC enable will append PEC byte to write data and check read data PEC byte
//set to 1 is preferable
//set to 0 for debugging block write-block read process 
//only user_smbus_bwr_brd_process be affected, according to APML spec 1.04
//note this will NOT effect SBRMI_x01 bit7:PECEn
#define IS_PEC_ENABLE 1

#define MAX_PEC_RETRIES 3

#define MAX_LPCRST_RETRIES 10

/* **************************************************************************/
/*                                                                          */
/*                       Local Function Declarations                        */
/*                                                                          */
/* **************************************************************************/


void apml_trace_cmd(
	uint8	proc_addr,
	uint32	msg_wr_bytes,
	uint8	*wr_data,
	uint32	msg_rd_bytes,
	uint8	*rd_data);

uint32 apml_read_rmi_msr(
	APML_DEV_CTL	*ctl,
	uint8		socketid,
	uint8		proc_addr,
	uint8		core_ndx,
	const CHAR	*in_cmd_id,
	uint32		reg_id,
	uint32		*reg_val_h,
	uint32		*reg_val_l,
	int			BMCInst);

uint32 apml_write_rmi_msr(
	APML_DEV_CTL	*ctl,
	uint8		proc_ndx,   //proc_ndx and socketid are identically the same
	uint8		proc_addr,
	uint8		core_ndx,
	const CHAR	*in_cmd_id,
	uint32		reg_id,
	uint8		reg_val_size,
	uint32		reg_val_h,
	uint32		reg_val_l,
	int			BMCInst);

uint32 apml_block_process_call(
	APML_DEV_CTL	*dev,
	uint8		socketid,
	uint8		proc_addr,
	uint32		wr_len,
	uint8		*wr_data,
	uint32		rd_len,
	uint8		*rd_data,
	int			BMCInst);



/* **************************************************************************/
/*                                                                          */
/*                            Public Functions                              */
/*                                                                          */
/* **************************************************************************/


/* Initializes the FDK for use */
uint32 apml_init_fdk(APML_DEV_CTL *ctl,int BMCInst)
{
	uint32	i, j, res;
	unsigned long reg;

	//res = APML_SUCCESS;

	ctl->maj_rev = APML_FDK_MAJOR;
	ctl->min_rev = APML_FDK_MINOR;

	ctl->min_addr_bytes = MIN_ADDR_BYTES_DEFAULT;

	ctl->trace_level = APML_TRACE_LEVEL_DEF;
	ctl->debug_level = APML_DEBUG_LEVEL_DEF;

	for (i=0 ; i < APML_MAX_PROCS ; i++)
	{
		ctl->threads[i] = 0;
		ctl->rmi_rev[i] = 0;
	    ctl->rmi_thread_target[i] = DEFAULT_RMI_CORE;

		for (j=0 ; j < APML_MAX_CORES ; j++)
		{
			ctl->last_sar_h[i][j] = SAR_H_DEFAULT;
			ctl->last_sar_l[i][j] = SAR_L_DEFAULT;
		}
	}

	ctl->stat.rcv_pec_errors     = 0;
	ctl->stat.cmd_cnt            = 0;
	ctl->stat.bytes_xmtd         = 0;
	ctl->stat.bytes_rcvd         = 0;


	res = user_platform_init(ctl,BMCInst);

    if(res == APML_SUCCESS)
    {
	    ctl->initialized = 1;
    }

	/* Set GPIO LPC_RST direction */
	if( !mmap_read32(GPIO_LPC_RST_DIR, &reg) )
	{
		reg &= ~0x80;
		mmap_write32( GPIO_LPC_RST_DIR, reg );
	}

	return(res);
}



/* Gracefully close the FDK */
uint32 apml_close_fdk(APML_DEV_CTL *ctl,int BMCInst)
{
	uint32	res;
        if(0)
        {
    	    BMCInst=BMCInst; /* -Wextra, fix for unused parameters */
        }
	ctl->initialized = 0;
	res = user_platform_close(ctl);
		
	return(res);
}

uint32 apml_check_LPC_RST(void)
{
	int i = 0;
	unsigned long reg;
    struct timeval tv;
	
	do
	{
		if( mmap_read32(GPIO_LPC_RST_DV, &reg) )
		{
			TCRIT( "APML: Read LPC_RST_L failed!\n" );
			return APML_FAILURE;
		}
		if( reg & 0x80 )
			return APML_SUCCESS;
		/* sleep 3 ms */
		tv.tv_sec = 0;
		tv.tv_usec = 3000;
		while(sigwrap_select(0, NULL, NULL, NULL, &tv) < 0);
		if( i == 0 )
			TCRIT( "APML: LPC_RST_L In Busy!\n" );
		i++;
	}while( i < MAX_LPCRST_RETRIES);
	
	TCRIT( "APML: Wait LPC_RST timeout error!\n" );
	
	return APML_FAILURE;
}


/* Core-specific read CPUID */
uint32 apml_read_cpuid(
	APML_DEV_CTL	*ctl,
	//uint8		proc,
	uint8       socketid,
	uint8       dieid,
	uint8		core,
	uint32		cpuid_func,
	uint32		*eax_val,
	uint32		*ebx_val,
	uint32		*ecx_val,
	uint32		*edx_val,
	int			BMCInst)
{
	uint8		*wr_data, *in_data, proc_addr, thread_ndx;
	uint32		i, res, k;
	APML_SMBUS_CMD	cmd;

	*eax_val = 0;
	*ebx_val = 0;
	*ecx_val = 0;
	*edx_val = 0;

	res = user_get_proc_addr(ctl, socketid,dieid, PROC_USE_RMI, &proc_addr);
	if (res != APML_SUCCESS)
		return(res);
	
	if ( (core*2) >= ctl->threads[socketid])
		return(APML_UNK_THREAD);
	// "core" range 00h - 3Fh, P0 use "thread_ndx" range 00h - 7Fh
    thread_ndx = core * 2;

	wr_data = cmd.xd;
	in_data = cmd.rd;

	cmd.xlen = 10;
	cmd.rlen = 10;

    for(k=0;k<2;k++)
    {
        if(k==0)
        	cmd.id = "CPUID EAX:EBX";
        else    // k==1
	        cmd.id = "CPUID ECX:EDX";

	wr_data[0] = 0x73;
	wr_data[1] = 0x08;
	wr_data[2] = 0x08;
	wr_data[3] = 0x91;
	wr_data[4] = thread_ndx;
	wr_data[5] = (cpuid_func)	& 0xFF;
	wr_data[6] = (cpuid_func >> 8)	& 0xFF;
	wr_data[7] = (cpuid_func >> 16)	& 0xFF;
	wr_data[8] = (cpuid_func >> 24)	& 0xFF;
	wr_data[9] = k;     //0b: Return ebx:eax,  1b: Return edx:ecx

	in_data[0] = 0;
	in_data[1] = 0xff;

	res = apml_block_process_call(ctl, socketid, proc_addr, cmd.xlen, wr_data, 
		cmd.rlen, in_data,BMCInst);
	if (res != APML_SUCCESS)
	{
		apml_hal_err_log(ctl, socketid, cmd, proc_addr, core, res);
		return(res);
	}

	res = apml_check_resp(in_data[1]);
	if (res != APML_SUCCESS)
	{
		apml_hal_err_log(ctl, socketid, cmd, proc_addr, core, res);
		return(res);
	}

    if( k==0)   //return ebx:eax
    {
    	i = (in_data[5] << 24) + (in_data[4] << 16) + (in_data[3] << 8) + 
	    	in_data[2];
    	*eax_val = i;
    	i = (in_data[9] << 24) + (in_data[8] << 16) + (in_data[7] << 8) + 
    		in_data[6];
    	*ebx_val = i;
    }
    else    // k==1 return edx:ecx
    {
    	i = (in_data[5] << 24) + (in_data[4] << 16) + (in_data[3] << 8) + 
    		in_data[2];
    	*ecx_val = i;
    	i = (in_data[9] << 24) + (in_data[8] << 16) + (in_data[7] << 8) + 
    		in_data[6];
    	*edx_val = i;
    }

    }   // for(k=0... )
	return(res);
}



/* Processor-wide read APML RMI version  */
uint32 apml_interface_version(
	APML_DEV_CTL	*ctl,
	//uint8		proc,
	uint8       socketid,
	uint8       dieid,
	uint8		*version,
    int BMCInst
    )
{
	uint8	proc_addr, proc_ndx;
	uint32	res;
	res = user_get_proc_addr(ctl, socketid,dieid, PROC_USE_RMI, &proc_addr);
	if (res != APML_SUCCESS)
		return(res);
	proc_ndx = socketid;
        if(0)
        {
    	    BMCInst=BMCInst; /* -Wextra, fix for unused parameters */
        }
	*version = ctl->rmi_rev[proc_ndx];

	return(APML_SUCCESS);
}




/* Core-specific read Machine Check Register (MCR) */
uint32 apml_read_mcr(
	APML_DEV_CTL	*ctl,
	//uint8		proc,
	uint8       socketid,
	uint8       dieid,
	uint8		core,
	uint32		mcr_addr,
	uint32		*reg_val_h,
	uint32		*reg_val_l,
	int			BMCInst)
{
        uint8		proc_addr;
        uint32		res;
        const CHAR	*cmd_id = "RMI READ MCR";

	res = user_get_proc_addr(ctl, socketid,dieid, PROC_USE_RMI, &proc_addr);
	if (res != APML_SUCCESS)
		return(res);
	
	res = apml_read_rmi_msr(ctl, socketid, proc_addr, core, cmd_id, mcr_addr, 
		reg_val_h, reg_val_l,BMCInst);

	return(res);
}



/* Core-specific write Machine Check Register (MCR) */
uint32 apml_write_mcr(
	APML_DEV_CTL	*ctl,
	//uint8		proc,
	uint8       socketid,
	uint8       dieid,
	uint8		core,
	uint32		mcr_addr,
	uint32		reg_val_size,
	uint32		reg_val_h,
	uint32		reg_val_l,
	int			BMCInst)
{
	uint8		proc_addr;
	uint32		res;
	const CHAR	*cmd_id = "RMI WRITE MCR";

	res = user_get_proc_addr(ctl, socketid,dieid, PROC_USE_RMI, &proc_addr);
	if (res != APML_SUCCESS)
		return(res);

	res = apml_write_rmi_msr(ctl, socketid, proc_addr, core, cmd_id, 
		mcr_addr, reg_val_size, reg_val_h, reg_val_l,BMCInst);

	return(res);
}



/* Processor-wide read 1 byte RMI Register */
uint32 apml_read_rmi_reg(
	APML_DEV_CTL	*dev,
	//uint8		proc,
	uint8       socketid,
	uint8       dieid,
	uint8		option,
	uint8		reg,
	uint8		*data,
	int			BMCInst)
{
	uint8	crc, addr, temp8, proc_addr, retries = 0;
	uint32	res, temp32 = 0;

	*data = 0;

	if( (res = apml_check_LPC_RST()) != APML_SUCCESS )
		return(res);

	res = user_get_proc_addr(dev, socketid,dieid, option, &proc_addr);
	if (res != APML_SUCCESS)
		return(res);

	dev->stat.cmd_cnt++;

	addr = proc_addr * 2; /* 8 bit address */


	for (retries = 0; retries < MAX_PEC_RETRIES; ++retries)
	{
		res = user_smbus_read_word(dev, socketid, proc_addr, reg, &temp32,BMCInst);
		if (res != APML_SUCCESS)
			return(res);

		crc = apml_pec(0, 1, &addr);
		crc = apml_pec(crc, 1, &reg);
		addr |= 0x01;
		crc = apml_pec(crc, 1, &addr);
		temp8 = temp32 & 0xff;
		crc = apml_pec(crc, 1, &temp8);

		if (crc == ((temp32 & 0xFF00) >> 8))
		{
			break;
		}
	}

	if (retries == MAX_PEC_RETRIES)
	{
		return(APML_PEC_FAIL);
	}

	if (dev->trace_level >= APML_TRACE_ON)
	{
		uint8 logr[2];

		logr[0] = temp32 & 0xFF;
		logr[1] = (temp32 & 0xFF00) >> 8;
		apml_trace_cmd(proc_addr, 1, &reg, 2, logr);
	}

	dev->stat.bytes_xmtd += 2;  /* 2 address cycles */
	dev->stat.bytes_rcvd += 2;  /* 1 byte data and 1 byte PEC */

	*data = temp32 & 0xFF;

	return(res);
}



/* Processor-wide write 1 byte RMI register */
uint32 apml_write_rmi_reg(
	APML_DEV_CTL	*dev,
	//uint8		proc,
	uint8       socketid,
	uint8       dieid,
	uint8		option,
	uint8		reg,
	uint8		data,
	int			BMCInst)
{
	uint8	crc, addr, proc_addr;
	uint32	res, temp32;

	if( (res = apml_check_LPC_RST()) != APML_SUCCESS )
		return(res);

	res = user_get_proc_addr(dev, socketid,dieid, option, &proc_addr);
	if (res != APML_SUCCESS)
		return(res);

	dev->stat.cmd_cnt++;

	addr = proc_addr * 2; /* 8 bit address */

	crc = apml_pec(0, 1, &addr);
	crc = apml_pec(crc, 1, &reg);
	crc = apml_pec(crc, 1, &data);
	temp32 = (crc << 8) + data;

	res = user_smbus_write_word(dev, socketid, proc_addr, reg, temp32,BMCInst);
	if (dev->trace_level >= APML_TRACE_ON)
	{
		uint8 logx[3];
	
		logx[0] = reg;
		logx[1] = data;
		logx[2] = crc;
		apml_trace_cmd(proc_addr, 3, logx, 0, 0);
	}

	dev->stat.bytes_xmtd += 3; /* 1 addr cycle, 1 byte data, 1 byte PEC */

	return(res);
}


/* Processor-wide read 1 byte TSI Register */
uint32 apml_read_tsi_reg(
	APML_DEV_CTL	*dev,
	//uint8		proc,
	uint8       socketid,
	uint8       dieid,
	uint8		reg,
	uint8		*data,
	int			BMCInst)
{
	uint8	proc_addr;
	uint32	res, temp32;

	*data = 0;

	if( (res = apml_check_LPC_RST()) != APML_SUCCESS )
		return(res);

	res = user_get_proc_addr(dev, socketid,dieid, PROC_USE_TSI, &proc_addr);
	if (res != APML_SUCCESS)
		return(res);

	res = user_smbus_read_byte(dev, socketid, proc_addr, reg, &temp32,BMCInst);
	if (res != APML_SUCCESS)
		return(res);

	if (dev->trace_level >= APML_TRACE_ON)
	{
		uint8 logr[2];

		logr[0] = temp32 & 0xFF;
		logr[1] = (temp32 & 0xFF00) >> 8;
		apml_trace_cmd(proc_addr, 1, &reg, 2, logr);
	}

	*data = temp32 & 0xFF;

	return(res);
}


/* Processor-wide write 1 byte tsi register */
uint32 apml_write_tsi_reg(
	APML_DEV_CTL	*dev,
	//uint8		proc,
	uint8       socketid,
	uint8       dieid,
	uint8		reg,
	uint8		data,
	int			BMCInst)
{
	uint8	proc_addr;
	uint32	res;

	if( (res = apml_check_LPC_RST()) != APML_SUCCESS )
		return(res);

	res = user_get_proc_addr(dev, socketid,dieid, PROC_USE_TSI, &proc_addr);
	if (res != APML_SUCCESS)
		return(res);

	res = user_smbus_write_word(dev, socketid, proc_addr, reg, data,BMCInst);
	if (dev->trace_level >= APML_TRACE_ON)
	{
		uint8 logx[2];
	
		logx[0] = reg;
		logx[1] = data;
		apml_trace_cmd(proc_addr, 2, logx, 0, 0);
	}

	return(res);
}
/* Processor-wide read APML TDP Limit Register */
uint32 apml_read_tdp_limit(
	APML_DEV_CTL	*ctl,
	//uint8		proc,
	uint8       socketid,
	uint8       dieid,
	uint8		*apml_tdp_limi_percent,
	uint16		*apml_tdp_limit,
	int			BMCInst)
{
	uint8		proc_addr;
	uint32		res, reg_id, reg_val_h, reg_val_l;
	const CHAR	*cmd_id = "RMI READ TDP LIMIT";

	reg_id = 0xC0010075;

	res = user_get_proc_addr(ctl, socketid,dieid, 0, &proc_addr);
	if (res != APML_SUCCESS)
		return(res);



	res = apml_read_rmi_msr(ctl, socketid, proc_addr, ctl->rmi_thread_target[socketid], cmd_id,
		reg_id, &reg_val_h, &reg_val_l,BMCInst);

	/* Extract Information */
	if (res == APML_SUCCESS) {
		/* the size of Apml Tdp Limit Percent is 6 bits */
		*apml_tdp_limi_percent = ((reg_val_l >> 16) & 0x3F);
		/* the size of Apml Tdp Limit is 12 bits */
		*apml_tdp_limit = (reg_val_l & 0xFFF);
	}

	return(res);
}



/* Processor-wide write APML TDP Limit Register */
uint32 apml_write_tdp_limit(
	APML_DEV_CTL	*ctl,
	//uint8		proc,
	uint8       socketid,
	uint8       dieid,
	uint8		apml_tdp_limi_percent,
	uint16		apml_tdp_limit,
	int			BMCInst)
{
	uint8		proc_addr, proc_ndx;
	uint32		res, reg_id, reg_val_l;
	const CHAR	*cmd_id = "RMI WRITE TDP LIMIT";

	reg_id = 0xC0010075;

	res = user_get_proc_addr(ctl, socketid,dieid, 0, &proc_addr);
	if (res != APML_SUCCESS)
		return(res);
	proc_ndx = socketid;

	reg_val_l = ((apml_tdp_limi_percent << 16) | apml_tdp_limit);

	res = apml_write_rmi_msr(ctl, proc_ndx, proc_addr, ctl->rmi_thread_target[socketid], cmd_id,
		reg_id, 8, 0, reg_val_l,BMCInst);

	return(res);
}



/* Processor-wide read Processor Power Register */
uint32 apml_read_processor_power(
	APML_DEV_CTL	*ctl,
	//uint8		proc,
	uint8       socketid,
	uint8       dieid,
	uint16		*tdp2_watt,
	uint16		*base_tdp,
	uint16		*processor_tdp,
	int			BMCInst)
{
	uint8		proc_addr;
	uint32		res, reg_id, reg_val_h, reg_val_l;
	const CHAR	*cmd_id = "RMI READ Processor Power";

	reg_id = 0xC0010077;

	res = user_get_proc_addr(ctl, socketid,dieid, 0, &proc_addr);
	if (res != APML_SUCCESS)
		return(res);



	res = apml_read_rmi_msr(ctl, socketid, proc_addr, ctl->rmi_thread_target[socketid], cmd_id,
		reg_id, &reg_val_h, &reg_val_l,BMCInst);

	/* Extract Information */
	if (res == APML_SUCCESS) {
		/* the size of LSB of Tdp to watt is 6 bits */
		/* the size of MSB of Tdp to watt is 10 bits */
		*tdp2_watt = ((reg_val_h & 0x3FF) << 10) | ((reg_val_h >> 10) & 0x3F);
		/* the size of Base Tdp is 16 bits */
		*base_tdp = ((reg_val_l >> 16) & 0xFFFF);
		/* the size of Processor Tdp is 16 bits */
		*processor_tdp = (reg_val_l & 0xFFFF);
	}

	return(res);
}



/* Processor-wide read Power Averaging Period Register */
uint32 apml_read_power_averaging_period(
	APML_DEV_CTL	*ctl,
	//uint8		proc,
	uint8       socketid,
	uint8       dieid,
	uint16		*apm_tdp_limit,
	uint32		*tdp_run_avg_acc_cap,
	uint8		*run_avg_range,
	int			BMCInst)
{
	uint8		proc_addr;
	uint32		res, reg_id, reg_val_h, reg_val_l;
	const CHAR	*cmd_id = "RMI READ Power Averaging Period";

	reg_id = 0xC0010078;

	res = user_get_proc_addr(ctl, socketid, dieid,0, &proc_addr);
	if (res != APML_SUCCESS)
		return(res);



	res = apml_read_rmi_msr(ctl, socketid, proc_addr, ctl->rmi_thread_target[socketid], cmd_id,
		reg_id, &reg_val_h, &reg_val_l,BMCInst);

	/* Extract Information */
	if (res == APML_SUCCESS) {
		/* the size of Apm Tdp Limit is 13 bits */
		*apm_tdp_limit = (reg_val_h & 0x1FFF);
		/* the size of Tdp Run Average Cap is 22 bits */
		*tdp_run_avg_acc_cap = ((reg_val_l >> 4) & 0x3FFFFF);
		/* the size of Run Average Range is 4 bits */
		*run_avg_range = (reg_val_l & 0x0F);
	}

	return(res);
}



/* Processor-wide read DRAM Controller Command Throttle Register */
uint32 apml_read_dram_throttle(
	APML_DEV_CTL	*ctl,
	//uint8		proc,
	uint8       socketid,
	uint8       dieid,
	uint8		*bw_cap_cmd_throttle_mode,
	uint8		*bw_cap_en,
	int			BMCInst)
{
	uint8		proc_addr;
	uint32		res, reg_id, reg_val_h, reg_val_l;
	const CHAR	*cmd_id = "RMI READ DRAM Throttle";

	reg_id = 0xC0010079;

    dieid = 0;  // uniformly applied to whole socket, ignore dieid parameter

	res = user_get_proc_addr(ctl, socketid,dieid, PROC_USE_RMI, &proc_addr);
	if (res != APML_SUCCESS)
		return(res);



	res = apml_read_rmi_msr(ctl, socketid, proc_addr, 0, cmd_id,   // send command to core 0, the first core
		reg_id, &reg_val_h, &reg_val_l,BMCInst);

	/* Extract Information */
	if (res == APML_SUCCESS) {
		if (reg_val_l & 0x800) 
            *bw_cap_en = APML_TRUE;
		else *bw_cap_en = APML_FALSE;
		    *bw_cap_cmd_throttle_mode = ((reg_val_l >> 20) & 0x0F);
	}

	return(res);
}



/* Processor-wide write DRAM Controller Command Throttle Register */
uint32 apml_write_dram_throttle(
	APML_DEV_CTL	*ctl,
	//uint8		proc,
	uint8       socketid,
	uint8       dieid,
	uint8		bw_cap_cmd_throttle_mode,
	uint8		bw_cap_en,
	int			BMCInst)
{
	uint8		proc_addr, proc_ndx;
	uint32		res, reg_id, reg_val_l;
	const CHAR	*cmd_id = "RMI WRITE DRAM Throttle";

	reg_id = 0xC0010079;

    dieid = 0;  // uniformly applied to whole socket, ignore dieid parameter

	res = user_get_proc_addr(ctl, socketid,dieid, PROC_USE_RMI, &proc_addr);
	if (res != APML_SUCCESS)
		return(res);
	proc_ndx = socketid;

	reg_val_l = (bw_cap_cmd_throttle_mode << 20) | ((bw_cap_en == APML_TRUE) ? 0x800: 0x000);

	res = apml_write_rmi_msr(ctl, proc_ndx, proc_addr, 0, cmd_id,  // send to core 0
		reg_id, 8, 0, reg_val_l,BMCInst);

	return(res);
}

uint32 apml_read_package_power_consumption(
    APML_DEV_CTL    *ctl,
        //uint8     proc,
        uint8       socketid,
        uint8       dieid,
        uint32       *pkg_pwr,
        int         BMCInst)
    {
        uint8       proc_addr;
        uint32      res; 
        uint8       sbrmi_reg_34,sbrmi_reg_33,sbrmi_reg_32,sbrmi_reg_31;

        res = user_get_proc_addr(ctl, socketid,dieid, PROC_USE_RMI, &proc_addr);
        if (res != APML_SUCCESS)
            return(res);
    
    
    
        res = apml_read_mailbox_service(ctl, socketid, proc_addr,
            &sbrmi_reg_34,&sbrmi_reg_33,&sbrmi_reg_32,&sbrmi_reg_31,BMCInst);
    
        /* Extract Information */
        if (res == APML_SUCCESS) {
            *pkg_pwr = sbrmi_reg_34 << 24 | sbrmi_reg_33 <<16 | sbrmi_reg_32 <<8 | sbrmi_reg_31;
        }
    
        return(res);
    }
    
uint32 apml_write_package_power_limit(
        APML_DEV_CTL    *ctl,
            //uint8     proc,
            uint8       socketid,
            uint8       dieid,
            uint32      pkg_pwr_limit,
            int         BMCInst)
        {
            uint8       proc_addr;
            uint32      res; 

        
            res = user_get_proc_addr(ctl, socketid,dieid, PROC_USE_RMI, &proc_addr);
            if (res != APML_SUCCESS)
                return(res);
  
        
        
            res = apml_write_mailbox_service(ctl, socketid, proc_addr, pkg_pwr_limit,BMCInst);

        
            return(res);
        }


uint32 apml_read_boot_code_status(
    APML_DEV_CTL    *ctl,
        uint8       socketid,
        uint8       dieid,
        uint8       *boot_code_status,
        int         BMCInst)
    {
        uint8       proc_addr;
        uint32      res; 
        uint32      reg_id = 0x80;
        uint32      reg_data = 0x0;
        int         i;
        //start from 0x80, Boot Code Status use 8 bytes to communicate.

		if( (res = apml_check_LPC_RST()) != APML_SUCCESS )
			return(res);

        res = user_get_proc_addr(ctl, socketid, dieid, PROC_USE_RMI, &proc_addr);
        if (res != APML_SUCCESS)
            return(res);
    
        for (i=0; i<8; i++){
        res = user_smbus_read_byte(ctl, socketid, proc_addr, reg_id, &reg_data,BMCInst);

        if (res != 0)
            return (res);
        else 
            *(boot_code_status + i) = (uint8) reg_data;

        reg_id++;
        }
        
        return(res);
    }


uint32 apml_read_rmi_mailbox_service(
        APML_DEV_CTL    *dev,
        uint8           socketid,
        uint8           dieid,
        uint8           mboxcmd,
        uint32          *mboxdata,
        int             BMCInst)
    {
        uint8       proc_addr;
        uint32      res; 
        uint8       sbrmi_reg_34,sbrmi_reg_33,sbrmi_reg_32,sbrmi_reg_31;

        res = user_get_proc_addr(dev, socketid,dieid, PROC_USE_RMI, &proc_addr);
        if (res != APML_SUCCESS)
            return(res);
    
    
    
        res = apml_read_mailbox_service_with_mboxcmd(dev, socketid, proc_addr, mboxcmd,
            &sbrmi_reg_34,&sbrmi_reg_33,&sbrmi_reg_32,&sbrmi_reg_31,BMCInst);
    
        /* Extract Information */
        if (res == APML_SUCCESS) {
            *mboxdata = sbrmi_reg_34 << 24 | sbrmi_reg_33 <<16 | sbrmi_reg_32 <<8 | sbrmi_reg_31;
        }
    
        return(res);
    }
    
uint32 apml_write_rmi_mailbox_service(
        APML_DEV_CTL    *dev,
        uint8           socketid,
        uint8           dieid,
        uint8           mboxcmd,
        uint32          mboxdata,
        int             BMCInst)
        {
            uint8       proc_addr;
            uint32      res; 

        
            res = user_get_proc_addr(dev, socketid,dieid, PROC_USE_RMI, &proc_addr);
            if (res != APML_SUCCESS)
                return(res);
  
        
        
            res = apml_write_mailbox_service_with_mboxcmd(dev, socketid, proc_addr, mboxcmd, mboxdata, BMCInst);

        
            return(res);
        }

/* **************************************************************************/
/*                                                                          */
/*                           Private Functions                              */
/*                                                                          */
/* **************************************************************************/


/* Generate an FDK status code from an APML command status code */
uint32 apml_check_resp(uint8 rsp)
{
	uint32 res, i;

	if (rsp == 0x00)
		res = APML_SUCCESS;
	else
	{
		res = APML_UNK_RESPONSE;

		for (i=0 ; i < (sizeof(err_map) / sizeof(APML_MAP)); i++)
		{
			if (err_map[i].icode == rsp)
			{
				res = err_map[i].acode;
				break;
			}
		}
	}

	return(res);
}



/* Log an error to stdout depending on debug level */
void apml_hal_err_log(
	APML_DEV_CTL	*ctl,
	uint8		socketid,
	APML_SMBUS_CMD	cmd,
	uint8		proc_addr,
	uint8		core,
	uint32		res)
{
	uint8	error_found;
	uint32	i, desc_len;

	error_found = APML_FALSE;

	if (ctl->debug_level >= APML_DEBUG_ON)
	{
		desc_len = sizeof(hal_err_desc) / sizeof(APML_ERR_DESC);

		printf("\n****** %s ERROR %d", cmd.id, (int)res);

		for (i = 0; i < desc_len; i++)
		{
			if (hal_err_desc[i].code == res)
			{
				printf(", %s", hal_err_desc[i].desc);
				error_found = 1;
				break;
			}
		}
		if (error_found != 1) {
			printf(", ");
			apml_print_user_error(res);
		}
		printf("\n*\n");

		printf("* SocketID: 0x%02x", socketid);
		printf("\n");

		printf("* PROC_ADDR: 0x%02x", proc_addr);
		printf("\n");

		printf("* Core: 0x%02x", core);
		printf("\n*\n");

		printf("*  XMT BFR[%02d]:", (int)cmd.xlen);
		printf(" %02X", proc_addr*2);
		for (i = 0 ; i < cmd.xlen ; i++)
			printf(" %02X", cmd.xd[i]);
		printf("\n*  RCV BFR[%02d]:", (int)cmd.rlen);
		printf(" %02X", proc_addr*2+1);
		for (i = 0 ; i < cmd.rlen ; i++)
			printf(" %02X", cmd.rd[i]);

		printf("\n*\n******************************************************\n");
	}
}



/* Log SMBus traffic to stdout */
void apml_trace_cmd(
	uint8	proc_addr,
	uint32	msg_wr_bytes,
	uint8	*wr_data,
	uint32	msg_rd_bytes,
	uint8	*rd_data)
{
	uint32 i;

	printf ("%02x", proc_addr * 2);

	for (i=0 ; i < msg_wr_bytes; i++)
		printf(" %02x", wr_data[i]);

	if (msg_rd_bytes > 0)
	{
		for (; i < 15 ; i++)
			printf("   ");
		printf ("< %02x", (int)((proc_addr * 2) + 1));
		for (i=0 ; i < msg_rd_bytes; i++)
			printf(" %02x", rd_data[i]);
	}

	printf("\n");
}



/* Generic RMI MSR Read Function
 * variation of SB-RMI Read Processor Register Command Protocol
 * used for reading SB-RMI Functions
*/
uint32 apml_read_rmi_msr(
	APML_DEV_CTL	*ctl,
	uint8		socketid,
	uint8		proc_addr,
	uint8		core_ndx,
	const CHAR	*in_cmd_id,
	uint32		reg_id,
	uint32		*reg_val_h,
	uint32		*reg_val_l,
	int			BMCInst)
{
	uint8		*wr_data, *in_data, i, j, thread_ndx;
	uint32		res, tmp;
	APML_SMBUS_CMD	cmd;

	cmd.id = in_cmd_id;

	if ( (core_ndx *2) >= ctl->threads[socketid])
		return(APML_UNK_THREAD);

    thread_ndx = core_ndx * 2;


	wr_data = cmd.xd;
	in_data = cmd.rd;

	wr_data[0] = 0x73; /* Read Register command */
	wr_data[1] = 0x07; /* Sending 7 bytes of data */
	wr_data[2] = 0x08; /* Receive 8 bytes back */
	wr_data[3] = 0x86; /* Read register command */
	wr_data[4] = thread_ndx; /* Target Thread */
	wr_data[5] = (reg_id)		& 0xFF;
	wr_data[6] = (reg_id >> 8)	& 0xFF;
	wr_data[7] = (reg_id >> 16)	& 0xFF;
	wr_data[8] = (reg_id >> 24)	& 0xFF;

	cmd.xlen = 9;
	cmd.rlen = 8 + 2; /* 8 bytes of data and 2 bytes of length+status */

	in_data[0] = 0;
	in_data[1] = 0xff;

	res = apml_block_process_call(ctl, socketid, proc_addr, cmd.xlen, wr_data, 
		cmd.rlen, in_data,BMCInst);
	if (res != APML_SUCCESS)
	{
		apml_hal_err_log(ctl, socketid, cmd, proc_addr, 0, res);
		return(res);
	}

	res = apml_check_resp(in_data[1]);
	if (res != APML_SUCCESS)
	{
		apml_hal_err_log(ctl, socketid, cmd, proc_addr, 0, res);
		return(res);
	}

    //store low 4 bytes into reg_val_l
	tmp = 0;

	for (i=0, j=0; i < 4 && j < 8; i++, j++)
	{
		tmp += (in_data[i+2] << (i*8));
	}
	*reg_val_l = tmp;

    //store high 4 bytes into reg_val_h
	tmp = 0;

	for (i=0; i < 4 && j < 8; i++, j++)
	{
		tmp += (in_data[i+6] << (i*8));
	}
	*reg_val_h = tmp;

	return(res);
}
	


/* Generic RMI MSR Write Function 
 * use both SB-RMI Load Address Command Protocol and SB-RMI Write Processor Register Command Protocol
*/
uint32 apml_write_rmi_msr(
	APML_DEV_CTL	*ctl,
	uint8		proc_ndx,   //proc_ndx and socketid are identically the same
	uint8		proc_addr,
	uint8		core_ndx,
	const CHAR	*in_cmd_id,
	uint32		reg_id,
	uint8		reg_val_size,
	uint32		reg_val_h,
	uint32		reg_val_l,
	int			BMCInst)
{
	uint8		i, j, *wr_data, *in_data, thread_ndx;
	uint32		res, tmp;
	APML_SMBUS_CMD	cmd;

	cmd.id = in_cmd_id;

	if ( (core_ndx*2) >= ctl->threads[proc_ndx])
		return(APML_UNK_THREAD);

	/* Load SAR with the register ID */
	res = apml_load_sar(ctl, LOAD_ADDRESS_COMMAND, proc_addr, proc_ndx, 
		core_ndx, 0, reg_id,BMCInst);
	
	if (res != APML_SUCCESS)
		return res;
	
	// "core" range 00h - 1Fh, P0 use "core_ndx" range 00h - 7Fh
    thread_ndx = core_ndx * 2;

	wr_data = cmd.xd;
	in_data = cmd.rd;

	wr_data[0] = 0x71;
	wr_data[1] = reg_val_size + 2;  //APML 1.04, this value == reg_val_size + 2
	wr_data[2] = 0x87;
	wr_data[3] = thread_ndx;  // Target Thread

    //store low 4 bytes into wr_data[4] ~ wr_data[7]
	tmp = reg_val_l;

	for (i=4, j=0; i < 8 && j < reg_val_size; i++, j++)
	{
		wr_data[i] = tmp & 0xff;
		tmp >>= 8;

	}

    //store high 4 bytes into wr_data[8] ~ wr_data[11]
	tmp = reg_val_h;

	for (; i < 12 && j < reg_val_size; i++, j++)
	{
		wr_data[i] = tmp & 0xff;
		tmp >>= 8;
	}

	cmd.xlen = i;
	cmd.rlen = 2;   // RdDataLen 1 byte, Status 1 byte

	in_data[0] = 0;
	in_data[1] = 0xff;
	
	res = apml_block_process_call(ctl, proc_ndx, proc_addr, cmd.xlen, wr_data, 
		cmd.rlen, in_data,BMCInst);

	if (res != APML_SUCCESS)
	{
		apml_hal_err_log(ctl, proc_ndx, cmd, proc_addr, core_ndx, res);
		return(res);
	}
	
	res = apml_check_resp(in_data[1]);
	if (res != APML_SUCCESS)
	{
		apml_hal_err_log(ctl, proc_ndx, cmd, proc_addr, core_ndx, res);
		return(res);
	}

	apml_update_sar(ctl, proc_ndx, core_ndx, reg_val_size);

	return(res);
}



/* SMBus Block Write-Block Read Process Call Helper Function -
 * 	Write PEC creation and statistic recording */
uint32 apml_exec_block_process_call(
	APML_DEV_CTL	*dev,
	uint8		socketid,
	uint8		proc_addr,
	uint32		wr_len,
	uint8		*wr_data,
	uint32		rd_len,
	uint8		*rd_data,
	int			BMCInst)
{

	uint32	res, msg_rd_bytes, msg_wr_bytes;


	msg_rd_bytes = rd_len;  //CPUID: 10

	msg_rd_bytes++;     //include PEC byte : CPUID:10+1==11 bytes


	msg_wr_bytes = wr_len;  //CPUID: 10, 11 if PEC appended    


	res = user_smbus_bwr_brd_process(dev, socketid, proc_addr, wr_data, 
		msg_wr_bytes, rd_data, msg_rd_bytes,BMCInst);

	dev->stat.bytes_xmtd += (msg_wr_bytes+2); /* +2 for the addr bytes */
	dev->stat.bytes_rcvd += (msg_rd_bytes);

	if (dev->trace_level >= APML_TRACE_ON)
		apml_trace_cmd(proc_addr, msg_wr_bytes, wr_data, 
			msg_rd_bytes, rd_data);

	return(res);
}



/* SMBus Block Write-Block Read Process Call -
 *  Calculating PEC for sending command,
 * 	Received PEC validation and command retry */
uint32 apml_block_process_call(
	APML_DEV_CTL	*dev,
	uint8		socketid,
	uint8		proc_addr,
	uint32		wr_len,
	uint8		*wr_data,
	uint32		rd_len,
	uint8		*rd_data,
	int			BMCInst)
{
#if IS_PEC_ENABLE == 1
	uint8	crc = 0, addr;
#endif
	uint32	res=0, i;

	if( (res = apml_check_LPC_RST()) != APML_SUCCESS )
		return(res);

	dev->stat.cmd_cnt++;

#if IS_PEC_ENABLE == 1
	// Calculate Sent PEC from wr_data
	addr = (proc_addr * 2);
	crc = apml_pec(0, (int32)1, &addr);
	crc = apml_pec(crc, wr_len, wr_data);
	wr_data[wr_len] = crc;  //append crc to wr_data
    wr_len ++;  //wr_len == 10+1 == 11
#endif

	for (i=0 ; i < APML_CMD_RETRIES ; i++)  //retry if PEC check fail
	{
		res = apml_exec_block_process_call(dev, socketid, proc_addr, wr_len, 
			wr_data, rd_len, rd_data,BMCInst);

		if (res != APML_SUCCESS)
			return(res);

		if (rd_data[0] != (rd_len-1))   //read data size wrong
		{
			if (rd_data[0] == 1) {  //check status codes
				/* APML error from the processor */
				res = apml_check_resp(rd_data[1]);
				return(res);
			}
			return(APML_RCV_LEN_MATCH);
		}

#if IS_PEC_ENABLE == 1
		    // Check Received PEC, Calculate crc from rd_data
		addr = (proc_addr * 2) | 0x01;
		crc = apml_pec(crc, (int32)1, &addr);   //initial value is wr_data[] crc
        crc = apml_pec(crc, rd_len, rd_data);

		if (crc != rd_data[rd_len]) //if PEC check fail, retry untill "APML_CMD_RETRIES" reached.
		{
			dev->stat.rcv_pec_errors++;
			res = APML_PEC_FAIL;
		}
		else    //PEC check succeeded
		{
			return(APML_SUCCESS);
		}
#endif  /* if IS_PEC_ENABLE == 1*/
        //no PEC checking
        return(APML_SUCCESS);

	}   /* for (i=0 ; i < APML_CMD_RETRIES ; i++) */

	return(res);
}

uint32 apml_read_mailbox_service_with_mboxcmd(
    APML_DEV_CTL	*dev,
    uint8       socketid,
	uint8		proc_addr,
	uint8		mboxcmd,
	uint8		*reg_34,
	uint8		*reg_33,
	uint8		*reg_32,
	uint8       *reg_31,
	int			BMCInst)

{
    uint8   crc, addr, reg;
    uint32	res, temp32;
    //uint8 retries = 0;
    uint8 i;
// user_smbus_write_byte only write 1 byte to a slave address, which is reg. so there is no data byte write to that reg.
// to write a data byte to a reg on a slave address, use user_smbus_write_word instead, or call apml_write_rmi_reg to support PEC calculation.

    uint8 data_ini[3] = { 0x80,  0x1, 0x01}; //initiation data, APML 1.04
    uint8 reg_ini[3] = { 0x3F, 0x38, 0x40}; //initiation reg , APML 1.04

    uint8 reg_read[4] = { 0x31, 0x32, 0x33, 0x34};   //read reg, APML 1.04

	if( (res = apml_check_LPC_RST()) != APML_SUCCESS )
		return(res);

    data_ini[1] = mboxcmd;
    // initiate mailbox service by writing SBRMI_x3F, SBRMI_x38, then write SBRMI_x40 to generate interrupt
    for(i=0; i<3; i++)
    {
        temp32 = 0x0;
    	dev->stat.cmd_cnt++;

        addr = proc_addr * 2; // 8 bit address for CRC calculation
    	crc = apml_pec(0, 1, &addr);
    	crc = apml_pec(crc, 1, &reg_ini[i]);
    	crc = apml_pec(crc, 1, &data_ini[i]);
    	temp32 = (crc << 8) + data_ini[i];

    	res = user_smbus_write_word(dev, socketid, proc_addr, reg_ini[i], temp32,BMCInst);
	    if (res != APML_SUCCESS)
            return (res);
    }

    // poll SB-RMI_x02 bit1 for Alert of indication of completion
    for (i = 0; i < 10; i ++) {
        reg = 0x02;
        temp32 = 0;
        res = user_smbus_read_byte(dev, socketid, proc_addr, reg, &temp32,BMCInst);
    	if (res == APML_SUCCESS && (temp32 & 0x02)== 0x02)
	        break;
    }
    
    if (i == 10)
    {
        TCRIT(" ALERT TIMEOUT\n");
        return USER_GEN_ERROR;
    }

    //printf (" sucessfully read SB-RMI-x02 res = 0x%x\n",res);
    if ((temp32 & 0x02)== 0x02) //indicate completion
    {
        //printf (" SB-RMI-x02 = 0x%x\n",temp32);
        
        //Read data from {SBRMI_x34(MSB):SBRMI_x31(LSB)}
        for( i=0 ; i < 4; i++)
        {
            reg= reg_read[i];
            temp32 = 0;

            res = user_smbus_read_byte(dev, socketid, proc_addr, reg, &temp32 ,BMCInst);
            if (res != APML_SUCCESS)
                return (res);
            else {
                if(i==0)
                    *reg_31 = temp32;
                if(i==1)
                    *reg_32 = temp32;
                if(i==2)
                    *reg_33 = temp32;
                if(i==3)
                    *reg_34 = temp32;
            }
        }
        reg = 0x02;
        temp32 = 0x02;
        res = user_smbus_write_word(dev, socketid, proc_addr, reg, temp32,BMCInst);
        if (res != APML_SUCCESS)
        {
            return (res);
        }
    }

    return (res);
}

uint32 apml_read_mailbox_service(
    APML_DEV_CTL	*dev,
    uint8       socketid,
	uint8		proc_addr,
	uint8		*reg_34,
	uint8		*reg_33,
	uint8		*reg_32,
	uint8       *reg_31,
	int			BMCInst)

{
    return (apml_read_mailbox_service_with_mboxcmd(dev, socketid, proc_addr, 0x1, reg_34, reg_33, reg_32, reg_31, BMCInst) );
}

uint32 apml_write_mailbox_service_with_mboxcmd(
        APML_DEV_CTL    *dev,
        uint8       socketid,
        uint8       proc_addr,
    	uint8		mboxcmd,
        uint32      mboxdata,
        int         BMCInst)
    
{
    uint32  res;
    uint32  temp32=0x0;
    uint8   crc, reg, addr, temp8=0x0, i;

    uint8 data_ini[3] = { 0x80,  0x2, 0x01}; //initiation data, APML 1.04
    uint8 reg_ini[3] =  { 0x3F, 0x38, 0x40}; //initiation reg , APML 1.04

    uint8 reg_write[4] = { 0x39, 0x3a, 0x3b, 0x3c};   //read reg, APML 1.04

	if( (res = apml_check_LPC_RST()) != APML_SUCCESS )
		return(res);
    
    data_ini[1] = mboxcmd;
    // initiate mailbox service by writing SBRMI_x3F, SBRMI_x38, for Write operation
    for(i=0; i<2; i++)
    {
        temp32 = 0x0;
    	dev->stat.cmd_cnt++;

        addr = proc_addr * 2; // 8 bit address for CRC calculation
    	crc = apml_pec(0, 1, &addr);
    	crc = apml_pec(crc, 1, &reg_ini[i]);
    	crc = apml_pec(crc, 1, &data_ini[i]);
    	temp32 = (crc << 8) + data_ini[i];

    	res = user_smbus_write_word(dev, socketid, proc_addr, reg_ini[i], temp32,BMCInst);
	    if (res != APML_SUCCESS)
            return (res);
    }

    for(i=0; i<4; i++)
    {
        temp32 = 0x0;
        temp8 = 0x0;
    	dev->stat.cmd_cnt++;
        temp8 = ((mboxdata >> (8*i))& 0xFF);

        addr = proc_addr * 2; // 8 bit address for CRC calculation
    	crc = apml_pec(0, 1, &addr);
    	crc = apml_pec(crc, 1, &reg_write[i]);
    	crc = apml_pec(crc, 1, &temp8);
    	temp32 = (crc << 8) + temp8;

    	res = user_smbus_write_word(dev, socketid, proc_addr, reg_write[i], temp32,BMCInst);
	    if (res != APML_SUCCESS)
            return (res);
    }

    // Generate interrupt to host by writing 0x1 to SBRMI_x40,
    temp32 = 0x0;
	dev->stat.cmd_cnt++;

    addr = proc_addr * 2; // 8 bit address for CRC calculation
	crc = apml_pec(0, 1, &addr);
	crc = apml_pec(crc, 1, &reg_ini[2]);
	crc = apml_pec(crc, 1, &data_ini[2]);
	temp32 = (crc << 8) + data_ini[2];

	res = user_smbus_write_word(dev, socketid, proc_addr, reg_ini[2], temp32,BMCInst);
    if (res != APML_SUCCESS)
        return (res);

    // poll SB-RMI_x02 bit1 for Alert of indication of completion
    for (i = 0; i < 10; i ++) {
        reg = 0x02;
        temp32 = 0;
        res = user_smbus_read_byte(dev, socketid, proc_addr, reg, &temp32,BMCInst);
    	if (res == APML_SUCCESS && (temp32 & 0x02)== 0x02)
	        break;
    }

    if (i == 10)
    {
        TCRIT(" ALERT TIMEOUT\n");
        return USER_GEN_ERROR;
    }

    reg = 0x02;
    temp32 = 0x02;
    res = user_smbus_write_word(dev, socketid, proc_addr, reg, temp32,BMCInst);

    return (res);

}

    
uint32 apml_write_mailbox_service(
        APML_DEV_CTL    *dev,
        uint8       socketid,
        uint8       proc_addr,
        uint32      mboxdata,
        int         BMCInst)
    
{
    return (apml_write_mailbox_service_with_mboxcmd(dev, socketid, proc_addr, 0x2, mboxdata, BMCInst) );
}
    
uint32 apml_writeread_rmi_mailbox_service(
        APML_DEV_CTL    *ctl,
            uint8       socketid,
            uint8       dieid,
        	uint8		mboxcmd,
            uint32      WRData,
            uint32      *Ptr_RDData,
            int         BMCInst)
        {
            uint8       proc_addr;
            uint32      res; 
            uint8       sbrmi_reg_34,sbrmi_reg_33,sbrmi_reg_32,sbrmi_reg_31;

        
            res = user_get_proc_addr(ctl, socketid,dieid, PROC_USE_RMI, &proc_addr);
            if (res != APML_SUCCESS)
                return(res);
  
        
        
            res = apml_writeread_mailbox_service_with_mboxcmd(ctl, socketid, proc_addr, mboxcmd, WRData,
                                                              &sbrmi_reg_34, &sbrmi_reg_33, &sbrmi_reg_32, &sbrmi_reg_31,
                                                              BMCInst);
            /* Extract Information */
            if (res == APML_SUCCESS) {
                *Ptr_RDData = sbrmi_reg_34 << 24 | sbrmi_reg_33 <<16 | sbrmi_reg_32 <<8 | sbrmi_reg_31;
            }

            return(res);
        }

uint32 apml_writeread_mailbox_service_with_mboxcmd(
        APML_DEV_CTL    *dev,
        uint8       socketid,
        uint8       proc_addr,
    	uint8		mboxcmd,
        uint32      mboxwrdata,
        uint8		*reg_34,
        uint8		*reg_33,
        uint8		*reg_32,
        uint8       *reg_31,
        int         BMCInst)
    
{
    uint32  res;
    uint32  temp32=0x0;
    uint8   crc, reg, addr, temp8=0x0, i;

    uint8 data_ini[4] = { 0x80,  0x2, 0x01,  0x2}; //initiation data
    uint8 reg_ini[4] =  { 0x3F, 0x38, 0x40, 0x30}; //initiation reg

    uint8 reg_write[4] = { 0x39, 0x3a, 0x3b, 0x3c};   //write reg
    uint8 reg_read[4] = { 0x31, 0x32, 0x33, 0x34};   //read reg

	if( (res = apml_check_LPC_RST()) != APML_SUCCESS )
		return(res);
    
    data_ini[1] = mboxcmd;
    data_ini[3] = mboxcmd;
    // initiate mailbox service by writing SBRMI_x3F, SBRMI_x38, for Write operation
    for(i=0; i<2; i++)
    {
        temp32 = 0x0;
    	dev->stat.cmd_cnt++;

        addr = proc_addr * 2; // 8 bit address for CRC calculation
    	crc = apml_pec(0, 1, &addr);
    	crc = apml_pec(crc, 1, &reg_ini[i]);
    	crc = apml_pec(crc, 1, &data_ini[i]);
    	temp32 = (crc << 8) + data_ini[i];

    	res = user_smbus_write_word(dev, socketid, proc_addr, reg_ini[i], temp32,BMCInst);
	    if (res != APML_SUCCESS)
            return (res);
    }

    for(i=0; i<4; i++)
    {
        temp32 = 0x0;
        temp8 = 0x0;
    	dev->stat.cmd_cnt++;
        temp8 = ((mboxwrdata >> (8*i))& 0xFF);

        addr = proc_addr * 2; // 8 bit address for CRC calculation
    	crc = apml_pec(0, 1, &addr);
    	crc = apml_pec(crc, 1, &reg_write[i]);
    	crc = apml_pec(crc, 1, &temp8);
    	temp32 = (crc << 8) + temp8;

    	res = user_smbus_write_word(dev, socketid, proc_addr, reg_write[i], temp32,BMCInst);
	    if (res != APML_SUCCESS)
            return (res);
    }

    // Write the command to SBRI_0x30,
    temp32 = 0x0;
	dev->stat.cmd_cnt++;

    addr = proc_addr * 2; // 8 bit address for CRC calculation
	crc = apml_pec(0, 1, &addr);
	crc = apml_pec(crc, 1, &reg_ini[3]);
	crc = apml_pec(crc, 1, &data_ini[3]);
	temp32 = (crc << 8) + data_ini[3];

	res = user_smbus_write_word(dev, socketid, proc_addr, reg_ini[2], temp32,BMCInst);
    if (res != APML_SUCCESS)
        return (res);

    // Generate interrupt to host by writing 0x1 to SBRMI_x40,
    temp32 = 0x0;
	dev->stat.cmd_cnt++;

    addr = proc_addr * 2; // 8 bit address for CRC calculation
	crc = apml_pec(0, 1, &addr);
	crc = apml_pec(crc, 1, &reg_ini[2]);
	crc = apml_pec(crc, 1, &data_ini[2]);
	temp32 = (crc << 8) + data_ini[2];

	res = user_smbus_write_word(dev, socketid, proc_addr, reg_ini[2], temp32,BMCInst);
    if (res != APML_SUCCESS)
        return (res);

    // poll SB-RMI_x02 bit1 for Alert of indication of completion
    for (i = 0; i < 10; i ++) {
        reg = 0x02;
        temp32 = 0;
        res = user_smbus_read_byte(dev, socketid, proc_addr, reg, &temp32,BMCInst);
    	if (res == APML_SUCCESS && (temp32 & 0x02)== 0x02)
	        break;
    }

    if (i == 10)
    {
        TCRIT(" ALERT TIMEOUT\n");
        return USER_GEN_ERROR;
    }

    //printf (" sucessfully read SB-RMI-x02 res = 0x%x\n",res);
    if ((temp32 & 0x02)== 0x02) //indicate completion
    {
        //printf (" SB-RMI-x02 = 0x%x\n",temp32);
        
        //Read data from {SBRMI_x34(MSB):SBRMI_x31(LSB)}
        for( i=0 ; i < 4; i++)
        {
            reg= reg_read[i];
            temp32 = 0;

            res = user_smbus_read_byte(dev, socketid, proc_addr, reg, &temp32 ,BMCInst);
            if (res != APML_SUCCESS)
                return (res);
            else {
                if(i==0)
                    *reg_31 = temp32;
                if(i==1)
                    *reg_32 = temp32;
                if(i==2)
                    *reg_33 = temp32;
                if(i==3)
                    *reg_34 = temp32;
            }
        }

        reg = 0x02;
        temp32 = 0x02;
        res = user_smbus_write_word(dev, socketid, proc_addr, reg, temp32,BMCInst);
        if (res != APML_SUCCESS)
        {
            return (res);
        }
    }
    return (res);

}
        


/* Calculate SMBus Packet Error Checking (PEC) */
uint8 apml_pec(
	uint8	init,
	uint32	count,
	uint8	*data)
{
	uint8	crc;
	uint32	i, j, tmp;

	crc = init;

	for(i = 0; i < count; i++)
	{
		tmp = ((crc ^ data[i]) << 8);

		for (j=0 ; j < 8 ; j++)
		{
			if ((tmp & 0x8000) != 0)
				tmp ^= 0x8380;

			tmp *= 2;
		}

		crc = ((tmp >> 8) & 0xFF);
	}

	return(crc);
}



/* Core-specific load APML Address Register */
uint32 apml_load_sar(
	APML_DEV_CTL	*ctl,
	uint8		command,
	uint8		proc_addr,
	uint8		proc_ndx,
	uint8		core_ndx,
	uint32		addr_h,
	uint32		addr_l,
	int			BMCInst)
{
	uint8		*wr_data, *in_data, thread_ndx;
	uint32		res, cmp, i, j;
	APML_SMBUS_CMD	cmd;

	/* Determine number of address bytes changed */
	cmp = ctl->last_sar_h[proc_ndx][core_ndx] ^ addr_h; // ctl->last_sar_h[0-1][0-31]

	for (i = 8 ; i > 4 ; i--)
	{
		if ((cmp & 0xFF000000) != 0)
			break;
		cmp <<= 8;
	}
	if (i == 4)
	{
		cmp = ctl->last_sar_l[proc_ndx][core_ndx] ^ addr_l; // ctl->last_sar_l[0-1][0-31]

		for (; i > 0 ; i--)
		{
			if ((cmp & 0xFF000000) != 0)
				break;
			cmp <<= 8;
		}
	}

	if (i < ctl->min_addr_bytes)
	{
		/* Load at least the user requested number of bytes */
		i=ctl->min_addr_bytes;
		if (i > 8)
			i=8;
	}

	if (i == 0)
    {
    		return(APML_SUCCESS);
    }

    thread_ndx = core_ndx * 2;

	wr_data = cmd.xd;
	in_data = cmd.rd;

	cmd.id = "LOAD ADDR REG";

	wr_data[0] = 0x71;
	wr_data[1] = i + 2;         // this should be fixed value 6 if 4 bytes address sent, but value 10 if 8 bytes address sent is working too.
	wr_data[2] = command;       // 0x81, Load Address Command.
	wr_data[3] = thread_ndx;

	cmp = addr_l;
	for (j=4; i > 0 && j < 8; i--, j++)
	{
		wr_data[j] = (cmp & 0xFF);
		cmp >>= 8;
	}

	cmp = addr_h;
	for (; i > 0; i--, j++)
	{
		wr_data[j] = (cmp & 0xFF);
		cmp >>= 8;
	}
	cmd.xlen = j;
	cmd.rlen = 2;   //NOT include PEC byte: RdDataLen 1 byte + Status 1 byte == 2 bytes

	in_data[0] = 0;
	in_data[1] = 0xff;

	res = apml_block_process_call(ctl, proc_ndx, proc_addr, cmd.xlen, wr_data, 
		cmd.rlen, in_data,BMCInst);
	if (res != APML_SUCCESS)
	{
		apml_hal_err_log(ctl, proc_ndx, cmd, proc_addr, core_ndx, res);
		return(res);
	}

	res = apml_check_resp(in_data[1]);
	if (res != APML_SUCCESS)
	{
		apml_hal_err_log(ctl, proc_ndx, cmd, proc_addr, core_ndx, res);
		return(res);
	}

	ctl->last_sar_h[proc_ndx][core_ndx] = addr_h;
	ctl->last_sar_l[proc_ndx][core_ndx] = addr_l;

	return(res);
}



/* Core-specific update APML_DEV_CTL address */
uint32 apml_update_sar(
	APML_DEV_CTL	*ctl,
	uint8		proc_ndx,
	uint8		core_ndx,
	uint32		inc)
{
	uint32 sar_h, sar_l;

	sar_h = ctl->last_sar_h[proc_ndx][core_ndx];
	sar_l = ctl->last_sar_l[proc_ndx][core_ndx];

	if (sar_l >= 0xFFFFFFF8)
	{
		if (inc > (0xFFFFFFFF - sar_l))
		{
			sar_h++;
			sar_l = ((inc-(0xFFFFFFFF-sar_l)-1));
		}
		else
		{
			sar_l += inc;
		}

		ctl->last_sar_l[proc_ndx][core_ndx] = sar_l;
		ctl->last_sar_h[proc_ndx][core_ndx] = sar_h;
	}
	else
	{
		sar_l += inc;
		ctl->last_sar_l[proc_ndx][core_ndx] = sar_l;
	}

	return(APML_SUCCESS);
}
