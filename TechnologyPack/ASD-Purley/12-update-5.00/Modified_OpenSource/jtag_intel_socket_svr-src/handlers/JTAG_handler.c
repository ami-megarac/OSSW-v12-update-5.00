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

#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#include "JTAG_handler.h"
#include "asd_common.h"
#include "logging.h"
#include <sys/mman.h>
//AMI_CHANGE_START
//safe_lib library not available, functions replaced in asd_common.h
#ifndef SPX_BMC
#include <safe_lib.h>
#endif
//AMI_CHANGE_END

static const char *JtagStatesString[] = {
    "TLR", "RTI", "SelDR", "CapDR", "ShfDR", "Ex1DR", "PauDR", "Ex2DR",
    "UpdDR", "SelIR", "CapIR", "ShfIR", "Ex1IR", "PauIR", "Ex2IR", "UpdIR"
};

STATUS JTAG_set_cntlr_mode(JTAG_Handler* state, const JTAGDriverState setMode);
STATUS JTAG_clock_cycle(int handle, unsigned char tms, unsigned char tdi);
STATUS perform_shift(JTAG_Handler* state, unsigned int number_of_bits,
                     unsigned int input_bytes, unsigned char* input,
                     unsigned int output_bytes, unsigned char* output,
                     JtagStates current_tap_state, JtagStates end_tap_state);


void initialize_jtag_chains(JTAG_Handler* state) {
//AMI_CHANGE_START
//declare before use
    int i;
    for (i=0; i<MAX_SCAN_CHAINS; i++) {
//AMI_CHANGE_END
        state->chains[i].shift_padding.drPre = 0;
        state->chains[i].shift_padding.drPost = 0;
        state->chains[i].shift_padding.irPre = 0;
        state->chains[i].shift_padding.irPost = 0;
        state->chains[i].tap_state = JtagTLR;
        state->chains[i].scan_state = JTAGScanState_Done;
    }
}

JTAG_Handler* JTAGHandler()
{
    JTAG_Handler* state = (JTAG_Handler*)malloc(sizeof(JTAG_Handler));
    if (state == NULL) {
        return NULL;
    }

    state->active_chain = &state->chains[SCAN_CHAIN_0];
    initialize_jtag_chains(state);
    state->sw_mode = true;
    memset_s(state->padDataOne, sizeof(state->padDataOne), ~0);
    memset_s(state->padDataZero, sizeof(state->padDataZero), 0);

    state->JTAG_driver_handle = open("/dev/ast-jtag", O_RDWR);
    if (state->JTAG_driver_handle == -1) {
        ASD_log(LogType_Error, "Can't open /dev/ast-jtag, please install driver");
        free(state);
        return NULL;
    }

    if (JTAG_set_cntlr_mode(state, JTAGDriverState_Slave) != ST_OK) {
        ASD_log(LogType_Error, "Failed to set JTAG mode to slave.");
        close(state->JTAG_driver_handle);
        free(state);
        return NULL;
    }
    return state;
}

STATUS JTAG_clock_cycle(int handle, unsigned char tms, unsigned char tdi)
{
    struct tck_bitbang bitbang = {0};

    bitbang.tms = tms;
    bitbang.tdi = tdi;

    if (ioctl(handle, AST_JTAG_BITBANG, &bitbang) < 0) {
        ASD_log(LogType_Error, "ioctl AST_JTAG_BITBANG failed");
        return ST_ERR;
    }
    return ST_OK;
}

// user level access to set the AST2500 JTAG controller in slave or master mode
STATUS JTAG_set_cntlr_mode(JTAG_Handler* state, const JTAGDriverState setMode)
{
    struct controller_mode_param params = {0};
    params.mode = state->sw_mode ? SW_MODE : HW_MODE;

    if ((setMode < JTAGDriverState_Master) || (setMode > JTAGDriverState_Slave)) {
        ASD_log(LogType_Error, "An invalid JTAG controller state was used");
        return ST_ERR;
    }

    params.controller_mode = setMode;

    ASD_log(LogType_Debug, "Setting JTAG controller mode to %s.",
            setMode == JTAGDriverState_Master ? "MASTER" : "SLAVE");
    if (ioctl(state->JTAG_driver_handle, AST_JTAG_SLAVECONTLR, &params) < 0) {
        ASD_log(LogType_Error, "ioctl AST_JTAG_SLAVECONTLR failed");
        return ST_ERR;
    }
    return ST_OK;
}

