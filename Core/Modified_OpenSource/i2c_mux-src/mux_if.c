#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/i2c.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <linux/i2c-mux.h>
#include <helper.h>
#include <linux/version.h>
#if (LINUX_VERSION_CODE > KERNEL_VERSION(3,14,17))
#include <linux/poll.h>
#endif

#include "mux_if.h"
#include "PILOT.h"
#include "PCA954x.h"

#define I2C_MUX_IF_PARAM_COUNT 	4
#define I2C_MUX_IF_PROC_DIR		"i2c_mux"
#define I2C_MUX_IF_FILE_STATUS		"status"
#define I2C_MUX_IF_FILE_BIND		"bind"
#define I2C_MUX_IF_FILE_UNBIND		"unbind"
#define I2C_MUX_IF_FILE_RESET		"reset"
#define I2C_MUX_IF_FILE_MASK		"mask"

typedef enum
{
	I2C_MUX_IF_STATUS_FILE = 0,
	I2C_MUX_IF_BIND_FILE,
	I2C_MUX_IF_UNBIND_FILE,
	I2C_MUX_IF_RESET_FILE,
	I2C_MUX_IF_MASK_FILE
}eProcFileType;

typedef enum
{
	I2C_MUX_IF_BUS = 0,
	I2C_MUX_IF_ADDR,
	I2C_MUX_IF_MASK,
	I2C_MUX_IF_RST_GPIO,
	I2C_MUX_IF_TYPE,
	I2C_MUX_IF_CHANNEL_1,
	I2C_MUX_IF_CHANNEL_2,
	I2C_MUX_IF_CHANNEL_3,
	I2C_MUX_IF_CHANNEL_4,
	I2C_MUX_IF_CHANNEL_5,
	I2C_MUX_IF_CHANNEL_6,
	I2C_MUX_IF_CHANNEL_7,
	I2C_MUX_IF_CHANNEL_8
}eParam;

static tI2cMuxIfChip *gChips = NULL;
static struct proc_dir_entry *i2c_mux_dir = NULL;

extern void i2c_set_mctp_complete_callback(void (*func)(int bus));

void i2c_mux_if_handle_mctp_complete(int bus)
{
	int i = 0;
	
	for (i = 0; i < I2C_MUX_IF_MAX_COUNT; ++i)
	{
		if ((gChips[i].enabled == 1) && (gChips[i].bus == bus))
		{
			if (gChips[i].deselectChan != NULL) {
				gChips[i].deselectChan(gChips[i].client);
			}
		}
	}
}

static tI2cMuxIfChip *findChip(u8 bus, u8 addr)
{
	u8 num;
	for(num = 0; num < I2C_MUX_IF_MAX_COUNT; ++num) {
		if( ( gChips[num].enabled == 1 ) && ( gChips[num].bus == (u8)bus) && ( gChips[num].addr == addr) ) 
			return &gChips[num];
		}
	return NULL;
}

