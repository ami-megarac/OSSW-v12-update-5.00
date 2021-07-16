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

#ifndef __PECI_INTERFACE_H__
#define __PECI_INTERFACE_H__

typedef struct _SPeciHeader
{
	unsigned char u8ClientAddr;
	unsigned char u8WriteLength;
	unsigned char u8ReadLength;
}__attribute__((packed)) SPeciHeader;

typedef struct _SGetDIBReq
{
	SPeciHeader  sHeader;
	unsigned char u8CmdCode;
}__attribute__((packed)) SGetDIBReq;

typedef struct _SGetDIBRes
{
	unsigned char u8DeviceInfo;
	unsigned char u8Revision;
	unsigned char u8Reserved[6];
}__attribute__((packed)) SGetDIBRes;

typedef struct _SGetTempReq
{
	SPeciHeader  sHeader;
	unsigned char u8CmdCode;
}__attribute__((packed)) SGetTempReq;

typedef struct _SGetTempRes
{
	unsigned char u8Temp[2];
}__attribute__((packed)) SGetTempRes;

typedef struct _SRdPkgConfigReq
{
	SPeciHeader  sHeader;
	unsigned char u8CmdCode;
	unsigned char u8HostID_Retry;		// 8-bit value defined as follows 31631 PECI 3.1 spec Figure 5-13:
										// [1:7] - Host ID is intended for use in future PECI systems that may support more than one PECI originator.
										// [0] - Retry, the originator should only set the "retry" bit when there is a genuine retry.
	unsigned char u8Index;
	unsigned short u16Parameter;
}__attribute__((packed)) SRdPkgConfigReq;

typedef struct _SRdPkgConfigRes
{
	unsigned char u8CompletionCode;
	unsigned char u8Data[8];
}__attribute__((packed)) SRdPkgConfigRes;

typedef struct _SWrPkgConfigReq
{
	SPeciHeader  sHeader;
	unsigned char u8CmdCode;
	unsigned char u8HostID_Retry;		// 8-bit value defined as follows:
										// [1:7] - Host ID is intended for use in future PECI systems that may support more than one PECI originator.
										// [0] - Retry, the originator should only set the "retry" bit when there is a genuine retry.
	unsigned char u8Index;
	unsigned short u16Parameter;
	unsigned char u8Data[4];
	unsigned char u8AWFCS;
}__attribute__((packed)) SWrPkgConfigReq;

typedef struct _SWrPkgConfigRes
{
	unsigned char u8CompletionCode;
}__attribute__((packed)) SWrPkgConfigRes;

typedef struct _SRdIAMSRReq
{
	SPeciHeader  sHeader;
	unsigned char u8CmdCode;
	unsigned char u8HostID_Retry;
	unsigned char u8ProcessorID;
	unsigned short u16Parameter;
}__attribute__((packed)) SRdIAMSRReq;

typedef struct _SRdIAMSRRes
{
	unsigned char u8CompletionCode;
	unsigned char u8Data[8];
}__attribute__((packed)) SRdIAMSRRes;

typedef struct _SRdPCIConfigReq
{
	SPeciHeader  sHeader;
	unsigned char u8CmdCode;
	unsigned char u8HostID_Retry;		// 8-bit value defined as follows:
										// [1:7] - Host ID is intended for use in future PECI systems that may support more than one PECI originator.
										// [0] - Retry, the originator should only set the "retry" bit when there is a genuine retry.
	unsigned char u8PCIConfigAddr[4];	// 32-bit address value is defined as follows:
										// [28:31] - reserved
										// [20:27] - Bus
										// [15:19] - Device
										// [12:14] - Function
										// [0:11] - Register

}__attribute__((packed)) SRdPCIConfigReq;

typedef struct _SRdPCIConfigRes
{
	unsigned char u8CompletionCode;
	unsigned char u8Data[4];
}__attribute__((packed)) SRdPCIConfigRes;

typedef struct _SRdPCIConfigLocalReq
{
	SPeciHeader  sHeader;
	unsigned char u8CmdCode;
	unsigned char u8HostID_Retry;		// 8-bit value defined as follows:
										// [1:7] - Host ID is intended for use in future PECI systems that may support more than one PECI originator.
										// [0] - Retry, the originator should only set the "retry" bit when there is a genuine retry.
	unsigned char u8PCIConfigAddr[3];	// 24-bit address value is defined as follows:
										// [20:23] - Bus
										// [15:19] - Device
										// [12:14] - Function
										// [0:11] - Register

}__attribute__((packed)) SRdPCIConfigLocalReq;

typedef struct _SRdPCIConfigLocalRes
{
	unsigned char u8CompletionCode;
	unsigned char u8Data[4];
}__attribute__((packed)) SRdPCIConfigLocalRes;

typedef struct _SWrPCIConfigReq
{
	SPeciHeader  sHeader;
	unsigned char u8CmdCode;
	unsigned char u8HostID_Retry;
	unsigned char u8PCIConfigAddr[4];
	unsigned char u8Data[4];
	unsigned char u8AWFCS;
}__attribute__((packed)) SWrPCIConfigReq;

typedef struct _SWrPCIConfigRes
{
	unsigned char u8CompletionCode;
}__attribute__((packed)) SWrPCIConfigRes;


#ifndef SPX_BMC_ACD
int PECI_Ping(SPeciHeader *psHeader);
int PECI_GetDIB(SGetDIBReq *psGetDIBReq, SGetDIBRes *psGetDIBRes);
int PECI_GetTemp(SGetTempReq *psGetTempReq, SGetTempRes *psGetTempRes);
#endif
int PECI_RdPkgConfig (SRdPkgConfigReq *psRdPkgConfigReq, SRdPkgConfigRes *psRdPkgConfigRes);
int PECI_WrPkgConfig(SWrPkgConfigReq *psWrPkgConfigReq, SWrPkgConfigRes *psWrPkgConfigRes);
#ifndef SPX_BMC_ACD
int PECI_RdIAMSR (SRdIAMSRReq *psRdIAMSRReq, SRdIAMSRRes *psRdIAMSRRes);
int PECI_RdPCIConfig(SRdPCIConfigReq *psRdPCIConfigReq, SRdPCIConfigRes *psRdPCIConfigRes);
#endif
int PECI_RdPCIConfigLocal(SRdPCIConfigLocalReq *psRdPCIConfigLocalReq, SRdPCIConfigLocalRes *psRdPCIConfigLocalRes);
#ifndef SPX_BMC_ACD
int PECI_WrPCIConfig(SWrPCIConfigReq *psWrPCIConfigReq, SWrPCIConfigRes *psWrPCIConfigRes);
#endif

#endif // __PECI_INTERFACE_H__
