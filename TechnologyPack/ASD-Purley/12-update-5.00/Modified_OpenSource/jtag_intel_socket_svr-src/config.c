/*
Copyright (c) 2017, Intel Corporation

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of Intel Corporation nor the names of its contributors
      may be used to endorse or promote products derived from this software
      without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/** @file config.c
 * This file is used to configure platform specific properties.
 * This file will be updated as needed when new platforms are supported.
 * As a customer, you are free to customize this code, per your needs.
 */

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "config.h"
#include "logging.h"

void initialize_config(config* config) {
    config->gpios.use_ipmi_interface = false;
    config->gpios.debug_enable.pin = 0;
    config->gpios.debug_enable.assert_high = false;
    config->gpios.debug_enable.is_present = false;
    config->gpios.platform_reset.pin = 0;
    config->gpios.platform_reset.assert_high = false;
    config->gpios.platform_reset.is_present = false;
    config->gpios.power_debug.pin = 0;
    config->gpios.power_debug.assert_high = false;
    config->gpios.power_debug.is_present = false;
    config->gpios.power_good.pin = 0;
    config->gpios.power_good.assert_high = false;
    config->gpios.power_good.is_present = false;
    config->gpios.prdy.pin = 0;
    config->gpios.prdy.assert_high = false;
    config->gpios.prdy.is_present = false;
    config->gpios.preq.pin = 0;
    config->gpios.preq.assert_high = false;
    config->gpios.preq.is_present = false;
    config->gpios.sys_pwr_ok.pin = 0;
    config->gpios.sys_pwr_ok.assert_high = false;
    config->gpios.sys_pwr_ok.is_present = false;
    config->gpios.rsm_reset.pin = 0;
    config->gpios.rsm_reset.assert_high = false;
    config->gpios.rsm_reset.is_present = false;
    config->gpios.tck_mux_select.pin = 0;
    config->gpios.tck_mux_select.assert_high = false;
    config->gpios.tck_mux_select.is_present = false;
    config->gpios.xdp_present.pin = 0;
    config->gpios.xdp_present.assert_high = false;
    config->gpios.xdp_present.is_present = false;
    config->gpios.power_btn.pin = 0;
    config->gpios.power_btn.assert_high = false;
    config->gpios.power_btn.is_present = false;
    config->gpios.reset_btn.pin = 0;
    config->gpios.reset_btn.assert_high = false;
    config->gpios.reset_btn.is_present = false;
    config->i2c.enable_i2c = false;
    config->i2c.default_bus = 0;
    for (int i=0; i<MAX_I2C_BUSES; i++) {
        config->i2c.allowed_buses[i] = false;
    }
}

