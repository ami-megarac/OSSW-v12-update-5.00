/*
Copyright (c) 2019, Intel Corporation

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

#include "gpio.h"

#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
// clang-format off
#include <safe_str_lib.h>
// clang-format on

#ifdef SPX_BMC
#include <gpio/gpio.h>
#include <sys/ioctl.h>
#include <dlfcn.h>
#include "logging.h"

#define EXTENDED_GPIO_START_NUM 30000
#endif

#define GPIO_EDGE_NONE_STRING "none"
#define GPIO_EDGE_RISING_STRING "rising"
#define GPIO_EDGE_FALLING_STRING "falling"
#define GPIO_EDGE_BOTH_STRING "both"

#define GPIO_DIRECTION_IN_STRING "in"
#define GPIO_DIRECTION_OUT_STRING "out"
#define GPIO_DIRECTION_HIGH_STRING "high"
#define GPIO_DIRECTION_LOW_STRING "low"
#define BUFF_SIZE 48

#ifdef SPX_BMC
static const ASD_LogStream stream = ASD_LogStream_Pins;
static const ASD_LogOption option = ASD_LogOption_None;

STATUS gpio_action(int gpio_fd, gpio_ioctl_data* data, int cmd)
{
    STATUS result = ST_OK;

    if (data == NULL || gpio_fd == -1)
    {
        result = ST_ERR;
    }
    else if (ioctl(gpio_fd, (unsigned long)cmd, data) < 0)
    {
        result = ST_ERR;
    }

    return result;
}

STATUS get_gpio_event_status(int gpio_fd, unsigned short pin, bool* triggered)
{
    STATUS result = ST_OK;
    gpio_ioctl_data gpio_data = {0};
    gpio_data.PinNum = pin;
    void *handle = NULL;
    int (*PDK_GetGPIOEventStatus)(int gpio_fd, unsigned short pin, bool* triggered) = NULL;

    /* BMC GPIO */
    if (pin < EXTENDED_GPIO_START_NUM)
    {
        result = gpio_action(gpio_fd, &gpio_data, IOCTL_GPIO_INT_GET_STATUS);
        if (result == ST_OK)
            *triggered = ((int)gpio_data.data == 1);
        else
            ASD_log(ASD_LogLevel_Error, stream, option, "Error in getting GPIO event status\n");
    }
    /* Extended GPIO */
    else
    {
        handle = dlopen(ASDCFG_PATH, RTLD_NOW | RTLD_GLOBAL | RTLD_NODELETE);
        if (handle == NULL)
        {
            ASD_log(ASD_LogLevel_Error, stream, option, "Error in loading library %s\n", dlerror());
            return ST_ERR;
        }

        PDK_GetGPIOEventStatus = dlsym(handle, "PDK_GetGPIOEventStatus");
        if (PDK_GetGPIOEventStatus == NULL)
        {
            ASD_log(ASD_LogLevel_Error, stream, option, "Error in getting symbol %s\n", dlerror());
            dlclose(handle);
            return ST_ERR;
        }

        if (PDK_GetGPIOEventStatus(gpio_fd, pin, triggered) < 0)
        {
            ASD_log(ASD_LogLevel_Error, stream, option, "Error in getting GPIO event status\n");
            dlclose(handle);
            return ST_ERR;
        }

        dlclose(handle);
    }

    return result;
}

STATUS clear_gpio_interrupts(int gpio_fd, unsigned short pin)
{
    STATUS result = ST_OK;
    gpio_ioctl_data gpio_data = {0};
    gpio_data.PinNum = pin;
    void *handle = NULL;
    int (*PDK_ClearGPIOInterrupts)(int gpio_fd, unsigned short pin) = NULL;

    /* BMC GPIO */
    if (pin < EXTENDED_GPIO_START_NUM)
    {
        if (ioctl(gpio_fd, (unsigned long)IOCTL_GPIO_INT_CLEAR, &gpio_data) < 0)
        {
            ASD_log(ASD_LogLevel_Error, stream, option, "Error in clearing GPIO interrupts %s\n", strerror(errno));
            result = ST_ERR;
        }
    }
    /* Extended GPIO */
    else
    {
        handle = dlopen(ASDCFG_PATH, RTLD_NOW | RTLD_GLOBAL | RTLD_NODELETE);
        if (handle == NULL)
        {
            ASD_log(ASD_LogLevel_Error, stream, option, "Error in loading library %s\n", dlerror());
            return ST_ERR;
        }

        PDK_ClearGPIOInterrupts = dlsym(handle, "PDK_ClearGPIOInterrupts");
        if (PDK_ClearGPIOInterrupts == NULL)
        {
            ASD_log(ASD_LogLevel_Error, stream, option, "Error in getting symbol %s\n", dlerror());
            dlclose(handle);
            return ST_ERR;
        }

        if (PDK_ClearGPIOInterrupts(gpio_fd, pin) < 0)
        {
            ASD_log(ASD_LogLevel_Error, stream, option, "Error in clearing GPIO interrupts\n");
            dlclose(handle);
            return ST_ERR;
        }

        dlclose(handle);
    }

    return result;
}

