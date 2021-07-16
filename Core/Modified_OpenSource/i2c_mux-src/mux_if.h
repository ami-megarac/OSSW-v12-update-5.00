#ifndef _LINUX_PROC_I2C_MUX_IF_H_
#define _LINUX_PROC_I2C_MUX_IF_H_

#define I2C_MUX_IF_MAX_COUNT	30
#define I2C_MUX_IF_MAX_NCHANS  8
#define I2C_MUX_IF_UNKNOWN_TYPE    0xFF
#define I2C_MUX_IF_MAX_MODIFIERS   4

typedef struct {
	u8 enabled;
	u8 bus;
	u8 addr;
    u8 type;
    void (*resetChip) (void *client);
    void (*removeChip)(void *client);
    void * (*addChip)(u8 bus, u8 addr, u8 mask, u8 rstGpio, int type, u8 *chanNumbering, u8 maxChanIndex, u16 *modifiers, u8 numModifiers );
    void (*setMaskChip)(void *client, u8 mask);
    int (*getStatus) (void *client, char *buf);
    void (*deselectChan)(void *client);
    void *client;
}tI2cMuxIfChip;


inline tI2cMuxIfChip *getPcaData(unsigned char id);

#endif //_LINUX_PROC_I2C_MUX_IF_H_
