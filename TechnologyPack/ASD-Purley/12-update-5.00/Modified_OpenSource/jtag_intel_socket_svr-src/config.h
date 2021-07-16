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

#ifndef __CONFIG_H_
#define __CONFIG_H_

#include <stdbool.h>
#include <stdint.h>
#include <dlfcn.h>

#include "asd_common.h"

#define DEFAULT_PLATFORM_ID 0x0088

typedef enum {
    JTAG_DRIVER_MODE_SOFTWARE = 0,
    JTAG_DRIVER_MODE_HARDWARE = 1
} JTAG_DRIVER_MODE;

typedef enum {
    JTAG_CHAIN_SELECT_MODE_SINGLE = 1,
    JTAG_CHAIN_SELECT_MODE_MULTI = 2
} JTAG_CHAIN_SELECT_MODE;

typedef struct gpio_config {
    // gpio pin number
    unsigned short pin;
    // true if the gpio asserts 'high'
    bool assert_high;
    // true if this pin is present and implemented
    bool is_present;
    //gpio properties 0b-dir, 1b-data 2b-event
    unsigned char gpio_prop;
} gpio_config;

typedef struct gpios_config {
    bool use_ipmi_interface;
    gpio_config debug_enable;
    gpio_config platform_reset;
    gpio_config power_debug;
    gpio_config power_good;
    gpio_config prdy;
    gpio_config preq;
    gpio_config tck_mux_select;
    gpio_config xdp_present;
    gpio_config sys_pwr_ok;
    gpio_config rsm_reset;
    gpio_config power_btn;
    gpio_config reset_btn;
//AMI_CHANGE_START
//Only to Get gpio Number
#ifdef SPX_BMC
    gpio_config xdp_jtag_sel;
    gpio_config xdp_bmc_jtag_sel;
#endif
//AMI_CHANGE_END
} gpios_config;

typedef struct jtag_config {
    // use HW or SW jtag driver.
    JTAG_DRIVER_MODE mode;
    JTAG_CHAIN_SELECT_MODE chain_mode;
} jtag_config;

#define MAX_I2C_BUSES 20
typedef struct i2c_config {
    bool enable_i2c;
    uint8_t default_bus;
    // if 0th item is false, bus 0 is not allowed
    bool allowed_buses[MAX_I2C_BUSES];
} i2c_config;

typedef struct config {
    gpios_config gpios;
    jtag_config jtag;
    logging_configuration logging;
    i2c_config i2c;
//AMI_CHANGE_START
//initializing gpio configs
#ifdef SPX_BMC
    int gpio_init_done;
#endif
} config;
#define ASDCFG_PATH	"/usr/local/lib/libasdconfiguration.so"

STATUS set_gpio_config(const uint16_t platform_id, const uint8_t config_id, config* config);
STATUS set_config_defaults(const uint16_t platform_id, config* config);
STATUS ASDGPIO_Init(unsigned short pin);
STATUS ASDGPIO_DeInit(unsigned short pin);

#endif // __CONFIG_H_
