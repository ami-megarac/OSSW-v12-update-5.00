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

#include <stdint.h>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>

#include "asd_common.h"
#include "debug_enable.h"
#include "gpio.h"
#include "ipmi.h"
#include "logging.h"
#include "platform_reset.h"
#include "power_btn.h"
#include "power_debug.h"
#include "power_good.h"
#include "prdy.h"
#include "preq.h"
#include "reset_btn.h"
#include "rsm_reset.h"
#include "sys_pwr_ok.h"
#include "target_handler.h"
#include "tck_mux_select.h"
#include "xdp_present.h"

#define JTAG_CLOCK_CYCLE_MICROSEC 1

void* gpioThread(void* args);
STATUS initializeGPIOs(Target_Control_Handle* state);
STATUS deinitializeGPIOs(Target_Control_Handle* state);
STATUS checkXDPstate(Target_Control_Handle* state);

Target_Control_Handle* TargetHandler(TargetHandlerEventFunctionPtr event_cb, gpios_config* gpios_config)
{
    if (event_cb == NULL || gpios_config == NULL)
        return NULL;
    Target_Control_Handle* state = (Target_Control_Handle*)malloc(sizeof(Target_Control_Handle));
    if (state == NULL)
        return NULL;
    state->event_cb = event_cb;
    state->thread_started = false;
    state->exit_thread = false;
    state->event_cfg.break_all = false;
    state->event_cfg.reset_break = false;
    state->event_cfg.report_PRDY = false;
    state->event_cfg.report_PLTRST = false;
    state->event_cfg.report_MBP = false;
    state->gpio_state.gpios_config = gpios_config;
    state->gpio_state.gpio_fd = -1;

    return state;
}

STATUS target_initialize(Target_Control_Handle* state, ClientAddrT cl_addr)
{
    if (state == NULL)
        return ST_ERR;

    state->event_cfg.break_all = false;
    state->event_cfg.reset_break = false;
    state->event_cfg.report_PRDY = false;
    state->event_cfg.report_PLTRST = false;
    state->event_cfg.report_MBP = false;
//AMI_CHANGE_START
//ipmi session is created while issuing command
#ifndef SPX_BMC
    if (ipmi_initialize(cl_addr) != ST_OK) {
        ASD_log(LogType_Error, "Failed to get intraBMC process");
        return ST_ERR;
    }
#endif
//AMI_CHANGE_END

    if (open_gpio_driver(&state->gpio_state.gpio_fd) != ST_OK) {
        ASD_log(LogType_Error, "Failed to open GPIO driver");
        return ST_ERR;
    }

    if (xdp_present_initialize(&state->gpio_state) != ST_OK) {
        ASD_log(LogType_Error, "Error initializing XDP_PRESENT pin");
        return ST_ERR;
    }

    if (checkXDPstate(state) != ST_OK) {
        ASD_log(LogType_Error, "Failed check XDP state or XDP not available");
        return ST_ERR;
    }

    if (initializeGPIOs(state) != ST_OK) {
        ASD_log(LogType_Error, "Failed initialize GPIOs");
        return ST_ERR;
    }

    if (pthread_mutex_init(&state->write_config_mutex, NULL) != 0) {
        ASD_log(LogType_Error, "Failed to init write config mutex");
        return ST_ERR;
    }

    state->exit_thread = false;
    if (pthread_create(&state->gpio_thread, NULL, &gpioThread, state)) {
        ASD_log(LogType_Error, "Error creating thread");
        return ST_ERR;
    }
    state->thread_started = true;

    return ST_OK;
}

STATUS target_deinitialize(Target_Control_Handle* state)
{
    int ret = 0;
    if (state == NULL)
        return ST_ERR;
    state->exit_thread = true;
    if (state->thread_started) {
        pthread_join(state->gpio_thread, NULL);
        ret = pthread_mutex_destroy(&state->write_config_mutex);
        if (ret != 0)
            ASD_log(LogType_Error, "pthread_mutex_destroy failed, returned: %d", ret);
        state->thread_started = false;
    }

//AMI_CHANGE_START
    // close ipmi interface.
#ifndef SPX_BMC
    ipmi_close();
#endif
//AMI_CHANGE_END

    return deinitializeGPIOs(state);
}