STATUS set_gpio_config(const uint16_t platform_id, const uint8_t config_id, config* config) {
    STATUS result = ST_OK;

    ASD_log(LogType_Debug, "set_gpio_config platformID=0x%04x configID=0x%02x", platform_id, config_id);
//AMI_CHANGE_START
    if(!config->gpio_init_done)
        initialize_config(config);

#ifndef SPX_BMC
     if (platform_id == 0x008d) {
        if (config_id == 0) {
            config->gpios.debug_enable.pin = 37;
            config->gpios.debug_enable.is_present = true;
            config->gpios.power_debug.pin = 135;
            config->gpios.power_debug.is_present = true;
            config->gpios.prdy.pin = 47;
            config->gpios.prdy.is_present = true;
            config->gpios.preq.pin = 212;
            config->gpios.preq.is_present = true;
            config->gpios.sys_pwr_ok.pin = 145;
            config->gpios.sys_pwr_ok.assert_high = false;
            config->gpios.sys_pwr_ok.is_present = true;
            config->gpios.rsm_reset.pin = 146;
            config->gpios.rsm_reset.is_present = true;
            config->gpios.power_btn.pin = 202;
            config->gpios.power_btn.is_present = true;
            config->gpios.reset_btn.pin = 33;
            config->gpios.reset_btn.is_present = true;
            config->gpios.tck_mux_select.pin = 213;
            config->gpios.tck_mux_select.assert_high = true;
            config->gpios.tck_mux_select.is_present = true;
        } else if (config_id == 1) {
            config->gpios.debug_enable.pin = 37;
            config->gpios.debug_enable.is_present = true;
            config->gpios.platform_reset.pin = 32;
            config->gpios.platform_reset.assert_high = true;
            config->gpios.platform_reset.is_present = true;
            config->gpios.power_debug.pin = 135;
            config->gpios.power_debug.is_present = true;
            // On this platform, we have to assume the system is powered
            // until a board modification is made.
//            config->gpios.power_good.pin = 146;
//            config->gpios.power_good.assert_high = true;
//            config->gpios.power_good.is_present = true;
            config->gpios.prdy.pin = 47;
            config->gpios.prdy.is_present = true;
            config->gpios.preq.pin = 212;
            config->gpios.preq.is_present = true;
            config->gpios.sys_pwr_ok.pin = 145;
            config->gpios.sys_pwr_ok.assert_high = false;
            config->gpios.sys_pwr_ok.is_present = true;
            config->gpios.rsm_reset.pin = 146;
            config->gpios.rsm_reset.is_present = true;
            config->gpios.power_btn.pin = 202;
            config->gpios.power_btn.is_present = true;
            config->gpios.reset_btn.pin = 33;
            config->gpios.reset_btn.is_present = true;
            config->gpios.tck_mux_select.pin = 213;
            config->gpios.tck_mux_select.assert_high = true;
            config->gpios.tck_mux_select.is_present = true;
        }
    } else {
        config->gpios.use_ipmi_interface = true;
        config->gpios.debug_enable.pin = 37;
        config->gpios.debug_enable.is_present = true;
        config->gpios.platform_reset.pin = 46;
        config->gpios.platform_reset.assert_high = true;
        config->gpios.platform_reset.is_present = true;
        config->gpios.power_debug.pin = 135;
        config->gpios.power_debug.is_present = true;
        config->gpios.power_good.pin = 201;
        config->gpios.power_good.assert_high = true;
        config->gpios.power_good.is_present = true;
        config->gpios.prdy.pin = 47;
        config->gpios.prdy.is_present = true;
        config->gpios.preq.pin = 212;
        config->gpios.preq.is_present = true;
        config->gpios.sys_pwr_ok.pin = 145;
        config->gpios.sys_pwr_ok.assert_high = false;
        config->gpios.sys_pwr_ok.is_present = true;
        config->gpios.rsm_reset.pin = 146;
        config->gpios.rsm_reset.is_present = true;
        config->gpios.tck_mux_select.pin = 213;
        config->gpios.tck_mux_select.assert_high = true;
        config->gpios.tck_mux_select.is_present = true;
        config->gpios.xdp_present.pin = 137;
        config->gpios.xdp_present.assert_high = true;
        config->gpios.xdp_present.is_present = true;
        config->i2c.enable_i2c = true;
        config->i2c.default_bus = 4;
        config->i2c.allowed_buses[4] = true;  // only allow bus 4
    }

#else
    if(!config->gpio_init_done)
    {
        void *handle = NULL;
        int (*GetASDGPIOCfg) (gpio_config*);
        bool (*UseIPMIInterface)();
        bool (*GetXDPI2C_Cfg)(uint8_t *default_bus);
        
        printf("%s inside \n",__func__);
        handle = dlopen(ASDCFG_PATH, RTLD_LAZY);
        if(!handle)
        {
            printf("%s\n",dlerror());
            return ST_ERR;
        }
        
        UseIPMIInterface = dlsym(handle, "UseIPMIInterface");
        if(UseIPMIInterface)
            config->gpios.use_ipmi_interface = UseIPMIInterface();
        else
            config->gpios.use_ipmi_interface = false;
        
        GetASDGPIOCfg = dlsym(handle, "GetXDPGPIO_Debug_en");
        if (!GetASDGPIOCfg)
            goto error;
        if(GetASDGPIOCfg(&config->gpios.debug_enable) == 0)
	    goto error;

        GetASDGPIOCfg = dlsym(handle, "GetXDPGPIO_pltrst");
        if (!GetASDGPIOCfg)
            goto error;
        if(GetASDGPIOCfg(&config->gpios.platform_reset) == 0)
            goto error;

        GetASDGPIOCfg = dlsym(handle, "GetXDPGPIO_pwrdbg");
        if (!GetASDGPIOCfg)
            goto error;
        if(GetASDGPIOCfg(&config->gpios.power_debug) == 0)
            goto error;

        GetASDGPIOCfg = dlsym(handle, "GetXDPGPIO_pwrgd");
        if (!GetASDGPIOCfg)
            goto error;
        if(GetASDGPIOCfg(&config->gpios.power_good) == 0)
            goto error;

        GetASDGPIOCfg = dlsym(handle, "GetXDPGPIO_prdy");
        if (!GetASDGPIOCfg)
            goto error;
        if(GetASDGPIOCfg(&config->gpios.prdy) == 0)
            goto error;

        GetASDGPIOCfg = dlsym(handle, "GetXDPGPIO_preq");
        if (!GetASDGPIOCfg)
            goto error;
        if(GetASDGPIOCfg(&config->gpios.preq) == 0)
            goto error;

        GetASDGPIOCfg = dlsym(handle, "GetXDPGPIO_tck_mux_select");
        if (!GetASDGPIOCfg)
            goto error;
        if(GetASDGPIOCfg(&config->gpios.tck_mux_select) == 0)
            goto error;

        GetASDGPIOCfg = dlsym(handle, "GetXDPGPIO_xdp_present");
        if (!GetASDGPIOCfg)
            goto error;
        if(GetASDGPIOCfg(&config->gpios.xdp_present) == 0)
            goto error;

        GetASDGPIOCfg = dlsym(handle, "GetXDPGPIO_sys_pwr_ok");
        if (!GetASDGPIOCfg)
            goto error;
        if(GetASDGPIOCfg(&config->gpios.sys_pwr_ok) == 0)
            goto error;

        GetASDGPIOCfg = dlsym(handle, "GetXDPGPIO_rms_rst");
        if (!GetASDGPIOCfg)
            goto error;
        if(GetASDGPIOCfg(&config->gpios.rsm_reset) == 0)
            goto error;

        GetASDGPIOCfg = dlsym(handle, "GetXDPGPIO_xdp_jtag_sel");
        if (!GetASDGPIOCfg)
            goto error;
        if(GetASDGPIOCfg(&config->gpios.xdp_jtag_sel) == 0)
           goto error;

        GetASDGPIOCfg = dlsym(handle, "GetXDPGPIO_bmc_jtag_sel");
        if (!GetASDGPIOCfg)
            goto error;
        if(GetASDGPIOCfg(&config->gpios.xdp_bmc_jtag_sel) == 0)
            goto error;

        GetXDPI2C_Cfg = dlsym(handle, "GetXDPI2C_Cfg");
        if(GetXDPI2C_Cfg)
        {
            if((config->i2c.enable_i2c = GetXDPI2C_Cfg(&config->i2c.default_bus)) == true )
            {
                config->i2c.allowed_buses[config->i2c.default_bus] = true;  // only allow bus 4
            }
        }

        
        dlclose(handle);
        config->gpio_init_done = 1;
        
        return ST_OK;

error:
    printf("%s\n",dlerror());
    dlclose(handle);
    return ST_ERR;

    }
#endif
//AMI_CHANGE_END

    return result;
}

