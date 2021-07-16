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

#include "preq.h"

STATUS preq_initialize(const GPIO_State* state)
{
    if (state == NULL || state->gpio_fd < 0)
        return ST_ERR;

    if (state->gpios_config->preq.is_present) {
        // it appears that the AST2500 needs to have this set as an input until
        // it's time to trigger otherwise it will cause a PREQ event
        // For other setups, you may be able to set it to an output right away
        return set_gpio_dir_input(state->gpio_fd, state->gpios_config->preq.pin);
    }
    return ST_OK;
}

STATUS preq_deinitialize(const GPIO_State* state)
{
    if (state == NULL || state->gpio_fd < 0)
        return ST_ERR;
    if (state->gpios_config->preq.is_present) {
        // For some boards, we must set PREQ back to input when we are done
        return set_gpio_dir_input(state->gpio_fd, state->gpios_config->preq.pin);
    }
    return ST_OK;
}

STATUS preq_assert(const GPIO_State* state, const bool assert)
{
    if (state == NULL || state->gpio_fd < 0)
        return ST_ERR;
    if (state->gpios_config->preq.is_present) {
        if (set_gpio_dir_output(state->gpio_fd, state->gpios_config->preq.pin) != ST_OK)
            return ST_ERR;

        return set_gpio_data(state->gpio_fd, state->gpios_config->preq.pin,
                             assert == state->gpios_config->preq.assert_high ? GPIO_HIGH : GPIO_LOW);
    }
    return ST_OK;
}

STATUS preq_is_asserted(const GPIO_State* state, bool* asserted)
{
    if (state == NULL || state->gpio_fd < 0 || asserted == NULL)
        return ST_ERR;
    *asserted = false;
    STATUS status = ST_OK;
    if (state->gpios_config->preq.is_present) {
        GPIO_status pinState = GPIO_LOW;
        status = get_gpio_data(state->gpio_fd, state->gpios_config->preq.pin, &pinState);
        if (status == ST_OK)
            *asserted = pinState == state->gpios_config->preq.assert_high ? GPIO_HIGH : GPIO_LOW;
    }
    return status;
}
