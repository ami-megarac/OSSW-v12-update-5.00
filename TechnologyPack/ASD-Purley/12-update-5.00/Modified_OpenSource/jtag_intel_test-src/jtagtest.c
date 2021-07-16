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

#include <fcntl.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdarg.h>
#include <signal.h>

//AMI_CHANGE_START
//safe_lib library not available
#ifndef SPX_BMC
#include <safe_lib.h>
#include <sys/time.h>
#include <jtag_drv/jtag_drv.h>

#else
//replace safe_lib functions
#define __STDC_WANT_LIB_EXT1__ 1
#include <stdio.h>
#include <string.h>
#include <jtag_drv.h>
#include <getopt.h>
#include <sys/time.h>

#define memset_s(dest, dest_size, ch) \
    memset((dest), (ch), (dest_size))
#define memcpy_s(dest, dest_size, src, count) \
    memcpy((dest), (src), (count))
#define memcmp_s(ptr1, ptr1_size, ptr2, num, ptr_diff) \
    memcmp((ptr1), (ptr2), (num))
#ifndef UN_USED
#define UN_USED(x) (void)(x)
#endif
typedef unsigned long long uint64_t;
#define perror printf
#endif
//AMI_CHANGE_END
#ifndef Boolean
typedef enum { FALSE = 0,
               TRUE = 1 } Boolean;
#endif

typedef enum {
    JTAG_MASTER = 0,
    JTAG_SLAVE
} JTAGDriverState;

#ifndef timersub
#define timersub(a, b, result)                           \
    do {                                                 \
        (result)->tv_sec = (a)->tv_sec - (b)->tv_sec;    \
        (result)->tv_usec = (a)->tv_usec - (b)->tv_usec; \
        if ((result)->tv_usec < 0) {                     \
            --(result)->tv_sec;                          \
            (result)->tv_usec += 1000000;                \
        }                                                \
    } while (0)
#endif

// Global variables
static int jtag_driver_handle = -1;
static int continueLoop = TRUE;
unsigned int totalBits;
static unsigned int tap_state = JtagTLR;

// interrupt handler for ctrl-c
void intHandler(int dummy) {
    continueLoop = FALSE;
}

static Boolean jtag_driver_init(void) {
    if (jtag_driver_handle != -1) return TRUE;

    jtag_driver_handle = open("/dev/ast-jtag", O_RDWR);
    if (jtag_driver_handle == -1) {
        perror("Can't open /dev/ast-jtag, please install the driver!! \n");
        return FALSE;
    }
    return TRUE;
}

// Request the TAP to go to the target state
Boolean BMC_SetTapState(xfer_mode mode, unsigned int to_state) {
    if (jtag_driver_init() == FALSE) return FALSE;
    struct tap_state_param param = {mode, tap_state, to_state};

    if (ioctl(jtag_driver_handle, AST_JTAG_SET_TAPSTATE, &param) == -1) {
        perror("ioctl AST_JTAG_SET_TAPSTATE failed!\n");
        return FALSE;
    }
    tap_state = to_state;
    return TRUE;
}

// user level access to set the AST2500 JTAG controller in slave or master mode
Boolean jtag_set_cntlr_mode(xfer_mode mode, const JTAGDriverState controller_mode) {
    if ((controller_mode < JTAG_MASTER) || (controller_mode > JTAG_SLAVE)) {
        perror("An invalid JTAG controller state was used\n");
        return FALSE;
    }

    struct controller_mode_param param = {mode, controller_mode};

    if (ioctl(jtag_driver_handle, AST_JTAG_SLAVECONTLR, &param) < 0) {
        perror("ioctl AST_JTAG_SLAVECONTLR failed\n");
        return FALSE;
    }
    return TRUE;
}

Boolean jtag_set_tck_clock(xfer_mode mode, unsigned int tck)
{
    struct set_tck_param set_param = {mode, tck};
    struct get_tck_param get_param = {mode, 0};
    if (ioctl(jtag_driver_handle, AST_JTAG_SET_TCK, &set_param) < 0) {
        perror("ioctl AST_JTAG_SET_TCK failed\n");
        return FALSE;
    }

    if (ioctl(jtag_driver_handle, AST_JTAG_GET_TCK, &get_param) < 0) {
        perror("ioctl AST_JTAG_GET_TCK failed!\n");
        return FALSE;
    }

    if (tck != get_param.tck) {
        fprintf(stderr, "ioctl AST_JTAG_SET_TCK failed, %d!\n", get_param.tck);
        return FALSE;
    }
    return TRUE;
}

