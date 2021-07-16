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

#include <fcntl.h>
//AMI_CHANGE_START
//Adapting AMI GPIO Specific definitions and functions
#ifndef SPX_BMC
#include <gpio_drv/gpio_drv.h>
#endif
#include <sys/ioctl.h>
#include <unistd.h>
#include "gpio.h"
#ifdef SPX_BMC
#include <gpio/gpio.h>
#endif
#include <string.h>
#include <stdlib.h>
#include <errno.h>


#ifndef SPX_BMC
STATUS gpio_action(int gpio_fd, struct gpio_ioctl_data* data);
#else
#include <stdio.h>
STATUS gpio_action(int gpio_fd, gpio_ioctl_data* data, int cmd );
#endif
STATUS open_gpio_driver(int* gpio_fd)
{
#ifndef SPX_BMC
    *gpio_fd = open("/dev/gpio", O_RDWR);
#else
    *gpio_fd = open("/dev/gpio0", O_RDWR);
#endif
    if (*gpio_fd == -1) {
        return ST_ERR;
    }
    return ST_OK;
}

STATUS close_gpio_driver(int* gpio_fd)
{
    close(*gpio_fd);
    *gpio_fd = -1;
    return ST_OK;
}

STATUS set_gpio_dir_input(int gpio_fd, unsigned short pin)
{
#ifndef SPX_BMC
    struct gpio_ioctl_data gpio_data;
    gpio_data.cmd = SET_GPIO_DIRECTION;
    gpio_data.pin = pin;
    gpio_data.data = GPIO_DIR_IN;
    return gpio_action(gpio_fd, &gpio_data);
#else
    gpio_ioctl_data gpio_data;
    gpio_data.PinNum = pin;
    gpio_data.data = GPIO_DIR_IN;
    return gpio_action(gpio_fd, &gpio_data,SET_GPIO_DIRECTION);
#endif
}

STATUS set_gpio_dir_output(int gpio_fd, unsigned short pin)
{
#ifndef SPX_BMC
    struct gpio_ioctl_data gpio_data;
    gpio_data.cmd = SET_GPIO_DIRECTION;
    gpio_data.pin = pin;
    gpio_data.data = GPIO_DIR_OUT;
    return gpio_action(gpio_fd, &gpio_data);
#else
    gpio_ioctl_data gpio_data;
    gpio_data.PinNum = pin;
    gpio_data.data = GPIO_DIR_OUT;
    return gpio_action(gpio_fd, &gpio_data,SET_GPIO_DIRECTION);
#endif
}

STATUS get_gpio_data(int gpio_fd, unsigned short pin, GPIO_status* state)
{
    STATUS result = ST_OK;
    *state = GPIO_LOW;
#ifndef SPX_BMC
    struct gpio_ioctl_data gpio_data;
    gpio_data.cmd = GET_GPIO_DATA_IN;
    gpio_data.pin = pin;
#else
    gpio_ioctl_data gpio_data;
    gpio_data.PinNum = pin;
#endif
    gpio_data.data = 0;

#ifndef SPX_BMC
    result = gpio_action(gpio_fd, &gpio_data);
#else
    result = gpio_action(gpio_fd, &gpio_data,GET_GPIO_DATA_IN);
#endif
    if (result == ST_OK)
        *state = ((int)gpio_data.data == GPIO_HIGH);

    return result;
}

STATUS set_gpio_data(int gpio_fd, unsigned short pin, GPIO_status state)
{
#ifndef SPX_BMC
    struct gpio_ioctl_data gpio_data;
    gpio_data.cmd = SET_GPIO_DATA_OUT;
#else
    gpio_ioctl_data gpio_data;
#endif
    gpio_data.PinNum = pin;
    gpio_data.data = state;

    if (state > GPIO_HIGH) {
        return ST_ERR;
    }

#ifndef SPX_BMC
    return gpio_action(gpio_fd, &gpio_data);
#else
    return gpio_action(gpio_fd, &gpio_data,SET_GPIO_DATA_OUT);
#endif
}