static int mux_bind(u8 bus, u8 addr, u8 chanMask, u8 rstGpio, char *typeStr, u8 *chanNumbering,  u8 maxChanIndex, u16 *modifiers, u8 numModifiers)
{
	int num;

	//check if device already bind
	if( NULL != findChip(bus, addr) )
		return -1;
	
	//bind device, search for free entry in topology structure
	for(num = 0; num < I2C_MUX_IF_MAX_COUNT; ++num) {
		if(gChips[num].enabled == 0)
		{
			memset(&gChips[num],0x00,sizeof(tI2cMuxIfChip));
			
			do
			{
				#if defined(SOC_PILOT_IV)
                		gChips[num].type = pca954x_getType(typeStr);
                		if(gChips[num].type != I2C_MUX_IF_UNKNOWN_TYPE) {
                                        gChips[num].getStatus = pca954x_getStatus;
                                        gChips[num].removeChip = pca954x_removeChip;
                                        gChips[num].addChip = pca954x_addChip;
                                        gChips[num].resetChip = pca954x_resetChip;
                                        gChips[num].setMaskChip= pca954x_setMaskChip;
                                        gChips[num].deselectChan = pca954x_notifyDeselectChan;
                                        break;
                		}
                		gChips[num].type = pilot_getType(typeStr);
				if(gChips[num].type != I2C_MUX_IF_UNKNOWN_TYPE) {
					gChips[num].getStatus = pilot_getStatus;
					gChips[num].removeChip = pilot_removeChip;
					gChips[num].addChip = pilot_addChip;
					gChips[num].resetChip = pilot_resetChip;
					gChips[num].setMaskChip = pilot_setMaskChip;
					gChips[num].deselectChan = pilot_notifyDeselectChan;
					break;
				}
				#endif
				
				return (-1);
			}
			while(0);
			
			gChips[num].client = gChips[num].addChip(bus, addr, chanMask, rstGpio, gChips[num].type, chanNumbering, maxChanIndex, modifiers, numModifiers);
			if(gChips[num].client != NULL) {
				gChips[num].bus = (u8)bus;
				gChips[num].addr = (u8)addr;
				gChips[num].enabled = 1;
				//printk(KERN_ERR "mux_bind: bus: %i addr: %x\n",gChips[num].bus,gChips[num].addr);
			}
			else {
				gChips[num].enabled = 0;
				printk(KERN_ERR "mux_bind:uppps \n");
			}
			return 0;
		}
		
	}
	return -1;
}
static int mux_unbind(u8 bus, u8 addr)
{
	tI2cMuxIfChip *pChip = findChip(bus, addr);
	if(pChip != NULL) {
		//printk(KERN_ERR "mux_unbind: bus: %i addr: %x\n",pChip->bus,pChip->addr);
		pChip->removeChip(pChip->client);
		pChip->enabled = 0;
		return 0;
	}
	else
		return -1;
		
}
static int mux_resetChip(u8 bus, u8 addr)
{
	tI2cMuxIfChip *pChip = findChip(bus, addr);
	//printk(KERN_ERR "mux_resetChip: bus: %i addr: %x\n",bus,addr);
	if(pChip != NULL) {
		//printk(KERN_ERR "mux_resetChip: bus: %i addr: %x\n",pChip->bus,pChip->addr);
		pChip->resetChip(pChip->client); 
		return 0;
	}
	else
		return -1;
}
static int mux_setMask(u8 bus, u8 addr, u8 mask )
{
	tI2cMuxIfChip *pChip = findChip(bus, addr);
	//printk(KERN_ERR "mux_setMask Parameter: bus: %i addr: %x\n",bus,addr);
	if(pChip != NULL) {
		//printk(KERN_ERR "mux_setMask >>: bus: %i addr: %x\n",pChip->bus,pChip->addr);
		pChip->setMaskChip(pChip->client,mask);
		return 0;
	}
	else
		return -1;
}
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,14,17))
static inline void *PDE_DATA(const struct inode *inode)
{
	return PDE(inode)->data;
}
static inline struct inode *file_inode(struct file *f)
{
	return f->f_dentry->d_inode;
}
#endif
static ssize_t i2c_mux_if_proc_write(struct file *filp, const char *buff, size_t len, loff_t *offp)
{
	char *kBuf = NULL, *pBuf = NULL, *oneMuxSettings = NULL, *param = NULL;
	u8 bus = 0, addr = 0, rstGpio = 0, chanMask = 0;
	u8 chan[I2C_MUX_IF_MAX_NCHANS];
	u8 chanNum, count;
	unsigned long tmp;
	char type[I2C_NAME_SIZE];
	char *mod = NULL;
	u16 modifiers[I2C_MUX_IF_MAX_MODIFIERS];
	u8 numModifiers;
	int ret = 0;
	eProcFileType fileType = (eProcFileType)PDE_DATA(file_inode(filp));
	
	kBuf = kmalloc(len + 1, GFP_KERNEL);
	if(kBuf == NULL)
		return -EFAULT;
	
	if(copy_from_user(kBuf, buff, len)) {
		kfree(kBuf);
		return -EFAULT;
	}
	kBuf[len] = 0;
	//printk (KERN_ERR "Kernel: <%s: val: %s, %i, I2C_NAME_SIZE: %i>\n",__FUNCTION__,kBuf,(int)len,I2C_NAME_SIZE);
	
	for(count = 0; count < len; ++count) //remove spaces on begin
		if(kBuf[count != ' '])
			break;
	
	pBuf = &kBuf[count];
	while((oneMuxSettings = strsep(&pBuf, " ")) != NULL) {
		count = 0;
		chanNum = 0;
		numModifiers = 0;
		while((param = strsep(&oneMuxSettings, ",")) != NULL) {
			switch(count) {
				case I2C_MUX_IF_BUS:
					#if (LINUX_VERSION_CODE > KERNEL_VERSION(3,14,17))
					 if(kstrtoul(param, 10, &tmp))
					#else
					 if(strict_strtoul(param, 10, &tmp))
					#endif	
						ret = -EFAULT;
					else
						bus = (u8)tmp;
					break;
				case I2C_MUX_IF_ADDR:
					#if (LINUX_VERSION_CODE > KERNEL_VERSION(3,14,17))
					if(kstrtoul(param, 16, &tmp))
					#else
					if(strict_strtoul(param, 16, &tmp))
					#endif
						ret = -EFAULT;
					else {
						addr = (u8)tmp;
					}
					break;
				case I2C_MUX_IF_MASK:
					#if (LINUX_VERSION_CODE > KERNEL_VERSION(3,14,17))
					if(kstrtoul(param, 16, &tmp))
					#else
					if(strict_strtoul(param, 16, &tmp))
					#endif
						ret = -EFAULT;
					else
						chanMask = (u8)tmp;
					break;
				case I2C_MUX_IF_RST_GPIO:
					#if (LINUX_VERSION_CODE > KERNEL_VERSION(3,14,17))
					if(kstrtoul(param, 10, &tmp))
					#else	
					if(strict_strtoul(param, 10, &tmp))
					#endif
						ret = -EFAULT;
					else
						rstGpio = (u8)tmp;
					break;
				case I2C_MUX_IF_TYPE:
					if ((mod = strsep(&param, ":")) != NULL) {
						strncpy(type,mod,I2C_NAME_SIZE);
						while ((mod = strsep(&param, ":")) != NULL) {
							if (numModifiers >= I2C_MUX_IF_MAX_MODIFIERS)
								break;

							#if (LINUX_VERSION_CODE > KERNEL_VERSION(3,14,17))
							if(kstrtoul(mod, 0, &tmp))
							#else
							if (strict_strtoul(mod, 0, &tmp))
							#endif
								ret = -EFAULT;
							else
								modifiers[numModifiers++] = (u16)tmp;
						}
					}
					break;
				case I2C_MUX_IF_CHANNEL_1:
				case I2C_MUX_IF_CHANNEL_2:
				case I2C_MUX_IF_CHANNEL_3:
				case I2C_MUX_IF_CHANNEL_4:
				case I2C_MUX_IF_CHANNEL_5:
				case I2C_MUX_IF_CHANNEL_6:
				case I2C_MUX_IF_CHANNEL_7:
				case I2C_MUX_IF_CHANNEL_8:
					#if (LINUX_VERSION_CODE > KERNEL_VERSION(3,14,17))
					if(kstrtoul(param, 10, &tmp))
					#else
					if(strict_strtoul(param, 10, &tmp))
					#endif
						ret = -EFAULT;
					else
						chan[chanNum++] = (u8)tmp;
					break;
				default:
					break;
			}
			++count;
		}
		if(ret ==0) {
			switch(fileType)
			{
				case I2C_MUX_IF_BIND_FILE:
					//printk (KERN_ERR "<BIND bus: %i, addr: %02X, mask: %02X, gpio: %i, type: %s chanNum: %i>\n",bus,addr,chanMask, rstGpio,type,chanNum);
					mux_bind(bus,addr,chanMask,rstGpio,type,chan,chanNum,modifiers,numModifiers);
					break;
				case I2C_MUX_IF_UNBIND_FILE:
					//printk (KERN_ERR "<UNBIND bus: %i, addr: %02X>\n",bus,addr);
					mux_unbind(bus,addr);
					break;
				case I2C_MUX_IF_RESET_FILE:
					//printk (KERN_ERR "<RESET bus: %i, addr: %02X>\n",bus,addr);
					mux_resetChip(bus,addr);
					break;
				case I2C_MUX_IF_MASK_FILE:
					//printk (KERN_ERR "<SET MASK bus: %i, addr: %02X, mask: %02X>\n",bus,addr,chanMask);
					mux_setMask(bus,addr,chanMask);
					break;
				default:
					break;
			}
		}
	}
	kfree(kBuf);
	return len;
}
static ssize_t i2c_mux_if_proc_read_status (struct file *filp, char *buf, size_t count, loff_t *f_pos)
{
    int len = 0;
    int i;
    eProcFileType fileType = (eProcFileType)PDE_DATA(file_inode(filp));

	if( *f_pos == 1 ) //count is typically 4kb. so we could read all data in one call  
		return 0; 
	else {
		if(fileType == I2C_MUX_IF_STATUS_FILE) {
			for(i = 0; i < I2C_MUX_IF_MAX_COUNT; ++i) {
				if(gChips[i].enabled == 1)
				{
					len += gChips[i].getStatus(gChips[i].client,buf+len);
				}
			}    
		}
		else {
			len = 0;
		}
		*f_pos = 1;

		return len;

	}
}
inline tI2cMuxIfChip *getPcaData(unsigned char id)
{
	return &gChips[id];
}

