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

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
//#include "dbgout.h"
#include "checksum.h"
#include "peci_interface.h"
#include "peciifc.h"
#include "common.h"

#define PECI_DEV_ID                     0x0 // This will be '0' by Default... /dev/peci0
#define PECI_DOMAIN                     0x0 // Multi-Domain support. Default is '0'
#define MAX_STR_LENGTH  128

#define GET_DIB_REQ	0
#define RD_PKG_CFG_REQ	1
#define WR_PKG_CFG_REQ	2
#define RD_PCI_CFG_LOCAL 3

static int SendPECICmd(void * peciReq, int reqSize, int cmdType, void * peciRes)
{
    int PECIFd = 0, PECIRet = 0, RetVal = 0, i = 0, Idx = 0;
    char PECIDeviceName[MAX_STR_LENGTH] = {0};
    peci_cmd_t PECICmd;

    UN_USED(reqSize);

    SRdPkgConfigReq *RdPkgConfigReq;
    SRdPkgConfigRes *RdPkgConfigRes;
    SWrPkgConfigReq *WrPkgConfigReq;
    SWrPkgConfigRes *WrPkgConfigRes;
    SRdPCIConfigLocalReq *RdPCIConfigLocalReq;
    SRdPCIConfigLocalRes *RdPCIConfigLocalRes;

    /* Form PECI Request */
    memset(&PECICmd, 0, sizeof(peci_cmd_t));
    PECICmd.dev_id = PECI_DEV_ID;
    PECICmd.domain = PECI_DOMAIN;

    if (cmdType == RD_PKG_CFG_REQ)
    {
        PRINT(PRINT_DBG, PRINT_INFO, "RD_PKG_CFG_REQ\n");
        RdPkgConfigReq = (SRdPkgConfigReq *)peciReq;
   	PECICmd.target = RdPkgConfigReq->sHeader.u8ClientAddr;
    	PECICmd.read_len = RdPkgConfigReq->sHeader.u8ReadLength;
    	PECICmd.write_len = RdPkgConfigReq->sHeader.u8WriteLength;
        PECICmd.write_buffer[Idx++] = RdPkgConfigReq->u8CmdCode;
        PECICmd.write_buffer[Idx++] = RdPkgConfigReq->u8HostID_Retry;
        PECICmd.write_buffer[Idx++] = RdPkgConfigReq->u8Index;
        PECICmd.write_buffer[Idx++] = RdPkgConfigReq->u16Parameter & 0xFF;
        PECICmd.write_buffer[Idx++] = (RdPkgConfigReq->u16Parameter >> 8) & 0xFF;
    }
    else if (cmdType == WR_PKG_CFG_REQ)
    {
        PRINT(PRINT_DBG, PRINT_INFO, "WR_PKG_CFG_REQ\n");
        WrPkgConfigReq = (SWrPkgConfigReq *)peciReq;
   	PECICmd.target = WrPkgConfigReq->sHeader.u8ClientAddr;
    	PECICmd.read_len = WrPkgConfigReq->sHeader.u8ReadLength;
    	PECICmd.write_len = WrPkgConfigReq->sHeader.u8WriteLength;
        PECICmd.write_buffer[Idx++] = WrPkgConfigReq->u8CmdCode;
        PECICmd.write_buffer[Idx++] = WrPkgConfigReq->u8HostID_Retry;
        PECICmd.write_buffer[Idx++] = WrPkgConfigReq->u8Index;
        PECICmd.write_buffer[Idx++] = WrPkgConfigReq->u16Parameter & 0xFF;
        PECICmd.write_buffer[Idx++] = (WrPkgConfigReq->u16Parameter >> 8) & 0xFF;
        for (i = 0; i < 4; i++)
        {
        	PECICmd.write_buffer[Idx++] = WrPkgConfigReq->u8Data[i];
        }
        PECICmd.write_buffer[Idx++] = WrPkgConfigReq->u8AWFCS;
    }
    else if (cmdType == RD_PCI_CFG_LOCAL)
    {
        PRINT(PRINT_DBG, PRINT_INFO, "RD_PCI_CFG_LOCAL\n");
        RdPCIConfigLocalReq = (SRdPCIConfigLocalReq *)peciReq;
   	PECICmd.target = RdPCIConfigLocalReq->sHeader.u8ClientAddr;
    	PECICmd.read_len = RdPCIConfigLocalReq->sHeader.u8ReadLength;
    	PECICmd.write_len = RdPCIConfigLocalReq->sHeader.u8WriteLength;
        PECICmd.write_buffer[Idx++] = RdPCIConfigLocalReq->u8CmdCode;
        PECICmd.write_buffer[Idx++] = RdPCIConfigLocalReq->u8HostID_Retry;
        for (i = 0; i < 3; i++)
        {
        	PECICmd.write_buffer[Idx++] = RdPCIConfigLocalReq->u8PCIConfigAddr[i];
        }
    }
    else
	return -1;

    PRINT(PRINT_DBG, PRINT_INFO, "PECI request: 0x%x 0x%x 0x%x ", PECICmd.target, PECICmd.write_len, PECICmd.read_len);
    for(Idx = 0; Idx < PECICmd.write_len; Idx++)
    {
        PRINT(PRINT_DBG, PRINT_INFO, "0x%x ", PECICmd.write_buffer[Idx]);
    }
    PRINT(PRINT_DBG, PRINT_INFO, "\n");
#if 0
    char printbuf[512] = {0};
    char tempbuf[512] = {0};

    strcpy(printbuf, "echo -e \"ipmitool -H $1 -U $2 -P $3 -I lanplus raw 0x30 0xe6 ");
    snprintf(tempbuf, sizeof(tempbuf)-1, "0x%x 0x%x 0x%x ", PECICmd.target, PECICmd.write_len, PECICmd.read_len);
    strncat(printbuf, tempbuf, strlen(tempbuf));
    for(Idx = 0; Idx < PECICmd.write_len; Idx++)
    {
        snprintf(tempbuf, sizeof(tempbuf)-1, "0x%x ", PECICmd.write_buffer[Idx]);
        strncat(printbuf, tempbuf, strlen(tempbuf));
    }
    strncat(printbuf, "\" >> /var/crashdump.sh", sizeof(printbuf)-1);
    system(printbuf);
#endif
    /* Send PECI Request */
    RetVal = snprintf(PECIDeviceName, MAX_STR_LENGTH, "/dev/peci%d", PECICmd.dev_id);
    if((RetVal < 0)||(RetVal >= (signed) sizeof(PECIDeviceName)))
    {
         printf("Buffer Overflow\n");
         return -1;
    }

    /* Open the peci device file */
    PECIFd = open(PECIDeviceName, O_RDWR );
    if( PECIFd == -1 )
    {
        printf("PECI Device Open Failed \n");
        return -1;
    }
    else
    {
        PECIRet = ioctl(PECIFd, (unsigned long)PECI_ISSUE_CMD, &PECICmd);
        if(PECIRet == -1 )
        {
            printf("PECI Issue Command failed due to ioctl error \n");
            if(close(PECIFd) != 0)
            {
                printf("Failed in closing PECIFd \n");
            }
            return -1;
        }
    }

    if(close(PECIFd) != 0)
    {
        printf("Failed in closing PECIFd \n");
        return -1;
    }

    if(PECICmd.status < 0)
    {
        printf("PECI Issue Command failed. Status :%d \n", PECICmd.status);
    }

    PRINT(PRINT_DBG, PRINT_INFO, "PECI response: ");
    for(Idx = 0; Idx < PECICmd.read_len; Idx++)
    {
        PRINT(PRINT_DBG, PRINT_INFO, "0x%x ", PECICmd.read_buffer[Idx]);
    }
    PRINT(PRINT_DBG, PRINT_INFO, "\n\n");

    /* Form PECI Response */
    if (cmdType == RD_PKG_CFG_REQ)
    {
	RdPkgConfigRes = (SRdPkgConfigRes *)peciRes;
	//RdPkgConfigRes->u8CompletionCode = PECI_CC_SUCCESS;
	RdPkgConfigRes->u8CompletionCode = PECICmd.read_buffer[0];
	for(Idx = 0; (Idx < ((PECICmd.read_len)-1)) && (Idx < 8); Idx++)
    	{
        	RdPkgConfigRes->u8Data[Idx] = PECICmd.read_buffer[Idx+1];
    	}
    }
    else if (cmdType == WR_PKG_CFG_REQ)
    {
	WrPkgConfigRes = (SWrPkgConfigRes *)peciRes;
	//WrPkgConfigRes->u8CompletionCode = PECI_CC_SUCCESS;
	WrPkgConfigRes->u8CompletionCode = PECICmd.read_buffer[0];
    }
    else if (cmdType == RD_PCI_CFG_LOCAL)
    {
	RdPCIConfigLocalRes = (SRdPCIConfigLocalRes *)peciRes;
	RdPCIConfigLocalRes->u8CompletionCode = PECICmd.read_buffer[0];
	for(Idx = 0; (Idx < ((PECICmd.read_len)-1)) && (Idx < 4); Idx++)
    	{
        	RdPCIConfigLocalRes->u8Data[Idx] = PECICmd.read_buffer[Idx+1];
    	}
    }
    else
	return -1;

    return 0;
}