STATUS initializeGPIOs(Target_Control_Handle* state)
{
    if (state == NULL)
        return ST_ERR;
    // configure GPIOs to their appropriate IO and check that an external probe is not connected
    STATUS result = ST_OK;
//AMI_CHANGE_START
//gpio passthru pin not available
#ifndef SPX_BMC
    result = disable_gpio_passthru(state->gpio_state.gpio_fd);
    if (result != ST_OK) {
        ASD_log(LogType_Error, "Failed to disable debug enable passthrough!");
        return result;
    }
#endif
//AMI_CHANGE_END

    result = platform_reset_initialize(&state->gpio_state);
    if (result != ST_OK) {
        ASD_log(LogType_Error, "Error initializing PLTRST pin");
        return result;
    }

    result = power_debug_initialize(&state->gpio_state);
    if (result != ST_OK) {
        ASD_log(LogType_Error, "Error initializing CPU_PWR_DEBUG pin");
        return result;
    }

    result = power_good_initialize(&state->gpio_state);
    if (result != ST_OK) {
        ASD_log(LogType_Error, "Error initializing CPU_PWRGD pin");
        return result;
    }

    result = prdy_initialize(&state->gpio_state);
    if (result != ST_OK) {
        ASD_log(LogType_Error, "Error initializing CPU_PRDY pin");
        return result;
    }

    result = debug_enable_initialize(&state->gpio_state);
    if (result != ST_OK) {
        ASD_log(LogType_Error, "Error initializing DEBUG_EN pin");
        return result;
    }

    result = preq_initialize(&state->gpio_state);
    if (result != ST_OK) {
        ASD_log(LogType_Error, "Error initializing PREQ pin");
        return result;
    }

    result = tck_mux_select_initialize(&state->gpio_state);
    if (result != ST_OK) {
        ASD_log(LogType_Error, "Error initializing TCK_MUX_SEL pin");
        return result;
    }

    result = rsm_reset_initialize(&state->gpio_state);
    if (result != ST_OK) {
        ASD_log(LogType_Error, "Error initializing RSM_RESET pin");
        return result;
    }

    result = sys_pwr_ok_initialize(&state->gpio_state);
    if (result != ST_OK) {
        ASD_log(LogType_Error, "Error initializing SYS_PWR_OK pin");
        return result;
    }

    if (state->gpio_state.gpios_config->use_ipmi_interface == false) {
        result = power_btn_initialize(&state->gpio_state);
        if (result != ST_OK) {
            ASD_log(LogType_Error, "Error initializing POWER_BTN pin");
            return result;
        }

        result = reset_btn_initialize(&state->gpio_state);
        if (result != ST_OK) {
            ASD_log(LogType_Error, "Error initializing RESET_BTN pin");
            return result;
        }

        result = power_good_initialize(&state->gpio_state);
        if (result != ST_OK) {
            ASD_log(LogType_Error, "Error initializing POWER_GOOD pin");
            return result;
        }
    }
//AMI_CHANGE_START
//enabling XDP 

#ifdef SPX_BMC
    if(state->gpio_state.gpios_config->xdp_jtag_sel.is_present) {
		result = ASDGPIO_Init(state->gpio_state.gpios_config->xdp_jtag_sel.pin);
		if (result != ST_OK) {
			ASD_log(LogType_Error, "Error initializing XDP JTAG pin!");
			return result;
		}
    }
    if(state->gpio_state.gpios_config->xdp_bmc_jtag_sel.is_present) {
		result = ASDGPIO_Init(state->gpio_state.gpios_config->xdp_bmc_jtag_sel.pin);
		if (result != ST_OK) {
			ASD_log(LogType_Error, "Error initializing XDP BMC JTAG pin!");
			return result;
		}
    }
#endif
//AMI_CHANGE_END

    ASD_log(LogType_Debug, "GPIOs initialized successfully");
    return result;
}