STATUS get_gpio_data(int gpio_fd, unsigned short pin, GPIO_status* state)
{
    STATUS result = ST_OK;
    *state = GPIO_LOW;
    gpio_ioctl_data gpio_data = {0};
    gpio_data.PinNum = pin;
    gpio_data.data = 0;
    void *handle = NULL;
    int (*PDK_GetGPIOData)(int gpio_fd, unsigned short pin, GPIO_status* state) = NULL;

    /* BMC GPIO */
    if (pin < EXTENDED_GPIO_START_NUM)
    {
        result = gpio_action(gpio_fd, &gpio_data, GET_GPIO_DATA_IN);
        if (result == ST_OK)
            *state = ((int)gpio_data.data == GPIO_HIGH);
        else
            ASD_log(ASD_LogLevel_Error, stream, option, "Error in getting GPIO data\n");
    }
    /* Extended GPIO */
    else
    {
        handle = dlopen(ASDCFG_PATH, RTLD_NOW | RTLD_GLOBAL | RTLD_NODELETE);
        if (handle == NULL)
        {       
            ASD_log(ASD_LogLevel_Error, stream, option, "Error in loading library %s\n", dlerror());
            return ST_ERR;
        }

        PDK_GetGPIOData = dlsym(handle, "PDK_GetGPIOData");
        if (PDK_GetGPIOData == NULL)
        {
            ASD_log(ASD_LogLevel_Error, stream, option, "Error in getting symbol %s\n", dlerror());
            dlclose(handle);
            return ST_ERR;
        }

        if (PDK_GetGPIOData(gpio_fd, pin, state) < 0)
        {
            ASD_log(ASD_LogLevel_Error, stream, option, "Error in getting GPIO data\n");
            dlclose(handle);
            return ST_ERR;
        }

        dlclose(handle);
    }

    return result;
}

STATUS set_gpio_data(int gpio_fd, unsigned short pin, GPIO_status state)
{
    STATUS result = ST_OK;
    gpio_ioctl_data gpio_data = {0};
    gpio_data.PinNum = pin;
    gpio_data.data = state;
    void *handle = NULL;
    int (*PDK_SetGPIOData)(int gpio_fd, unsigned short pin, GPIO_status state) = NULL;

    /* BMC GPIO */
    if (pin < EXTENDED_GPIO_START_NUM)
    {
        if (state > GPIO_HIGH)
        {
            ASD_log(ASD_LogLevel_Error, stream, option, "Error in setting GPIO data. Invalid input\n");
            return ST_ERR;
        }

        result = gpio_action(gpio_fd, &gpio_data, SET_GPIO_DATA_OUT);
        if (result != ST_OK)
            ASD_log(ASD_LogLevel_Error, stream, option, "Error in setting GPIO data\n");
    }
    /* Extended GPIO */
    else
    {
        handle = dlopen(ASDCFG_PATH, RTLD_NOW | RTLD_GLOBAL | RTLD_NODELETE);
        if (handle == NULL)
        {
            ASD_log(ASD_LogLevel_Error, stream, option, "Error in loading library %s\n", dlerror());
            return ST_ERR;
        }

        PDK_SetGPIOData = dlsym(handle, "PDK_SetGPIOData");
        if (PDK_SetGPIOData == NULL)
        {
            ASD_log(ASD_LogLevel_Error, stream, option, "Error in getting symbol %s\n", dlerror());
            dlclose(handle);
            return ST_ERR;
        }

        if (PDK_SetGPIOData(gpio_fd, pin, state) < 0)
        {
            ASD_log(ASD_LogLevel_Error, stream, option, "Error in setting GPIO data\n");
            dlclose(handle);
            return ST_ERR;
        }

        dlclose(handle);
    }

    return result;
}
#endif