#ifndef SPX_BMC_ACD
int PECI_Ping(SPeciHeader *psHeader)
{
	int ret = 0;

	UN_USED(psHeader);
	// System specific code goes here.

	return(ret);

}

int PECI_GetDIB(SGetDIBReq *psGetDIBReq, SGetDIBRes *psGetDIBRes)
{
	int ret = 0;

	if(!psGetDIBReq || !psGetDIBRes)
	{
		printf("%s: Required parameters invalid.\n", __FUNCTION__);
		return -1;
	}

	// System specific code goes here.

	return(ret);
}

int PECI_GetTemp(SGetTempReq *psGetTempReq, SGetTempRes *psGetTempRes)
{
	int ret = 0;

	if(!psGetTempReq || !psGetTempRes)
	{
		printf("%s: Required parameters invalid.\n", __FUNCTION__);
		return -1;
	}

	// System specific code goes here.

	return(ret);
}
#endif

int PECI_RdPkgConfig (SRdPkgConfigReq *psRdPkgConfigReq, SRdPkgConfigRes *psRdPkgConfigRes)
{
	int ret = 0;

	if(!psRdPkgConfigReq || !psRdPkgConfigRes)
	{
		printf("%s: Required parameters invalid.\n", __FUNCTION__);
		return -1;
	}

	// System specific code goes here.
	if (SendPECICmd(psRdPkgConfigReq, sizeof(SRdPkgConfigReq), RD_PKG_CFG_REQ, psRdPkgConfigRes) != 0)
		return -1;

	return(ret);
}