STATUS deinitializeGPIOs(Target_Control_Handle* state)
{
    if (state == NULL)
        return ST_ERR;

    if (state->gpio_state.gpio_fd != -1) {
        if (preq_deinitialize(&state->gpio_state) != ST_OK) {
            ASD_log(LogType_Error, "Failed to deinitialize PREQ pin");
        }
        if (debug_enable_deinitialize(&state->gpio_state) != ST_OK) {
            ASD_log(LogType_Error, "Failed to deinitialize DEBUG_EN pin");
        }
        if (power_good_deinitialize(&state->gpio_state) != ST_OK) {
            ASD_log(LogType_Error, "Failed to deinitialize CPU_PWRGD pin");
        }
        if (power_debug_deinitialize(&state->gpio_state) != ST_OK) {
            ASD_log(LogType_Error, "Failed to deinitialize CPU_PWR_DEBUG pin");
        }
        if (tck_mux_select_deinitialize(&state->gpio_state) != ST_OK) {
            ASD_log(LogType_Error, "Failed to deinitialize TCK_MUX_SEL pin");
        }
        if (platform_reset_deinitialize(&state->gpio_state) != ST_OK) {
            ASD_log(LogType_Error, "Failed to deinitialize PLTRST pin");
        }
        if (prdy_deinitialize(&state->gpio_state) != ST_OK) {
            ASD_log(LogType_Error, "Failed to deinitialize CPU_PRDY pin");
        }
        if (xdp_present_deinitialize(&state->gpio_state) != ST_OK) {
            ASD_log(LogType_Error, "Failed to deinitialize XDP_PRESET pin");
        }
        if (rsm_reset_deinitialize(&state->gpio_state) != ST_OK) {
            ASD_log(LogType_Error, "Failed to deinitialize RSM_RESET pin");
        }
        if (sys_pwr_ok_deinitialize(&state->gpio_state) != ST_OK) {
            ASD_log(LogType_Error, "Failed to deinitialize SYS_PWR_OK pin");
        }
        if (state->gpio_state.gpios_config->use_ipmi_interface == false) {
            if (power_btn_deinitialize(&state->gpio_state) != ST_OK) {
                ASD_log(LogType_Error, "Error deinitializing POWER_BTN pin");
            }

            if (reset_btn_deinitialize(&state->gpio_state) != ST_OK) {
                ASD_log(LogType_Error, "Error deinitializing RESET_BTN pin");
            }

            if (power_good_deinitialize(&state->gpio_state) != ST_OK) {
                ASD_log(LogType_Error, "Error deinitializing POWER_GOOD pin");
            }
        }
//AMI_CHANGE_START
//disabling XDP
#ifdef SPX_BMC
        if(state->gpio_state.gpios_config->xdp_jtag_sel.is_present) {
			if (ASDGPIO_DeInit(state->gpio_state.gpios_config->xdp_jtag_sel.pin) != ST_OK) {
				ASD_log(LogType_Error, "Failed to deinitialize XDP_JTAG_SEL pin");
			}
        }
		if(state->gpio_state.gpios_config->xdp_bmc_jtag_sel.is_present) {
			if (ASDGPIO_DeInit(state->gpio_state.gpios_config->xdp_bmc_jtag_sel.pin) != ST_OK) {
				ASD_log(LogType_Error, "Failed to deinitialize XDP BMC JTAG pin!");
			}
		}
#endif  
//AMI_CHANGE_END
        if (close_gpio_driver(&state->gpio_state.gpio_fd) != ST_OK) {
            ASD_log(LogType_Error, "Failed to close GPIO driver pin");
        }
    }

    ASD_log(LogType_Debug, "GPIOs deinitalized successfully");
    return ST_OK;
}

