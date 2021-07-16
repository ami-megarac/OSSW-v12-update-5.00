/****************************************************************
 **                                                            **
 **    (C)Copyright 2006-2009, American Megatrends Inc.        **
 **                                                            **
 **            All Rights Reserved.                            **
 **                                                            **
 **        5555 Oakbrook Pkwy Suite 200, Norcross,             **
 **                                                            **
 **        Georgia - 30093, USA. Phone-(770)-246-8600.         **
 **                                                            **
****************************************************************/

#ifndef ETH_MOD_H
#define ETH_MOD_H

#include "usb_core.h"
#include "dbgout.h"

#define DEBUG_ETH		0x01

TDBG_DECLARE_DBGVAR_EXTERN(eth);

extern uint8 NotifyEpNum []; /* Fortify [Type Mismatch: Signed to Unsigned]:: False Positive *//* Reason for False Positive - Change the data type of parameter "NotifyEpNum" from int to uint8 to fit return value. */
extern int DataOutEpNum [];
extern uint8 DataInEpNum []; /* Fortify [Type Mismatch: Signed to Unsigned]:: False Positive *//* Reason for False Positive - Change the data type of parameter "DataInEpNum" from int to uint8 to fit return value. */
extern USB_CORE UsbCore;
extern int EthDevNo;



#endif