STATUS JTAG_initialize(JTAG_Handler* state, bool sw_mode)
{
    if (state == NULL)
        return ST_ERR;

    state->sw_mode = sw_mode;

    if (JTAG_set_cntlr_mode(state, JTAGDriverState_Master) != ST_OK) {
        ASD_log(LogType_Error, "Failed to set JTAG mode to master.");
        return ST_ERR;
    }

    initialize_jtag_chains(state);
    return ST_OK;
}

STATUS JTAG_deinitialize(JTAG_Handler* state)
{
    if (state == NULL)
        return ST_ERR;

    STATUS result = JTAG_set_cntlr_mode(state, JTAGDriverState_Slave);
    if (result != ST_OK) {
        ASD_log(LogType_Error, "Failed to set JTAG mode to slave.");
    }
    return result;
}

STATUS JTAG_set_padding(JTAG_Handler* state, const JTAGPaddingTypes padding, const int value)
{
    if (state == NULL || value > MAXPADSIZE)
        return ST_ERR;

    if (padding == JTAGPaddingTypes_DRPre) {
        state->active_chain->shift_padding.drPre = value;
    } else if (padding == JTAGPaddingTypes_DRPost) {
        state->active_chain->shift_padding.drPost = value;
    } else if (padding == JTAGPaddingTypes_IRPre) {
        state->active_chain->shift_padding.irPre = value;
    } else if (padding == JTAGPaddingTypes_IRPost) {
        state->active_chain->shift_padding.irPost = value;
    } else {
        ASD_log(LogType_Error, "Unknown padding value: %d", value);
        return ST_ERR;
    }
    return ST_OK;
}

//
// Reset the Tap and wait in idle state
//
STATUS JTAG_tap_reset(JTAG_Handler* state)
{
    if (state == NULL)
        return ST_ERR;
    return JTAG_set_tap_state(state, JtagTLR);
}

//
// Request the TAP to go to the target state
//
STATUS JTAG_set_tap_state(JTAG_Handler* state, JtagStates tap_state)
{
    if (state == NULL)
        return ST_ERR;

    struct tap_state_param params = {0};
    params.mode = state->sw_mode ? SW_MODE : HW_MODE;
    params.from_state = state->active_chain->tap_state;
    params.to_state = tap_state;

    if (ioctl(state->JTAG_driver_handle, AST_JTAG_SET_TAPSTATE, &params) < 0) {
        ASD_log(LogType_Error, "ioctl AST_JTAG_SET_TAPSTATE failed");
        return ST_ERR;
    }

    state->active_chain->tap_state = tap_state;

    if ((tap_state == JtagRTI) || (tap_state == JtagPauDR))
        if (JTAG_wait_cycles(state, 5) != ST_OK)
            return ST_ERR;

    if (ASD_should_log(LogType_IRDR)) {
        ASD_log(LogType_IRDR, "Goto state: %s (%d)",
                tap_state >= (sizeof(JtagStatesString) / sizeof(JtagStatesString[0])) ? "Unknown" : JtagStatesString[tap_state],
                tap_state);
    }
    return ST_OK;
}

//
// Retrieve the current the TAP state
//
STATUS JTAG_get_tap_state(JTAG_Handler* state, JtagStates* tap_state)
{
    if (state == NULL || tap_state == NULL)
        return ST_ERR;
    *tap_state = state->active_chain->tap_state;
    return ST_OK;
}