void* gpioThread(void* args)
{
    Target_Control_Handle* state = (Target_Control_Handle*)args;
    STATUS status = ST_OK;
    bool asserted = false;
    bool triggered = false;
    int ret = 0;
//AMI_CHANGE_START
//No Event support for Power good
#ifdef SPX_BMC
    static int cpuPwrgd = -1;
#endif
    ASD_log(LogType_Debug, "GPIO Monitoring Thread Started");

#ifdef SPX_BMC
   status = power_good_is_asserted(&state->gpio_state, &asserted);
    if (status != ST_OK) {
        ASD_log(LogType_Error, "Failed to get gpio data for CPU_PWRGD: %d", status);
    } else {
        cpuPwrgd = asserted;
    }
#endif
//AMI_CHANGE_END
    while (!state->exit_thread) {
        ret = pthread_mutex_lock(&state->write_config_mutex);
        if (ret != 0) {
            ASD_log(LogType_Error, "pthread_mutex_lock failed, returned: %d", ret);
            break;
        }

        // Power
//AMI_CHANGE_START
//No Event support for Power good
#ifndef SPX_BMC
        status = power_good_is_event_triggered(&state->gpio_state, &triggered);
        if (status != ST_OK) {
            ASD_log(LogType_Error, "Failed to check event trigger for CPU_PWRGD: %d", status);
        } else if (triggered) {
            status = power_good_is_asserted(&state->gpio_state, &asserted);
            if (status != ST_OK) {
                ASD_log(LogType_Error, "Failed to get gpio data for CPU_PWRGD: %d", status);
            } else if (asserted) {
#else
        status = power_good_is_asserted(&state->gpio_state, &asserted);
        if (status != ST_OK) {
            ASD_log(LogType_Error, "Failed to get gpio data for CPU_PWRGD: %d", status);
        } else if (asserted != cpuPwrgd) {
            cpuPwrgd = asserted;
            if (asserted) {
#endif
//AMI_CHANGE_END

                ASD_log(LogType_Debug, "Power restored");
                // send info back to the plugin that power was restored
                state->event_cb(PIN_EVENT, ASD_EVENT_PWRRESTORE);
            } else {
                ASD_log(LogType_Debug, "Power fail");
                // send info back to the plugin that power failed
                state->event_cb(PIN_EVENT, ASD_EVENT_PWRFAIL);
            }
        }

        // Platform reset
        status = platform_reset_is_event_triggered(&state->gpio_state, &triggered);
        if (status != ST_OK) {
            ASD_log(LogType_Error, "Failed to get gpio event status for PLTRST: %d", status);
        } else if (triggered) {
            status = platform_reset_is_asserted(&state->gpio_state, &asserted);
            if (status != ST_OK) {
                ASD_log(LogType_Error, "Failed to get gpio data for PLTRST: %d", status);
            } else if (asserted) {
                ASD_log(LogType_Debug, "Platform reset asserted");
                state->event_cb(PIN_EVENT, ASD_EVENT_PLRSTASSERT);  // Reset asserted
#ifdef SUPPORT_RESET_BREAK
                if (state->event_cfg.report_PRDY && state->event_cfg.reset_break) {
                    ASD_log(LogType_Debug,
                            "ResetBreak detected PLT_RESET "
                            "assert, asserting PREQ");
                    if (preq_assert(&state->gpio_state, true) != ST_OK) {
                        ASD_log(LogType_Error, "Failed to assert PREQ");
                    }
                }
#endif
            } else {
                ASD_log(LogType_Debug, "Platform reset de-asserted");
                state->event_cb(PIN_EVENT, ASD_EVENT_PLRSTDEASSRT);  // Reset de-asserted

#ifdef HANDLE_RESET_BREAK_ON_PLATFORM_RESET_DEASSERT
                // OpenIPC handles the Reset Break on Platform Reset Deassert.
                // If we ever need to control it from the BMC (IE: if network performance
                // isn't sufficient), then we would re-enable this code.
                if (state->event_cfg.report_PRDY && state->event_cfg.reset_break) {
                    ASD_log(LogType_Debug,
                            "ResetBreak detected PLT_RESET "
                            "deassert, asserting PREQ again");
                    // wait 10 ms
                    usleep(10000);
                    status = prdy_is_event_triggered(&state->gpio_state, &triggered);
                    if (status != ST_OK) {
                        ASD_log(LogType_Error, "Failed to get gpio event status for CPU_PRDY: %d", status);
                    } else if (triggered) {
                        ASD_log(LogType_Debug, "CPU_PRDY asserted event cleared.");
                    }
                    // deassert, assert, then again deassert PREQ
                    if (preq_assert(&state->gpio_state, false) != ST_OK) {
                        ASD_log(LogType_Error, "Failed to deassert PREQ");
                    }
                    if (preq_assert(&state->gpio_state, true) != ST_OK) {
                        ASD_log(LogType_Error, "Failed to assert PREQ");
                    }
                    if (preq_assert(&state->gpio_state, false) != ST_OK) {
                        ASD_log(LogType_Error, "Failed to deassert PREQ");
                    }
                }
#endif
            }
        }

        // PRDY Event handling
        if (state->event_cfg.report_PRDY) {
            status = prdy_is_event_triggered(&state->gpio_state, &triggered);
            if (status != ST_OK) {
                ASD_log(LogType_Error, "Failed to get gpio event status for CPU_PRDY: %d", status);
            } else if (triggered) {
                ASD_log(LogType_Debug, "CPU_PRDY Asserted Event Detected. Sending PRDY event to plugin");
                state->event_cb(PIN_EVENT, ASD_EVENT_PRDY_EVENT);
                if (state->event_cfg.break_all) {
                    ASD_log(LogType_Debug, "BreakAll detected PRDY, asserting PREQ");
                    if (preq_assert(&state->gpio_state, true) != ST_OK) {
                        ASD_log(LogType_Error, "Failed to assert PREQ");
                    }
                    usleep(10000);
                    if (preq_assert(&state->gpio_state, false) != ST_OK) {
                        ASD_log(LogType_Error, "Failed to deassert PREQ");
                    }
                }
            }
        }

        // XDP present detection - This should go last because it can deinit all the pins.
//AMI_CHANGE_START
//No Event support for XDP_PRESENT
#ifndef SPX_BMC
        status = xdp_present_is_event_triggered(&state->gpio_state, &triggered);
        if (status != ST_OK) {
            ASD_log(LogType_Error, "Failed to event status for XDP_PRESENT: %d", status);
        } else if (triggered) {
            ASD_log(LogType_Debug, "XDP Present state change detected");
            if(checkXDPstate(state) != ST_OK) {
                state->exit_thread = true;
                state->thread_started = false;
                ret = pthread_mutex_unlock(&state->write_config_mutex);
                if (ret != 0)
                    ASD_log(LogType_Error, "pthread_mutex_unlock failed, returned: %d", ret);
                return NULL;
            }
        }
#endif

        ret = pthread_mutex_unlock(&state->write_config_mutex);
        if (ret != 0) {
            ASD_log(LogType_Error, "pthread_mutex_unlock failed, returned: %d", ret);
            break;
        }
        usleep(5000);
    }
    return NULL;
}

STATUS target_write(Target_Control_Handle* state, const Pin pin, const bool assert)
{
    STATUS retVal = ST_OK;

    if (state == NULL) {
        ASD_log(LogType_Error, "target_write, null state error.");
        return ST_ERR;
    }

    switch (pin) {
        case PIN_EARLY_BOOT_STALL: {
//AMI_CHANGE_START
//PIN_EARLY_BOOT_STALL not supported
#ifndef SPX_BMC
            ASD_log(LogType_Debug, "Pin Write: %s CPU_PWR_DEBUG",
                    assert ? "assert" : "deassert");
            if (power_debug_assert(&state->gpio_state, assert) != ST_OK) {
                ASD_log(LogType_Error, "Failed to set PWR_DEBUG");
                retVal = ST_ERR;
            }
#else
           ASD_log(LogType_Debug, "target_write: PIN_EARLY_BOOT_STALL not supported\n");
            retVal = ST_OK;
#endif
//AMI_CHANGE_END
            break;
        }
        case PIN_POWER_BUTTON: {
            ASD_log(LogType_Debug, "Pin Write: %s POWER_BUTTON",
                    assert ? "assert" : "deassert");
            if (state->gpio_state.gpios_config->use_ipmi_interface) {

                if (ipmi_set_power_state(assert) != ST_OK) {
                    ASD_log(LogType_Error, "jtag_socket_svr: Failed to set power state!");
                    retVal = ST_ERR;
                }
                if (assert && state->event_cfg.reset_break) {
                     if (preq_assert(&state->gpio_state, true) != ST_OK) {
                         ASD_log(LogType_Error, "Failed to assert PREQ");
                     }
                }
            } else {
                if (power_btn_assert(&state->gpio_state, assert) != ST_OK) {
                    ASD_log(LogType_Error, "Failed to assert power button.");
                    retVal = ST_ERR;
                }
            }
            break;
        }
        case PIN_RESET_BUTTON: {
            ASD_log(LogType_Debug, "Pin Write: %s RESET_BUTTON",
                    assert ? "assert" : "deassert");
            // Reset doesn't have a de-asserted state; only care if it's been asserted
            if (state->gpio_state.gpios_config->use_ipmi_interface) {
                if (assert) {
                    if ((ipmi_reset_target() != ST_OK)) {
                        ASD_log(LogType_Error, "Failed to reset target!");
                        retVal = ST_ERR;
                    }
                    if (state->event_cfg.reset_break) {
                        if (preq_assert(&state->gpio_state, true) != ST_OK) {
                            ASD_log(LogType_Error, "Failed to assert PREQ");
                        }
                    }
                }
            } else {
                if (reset_btn_assert(&state->gpio_state, assert) != ST_OK) {
                    ASD_log(LogType_Error, "reset_btn_assert failed");
                    retVal = ST_ERR;
                } else {
                    usleep(500000);
                }
            }
            break;
        }
        case PIN_PREQ: {
            ASD_log(LogType_Debug, "Pin Write: %s PREQ",
                    assert ? "assert" : "deassert");
            if (preq_assert(&state->gpio_state, assert) != ST_OK) {
                ASD_log(LogType_Error, "Failed to set PREQ pin");
                retVal = ST_ERR;
            }
            break;
        }
        case PIN_SYS_PWR_OK: {
            ASD_log(LogType_Debug, "Pin Write: %s SYS_PWR_OK",
                    assert ? "assert" : "deassert");
            if (sys_pwr_ok_assert(&state->gpio_state, assert) != ST_OK) {
                ASD_log(LogType_Error, "Failed to set SYS_PWR_OK pin");
                retVal = ST_ERR;
            }
            break;
        }
        default: {
            ASD_log(LogType_Debug, "Pin write: unsupported pin 0x%02x", pin);
            retVal = ST_ERR;
        }
    }

    return retVal;
}

STATUS target_read(Target_Control_Handle* state, const ReadType statusRegister,
                   const uint8_t pin, bool* asserted)
{
    int ret = 0;
    if (state == NULL || asserted == NULL) {
        ASD_log(LogType_Error, "target_read, null state error.");
        return ST_ERR;
    }
    // the value of the "pin" is passed back via the "asserted" argument
    *asserted = false;

    if (statusRegister == READ_TYPE_PROBE) {
        if (pin == PRDY_EVENT_DETECTED) {
            bool triggered = false;
            ret = pthread_mutex_lock(&state->write_config_mutex);
            if (ret != 0) {
                ASD_log(LogType_Error, "pthread_mutex_lock failed, returned: %d", ret);
                return ST_ERR;
            }
            STATUS status = prdy_is_event_triggered(&state->gpio_state, &triggered);
            if (status != ST_OK) {
                ASD_log(LogType_Error, "Failed to get gpio event status for CPU_PRDY: %d", status);
                ret = pthread_mutex_unlock(&state->write_config_mutex);
                if (ret != 0) {
                    ASD_log(LogType_Error, "pthread_mutex_unlock failed, returned: %d", ret);
                }
                return status;
            } else if (triggered) {
                ASD_log(LogType_Debug, "CPU_PRDY state change detected, clearing PRDY event.");
                *asserted = true;
            }
            ret = pthread_mutex_unlock(&state->write_config_mutex);
            if (ret != 0) {
                ASD_log(LogType_Error, "pthread_mutex_unlock failed, returned: %d", ret);
                return ST_ERR;
            }
        } else {
            ASD_log(LogType_Error, "Unknown Probe Status pin.");
        }
    } else if (statusRegister == READ_TYPE_PIN) {
        switch (pin) {
            case PIN_PWRGOOD: {  // Power Good
                if (state->gpio_state.gpios_config->use_ipmi_interface) {
                    int value = 0;
                    if (ipmi_get_power_state(&value) != ST_OK) {
                        ASD_log(LogType_Error, "IntraBmcCmd command to get target power state failed!");
                        return ST_ERR;
                    }
                    *asserted = (value == 1);
                } else {
                    bool pinState = false;
                    STATUS status = power_good_is_asserted(&state->gpio_state, &pinState);
                    if (status != ST_OK) {
                        ASD_log(LogType_Error, "power_good_is_asserted failed: %d", status);
                        return ST_ERR;
                    }
                    *asserted = pinState;
                }
                ASD_log(LogType_Debug, "Pin read: Power Good %s",
                        *asserted ? "asserted" : "deasserted");
                break;
            }
            case PIN_PREQ: {  // PREQ
                bool pinState = false;
                STATUS status = preq_is_asserted(&state->gpio_state, &pinState);
                if (status != ST_OK) {
                    ASD_log(LogType_Error, "preq_is_asserted failed: %d", status);
                    return ST_ERR;
                }
                *asserted = pinState;
                ASD_log(LogType_Debug, "Pin read: PREQ %s",
                        *asserted ? "asserted" : "deasserted");
                break;
            }
            case PIN_RESET_BUTTON: {  // Reset Button
                ASD_log(LogType_Debug, "Pin read: Reset Button Not Supported");
                break;
            }
            case PIN_POWER_BUTTON: {  // Power Button
                if (state->gpio_state.gpios_config->use_ipmi_interface) {
                    int value = 0;
                    if (ipmi_get_power_state(&value) != ST_OK) {
                        ASD_log(LogType_Error,
                                "IntraBmcCmd command to get target power state failed!");
                        return ST_ERR;
                    }
                    *asserted = value == 1;
                } else {
                    bool pinState = false;
                    STATUS status = power_btn_is_asserted(&state->gpio_state, &pinState);
                    if (status != ST_OK) {
                        ASD_log(LogType_Error, "power_btn_is_asserted failed: %d", status);
                        return ST_ERR;
                    }
                    *asserted = pinState;
                }
                ASD_log(LogType_Debug, "Pin read: Power Button %s",
                        *asserted ? "asserted" : "deasserted");
                break;
            }
            case PIN_EARLY_BOOT_STALL: {  // Early Boot Stall
//AMI_CHANGE_START
//Pin read: Early Boot Stall Not Supported
#ifndef SPX_BMC
                STATUS status = power_debug_is_asserted(&state->gpio_state, asserted);
                if (status != ST_OK) {
                    ASD_log(LogType_Error, "Failed to get state for CPU_PWR_DEBUG: %d", status);
                    return ST_ERR;
                }
                ASD_log(LogType_Debug, "Pin read: Early Boot Stall %s",
                        *asserted ? "asserted" : "deasserted");
#else
                ASD_log(LogType_Debug, "Pin read: Early Boot Stall Not Supported");
#endif
//AMI_CHANGE_END
                break;
            }
            case PIN_SYS_PWR_OK: {
                STATUS status = sys_pwr_ok_is_asserted(&state->gpio_state, asserted);
                if (status != ST_OK) {
                    ASD_log(LogType_Error, "Failed to get state for SYS_PWR_OK: %d", status);
                    return ST_ERR;
                }
                ASD_log(LogType_Debug, "Pin read: Sys Power OK %s",
                        *asserted ? "asserted" : "deasserted");
                break;
            }
            case PIN_PRDY: {  // PRDY
                bool pinState = false;
                STATUS status = prdy_is_asserted(&state->gpio_state, &pinState);
                if (status != ST_OK) {
                    ASD_log(LogType_Error, "get gpio data for CPU_PRDY failed: %d", status);
                    return ST_ERR;
                }
                *asserted = pinState;
                ASD_log(LogType_Debug, "Pin read: CPU_PRDY %s",
                        *asserted ? "asserted" : "deasserted");
                break;
            }
            default: {
                ASD_log(LogType_Error, "Pin status read: unsupported pin 0x%02x", pin);
                return ST_ERR;
            }
        }
    } else {
        ASD_log(LogType_Debug, "Pin read: unsupported read status register 0x%02x",
                statusRegister);
    }
    return ST_OK;
}

STATUS target_write_event_config(Target_Control_Handle* state, const WriteConfig event_cfg,
                                 const bool enable)
{
    STATUS status = ST_OK;
    int ret = pthread_mutex_lock(&state->write_config_mutex);
    if (ret != 0) {
        ASD_log(LogType_Error, "pthread_mutex_lock failed, returned: %d", ret);
        return ST_ERR;
    }
    switch (event_cfg) {
        case WRITE_CONFIG_BREAK_ALL: {
            ASD_log(LogType_Debug, "BREAK_ALL %s", enable ? "enabled" : "disabled");
            state->event_cfg.break_all = enable;
            break;
        }
        case WRITE_CONFIG_RESET_BREAK: {
            ASD_log(LogType_Debug, "RESET_BREAK %s", enable ? "enabled" : "disabled");
            state->event_cfg.reset_break = enable;
            break;
        }
        case WRITE_CONFIG_REPORT_PRDY: {
            ASD_log(LogType_Debug, "REPORT_PRDY %s", enable ? "enabled" : "disabled");
            if (state->event_cfg.report_PRDY != enable) {
                bool triggered = false;
                status = prdy_is_event_triggered(&state->gpio_state, &triggered);
                if (status != ST_OK) {
                    ASD_log(LogType_Error, "Failed to get gpio event status for CPU_PRDY: %d", status);
                    ret = pthread_mutex_unlock(&state->write_config_mutex);
                    if (ret != 0)
                        ASD_log(LogType_Error, "pthread_mutex_unlock failed, returned: %d", ret);
                    return status;
                } else if (triggered) {
                    ASD_log(LogType_Debug, "Prior to changing REPORT_PRDY, cleared outstanding PRDY event.");
                }
            }
            state->event_cfg.report_PRDY = enable;
            break;
        }
        case WRITE_CONFIG_REPORT_PLTRST: {
            ASD_log(LogType_Debug, "REPORT_PLTRST %s", enable ? "enabled" : "disabled");
            state->event_cfg.report_PLTRST = enable;
            break;
        }
        case WRITE_CONFIG_REPORT_MBP: {
            ASD_log(LogType_Debug, "REPORT_MBP %s", enable ? "enabled" : "disabled");
            state->event_cfg.report_MBP = enable;
            break;
        }
        default: {
            ASD_log(LogType_Error, "Invalid event config %d", event_cfg);
            status = ST_ERR;
        }
    }
    ret = pthread_mutex_unlock(&state->write_config_mutex);
    if (ret != 0) {
        ASD_log(LogType_Error, "pthread_mutex_unlock failed, returned: %d", ret);
        return ST_ERR;
    }
    return status;
}


STATUS target_wait_PRDY(Target_Control_Handle* state, const uint8_t log2time) {
    // The design for this calls for waiting for PRDY or until a timeout occurs.
    // The timeout is computed using the PRDY timeout setting (log2time) and
    // the JTAG TCLK.
    struct timeval start, end;
    long utime=0, seconds=0, useconds=0;
    int ret = 0;
    // The timeout for commands that wait for a PRDY pulse is defined by the period of
    // the JTAG clock multiplied by 2^log2time.
    int timeout = JTAG_CLOCK_CYCLE_MICROSEC * (1 << log2time);

    gettimeofday(&start, NULL);

    ret = pthread_mutex_lock(&state->write_config_mutex);
    if (ret != 0) {
        ASD_log(LogType_Error, "pthread_mutex_lock failed, returned: %d", ret);
        return ST_ERR;
    }
    bool detected = false;
    while(1) {
        STATUS status = prdy_is_event_triggered(&state->gpio_state, &detected);
        if (status != ST_OK) {
            ASD_log(LogType_Error, "Failed to get_gpio_event_status for CPU_PRDY: %d", status);
        } else if (detected) {
            break;
        }
        gettimeofday(&end, NULL);
        seconds  = end.tv_sec  - start.tv_sec;
        useconds = end.tv_usec - start.tv_usec;
        utime = ((seconds) * 1000000 + useconds) + 0.5;

        if (utime >= timeout || timeout <= 4) {
            // If the timeout is <= 4 microseconds, this is such a small period of time where it
            // becomes more reasonable to simply timeout and not sleep. Any sleep on most boards
            // will likely result in a thread context switch totaling far more than the desired
            // timeout value.
            break;
        } else {
            // It should be noted that in reality many boards will sleep longer than the 1ms
            // specified below.
            usleep(1000);
        }
    }
    ret = pthread_mutex_unlock(&state->write_config_mutex);
    if (ret != 0) {
        ASD_log(LogType_Error, "pthread_mutex_unlock failed, returned: %d", ret);
        return ST_ERR;
    }
    if(detected)
        ASD_log(LogType_Debug, "Wait PRDY complete, detected PRDY");
    else
        ASD_log(LogType_Debug, "Wait PRDY timed out after %f ms with timeout setting of %f ms.", (float)utime / 1000, (float)timeout/1000);
    return ST_OK;
}

STATUS checkXDPstate(Target_Control_Handle* state)
{
    STATUS result = ST_OK;
    if (state->gpio_state.gpios_config->xdp_present.is_present) {
        bool asserted = false;
        result = xdp_present_is_asserted(&state->gpio_state, &asserted);
        if (result != ST_OK) {
            ASD_log(LogType_Error, "get state failed for XDP_PRESENT pin: %d", result);
            return result;
        }

        if (!asserted) {
            // Probe is connected to the XDP and all GPIO's need to be set as
            //  inputs so that they do not interfere with the probe and the
            //  BMC JTAG interface needs to be set as a slave device
            if (state->event_cb && state->event_cb(XDP_PRESENT_EVENT, 0) != ST_OK) {
                ASD_log(LogType_Error, "Failed to send XDP Present event.");
            }
            return ST_ERR;
        }
    }
    return ST_OK;
}

STATUS target_jtag_chain_select(Target_Control_Handle* state, const uint8_t scan_chain) {
    STATUS status = ST_ERR;
    if (scan_chain == SCAN_CHAIN_0) {
        ASD_log(LogType_Debug, "Scan Chain Select: JTAG");
        status = tck_mux_select_assert(&state->gpio_state, GPIO_LOW);
    } else if (scan_chain == SCAN_CHAIN_1) {
        ASD_log(LogType_Debug, "Scan Chain Select: PCH");
        status = tck_mux_select_assert(&state->gpio_state, GPIO_HIGH);
    } else {
        ASD_log(LogType_Error, "Scan Chain Select: Unknown Scan Chain -> %d", scan_chain);
    }
    return status;
}
