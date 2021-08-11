/****************************************************************
 **                                                            **
 **    (C)Copyright 2006-2009, American Megatrends Inc.        **
 **                                                            **
 **            All Rights Reserved.                            **
 **                                                            **
 **        5555 Oakbrook Pkwy Suite 200, Norcross              **
 **                                                            **
 **        Georgia - 30093, USA. Phone-(770)-246-8600.         **
 **                                                            **
****************************************************************/

#ifndef _JTAG_IOCTL_H_
#define _JTAG_IOCTL_H_

#include <linux/socket.h>
#include <linux/tcp.h>

typedef struct IO_ACCESS_DATA_T {
    unsigned char    Type;
    unsigned long    Address;
    unsigned long    Data;
    unsigned long    Value;
    unsigned long    I2CValue;
    int        kernel_socket;
    unsigned long    Input_Buffer_Base;
} IO_ACCESS_DATA_T;

typedef IO_ACCESS_DATA_T jtag_ioaccess_data;

#define  RELOCATE_OFFSET        0x380

#define  IOCTL_IO_READ                 _IOR('j', 0, int)            
#define  IOCTL_IO_WRITE                _IOW('j', 1, int)            
#define  IOCTL_REMAP                   _IOW('j', 2, int)
#define  IOCTL_REAL_IO_READ            _IOR('j', 3, int) 
#define  IOCTL_REAL_IO_WRITE           _IOW('j', 4, int)
#define  IOCTL_BIT_STREAM_BASE         _IOW('j', 5, int)    
#define  IOCTL_TX_BIT_STREAM           _IOW('j', 6, int)    
#define  IOCTL_GET_SOCKET              _IOR('j', 7, int)
#define  IOCTL_AUTOMODE_TRIGGER        _IOW('j', 8, int)    
#define  IOCTL_PASS3_TRIGGER           _IOW('j', 9, int)    
#define  IOCTL_I2C_READ                _IOR('j', 10, int) 
#define  IOCTL_I2C_WRITE               _IOW('j', 11, int)
#define  IOCTL_JTAG_RESET              _IOW('j', 12, int)
#define  IOCTL_JTAG_IDCODE_READ        _IOR('j', 13, int)
#define  IOCTL_JTAG_ERASE_DEVICE       _IOW('j', 14, int)
#define  IOCTL_JTAG_PROGRAM_DEVICE     _IOW('j', 15, int)
#define  IOCTL_JTAG_VERIFY_DEVICE      _IOW('j', 16, int)
#define  IOCTL_JTAG_DEVICE_CHECKSUM    _IOW('j', 17, int)
#define  IOCTL_JTAG_DEVICE_TFR         _IOW('j', 18, int)
#define  IOCTL_JTAG_UPDATE_DEVICE      _IOW('j', 19, int)

#define  IOCTL_JTAG_READ_USERCODE      _IOR('j', 20, int)
#define  IOCTL_JTAG_SET_IO             _IOW('j', 21, int)
#define  IOCTL_JTAG_UPDATE_JBC         _IOW('j', 22, int)
#define  IOCTL_JTAG_SIR                _IOWR('j', 23, IO_ACCESS_DATA_T)
typedef enum jtag_xfer_mode {
    HW_MODE = 0,   
    SW_MODE
} xfer_mode;

struct runtest_idle {
    xfer_mode     mode;        //0 :HW mode, 1: SW mode    
    unsigned char     reset;        //Test Logic Reset 
    unsigned char     end;            //o: idle, 1: ir pause, 2: drpause
    unsigned char     tck;            //keep tck 
};

struct sir_xfer {
    xfer_mode     mode;        //0 :HW mode, 1: SW mode    
    unsigned short length;    //bits
    unsigned int  *tdi;
    unsigned int  *tdo;
    unsigned char endir;    //0: idle, 1:pause
};

struct sdr_xfer {
    xfer_mode     mode;        //0 :HW mode, 1: SW mode    
    unsigned char direct; // 0 ; read , 1 : write     
    unsigned int length;    //bits
    unsigned int *tdio;
    unsigned char enddr;    //0: idle, 1:pause    
};

struct io_xfer {
    xfer_mode     mode;        //0 :HW mode, 1: SW mode    
    unsigned long     Address;
    unsigned long    Data;
};

struct trst_reset {
    unsigned long     operation; // 0 ; read , 1 : write 
    unsigned long    Data;     // 0 means low, 1 means high - TRST pin
};

#define  ASPEED_JTAG_IOCRUNTEST             _IOW('j', 23, struct runtest_idle)
#define  ASPEED_JTAG_IOCSIR                 _IOWR('j',24, struct sir_xfer)
#define  ASPEED_JTAG_IOCSDR                 _IOWR('j',25, struct sdr_xfer)
#define  ASPEED_JTAG_SIOCFREQ               _IOW('j', 26, unsigned int)
#define  ASPEED_JTAG_GIOCFREQ               _IOR('j', 27, unsigned int)
#define  ASPEED_JTAG_IOWRITE                _IOW('j', 28, struct io_xfer)
#define  ASPEED_JTAG_IOREAD                 _IOR('j', 29, struct io_xfer)
#define  ASPEED_JTAG_RESET                  _IOW('j', 30, struct io_xfer)
#define  ASPEED_JTAG_TRST_RESET             _IOW('j', 31, struct trst_reset)
#define  ASPEED_JTAG_RUNTCK                 _IOW('j', 32, struct io_xfer)



#endif /* _JTAG_IOCTL_H_ */