void shiftr(unsigned char *object, size_t size)
{
    int carry = 0;
    for (int i = size - 1; i >= 0; --i) {
        int next = (object[i] & 1) ? 0x80 : 0;
        object[i] = carry | (object[i] >> 1);
        carry = next;
    }
}

void printQ(unsigned int disablePrint, const char* format, ... ) {
    if (disablePrint) {
        return;
    }
    va_list args;
    va_start( args, format );
    vprintf( format, args );
    va_end( args );
}

void printBuffer(unsigned int qFlag, unsigned int num_bits, unsigned char* buffer) {
    printQ(qFlag,"[%db] 0x", num_bits);
    unsigned int num_bytes = (num_bits + 7) / 8;
    unsigned char remainder = num_bits % 8;
    for(int j=num_bytes; j>0; j--) {
        if ( j != num_bytes || remainder == 0)
            printQ(qFlag,"%02X", buffer[j-1]);
        else {
            printQ(qFlag,"%02X", (buffer[j-1] & remainder));
        }
    }
    printQ(qFlag,"\n");
}

//  Optionally write and read the requested number of bits and go to the requested target state
Boolean BMC_ReadWriteScan(xfer_mode mode, unsigned int numbits, unsigned int input_bytes, unsigned char *tap_input, unsigned int output_bytes, unsigned char *tap_output, unsigned int end_tapstate) {
    struct scan_xfer scan_xfer;

    totalBits += numbits;

    if (jtag_driver_init() == FALSE) return FALSE;

    scan_xfer.mode = mode;
    scan_xfer.tap_state = tap_state;
    scan_xfer.length = numbits;
    scan_xfer.tdi_bytes = input_bytes;
    scan_xfer.tdi = tap_input;
    scan_xfer.tdo_bytes = output_bytes;
    scan_xfer.tdo = tap_output;
    scan_xfer.end_tap_state = end_tapstate;

    if (ioctl(jtag_driver_handle, AST_JTAG_READWRITESCAN, &scan_xfer) == -1) {
        perror("ioctl AST_JTAG_READWRITESCAN failed!\n");
        return FALSE;
    }
    tap_state = end_tapstate;

    return TRUE;
}

void showUsage(char **argv, unsigned int qFlag) {
    printQ(qFlag,"Usage: %s [option]\n", argv[0]);
    printQ(qFlag,"  -q              Run quietly, no printing to the console\n");
    printQ(qFlag,"  -f              Run endlessly until ctrl-c is used\n");
    printQ(qFlag,"  -i <number>     Run [number] of iterations\n");
    printQ(qFlag,"  --dr-overshift=<hex value>\n");
    printQ(qFlag,"                  Specify 64bit overscan value. Default: 0xdeadbeefbad4f00d\n");
    printQ(qFlag,"  -h              Run in Hardware JTAG mode (default: Software)\n");
    printQ(qFlag,"  -t              JTAG tck speed (default:1)\n");  // This can be treated differently on systems as needed. It can be a divisor or actual frequency as needed.
    printQ(qFlag,"\n");
    printQ(qFlag,"  --ir-size= <hexadecimal bits>         Specify IR size manually\n");
    printQ(qFlag,"  --dr-size= <hexadecimal bits>         Specify DR size manually\n");
    printQ(qFlag,"  --ir-value= <hexadecimal value>       Specify IR command manually\n");
    printQ(qFlag,"  --help                                Show this list\n");
    printQ(qFlag,"\n");
}