static struct file_operations i2c_mux_if_read_fops = {
		.owner     	=  THIS_MODULE,
		.read = i2c_mux_if_proc_read_status,	
		.write = NULL,
};
static struct file_operations i2c_mux_if_write_fops = {
		.owner     	=  THIS_MODULE,
		.read = NULL,
		.write = i2c_mux_if_proc_write,		
};

static void i2c_mux_if_removeAllProc(void)//	int status;
{
	remove_proc_entry(I2C_MUX_IF_FILE_STATUS,	i2c_mux_dir);
	remove_proc_entry(I2C_MUX_IF_FILE_BIND,	i2c_mux_dir);
	remove_proc_entry(I2C_MUX_IF_FILE_UNBIND,	i2c_mux_dir);
	remove_proc_entry(I2C_MUX_IF_FILE_MASK,	i2c_mux_dir);
	remove_proc_entry(I2C_MUX_IF_FILE_RESET,	i2c_mux_dir);
	remove_proc_entry(I2C_MUX_IF_PROC_DIR, 	NULL);
}


static int i2c_mux_if_createProc(void)
{
	int ret = 0, loop;
	
    if((gChips = kmalloc((sizeof(tI2cMuxIfChip) * I2C_MUX_IF_MAX_COUNT), GFP_KERNEL)) == NULL)
       	return -ENOMEM;
       
    for(loop = 0; loop < I2C_MUX_IF_MAX_COUNT; ++loop) 
    	gChips[loop].enabled = 0;	
    
    i2c_mux_dir = proc_mkdir(I2C_MUX_IF_PROC_DIR, NULL);
    if (i2c_mux_dir == NULL)
    {
    	printk(KERN_ERR "Error: Could not initialize /proc/%s\n",I2C_MUX_IF_PROC_DIR);
    	ret = -EFAULT;
    	goto no_dir;
    }
	if( (NULL == proc_create_data(I2C_MUX_IF_FILE_STATUS,  (S_IFREG |S_IRUGO),i2c_mux_dir,&i2c_mux_if_read_fops,(void*)I2C_MUX_IF_STATUS_FILE) ) ||
		(NULL == proc_create_data(I2C_MUX_IF_FILE_BIND,	(S_IFREG |S_IWUGO),i2c_mux_dir,&i2c_mux_if_write_fops,(void*)I2C_MUX_IF_BIND_FILE)  ) || 
		(NULL == proc_create_data(I2C_MUX_IF_FILE_UNBIND,	(S_IFREG |S_IWUGO),i2c_mux_dir,&i2c_mux_if_write_fops,(void*)I2C_MUX_IF_UNBIND_FILE)) ||
		(NULL == proc_create_data(I2C_MUX_IF_FILE_MASK,	(S_IFREG |S_IWUGO),i2c_mux_dir,&i2c_mux_if_write_fops,(void*)I2C_MUX_IF_MASK_FILE)  ) ||
		(NULL == proc_create_data(I2C_MUX_IF_FILE_RESET,	(S_IFREG |S_IWUGO),i2c_mux_dir,&i2c_mux_if_write_fops,(void*)I2C_MUX_IF_RESET_FILE) ) )
	{
		printk(KERN_ERR "Error: Could not initialize /proc/%s files\n",I2C_MUX_IF_PROC_DIR);
		i2c_mux_if_removeAllProc();
		ret = -EFAULT;
		goto no_file;
	}

	/* Register callback with I2C driver to be executed when an MCTP transfer is received */

	#if (LINUX_VERSION_CODE <= KERNEL_VERSION(3,14,17))
	i2c_set_mctp_complete_callback(i2c_mux_if_handle_mctp_complete);
	#endif
	return 0;
	
no_file:
	i2c_mux_if_removeAllProc();
no_dir:
	kfree(gChips);
	
	return ret;
}
static int i2c_mux_if_removeProc(void)
{
	u8 num;
	i2c_mux_if_removeAllProc();
	
	for(num = 0; num < I2C_MUX_IF_MAX_COUNT; ++num) {
		if( gChips[num].enabled == 1 )  {
			 gChips[num].removeChip(gChips[num].client);
			 gChips[num].enabled = 0;
		}
	}
	kfree(gChips);
	return 0;
}
static int __init i2c_mux_if_init(void)
{
    return i2c_mux_if_createProc();
}

static void __exit i2c_mux_if_exit(void)
{
	i2c_mux_if_removeProc();
}


int platform_i2cmux_register(void)
{

    return 0;
}

int platform_i2cmux_unregister(void)
{

    return 0;
}

module_init(i2c_mux_if_init);
module_exit(i2c_mux_if_exit);

MODULE_DESCRIPTION("I2C Mux Driver Interface");
MODULE_LICENSE("GPL v2");