STATUS set_gpio_event_mode(int gpio_fd, unsigned short pin, ASD_GPIO_EVENT_TRIGGER_MODE mode)
{
#ifndef SPX_BMC
    struct gpio_ioctl_data gpio_data;
    gpio_data.cmd = SET_GPIO_EVENT_MODE;
    gpio_data.pin = pin;
    gpio_data.data = mode;
    return gpio_action(gpio_fd, &gpio_data);
#else
    STATUS result = ST_OK;
    gpio_interrupt_sensor gpio_intr = {0}; 

    gpio_intr.int_num = pin;
    gpio_intr.gpio_number = pin;
    if(mode == ASD_GPIO_TRIGGER_MODE_LEVEL_HIGH)
    {
        gpio_intr.trigger_method = GPIO_LEVEL;
        gpio_intr.trigger_type = GPIO_HIGH_LEVEL;
    }
    else if(mode == ASD_GPIO_TRIGGER_MODE_LEVEL_LOW)
    {
        gpio_intr.trigger_method = GPIO_LEVEL;
        gpio_intr.trigger_type = GPIO_LOW_LEVEL;
    }
    else if(mode == ASD_GPIO_TRIGGER_MODE_EDGE_FALLING)
    {
        gpio_intr.trigger_method = GPIO_EDGE;
        gpio_intr.trigger_type = GPIO_FALLING_EDGE;
    } 
    else if(mode == ASD_GPIO_TRIGGER_MODE_EDGE_RISING)
    {
        gpio_intr.trigger_method = GPIO_EDGE;
        gpio_intr.trigger_type = GPIO_RISING_EDGE;
    }
    else if(mode == ASD_GPIO_TRIGGER_MODE_EDGE_BOTH)
    {
        gpio_intr.trigger_method = GPIO_EDGE;
        gpio_intr.trigger_type = GPIO_BOTH_EDGES;
    }


    if(ioctl(gpio_fd, (unsigned long)IOCTL_GPIO_INT_REGISTER, &gpio_intr) < 0) 
    {
        result = ST_ERR;
    }
    return result;
#endif
}

STATUS set_gpio_event_enable(int gpio_fd, unsigned short pin, unsigned int enable)
{
#ifndef SPX_BMC
    struct gpio_ioctl_data gpio_data;
    gpio_data.cmd = SET_GPIO_EVENT_ENABLE;
    gpio_data.pin = pin;
    gpio_data.data = enable;

    return gpio_action(gpio_fd, &gpio_data);
#else
    STATUS result = ST_OK;
    if(enable == 0)
    {
        gpio_ioctl_data gpio_data;
        gpio_data.PinNum = pin;
        gpio_data.data = enable;
        result =  gpio_action(gpio_fd, &gpio_data,IOCTL_GPIO_INT_UNREGISTER);
    }
    
    return result;
#endif
}

STATUS get_gpio_event_status(int gpio_fd, unsigned short pin, bool* triggered)
{
    STATUS result = ST_OK;
#ifndef SPX_BMC
    if (triggered == NULL)
        return ST_ERR;
    *triggered = false;
    struct gpio_ioctl_data gpio_data;
    gpio_data.cmd = GET_GPIO_EVENT_STATUS;
    gpio_data.pin = pin;
    gpio_data.data = 0;

    result = gpio_action(gpio_fd, &gpio_data);
    if (result == ST_OK)
        *triggered = ((int)gpio_data.data == 1);
#else
    gpio_ioctl_data gpio_data;
    gpio_data.PinNum = pin;

    result = gpio_action(gpio_fd, &gpio_data,IOCTL_GPIO_INT_GET_STATUS);
    if (result == ST_OK)
        *triggered = ((int)gpio_data.data == 1);
#endif
    return result;
}

STATUS clr_gpio_event_status(int gpio_fd, unsigned short pin)
{
#ifndef SPX_BMC
    struct gpio_ioctl_data gpio_data;
    gpio_data.cmd = CLR_GPIO_EVENT_STATUS;
    gpio_data.pin = pin;
    gpio_data.data = 0;

    return gpio_action(gpio_fd, &gpio_data);
#else
    gpio_ioctl_data gpio_data;
    gpio_data.PinNum = pin;
    return gpio_action(gpio_fd, &gpio_data,IOCTL_GPIO_INT_CLEAR);
#endif
}

STATUS disable_gpio_passthru(int gpio_fd)
{
    STATUS result = ST_OK;
#ifndef SPX_BMC
    struct passthru_ioctl_data passthru_data;

    passthru_data.cmd = SET_GPIO_PASSTHRU_ENABLE;
    passthru_data.idx = GPIO_PASSTHRU2;  // GPIOE4 -> GPIOE5 passthrough idx
    passthru_data.data = 0;              // Disable passthrough

    if (gpio_fd == -1 || ioctl(gpio_fd, GPIO_IOC_PASSTHRU, &passthru_data) < 0) {
        result = ST_ERR;
    }
#endif
    return result;
}

#ifndef SPX_BMC
STATUS gpio_action(int gpio_fd, struct gpio_ioctl_data* data)
#else
STATUS gpio_action(int gpio_fd, gpio_ioctl_data* data, int cmd )
#endif
{
    STATUS result = ST_OK;
    if (data == NULL || gpio_fd == -1) {
        result = ST_ERR;
#ifndef SPX_BMC
    } else if (ioctl(gpio_fd, GPIO_IOC_COMMAND, data) < 0) {
#else
    } else if (ioctl(gpio_fd, (unsigned long)cmd, data) < 0) {
#endif
        result = ST_ERR;
    }
    return result;
}
//AMI_CHANGE_END