STATUS set_config_defaults(const uint16_t platform_id, config* config) {
    config->jtag.mode = JTAG_DRIVER_MODE_SOFTWARE;
    config->jtag.chain_mode = JTAG_CHAIN_SELECT_MODE_SINGLE;

    config->logging.logging_level = IPC_LogType_Off;
    config->logging.logging_stream = 0;

    return set_gpio_config(platform_id, 0, config);
}

STATUS ASDGPIO_Init(unsigned short pin)
{
    void *handle = NULL;
    bool (*ASDGPIO_Init)(unsigned short pin);

    handle = dlopen(ASDCFG_PATH, RTLD_LAZY);
    if(!handle)
    {
        printf("%s\n",dlerror());
        return ST_ERR;
    }
    ASDGPIO_Init = dlsym(handle, "ASDGPIO_Init");
    if (!ASDGPIO_Init)
        goto error;
    if(ASDGPIO_Init(pin) == 0)
        goto error;

    dlclose(handle);
    return ST_OK;
error:
    dlclose(handle);
    return ST_ERR;

}
STATUS ASDGPIO_DeInit(unsigned short pin)
{
    void *handle = NULL;
    bool (*ASDGPIO_DeInit)(unsigned short pin);

    handle = dlopen(ASDCFG_PATH, RTLD_LAZY);
    if(!handle)
    {
        printf("%s\n",dlerror());
        return ST_ERR;
    }
    ASDGPIO_DeInit = dlsym(handle, "ASDGPIO_DeInit");
    if (!ASDGPIO_DeInit)
        goto error;
    if(ASDGPIO_DeInit(pin) == 0)
        goto error;
    
    dlclose(handle);
    return ST_OK;
error:
    dlclose(handle);
    return ST_ERR;
}