STATUS gpio_export(int gpio, int* gpio_fd)
{
#ifndef SPX_BMC
    int fd;
    char buf[BUFF_SIZE];
    int ia[1];
    STATUS result = ST_OK;
    if (!gpio_fd)
        return ST_ERR;
    ia[0] = gpio;
    sprintf_s(buf, sizeof(buf), "/sys/class/gpio/gpio%d/value", ia, 1);
    *gpio_fd = open(buf, O_WRONLY);
    if (*gpio_fd == -1)
    {
        fd = open("/sys/class/gpio/export", O_WRONLY);
        if (fd >= 0)
        {
            sprintf_s(buf, sizeof(buf), "%d", ia, 1);
            if (write(fd, buf, strlen(buf)) < 0)
            {
                result = ST_ERR;
            }
            else
            {
                sprintf_s(buf, sizeof(buf), "/sys/class/gpio/gpio%d/value", ia,
                          1);
                *gpio_fd = open(buf, O_RDWR);
                if (*gpio_fd == -1)
                    result = ST_ERR;
            }
            close(fd);
        }
        else
        {
            result = ST_ERR;
        }
    }
    return result;
#else
	*gpio_fd = open("/dev/gpio0", O_RDWR);
	if (*gpio_fd == -1)
    {
		return ST_ERR;
	}
	return ST_OK;
#endif
}

STATUS gpio_unexport(int gpio)
{
#ifndef SPX_BMC
    int fd;
    char buf[BUFF_SIZE];
    STATUS result = ST_OK;
    int ia[1];
    ia[0] = gpio;
    sprintf_s(buf, sizeof(buf), "/sys/class/gpio/gpio%d/value", ia, 1);
    fd = open(buf, O_WRONLY);
    if (fd >= 0)
    {
        // the gpio exists
        close(fd);
        fd = open("/sys/class/gpio/unexport", O_WRONLY);
        int ia[1];
        ia[0] = gpio;

        if (fd >= 0)
        {
            sprintf_s(buf, sizeof(buf), "%d", ia, 1);
            if (write(fd, buf, strlen(buf)) < 0)
            {
                result = ST_ERR;
            }
            close(fd);
        }
        else
        {
            result = ST_ERR;
        }
    }
    return result;
#else
    return ST_OK;
#endif
}

#ifndef SPX_BMC
STATUS gpio_get_value(int fd, int* value)
#else
STATUS gpio_get_value(int fd, int gpio, int *value)
#endif
{
#ifndef SPX_BMC
    STATUS result = ST_ERR;
    char ch;

    if (value && fd >= 0)
    {
        lseek(fd, 0, SEEK_SET);
        read(fd, &ch, 1);
        *value = ch != '0' ? 1 : 0;
        result = ST_OK;
    }
    return result;
#else
    GPIO_status state;
    STATUS result = ST_ERR;
    result = get_gpio_data(fd, gpio, &state);
    if (result == ST_OK)
    {
        *value = state != GPIO_LOW ? 1 : 0;
        result = ST_OK;
    }
    return result;
#endif
}

#ifndef SPX_BMC
STATUS gpio_set_value(int fd, int value)
#else
STATUS gpio_set_value(int fd, int gpio, int value)
#endif
{
#ifndef SPX_BMC
    STATUS result = ST_ERR;

    if (fd >= 0)
    {
        lseek(fd, 0, SEEK_SET);
        ssize_t written = write(fd, value == 1 ? "1" : "0", sizeof(char));
        if (written == sizeof(char))
        {
            result = ST_OK;
        }
    }
    return result;
#else
    if (value == 0)
        return set_gpio_data(fd, gpio, GPIO_LOW);
    else if (value == 1)
        return set_gpio_data(fd, gpio, GPIO_HIGH);
    else
        return ST_ERR;
#endif
}

