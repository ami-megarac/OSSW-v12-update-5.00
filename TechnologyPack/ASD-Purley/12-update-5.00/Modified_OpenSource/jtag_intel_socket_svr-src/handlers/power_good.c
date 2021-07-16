/*
Copyright (c) 2018, Intel Corporation

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

#include <stddef.h>

#include "power_good.h"

STATUS power_good_initialize(const GPIO_State* state)
{
    if (state == NULL || state->gpio_fd < 0)
        return ST_ERR;
    if (state->gpios_config->power_good.is_present) {
        if (set_gpio_dir_input(state->gpio_fd, state->gpios_config->power_good.pin) != ST_OK)
            return ST_ERR;
//AMI_CHANGE_START
//No Event Support
//#ifndef SPX_BMC
        if((state->gpios_config->power_good.gpio_prop >> 2) & 0x01)
        {
            if (set_gpio_event_mode(state->gpio_fd, state->gpios_config->power_good.pin, ASD_GPIO_TRIGGER_MODE_EDGE_BOTH) != ST_OK)
                return ST_ERR;
            if (set_gpio_event_enable(state->gpio_fd, state->gpios_config->power_good.pin, 1) != ST_OK)
                return ST_ERR;
        }
//#endif
//AMI_CHANGE_END
    }
    return ST_OK;
}

STATUS power_good_deinitialize(const GPIO_State* state)
{
    if (state == NULL || state->gpio_fd < 0)
        return ST_ERR;
    if (state->gpios_config->power_good.is_present) {
        // disable the event
//AMI_CHANGE_START
//No Event Support
//#ifndef SPX_BMC
        if((state->gpios_config->power_good.gpio_prop >> 2) & 0x01)
        {
            if (set_gpio_event_enable(state->gpio_fd, state->gpios_config->power_good.pin, 0) != ST_OK)
                return ST_ERR;
        }
//#endif
//AMI_CHANGE_END
        if (set_gpio_dir_input(state->gpio_fd, state->gpios_config->power_good.pin) != ST_OK)
            return ST_ERR;
    }
    return ST_OK;
}

STATUS power_good_is_event_triggered(const GPIO_State* state, bool* triggered)
{
//AMI_CHANGE_START
//No Event Support
//#ifndef SPX_BMC
    if (state == NULL || state->gpio_fd < 0)
        return ST_ERR;
    if(((state->gpios_config->power_good.gpio_prop >> 2) & 0x01) == 0x0)
        return ST_OK;
    *triggered = false;
    if (state->gpios_config->power_good.is_present) {
        if (get_gpio_event_status(state->gpio_fd, state->gpios_config->power_good.pin, triggered) != ST_OK)
            return ST_ERR;
        if (*triggered)
            return clr_gpio_event_status(state->gpio_fd, state->gpios_config->power_good.pin);
    }
//#endif
//AMI_CHANGE_END
    return ST_OK;
}

STATUS power_good_is_asserted(const GPIO_State* state, bool* asserted)
{
    if (state == NULL || state->gpio_fd < 0)
        return ST_ERR;
    STATUS status = ST_OK;
    *asserted = true;
    if (state->gpios_config->power_good.is_present) {
        GPIO_status pinState = GPIO_LOW;
        status = get_gpio_data(state->gpio_fd, state->gpios_config->power_good.pin, &pinState);
        if (status == ST_OK)
            *asserted = pinState == state->gpios_config->power_good.assert_high ? GPIO_HIGH : GPIO_LOW;
    }
    return status;
}
