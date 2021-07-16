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

#ifndef _GPIO_H_
#define _GPIO_H_

#include "asd_common.h"
#include "config.h"
#include <stdbool.h>

//AMI_CHANGE_START
//Adapting GPIO specific definitions
#ifdef SPX_BMC
#include <sys/types.h>
#include <unistd.h>

#define SET_GPIO_DIRECTION              _IOC(_IOC_WRITE,'K',0x02,0x3FFF)
#define READ_GPIO                       _IOC(_IOC_WRITE,'K',0x07,0x3FFF)
#define WRITE_GPIO                      _IOC(_IOC_WRITE,'K',0x08,0x3FFF)
#define GPIO_DIR_IN    0
#define GPIO_DIR_OUT   1

#define GET_GPIO_DATA_IN    READ_GPIO
#define SET_GPIO_DATA_OUT   WRITE_GPIO

#endif
//AMI_CHANGE_END
typedef enum {
    ASD_GPIO_TRIGGER_MODE_LEVEL_HIGH = 0,
    ASD_GPIO_TRIGGER_MODE_LEVEL_LOW,
    ASD_GPIO_TRIGGER_MODE_EDGE_RISING,
    ASD_GPIO_TRIGGER_MODE_EDGE_FALLING,
    ASD_GPIO_TRIGGER_MODE_EDGE_BOTH
} ASD_GPIO_EVENT_TRIGGER_MODE;

typedef enum {
    GPIO_LOW = 0,
    GPIO_HIGH
} GPIO_status;

typedef struct GPIO_State {
    int gpio_fd;
    pid_t pid;
    gpios_config* gpios_config;
} GPIO_State;

STATUS open_gpio_driver(int* gpio_fd);
STATUS close_gpio_driver(int* gpio_fd);
STATUS set_gpio_dir_input(int gpio_fd, unsigned short pin);
STATUS set_gpio_dir_output(int gpio_fd, unsigned short pin);
STATUS get_gpio_data(int gpio_fd, unsigned short pin, GPIO_status* state);
STATUS set_gpio_data(int gpio_fd, unsigned short pin, GPIO_status state);
STATUS set_gpio_event_mode(int gpio_fd, unsigned short pin, ASD_GPIO_EVENT_TRIGGER_MODE mode);
STATUS set_gpio_event_enable(int gpio_fd, unsigned short pin, unsigned int enable);
STATUS get_gpio_event_status(int gpio_fd, unsigned short pin, bool* triggered);
STATUS clr_gpio_event_status(int gpio_fd, unsigned short pin);
STATUS disable_gpio_passthru(int gpio_fd);

#endif  //_GPIO_H_