#ifndef SPX_BMC
STATUS gpio_set_edge(int gpio, GPIO_EDGE edge)
#else
STATUS gpio_set_edge(int fd, int gpio, _GPIO_EDGE edge)
#endif
{
#ifndef SPX_BMC
    int fd;
    char buf[BUFF_SIZE];
    int ia[1];
    ia[0] = gpio;
    STATUS result = ST_ERR;

    sprintf_s((buf), sizeof(buf), "/sys/class/gpio/gpio%d/edge", ia, 1);
    fd = open(buf, O_WRONLY);
    if (fd >= 0)
    {
        if (edge == GPIO_EDGE_NONE)
            write(fd, GPIO_EDGE_NONE_STRING, strlen(GPIO_EDGE_NONE_STRING));
        else if (edge == GPIO_EDGE_RISING)
            write(fd, GPIO_EDGE_RISING_STRING, strlen(GPIO_EDGE_RISING_STRING));
        else if (edge == GPIO_EDGE_FALLING)
            write(fd, GPIO_EDGE_FALLING_STRING,
                  strlen(GPIO_EDGE_FALLING_STRING));
        else if (edge == GPIO_EDGE_BOTH)
            write(fd, GPIO_EDGE_BOTH_STRING, strlen(GPIO_EDGE_BOTH_STRING));
        close(fd);
        result = ST_OK;
    }
    return result;
#else
    STATUS result = ST_OK;
    gpio_interrupt_sensor gpio_intr = {0}; 
    gpio_intr.int_num = gpio;
    gpio_intr.gpio_number = gpio;
    void *handle = NULL;
    int (*PDK_GPIOSetEdge)(int fd, int gpio, _GPIO_EDGE edge) = NULL;

    /* BMC GPIO */
    if (gpio < EXTENDED_GPIO_START_NUM)
    {
        if (edge == GPIO_EDGE_FALLING)
        {
            gpio_intr.trigger_method = GPIO_EDGE;
            gpio_intr.trigger_type = GPIO_FALLING_EDGE;
        }
        else if (edge == GPIO_EDGE_RISING)
        {
            gpio_intr.trigger_method = GPIO_EDGE;
            gpio_intr.trigger_type = GPIO_RISING_EDGE;
        }
        else if (edge == GPIO_EDGE_BOTH)
        {
            gpio_intr.trigger_method = GPIO_EDGE;
            gpio_intr.trigger_type = GPIO_BOTH_EDGES;
        }

        if (ioctl(fd, (unsigned long)IOCTL_GPIO_INT_REGISTER, &gpio_intr) < 0)
        {
            ASD_log(ASD_LogLevel_Error, stream, option, "Error in setting GPIO edge %s\n", strerror(errno));
            result = ST_ERR;
        }
    }
    /* Extended GPIO */
    else
    {
        handle = dlopen(ASDCFG_PATH, RTLD_NOW | RTLD_GLOBAL | RTLD_NODELETE);
        if (handle == NULL)
        {
            ASD_log(ASD_LogLevel_Error, stream, option, "Error in loading library %s\n", dlerror());
            return ST_ERR;
        }

        PDK_GPIOSetEdge = dlsym(handle, "PDK_GPIOSetEdge");
        if (PDK_GPIOSetEdge == NULL)
        {
            ASD_log(ASD_LogLevel_Error, stream, option, "Error in getting symbol %s\n", dlerror());
            dlclose(handle);
            return ST_ERR;
        }

        if (PDK_GPIOSetEdge(fd, gpio, edge) < 0)
        {
            ASD_log(ASD_LogLevel_Error, stream, option, "Error in setting GPIO edge\n");
            dlclose(handle);
            return ST_ERR;
        }

        dlclose(handle);
    }

    return result;
#endif
}