int main (int argc, char **argv) {
    unsigned char shiftDataOut[2048]; // 16384 bits is probably more than needed for shifts
#ifndef SPX_BMC
    unsigned char compareData[32];
#endif
    unsigned char idcode[32];
    //unsigned char dead_beef[8] = {0x0d, 0xf0, 0xd4, 0xba, 0xef, 0xbe, 0xad, 0xde};  // Used for tap data comparison
    unsigned char dead_beef[8];  // Used for tap data comparison
    unsigned long long human_readable = 0xdeadbeefbad4f00d; // Used for tap data comparison
    uint64_t usec;
    unsigned int ir_shift_size = 11; // 11 bits per uncore
    unsigned int numUncores = 0;
    unsigned int writeSize = 0;
    unsigned long long ir_command = 0; // 8 bytes (enough to support 4 sockets)
    unsigned int shiftSize = 0;
    Boolean loop_forever = FALSE;
    unsigned int qFlag = 0;
    unsigned int numIterations = 1;
    unsigned int shift_size_in_bits = 0;
    unsigned int ir_value = 2; // overridden in manual mode
    unsigned int dr_shift_size = 32; // overridden in manual mode
    Boolean manual_mode = FALSE;
    unsigned int c = 0, i = 0, j = 0, loopCnt = 0;
    int indexDiff = 0;
    struct timeval tval_before, tval_after, tval_result;
    xfer_mode mode = SW_MODE;
    unsigned int tck = 1;

    signal(SIGINT, intHandler);  // catch ctrl-c

    enum { ARG_IR_SIZE=256, ARG_DR_SIZE, ARG_IR_VALUE, ARG_DR_OVERSHIFT,
           ARG_HELP };

    struct option opts[] = {
        { "ir-size",       1, NULL, ARG_IR_SIZE      },
        { "dr-size",       1, NULL, ARG_DR_SIZE      },
        { "ir-value",      1, NULL, ARG_IR_VALUE     },
        { "dr-overshift",  1, NULL, ARG_DR_OVERSHIFT },
        { "help",          0, NULL, ARG_HELP         },
        { NULL, 0, NULL, 0 },
    };

    opterr = 0;
    while ((c = getopt_long(argc, argv, "qfi:ht:?", opts, NULL)) != -1)
        switch (c) {
            case 'q':
                qFlag = 1;
                break;
            case 'f':
                loop_forever = TRUE;
                break;
            case 'i':
                if ((numIterations = atoi(optarg)) <= 0) {
                    showUsage(argv, qFlag);
                    return -1;
                }
                break;
            case 'h':
                mode = HW_MODE;
                break;
            case 't':
                tck = (unsigned int)atoi(optarg);
                break;

            case ARG_IR_SIZE:
                ir_shift_size = strtol(optarg, NULL, 16);
                break;

            case ARG_DR_SIZE:
                dr_shift_size = strtol(optarg, NULL, 16);
                manual_mode = TRUE;
                break;

            case ARG_IR_VALUE:
                ir_value = strtol(optarg, NULL, 16);
                manual_mode = TRUE;
                break;

            case ARG_DR_OVERSHIFT:
                human_readable = strtoull(optarg, NULL, 16);
                break;

            case '?':
            case ARG_HELP:
	    default:
                showUsage(argv, qFlag);
                return -1;
        }

    if (dr_shift_size > (sizeof(shiftDataOut) * 8)) {
        printQ(qFlag, "DR shift size cannot be larger than %d\n",
               sizeof(shiftDataOut) * 8);
        showUsage(argv, qFlag);
        return -1;
    }

    if (manual_mode) {
        printQ(qFlag, "IR Value = 0x%x\n", ir_value);
        printQ(qFlag, "IR shift size = 0x%x\n", ir_shift_size);
        printQ(qFlag, "DR shift size = 0x%x\n", dr_shift_size);
        manual_mode = TRUE;
    }

    memcpy_s(dead_beef, sizeof(dead_beef), &human_readable, sizeof(human_readable));

    // check that all the arguments have been processed and that the user didn't provide any we can't process
    if (optind < argc) {
        showUsage(argv, qFlag);
        return -1;
    }

    // load the driver
    if (jtag_driver_init() == FALSE) {
        printQ(qFlag,"Failed to initialize the driver!\n");
        return -1;
    }

    if (jtag_set_cntlr_mode(mode, JTAG_MASTER) == FALSE) {
        printQ(qFlag,"Failed to set ctrl mode to Master!\n");
        goto error;
    }

    if (mode == HW_MODE && jtag_set_tck_clock(mode, tck) == FALSE) {
        printQ(qFlag, "Failed to set tck clock!\n");
        goto error;
    }

    //*************************************************************************
    // Start automatic uncore discovery

    if (BMC_SetTapState(mode, JtagTLR) == FALSE) {
        printQ(qFlag,"Unable to set TLR tap state!\n");
        goto error;
    }

    if (BMC_SetTapState(mode, JtagRTI) == FALSE) {
        printQ(qFlag,"Unable to set RTI tap state!\n");
        goto error;
    }

    // determine idcodes in the tap
    if (BMC_SetTapState(mode, JtagShfDR) == FALSE) {
        printQ(qFlag,"Unable to set the tap state to 2!\n");
        goto error;
    }

    // Fill the buffer to be shifted out with 1s
    memset_s(shiftDataOut, sizeof(shiftDataOut), 0xff);

    shiftSize = 192; // bits
    // shift 192 bits including our known pattern so that this allows 4x32bit idcodes+known pattern
    if (BMC_ReadWriteScan(mode, shiftSize, sizeof(dead_beef),(unsigned char *)&dead_beef, sizeof(shiftDataOut), (unsigned char *)&shiftDataOut, JtagRTI) == FALSE) {
        printQ(qFlag,"Unable to read idcode!\n");
        goto error;
    }

    // this assumes that IDCODE is byte aligned since the JTAG spec says they're 32bits each.
    for (i = 0; i < (shiftSize/8); i++) {
        if (memcmp_s(&shiftDataOut[i], sizeof(shiftDataOut), (unsigned char *)&dead_beef, sizeof(dead_beef), &indexDiff) == 0 && indexDiff == 0) {
            shift_size_in_bits = i*8;
            printQ(qFlag,"\nFound TDI data on TDO after %d bits.\n", shift_size_in_bits);
            break;
        } else {
            shift_size_in_bits = 0;
        }
    }

    // check to see the shift_size_in_bits is not 0 and that we're on a power of two
    if ((shift_size_in_bits == 0) || ((shift_size_in_bits & (shift_size_in_bits - 1)) != 0)){
        printQ(qFlag,"TDI data was not seen on TDO.  Please ensure the target is on.\n");
        printQ(qFlag,"Here is the first %d bits of data seen on TDO that might help to debug the problem:\n", shiftSize);
        for (i = 0; i < (shiftSize/8); i += 8) {
            printQ(qFlag,"0x%x: ", i);
            for (j = 0; j < 8; ++j) {
                printQ(qFlag,"0x%02x ", shiftDataOut[i + j]);
            }
            printQ(qFlag,"\n");
        }
        printQ(qFlag,"\n");
        goto error;
    }

    // The number of uncores in the system will be the number of bits in the shiftDR / 32 bits
    numUncores = shift_size_in_bits/32;

    printQ(qFlag,"Found %d possible device%s\n",numUncores,numUncores == 1 ? "" : "s");

    for (i = 0; i < numUncores; i++) {
        printQ(qFlag,"Device %d: 0x", i);
        for (j = (4+(4*i)); j > i*4; j--) {
            printQ(qFlag,"%02x", shiftDataOut[j-1]);
        }
        printQ(qFlag,"\n");
    }

    printQ(qFlag,"Additional data shifted through the tap: 0x");
    for (i = (numUncores * 4)+8; i > (numUncores * 4); i--) {
        printQ(qFlag,"%02x", shiftDataOut[i-1]);
    }
    printQ(qFlag,"\n");

    // save the idcodes for later comparison
    memcpy_s(idcode, sizeof(idcode), shiftDataOut, 4*numUncores);

    // End automatic uncore discovery
    //*************************************************************************


    //*************************************************************************
    // Run test pattern

    // reset tap
    if (BMC_SetTapState(mode, JtagTLR) == FALSE) {
        printQ(qFlag,"Unable to set TLR tap state!\n");
        goto error;
    }

    if (BMC_SetTapState(mode, JtagRTI) == FALSE) {
        printQ(qFlag,"Unable to set RTI tap state!\n");
        goto error;
    }

    totalBits = 0;

    // set idcode IR for the numUncores found in the drShift
    ir_command = 0;
    for (i = 0; i < numUncores; i++) {
        ir_command = ((ir_command << ir_shift_size) | ir_value );
    }

    // Start of timer
    gettimeofday(&tval_before, NULL);

    // the main body of the iterations the user requested
    for (loopCnt = 0; loop_forever || loopCnt < numIterations; loopCnt++) {
        // Set the tap state to Select IR
        if (BMC_SetTapState(mode, JtagShfIR) == FALSE) {
            printQ(qFlag,"Unable to set the tap state to JtagShfIR!\n");
            goto error;
        }

        writeSize = ir_shift_size*numUncores;
        // ShiftIR and remember to set the end state to something OTHER than ShiftDR since we can't move from shiftIR to ShiftDR directly.
        if (BMC_ReadWriteScan(mode, writeSize, sizeof(ir_command), (unsigned char *)&ir_command, 0, NULL, JtagRTI) == FALSE) {
            printQ(qFlag,"Unable to write IR for idcode!\n");
            goto error;
        }

        if (BMC_SetTapState(mode, JtagShfDR) == FALSE) {
            printQ(qFlag,"Unable to set the tap state to JtagShfDR!\n");
            goto error;
        }

        memset_s(shiftDataOut, sizeof(shiftDataOut), 0x00);

        writeSize = (numUncores*dr_shift_size)+(sizeof(dead_beef)*8); // 32 bits per uncore idcode plus 64 bits for deadbeefbad4f00d
        if (BMC_ReadWriteScan(mode, writeSize, sizeof(dead_beef), (unsigned char *)&dead_beef, sizeof(shiftDataOut), (unsigned char *)&shiftDataOut, JtagRTI) == FALSE) {
            printQ(qFlag,"Unable to read shift data!\n");
            goto error;
        }

#ifndef SPX_BMC
        if(manual_mode == TRUE) {
            for(i=0; i<numUncores; i++) {
                printBuffer(qFlag, dr_shift_size, shiftDataOut);
                for(j=0; j<dr_shift_size; j++) {
                    shiftr(shiftDataOut, sizeof(shiftDataOut));
                }
            }

            // print what should be the dead_beef value
            printBuffer(qFlag, sizeof(dead_beef)*8, shiftDataOut);
        } else {
            memset_s(compareData, sizeof(compareData), 0x00);     // fill compareData with zeros
            memcpy_s(compareData, sizeof(compareData), idcode, 4*numUncores); // copy the idcode from tap reset and shiftdr into compareData
            memcpy_s(&compareData[(4*numUncores)], sizeof(compareData), &dead_beef, sizeof(dead_beef)); // copy deadbeefbad4f00d into compareData after idcode

            if (memcmp_s(compareData,((writeSize+7)/8),shiftDataOut,((writeSize+7)/8), &indexDiff) != 0 || indexDiff != 0) {
                printQ(qFlag,"TAP results comparison failed!\n");
                printQ(qFlag,"Shifted data: \n");
                for (i = 0; i < (writeSize+7)/8; i += 8) {
                    printQ(qFlag,"0x%x: ", i);
                    for (j = 0; j < 8; ++j) {
                        printQ(qFlag,"0x%02x ", shiftDataOut[i + j]);
                    }
                    printQ(qFlag,"\n");
                }
                printQ(qFlag,"\n");

                printQ(qFlag,"Expected data: \n");
                for (i = 0; i < (writeSize+7)/8; i += 8) {
                    printQ(qFlag,"0x%x: ", i);
                    for (j = 0; j < 8; ++j) {
                        printQ(qFlag,"0x%02x ", compareData[i + j]);
                    }
                    printQ(qFlag,"\n");
                }
                printQ(qFlag,"\n");
                goto error;
            }
        }
#endif
        if (continueLoop == FALSE)
            break;
    } // end iterations loop

    // end of timer
    gettimeofday(&tval_after, NULL);
    timersub(&tval_after, &tval_before, &tval_result);
    printQ(qFlag,"Total bits: %d ", totalBits);
    usec = ((uint64_t)tval_result.tv_sec*(uint64_t)1000000)+(uint64_t)tval_result.tv_usec;
    printQ(qFlag,"Time elapsed: %f seconds ", (float)usec/1000000);
    if ( usec != 0)
        printQ(qFlag,"At: %d bps (%f mbps)\n", ((uint64_t)1000000*totalBits)/usec, (float)(((uint64_t)1000000*totalBits)/usec)/1000000);
    else
        printQ(qFlag,"(measured zero time, so could not compute bandwidth) \n");

    printQ(qFlag,"Successfully finished %d iteration%s of idcode with 64 bits of overshifted data!\n", numIterations, numIterations >1?"s":"");

    if (jtag_set_cntlr_mode(mode, JTAG_SLAVE) == FALSE)
        printQ(qFlag, "Failed to set ctrl mode to Slave!\n");
    close(jtag_driver_handle);
    return 0;

error:
    if (jtag_set_cntlr_mode(mode, JTAG_SLAVE) == FALSE)
        printQ(qFlag, "Failed to set ctrl mode to Slave!\n");
    close(jtag_driver_handle);
    return -1;
}