static unsigned char calculate_fcs(SWrPkgConfigReq *psWrPkgConfigReq)
{
    unsigned char tbuf[12];

    /*tbuf[0] = psWrPkgConfigReq->sHeader.u8ClientAddr;
    tbuf[1] = psWrPkgConfigReq->sHeader.u8WriteLength;
    tbuf[2] = psWrPkgConfigReq->sHeader.u8ReadLength;
    tbuf[3] = psWrPkgConfigReq->u8CmdCode;
    tbuf[4] = psWrPkgConfigReq->u8HostID_Retry;*/
    memcpy(&tbuf[0], psWrPkgConfigReq, (psWrPkgConfigReq->sHeader.u8WriteLength) + 2);

    return CalculateCRC8(tbuf, (psWrPkgConfigReq->sHeader.u8WriteLength) + 2);
}

int PECI_WrPkgConfig(SWrPkgConfigReq *psWrPkgConfigReq, SWrPkgConfigRes *psWrPkgConfigRes)
{
	int ret = 0;

	if(!psWrPkgConfigReq|| !psWrPkgConfigRes)
	{
		printf("%s: Required parameters invalid.\n", __FUNCTION__);
		return -1;
	}

	psWrPkgConfigReq->u8AWFCS = calculate_fcs(psWrPkgConfigReq) ^ 0x80;

	// System specific code goes here.
	if (SendPECICmd(psWrPkgConfigReq, sizeof(SWrPkgConfigReq), WR_PKG_CFG_REQ, psWrPkgConfigRes) != 0)
		return -1;

	return(ret);
}

#ifndef SPX_BMC_ACD
int PECI_RdIAMSR (SRdIAMSRReq *psRdIAMSRReq, SRdIAMSRRes *psRdIAMSRRes)
{
	int ret = 0;

	if(!psRdIAMSRReq || !psRdIAMSRRes)
	{
		printf("%s: Required parameters invalid.\n", __FUNCTION__);
		return -1;
	}

	// System specific code goes here.

	return(ret);
}

int PECI_RdPCIConfig(SRdPCIConfigReq *psRdPCIConfigReq, SRdPCIConfigRes *psRdPCIConfigRes)
{
	int ret = 0;

	if(!psRdPCIConfigReq || !psRdPCIConfigRes)
	{
		printf("%s: Required parameters invalid.\n", __FUNCTION__);
		return -1;
	}

	// System specific code goes here.

	return(ret);
}
#endif

int PECI_RdPCIConfigLocal(SRdPCIConfigLocalReq *psRdPCIConfigLocalReq, SRdPCIConfigLocalRes *psRdPCIConfigLocalRes)
{
	int ret = 0;

	if(!psRdPCIConfigLocalReq || !psRdPCIConfigLocalRes)
	{
		printf("%s: Required parameters invalid.\n", __FUNCTION__);
		return -1;
	}
	else if (psRdPCIConfigLocalReq->sHeader.u8ReadLength != 2
		&& psRdPCIConfigLocalReq->sHeader.u8ReadLength != 3
		&& psRdPCIConfigLocalReq->sHeader.u8ReadLength != 5)
	{
		printf("%s: Invalid Option", __FUNCTION__);
		return -1;
	}

	// System specific code goes here.
	if (SendPECICmd(psRdPCIConfigLocalReq, sizeof(SRdPCIConfigLocalReq), RD_PCI_CFG_LOCAL, psRdPCIConfigLocalRes) != 0)
		return -1;

	return(ret);
}

#ifndef SPX_BMC_ACD
int PECI_WrPCIConfig(SWrPCIConfigReq *psWrPCIConfigReq, SWrPCIConfigRes *psWrPCIConfigRes)
{
	int ret = 0;

	if(!psWrPCIConfigReq || !psWrPCIConfigRes)
		{
			printf("%s: Required parameters invalid.\n", __FUNCTION__);
			return -1;
		}

	// System specific code goes here.

	return(ret);
}
#endif