#ifndef SPX_BMC
STATUS gpio_set_direction(int gpio, GPIO_DIRECTION direction)
#else
STATUS gpio_set_direction(int fd, int gpio, GPIO_DIRECTION direction)
#endif
{
#ifndef SPX_BMC 
    int fd;
    char buf[BUFF_SIZE];
    int ia[1];
    ia[0] = gpio;
    STATUS result = ST_ERR;
    sprintf_s(buf, sizeof(buf), "/sys/class/gpio/gpio%d/direction", ia, 1);
    fd = open(buf, O_WRONLY);
    if (fd >= 0)
    {
        if (direction == GPIO_DIRECTION_IN)
            write(fd, GPIO_DIRECTION_IN_STRING,
                  strlen(GPIO_DIRECTION_IN_STRING));
        else if (direction == GPIO_DIRECTION_OUT)
            write(fd, GPIO_DIRECTION_OUT_STRING,
                  strlen(GPIO_DIRECTION_OUT_STRING));
        else if (direction == GPIO_DIRECTION_HIGH)
            write(fd, GPIO_DIRECTION_HIGH_STRING,
                  strlen(GPIO_DIRECTION_HIGH_STRING));
        else if (direction == GPIO_DIRECTION_LOW)
            write(fd, GPIO_DIRECTION_LOW_STRING,
                  strlen(GPIO_DIRECTION_LOW_STRING));
        close(fd);
        result = ST_OK;
    }
    return result;
#else
    STATUS result = ST_ERR;
    gpio_ioctl_data gpio_data;
    gpio_data.PinNum = gpio;
    void *handle = NULL;
    int (*PDK_GPIOSetDirection)(int fd, int gpio, GPIO_DIRECTION direction) = NULL;

    /* BMC GPIO */
    if (gpio < EXTENDED_GPIO_START_NUM)
    {
        if (direction == GPIO_DIRECTION_IN)
        {
            gpio_data.data = GPIO_DIR_IN;
            result = gpio_action(fd, &gpio_data, SET_GPIO_DIRECTION);
            if (result != ST_OK)
            {
                ASD_log(ASD_LogLevel_Error, stream, option, "Error in setting GPIO direction\n");
                return result;
            }
        }
        else if (direction == GPIO_DIRECTION_OUT)
        {
            gpio_data.data = GPIO_DIR_OUT;
            result = gpio_action(fd, &gpio_data, SET_GPIO_DIRECTION);
            if (result != ST_OK)
            {
                ASD_log(ASD_LogLevel_Error, stream, option, "Error in setting GPIO direction\n");
                return result;
            }
        }
        else if (direction == GPIO_DIRECTION_LOW)
        {
            gpio_data.data = GPIO_DIR_OUT;
            result = gpio_action(fd, &gpio_data, SET_GPIO_DIRECTION);
            if (result != ST_OK)
            {
                ASD_log(ASD_LogLevel_Error, stream, option, "Error in setting GPIO direction\n");
                return result;
            }

            result = set_gpio_data(fd, gpio, GPIO_LOW);
            if (result != ST_OK)
            {
                ASD_log(ASD_LogLevel_Error, stream, option, "Error in setting GPIO direction\n");
                return result;
            }
        }
        else if (direction == GPIO_DIRECTION_HIGH)
        {
            gpio_data.data = GPIO_DIR_OUT;
            result = gpio_action(fd, &gpio_data, SET_GPIO_DIRECTION);
            if (result != ST_OK)
            {
                ASD_log(ASD_LogLevel_Error, stream, option, "Error in setting GPIO direction\n");
                return result;
            }

            result = set_gpio_data(fd, gpio, GPIO_HIGH);
            if (result != ST_OK)
            {
                ASD_log(ASD_LogLevel_Error, stream, option, "Error in setting GPIO direction\n");
                return result;
            }
        }
    }
    /* Extended GPIO */
    else
    {
        handle = dlopen(ASDCFG_PATH, RTLD_NOW | RTLD_GLOBAL | RTLD_NODELETE);
        if (handle == NULL)
        {
            ASD_log(ASD_LogLevel_Error, stream, option, "Error in loading library %s\n", dlerror());
            return ST_ERR;
        }

        PDK_GPIOSetDirection = dlsym(handle, "PDK_GPIOSetDirection");
        if (PDK_GPIOSetDirection == NULL)
        {
            ASD_log(ASD_LogLevel_Error, stream, option, "Error in getting symbol %s\n", dlerror());
            dlclose(handle);
            return ST_ERR;
        }

        if (PDK_GPIOSetDirection(fd, gpio, direction) < 0)
        {
            ASD_log(ASD_LogLevel_Error, stream, option, "Error in setting GPIO direction\n");
            dlclose(handle);
            return ST_ERR;
        }

        result = ST_OK;
        dlclose(handle);
    }

    return result;
#endif
}

#ifndef SPX_BMC
STATUS gpio_set_active_low(int gpio, bool active_low)
#else
STATUS gpio_set_active_low(int fd, int gpio, bool active_low)
#endif
{
#ifndef SPX_BMC
    int fd;
    char buf[BUFF_SIZE];
    STATUS result = ST_ERR;
    int ia[1];
    ia[0] = gpio;
    sprintf_s(buf, sizeof(buf), "/sys/class/gpio/gpio%d/active_low", ia, 1);
    fd = open(buf, O_WRONLY);
    if (fd >= 0)
    {
        if (write(fd, active_low ? "1" : "0", 1) == 1)
        {
            result = ST_OK;
        }
        close(fd);
    }
    return result;
#else
    return ST_OK;
#endif
}