//
//  Optionally write and read the requested number of
//  bits and go to the requested target state
//
STATUS JTAG_shift(JTAG_Handler* state, unsigned int number_of_bits,
                  unsigned int input_bytes, unsigned char* input,
                  unsigned int output_bytes, unsigned char* output,
                  JtagStates end_tap_state)
{
    if (state == NULL)
        return ST_ERR;

    unsigned int preFix = 0;
    unsigned int postFix = 0;
    unsigned char* padData = state->padDataOne;
    JtagStates current_state;
    JTAG_get_tap_state(state, &current_state);

    if (current_state == JtagShfIR) {
        preFix = state->active_chain->shift_padding.irPre;
        postFix = state->active_chain->shift_padding.irPost;
        padData = state->padDataOne;
    } else if (current_state == JtagShfDR) {
        preFix = state->active_chain->shift_padding.drPre;
        postFix = state->active_chain->shift_padding.drPost;
        padData = state->padDataZero;
    } else {
        ASD_log(LogType_Error, "Shift called but the tap is not in a ShiftIR/DR tap state");
        return ST_ERR;
    }

    if (state->active_chain->scan_state == JTAGScanState_Done) {
        state->active_chain->scan_state = JTAGScanState_Run;
        if (preFix) {
            if (perform_shift(state, preFix, MAXPADSIZE, padData,
                              0, NULL, current_state, current_state) != ST_OK)
                return ST_ERR;
        }
    }

    if ((postFix) && (current_state != end_tap_state)) {
        state->active_chain->scan_state = JTAGScanState_Done;
        if (perform_shift(state, number_of_bits, input_bytes, input,
                          output_bytes, output, current_state, current_state) != ST_OK)
            return ST_ERR;
        if (perform_shift(state, postFix, MAXPADSIZE, padData, 0,
                          NULL, current_state, end_tap_state) != ST_OK)
            return ST_ERR;
    } else {
        if (perform_shift(state, number_of_bits, input_bytes, input,
                          output_bytes, output, current_state, end_tap_state) != ST_OK)
            return ST_ERR;
        if (current_state != end_tap_state) {
            state->active_chain->scan_state = JTAGScanState_Done;
        }
    }
    return ST_OK;
}

//
//  Optionally write and read the requested number of
//  bits and go to the requested target state
//
STATUS perform_shift(JTAG_Handler* state, unsigned int number_of_bits,
                     unsigned int input_bytes, unsigned char* input,
                     unsigned int output_bytes, unsigned char* output,
                     JtagStates current_tap_state, JtagStates end_tap_state)
{
    struct scan_xfer scan_xfer = {0};
    scan_xfer.mode = state->sw_mode ? SW_MODE : HW_MODE;
    scan_xfer.tap_state = current_tap_state;
    scan_xfer.length = number_of_bits;
    scan_xfer.tdi_bytes = input_bytes;
    scan_xfer.tdi = input;
    scan_xfer.tdo_bytes = output_bytes;
    scan_xfer.tdo = output;
    scan_xfer.end_tap_state = end_tap_state;

    if (ioctl(state->JTAG_driver_handle, AST_JTAG_READWRITESCAN, &scan_xfer) < 0) {
        ASD_log(LogType_Error, "ioctl AST_JTAG_READWRITESCAN failed!");
        return ST_ERR;
    }
    state->active_chain->tap_state = end_tap_state;
    if (ASD_should_log(LogType_IRDR)) {
        if (input != NULL)
            ASD_log_shift((current_tap_state == JtagShfDR), true, number_of_bits, input_bytes, input);
        if (output != NULL)
            ASD_log_shift((current_tap_state == JtagShfDR), false, number_of_bits, output_bytes, output);
    }

    return ST_OK;
}

//
// Wait for the requested cycles.
//
// Note: It is the responsibility of the caller to make sure that
// this call is made from RTI, PauDR, PauIR states only. Otherwise
// will have side effects !!!
//
STATUS JTAG_wait_cycles(JTAG_Handler* state, unsigned int number_of_cycles)
{
//AMI_CHANGE_START
//declare before use
	unsigned int i;
    if (state == NULL)
        return ST_ERR;
    if (state->sw_mode) {
        for (i = 0; i < number_of_cycles; i++) {
//AMI_CHANGE_END
            if (JTAG_clock_cycle(state->JTAG_driver_handle, 0, 0) != ST_OK)
                return ST_ERR;
        }
    }
    return ST_OK;
}

STATUS JTAG_set_jtag_tck(JTAG_Handler* state, unsigned int tck)
{
    if (state == NULL)
        return ST_ERR;

    struct set_tck_param params = {0};
    params.mode = state->sw_mode ? SW_MODE : HW_MODE;
    params.tck = tck;

    if (ioctl(state->JTAG_driver_handle, AST_JTAG_SET_TCK, &params) < 0) {
        ASD_log(LogType_Error, "ioctl AST_JTAG_SET_TCK failed");
        return ST_ERR;
    }
    return ST_OK;
}

STATUS JTAG_set_active_chain(JTAG_Handler* state, scanChain chain) {
    if (state == NULL)
        return ST_ERR;

    if (chain < 0 || chain >= MAX_SCAN_CHAINS) {
        ASD_log(LogType_Error, "Invalid scan chain.");
        return ST_ERR;
    }

    state->active_chain = &state->chains[chain];
    return ST_OK;
}
