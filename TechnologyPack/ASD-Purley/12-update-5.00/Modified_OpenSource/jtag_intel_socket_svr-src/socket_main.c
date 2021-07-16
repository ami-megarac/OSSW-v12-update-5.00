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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/eventfd.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <stdbool.h>
//AMI_CHANGE_START
//safe_lib library not available
#ifndef SPX_BMC
#include <safe_lib.h>
#endif
//AMI_CHANGE_END
#include <poll.h>
#include <pthread.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <limits.h>

#include "asd_msg.h"
#include "JTAG_handler.h"
#include "target_handler.h"
#ifdef SPX_BMC
#include "i2c_handler.h"
#endif
#include "logging.h"
#include "ext_network.h"
#include "asd_common.h"
#include "session.h"
#include "authenticate.h"
#include "config.h"

#define EVENT_FD_INDEX 0
#define HOST_FD_INDEX 1
#define CLIENT_FD_INDEX 2
#define MAX_DATA_SIZE 4106
#define NUM_IN_FLIGHT_BUFFERS_TO_USE 20
//AMI_CHANGE_START
//changed default port and added process pid file
#ifndef SPX_BMC
#define DEFAULT_PORT 5123
#else
#define DEFAULT_PORT 5125
#define REMOTE_DEBUG_PID_FILE "/var/run/RemoteDebugServer.pid"
#define MAX_PID_NUMBER 4194304
#endif
//AMI_CHANGE_END

// Two simple rules for the version string are:
// 1. less than 265 in length (or it will be truncated in the plugin)
// 2. no dashes, as they are used later up the sw stack between components.
//AMI_CHANGE_START
//Added aux or point version
#ifndef SPX_BMC
static char asd_version[] = "ASD_BMC_v1.3";
#else
static char asd_version[] = "ASD_BMC_v1.3.3";
#endif
//AMI_CHANGE_END
typedef enum {
     CLOSE_CLIENT_EVENT = 1
} InternalEventTypes;

typedef enum {
     SOCKET_READ_STATE_INITIAL = 0,
     SOCKET_READ_STATE_HEADER,
     SOCKET_READ_STATE_BUFFER,
} SocketReadState;

static int host_fd = 0, event_fd = 0;
static uint8_t prdy_timeout = 1;

static struct spi_message out_msg;
char* send_buffer;
static pthread_mutex_t send_buffer_mutex;

static JTAG_Handler* jtag_handler = NULL;
static Target_Control_Handle* target_control_handle = NULL;
static I2C_Handler* i2c_handler = NULL;
static bool handlers_initialized = false;
static ClientAddrT sg_cl_addr = {0};

bool prnt_irdr = false, prnt_net = false,
     prnt_jtagMsg = false, prnt_Debug = false;

static struct {
    int n_port_number;
    char *cp_certkeyfile;
    char *cp_net_bind_device;
    extnet_hdlr_type_t e_extnet_type;
    auth_hdlr_type_t e_auth_type;
} sg_options = {DEFAULT_PORT, NULL, NULL, EXTNET_HDLR_NON_ENCRYPT, AUTH_HDLR_NONE};

static config sg_config;
static uint16_t sg_platform_id = DEFAULT_PLATFORM_ID;

bool shouldRemoteLog(ASD_LogType asd_level);
void sendRemoteLoggingMessage(ASD_LogType asd_level, const char* message);

void showUsage(char **argv) {
    fprintf(stderr, "Usage: %s [option(s)]\n", argv[0]);
    fprintf(stderr, "  -l <number>   Print verbose debug data:\n");
    fprintf(stderr, "     1          IR/DR transactions\n");
    fprintf(stderr, "     2          network transactions\n");
    fprintf(stderr, "     3          JTAG MSG from plugin\n");
    fprintf(stderr, "     4          All log types\n");
    fprintf(stderr, "     5          Debug log messages\n");
    fprintf(stderr, "  -p <number>   Port number (default=%d)\n", DEFAULT_PORT);
    fprintf(stderr, "  -s            Route log messages to the system log\n");
    fprintf(stderr, "  -k file       Specify SSL Certificate/Key file\n");
    fprintf(stderr, "  -n net_dev    Bind only to specific network device (eth0, etc)\n");
    fprintf(stderr, "  -i            hexadecimal platform ID (default=0x%02x)\n\n", DEFAULT_PLATFORM_ID);
}

//AMI_CHANGE_START
//added ASD version in to the file, which is used in response of oem command
#ifndef SPX_BMC
#define OPTIONS "l:p:k:n:si:"
#else
#define OPTIONS "l:p:k:n:si:u"
void updateVersion()
{
	int majorVer,minorVer,pointVer;
	FILE *fp;
	sscanf(asd_version,"ASD_BMC_v%d.%d.%d",&majorVer,&minorVer,&pointVer);
	if((fp=fopen("/conf/IntelVersion","w"))==NULL)
	{
		return;
	}
	fprintf(fp,"[Firmware]\n");
	fprintf(fp,"majorVer=%d\n",majorVer);
	fprintf(fp,"minorVer=%d\n",minorVer);
	fprintf(fp,"pointVer=%d\n",pointVer);
	fclose(fp);
	return;
	
}
#endif
//AMI_CHANGE_END
void process_command_line(int argc, char **argv) {
    int c = 0;
    ASD_LogType logType = LogType_None;
    bool b_usesyslog = false;
    while ((c = getopt(argc, argv, OPTIONS)) != -1) {
        switch (c) {
            case 'l': {
                int i = atoi(optarg);
                if (i < INT_MAX)
                    i++;
                if(i > LogType_MIN && i < LogType_MAX)
                    logType = (ASD_LogType)i;
                else
                    fprintf(stderr, "invalid log logType %d\n", i);

                break;
            }
            case 'p': {
                int port = atoi(optarg);
                fprintf(stderr, "Setting Port: %d\n", port);
                sg_options.n_port_number = port;
                break;
            }
            case 's': {
                b_usesyslog = true;
                break;
            }
            case 'k':
                sg_options.cp_certkeyfile = optarg;
                sg_options.e_extnet_type = EXTNET_HDLR_TLS;
                sg_options.e_auth_type = AUTH_HDLR_PAM;
                break;
            case 'n':
                sg_options.cp_net_bind_device = optarg;
                break;
            case 'i': {
                int platform_id = strtol(optarg, NULL, 16);
                sg_platform_id = (uint16_t)platform_id;
                fprintf(stderr, "Setting Platform ID: 0x%04hx\n", sg_platform_id);
                break;
            }
//AMI_CHANGE_START
//added ASD version in to the file, which is used in response of oem command
#ifdef SPX_BMC
            case 'u':
            {
            	updateVersion();
            	exit(EXIT_SUCCESS);
            }
#endif
//AMI_CHANGE_END
            default:  // h, ?, and other
            {
                showUsage(argv);
                exit(EXIT_SUCCESS);
            }
        }
    }
    ASD_initialize_log_settings(logType, b_usesyslog, &shouldRemoteLog, &sendRemoteLoggingMessage);
    if (optind < argc) {
        fprintf(stderr, "invalid non-option argument(s)\n");
        showUsage(argv);
        exit(EXIT_SUCCESS);
    }
}

void print_version() {
#ifndef ADTM_VERSION
    #define ADTM_VERSION __DATE__
#endif
    fprintf(stderr, "version: " ADTM_VERSION "\n");
}

void exitAll() {
    session_close_all();
    if (host_fd != 0)
        close(host_fd);
    if (out_msg.buffer)
        free(out_msg.buffer);
    if (send_buffer)
        free(send_buffer);
    if (jtag_handler) {
        free(jtag_handler);
        jtag_handler = NULL;
    }
    if (target_control_handle) {
        free(target_control_handle);
        target_control_handle = NULL;
    }
    if (i2c_handler) {
        free(i2c_handler);
        i2c_handler = NULL;
    }
}

void send_error_message(extnet_conn_t *p_extconn,
                        struct spi_message *input_message,
                        ASDError cmd_stat) {
    ssize_t cnt = 0;
    struct spi_message error_message = {{0}};  // initialize the struct

    if (p_extconn->sockfd < 0)
        return;

    memcpy_s(&error_message.header, sizeof(error_message.header),
             &(input_message->header), sizeof(struct message_header));

    error_message.header.cmd_stat = cmd_stat;
    cnt = extnet_send(p_extconn, &error_message.header, sizeof(struct message_header));
    if (cnt != sizeof(error_message.header)) {
        ASD_log(LogType_Error, "Failed to send an error message to client: %d", cnt);
    }
}

STATUS write_event_config(const uint8_t data_byte) {
    STATUS status = ST_OK;
    // If bit 7 is set, then we will be enabling an event, otherwise disabling
    bool enable = (data_byte >> 7) == 1;
    // Bits 0 through 6 are an unsigned int indicating the event config register
    uint8_t event_cfg_raw = data_byte & WRITE_CFG_MASK;
    if (event_cfg_raw > WRITE_CONFIG_MIN
        && event_cfg_raw < WRITE_CONFIG_MAX) {
        WriteConfig event_cfg = (WriteConfig)event_cfg_raw;
        status = target_write_event_config(target_control_handle, event_cfg, enable);
    } else {
        // We will not return an error for this, as unsupported may be okay
        ASD_log(LogType_Error, "Invalid or unsupported write event config register: "
                "0x%02x", event_cfg_raw);
    }
    return status;
}


STATUS write_cfg(const writeCfg cmd, struct packet_data *packet) {
    STATUS status = ST_OK;
    unsigned char *data;

    if (cmd == JTAG_FREQ) {
        // JTAG_FREQ (Index 1, Size: 1 Byte)
        //  Bit 7:6 – Prescale Value (b'00 – 1, b'01 – 2, b'10 – 4, b'11 – 8)
        //  Bit 5:0 – Divisor (1-64, 64 is expressed as value 0)
        //  The TAP clock frequency is determined by dividing the system clock
        //  of the TAP Master first through the prescale value (1,2,4,8) and
        //  then through the divisor (1-64).
        // e.g. system clock/(prescale * divisor)
        int prescaleVal = 0, divisorVal = 0, tCLK = 0;
        data = get_packet_data(packet, 1);
        if (data == NULL) {
            status = ST_ERR;
            ASD_log(LogType_Error, "Unable to read JTAG_FREQ data byte");
        } else {
            prescaleVal = data[0] >> 5;
            divisorVal = data[0] & 0x1f;

            if (prescaleVal == 0) {
                prescaleVal = 1;
            } else if (prescaleVal == 1) {
                prescaleVal = 2;
            } else if (prescaleVal == 2) {
                prescaleVal = 4;
            } else if (prescaleVal == 3) {
                prescaleVal = 8;
            } else {
                prescaleVal = 1;
            }

            if (divisorVal == 0) {
                divisorVal = 64;
            }

            tCLK = (prescaleVal*divisorVal);
            ASD_log(LogType_Debug, "Set JTAG TAP Pre: %d  Div: %d  TCK: %d", prescaleVal, divisorVal, tCLK);

            status = JTAG_set_jtag_tck(jtag_handler, tCLK);
            if (status != ST_OK)
                ASD_log(LogType_Error, "Unable to set the JTAG TAP TCK!");
        }
    } else if (cmd == DR_PREFIX) {
        // DR Postfix (A.K.A. DR Prefix in At-Scale Debug Arch. Spec.)
        // set drPaddingNearTDI 1 byte of data

        data = get_packet_data(packet, 1);
        if (data == NULL) {
            status = ST_ERR;
            ASD_log(LogType_Error, "Unable to read DRPost data byte");
        } else {
            ASD_log(LogType_IRDR, "Setting DRPost padding to %d", data[0]);
            status = JTAG_set_padding(jtag_handler, JTAGPaddingTypes_DRPost, data[0]);
            if (status != ST_OK)
                ASD_log(LogType_Error, "failed to set DRPost padding");
        }
    } else if (cmd == DR_POSTFIX) {
        // DR preFix (A.K.A. DR Postfix in At-Scale Debug Arch. Spec.)
        // drPaddingNearTDO 1 byte of data

        data = get_packet_data(packet, 1);
        if (data == NULL) {
            status = ST_ERR;
            ASD_log(LogType_Error, "Unable to read DRPre data byte");
        } else {
            ASD_log(LogType_IRDR, "Setting DRPre padding to %d", data[0]);
            status = JTAG_set_padding(jtag_handler, JTAGPaddingTypes_DRPre, data[0]);
            if (status != ST_OK)
                ASD_log(LogType_Error, "failed to set DRPre padding");
        }
    } else if (cmd == IR_PREFIX) {
        // IR Postfix (A.K.A. IR Prefix in At-Scale Debug Arch. Spec.)
        // irPaddingNearTDI 2 bytes of data

        data = get_packet_data(packet, 2);
        if (data == NULL) {
            status = ST_ERR;
            ASD_log(LogType_Error, "Unable to read IRPost data byte");
        } else {
            if (ASD_should_log(LogType_IRDR))
                ASD_log(LogType_IRDR, "Setting IRPost padding to %d", (data[1] << 8) | data[0]);
            status = JTAG_set_padding(jtag_handler, JTAGPaddingTypes_IRPost, (data[1] << 8) | data[0]);
            if (status != ST_OK)
                ASD_log(LogType_Error, "failed to set IRPost padding");
        }
    } else if (cmd == IR_POSTFIX) {
        // IR Prefix (A.K.A. IR Postfix in At-Scale Debug Arch. Spec.)
        // irPaddingNearTDO 2 bytes of data

        data = get_packet_data(packet, 2);
        if (data == NULL) {
            status = ST_ERR;
            ASD_log(LogType_Error, "Unable to read IRPre data byte");
        } else {
            if (ASD_should_log(LogType_IRDR))
                ASD_log(LogType_IRDR, "Setting IRPre padding to %d", (data[1] << 8) | data[0]);
            status = JTAG_set_padding(jtag_handler, JTAGPaddingTypes_IRPre, (data[1] << 8) | data[0]);
            if (status != ST_OK)
                ASD_log(LogType_Error, "failed to set IRPre padding");
        }
    } else if (cmd == PRDY_TIMEOUT) {
        // PRDY timeout
        // 1 bytes of data

        data = get_packet_data(packet, 1);
        if (data == NULL) {
            status = ST_ERR;
            ASD_log(LogType_Error, "Unable to read PRDY_TIMEOUT data byte");
        } else {
            ASD_log(LogType_Debug, "PRDY Timeout config set to %d", data[0]);
            prdy_timeout = data[0];
        }
    } else {
        ASD_log(LogType_Error, "ERROR: WriteCFG encountered unrecognized command (%d)", cmd);
        status = ST_ERR;
    }

    return status;
}

STATUS read_status(const ReadType index, uint8_t pin,
                   unsigned char *return_buffer,
                   const int return_buffer_size, int *bytes_written) {
    STATUS status = ST_OK;
    *bytes_written = 0;
    bool pinAsserted = false;
    if (return_buffer_size < 2) {
        ASD_log(LogType_Error, "Failed to process READ_STATUS. Response buffer already full");
        status = ST_ERR;
    }
    if (status == ST_OK) {
        status = target_read(target_control_handle, index, pin, &pinAsserted);
    }
    if (status == ST_OK) {
        return_buffer[(*bytes_written)++] = READ_STATUS_MIN + index;
        return_buffer[*bytes_written] = pin;
        if (pinAsserted)
            return_buffer[*bytes_written] |= 1 << 7;  // set 7th bit
        (*bytes_written)++;
    }
    return status;
}

STATUS send_out_msg_on_socket(struct spi_message *message) {
    int send_buffer_size = 0;
    int ret = 0;
    extnet_conn_t authd_conn;

    if (session_get_authenticated_conn(&authd_conn) != ST_OK || message == NULL) {
        return ST_ERR;
    }

    int cnt = 0;
    int size = get_message_size(message);
    if (size == -1) {
        ASD_log(LogType_Error | LogType_NoRemote, "Failed to send message because get message size failed.");
        return ST_ERR;
    }

    if (ASD_should_log(LogType_NETWORK)) {
        ASD_log(LogType_NETWORK | LogType_NoRemote,
                "Response header:  origin_id: 0x%02x\n"
                "    reserved: 0x%02x    enc_bit: 0x%02x\n"
                "    type: 0x%02x        size_lsb: 0x%02x\n"
                "    size_msb: 0x%02x    tag: 0x%02x\n"
                "    cmd_stat: 0x%02x",
                message->header.origin_id, message->header.reserved,
                message->header.enc_bit, message->header.type,
                message->header.size_lsb, message->header.size_msb,
                message->header.tag, message->header.cmd_stat);
        ASD_log(LogType_NETWORK | LogType_NoRemote, "Response Buffer size: %d", size);
        ASD_log_buffer(LogType_NETWORK | LogType_NoRemote, message->buffer, size, "NetRsp");
    }

    send_buffer_size = sizeof(struct message_header) + size;

    ret = pthread_mutex_lock(&send_buffer_mutex);
    if (ret != 0) {
        ASD_log(LogType_Error, "pthread_mutex_lock failed, returned: %d", ret);
        return ST_ERR;
    }

    memcpy_s(send_buffer, MAX_PACKET_SIZE,
             (unsigned char*)&message->header, sizeof(message->header));
    memcpy_s(send_buffer+sizeof(message->header), MAX_DATA_SIZE,
             message->buffer, size);
    cnt = extnet_send(&authd_conn, send_buffer, send_buffer_size);
    ret = pthread_mutex_unlock(&send_buffer_mutex);
    if (ret != 0) {
        ASD_log(LogType_Error, "pthread_mutex_unlock failed, returned: %d", ret);
        return ST_ERR;
    }
    if (cnt != send_buffer_size) {
        ASD_log(LogType_Error | LogType_NoRemote, "Failed to write message buffer to the socket: %d", cnt);
        return ST_ERR;
    }

    return ST_OK;
}

static void get_scan_length(const char cmd, uint8_t *num_of_bits, uint8_t *num_of_bytes) {
    *num_of_bits = (cmd & SCAN_LENGTH_MASK);
    *num_of_bytes = (*num_of_bits + 7)/8;

    if (*num_of_bits == 0) {
        // this is 64bits
        *num_of_bytes = 8;
        *num_of_bits = 64;
    }
}

typedef enum {
    ScanType_Read,
    ScanType_Write,
    ScanType_ReadWrite
} ScanType;

STATUS determine_shift_end_state(ScanType scan_type, struct packet_data *packet, JtagStates *end_state) {
    unsigned char *next_cmd_ptr = NULL;
    unsigned char *next_cmd2_ptr = NULL;
    STATUS status = ST_OK;

    if (end_state == NULL) {
        ASD_log(LogType_Error, "Cannot determine end state. Null end_state.");
        status = ST_ERR;
    } else {
        // First we will get the default end_state, to use if there are no more bytes to read from the packet.
        status = JTAG_get_tap_state(jtag_handler, end_state);
    }
    if (status == ST_OK) {
        // Peek ahead to get next command byte
        next_cmd_ptr = get_packet_data(packet, 1);
        if (next_cmd_ptr != NULL) {
            if (sg_config.jtag.mode == JTAG_DRIVER_MODE_HARDWARE) {
                // If in hardware mode, we must peek ahead 2 bytes in order to determine the end state
                next_cmd2_ptr = get_packet_data(packet, 1);
            }

            if (*next_cmd_ptr >= TAP_STATE_MIN &&
                *next_cmd_ptr <= TAP_STATE_MAX) {
                if (next_cmd2_ptr && ((*next_cmd2_ptr & TAP_STATE_MASK) == JtagPauDR || (*next_cmd2_ptr & TAP_STATE_MASK) == JtagPauIR)) {
                    ASD_log(LogType_IRDR, "Staying in state: 0x%02x", *end_state);
                } else {
                    // Next command is a tap state command
                    *end_state = (JtagStates)(*next_cmd_ptr & TAP_STATE_MASK);
                }
            } else if (scan_type == ScanType_Read && (*next_cmd_ptr < READ_SCAN_MIN || *next_cmd_ptr > READ_SCAN_MAX)) {
                ASD_log(LogType_Error, "Unexpected sequence during read scan: 0x%02x", *next_cmd_ptr);
                status = ST_ERR;
            } else if (scan_type == ScanType_Write && (*next_cmd_ptr < WRITE_SCAN_MIN || *next_cmd_ptr > WRITE_SCAN_MAX)) {
                ASD_log(LogType_Error, "Unexpected sequence during write scan: 0x%02x", *next_cmd_ptr);
                status = ST_ERR;
            } else if (scan_type == ScanType_ReadWrite && (*next_cmd_ptr < READ_WRITE_SCAN_MIN || *next_cmd_ptr > READ_WRITE_SCAN_MAX)) {
                ASD_log(LogType_Error, "Unexpected sequence during read write scan: 0x%02x", *next_cmd_ptr);
                status = ST_ERR;
            }

            packet->next_data--;  // Unpeek next_cmd_ptr
            packet->used--;
            if (next_cmd2_ptr) {
                packet->next_data--;  // Unpeek next_cmd2_ptr
                packet->used--;
            }
        }
    }
    return status;
}

STATUS process_jtag_message(struct spi_message *s_message) {
    int response_cnt = 0;
    JtagStates end_state;
    STATUS status = ST_OK;
    int size = get_message_size(s_message);
    struct packet_data packet;
    unsigned char *data_ptr;
    uint8_t cmd;

    if (size == -1) {
        ASD_log(LogType_Error, "Failed to process jtag message because "
               "get message size failed.");
        return ST_ERR;
    }

    memset_s(&out_msg.header, sizeof(struct message_header), 0);
    memset_s(out_msg.buffer, MAX_DATA_SIZE, 0);

    if (ASD_should_log(LogType_JTAG)) {
        ASD_log(LogType_JTAG, "NetReq tag: %d size: %d", s_message->header.tag, size);
        ASD_log_buffer(LogType_JTAG, s_message->buffer, size, "NetReq");
    }

    packet.next_data = s_message->buffer;
    packet.used = 0;
    packet.total = size;

    while (packet.used < packet.total) {
        data_ptr = get_packet_data(&packet, 1);
        if (data_ptr == NULL) {
            ASD_log(LogType_Error, "no command to read, short packet");
            status = ST_ERR;
            break;
        }

        cmd = *data_ptr;
        if (cmd == WRITE_EVENT_CONFIG) {
            data_ptr = get_packet_data(&packet, 1);
            if (data_ptr == NULL) {
                ASD_log(LogType_Error, "Failed to read data for WRITE_EVENT_CONFIG, short packet");
                status = ST_ERR;
                break;
            }

            status = write_event_config(*data_ptr);
            if(status != ST_OK) {
                ASD_log(LogType_Error, "write_event_config failed, %s", status);
                break;
            }
        } else if (cmd >= WRITE_CFG_MIN && cmd <= WRITE_CFG_MAX) {
            status = write_cfg((writeCfg)cmd, &packet);
            if(status != ST_OK) {
                ASD_log(LogType_Error, "write_cfg failed, %s", status);
                break;
            }
        } else if (cmd == WRITE_PINS) {
            data_ptr = get_packet_data(&packet, 1);
            if (data_ptr == NULL) {
                ASD_log(LogType_Error, "Failed to read data for WRITE_PINS, short packet");
                status = ST_ERR;
                break;
            }

            uint8_t data = *data_ptr;
            bool assert = (data >> 7) == 1;
            Pin pin = PIN_MIN;
            uint8_t index = data & WRITE_PIN_MASK;

            if (index > PIN_MIN && index < PIN_MAX) {
                pin = (Pin)index;
                status = target_write(target_control_handle, pin, assert);
                if(status != ST_OK) {
                    ASD_log(LogType_Error, "target_write failed, %s", status);
                    break;
                }
            } else if ((index & SCAN_CHAIN_SELECT) == SCAN_CHAIN_SELECT) {
                if (sg_config.jtag.chain_mode == JTAG_CHAIN_SELECT_MODE_SINGLE) {
                    uint8_t scan_chain = (index & SCAN_CHAIN_SELECT_MASK);
                    if (scan_chain >= MAX_SCAN_CHAINS) {
                        ASD_log(LogType_Error, "Unexpected scan chain: 0x%02x", scan_chain);
                        status = ST_ERR;
                        break;
                    }
                    status = target_jtag_chain_select(target_control_handle, (scanChain)scan_chain);
                    if(status != ST_OK) {
                        ASD_log(LogType_Error, "target_jtag_chain_select failed, %s", status);
                        break;
                    }
                    status = JTAG_set_active_chain(jtag_handler, (scanChain)scan_chain);
                    if(status != ST_OK) {
                        ASD_log(LogType_Error, "JTAG_set_active_chain failed, %s", status);
                        break;
                    }
                }
                else {
                    uint8_t chain_bytes_length = (index & SCAN_CHAIN_SELECT_MASK) + 1;
                    uint8_t chain_bytes[16];
                    // e.g.: chain_bytes_length = 1
                    // chain_bytes[0] = b'11111111 - Chain 0-7 are selected
                    // chain_bytes[1] = b'00000011 - Chain 8 (bit 0) and 9 (bit 1) are selected
                    // Notes: support up to 16 chain_bytes = 128 jtag chains

                    for (int i = 0; i < chain_bytes_length; i++) {
                        data_ptr = get_packet_data(&packet, 1);
                        if (data_ptr == NULL) {
                            ASD_log(LogType_Error, "Failed to read data for chain_bytes");
                            status = ST_ERR;
                            break;
                        }
                        chain_bytes[i] = *data_ptr;
                    }

                    if (ASD_should_log(LogType_Debug)) {
                        // Implementation specific code for multi-jtag chain select add here:
                        ASD_log(LogType_Debug, "Multi-jtag chain select command not yet implemented");

                        // e.g.: < Chain 21 > Received chain_bytes_length: 3
                        // Chain: 0x20 0000
                        char line[80];
                        uint8_t pos = 0;
                        memset_s(line, sizeof(line), '\0');
                        for (int i = chain_bytes_length-1; i >= 0; i--) {
                            snprintf(&line[pos], sizeof(line), "%02X", chain_bytes[i]);
                            pos = pos + 2;
                            if (i % 2 == 0 && i != 0) {
                                snprintf(&line[pos], sizeof(line), " ");
                                pos++;
                            }
                        }
                        ASD_log(LogType_Debug, "Chain : 0x%s", line);
                    }

                    if(status != ST_OK)
                        break;
                }
            } else {
                ASD_log(LogType_Error, "Unexpected WRITE_PINS index: 0x%02x", index);
                status = ST_ERR;
                break;
            }
        } else if (cmd >= READ_STATUS_MIN && cmd <= READ_STATUS_MAX) {
            int bytes_written = 0;
            ReadType readStatusTypeIndex;
            uint8_t index = (cmd & READ_STATUS_MASK);
            if (index > READ_TYPE_MIN && index < READ_TYPE_MAX)
                readStatusTypeIndex = (ReadType)index;
            else {
                ASD_log(LogType_Error,  "Unexpected READ_STATUS index: 0x%02x", index);
                status = ST_ERR;
                break;
            }

            data_ptr = get_packet_data(&packet, 1);
            if (data_ptr == NULL) {
                ASD_log(LogType_Error, "Failed to read data for Read Status, short packet");
                status = ST_ERR;
                break;
            }

            if (response_cnt+2 > MAX_DATA_SIZE) {
                ASD_log(LogType_Error, "Failed to process READ_STATUS. "
                        "Response buffer already full");
                status = ST_ERR;
                break;
            }

            uint8_t pin = (*data_ptr & READ_STATUS_PIN_MASK);
            status = read_status(readStatusTypeIndex, pin, &out_msg.buffer[response_cnt],
                                 MAX_DATA_SIZE-response_cnt, &bytes_written);
            if(status != ST_OK) {
                ASD_log(LogType_Error, "read_status failed, %d", status);
                break;
            }
            response_cnt += bytes_written;
        } else if (cmd == WAIT_CYCLES_TCK_DISABLE ||
                   cmd == WAIT_CYCLES_TCK_ENABLE) {

            data_ptr = get_packet_data(&packet, 1);
            if (data_ptr == NULL) {
                ASD_log(LogType_Error, "Failed to read data for WAIT_CYCLES_TCK, short packet");
                status = ST_ERR;
                break;
            }

            unsigned int number_of_cycles = *data_ptr;
            if (number_of_cycles==0)
                number_of_cycles = 256;
            status = JTAG_wait_cycles(jtag_handler, number_of_cycles);
            if(status != ST_OK) {
                ASD_log(LogType_Error, "JTAG_wait_cycles failed, %s", status);
                break;
            }
        } else if (cmd == WAIT_PRDY) {
            status = target_wait_PRDY(target_control_handle, prdy_timeout);
            if(status != ST_OK) {
                ASD_log(LogType_Error, "target_wait_PRDY failed, %s", status);
                break;
            }
        } else if (cmd == CLEAR_TIMEOUT) {
            // Command not yet implemented. This command does not apply to JTAG
            // so we will likely not implement it.
            ASD_log(LogType_Debug, "Clear Timeout command not yet implemented");
        } else if (cmd == TAP_RESET) {
            status = JTAG_tap_reset(jtag_handler);
            if(status != ST_OK) {
                ASD_log(LogType_Error, "JTAG_tap_reset failed, %s", status);
                break;
            }
        } else if (cmd >= TAP_STATE_MIN && cmd <= TAP_STATE_MAX) {
            status = JTAG_set_tap_state(jtag_handler, (JtagStates)(cmd & TAP_STATE_MASK));
            if(status != ST_OK) {
                ASD_log(LogType_Error, "JTAG_set_tap_state failed, %s", status);
                break;
            }
        } else if (cmd >= WRITE_SCAN_MIN && cmd <= WRITE_SCAN_MAX) {
            uint8_t num_of_bits = 0;
            uint8_t num_of_bytes = 0;

            get_scan_length(cmd, &num_of_bits, &num_of_bytes);
            data_ptr = get_packet_data(&packet, num_of_bytes);
            if (data_ptr == NULL) {
                ASD_log(LogType_Error, "Failed to read data from buffer: %d", num_of_bytes);
                status = ST_ERR;
                break;
            }

            status = determine_shift_end_state(ScanType_Write, &packet, &end_state);
            if(status != ST_OK) {
                ASD_log(LogType_Error, "determine_shift_end_state failed, %s", status);
                break;
            }
            status = JTAG_shift(jtag_handler, num_of_bits,
                                MAX_DATA_SIZE - packet.used - num_of_bytes,
                                data_ptr, 0, NULL, end_state);
            if(status != ST_OK) {
                ASD_log(LogType_Error, "JTAG_shift failed, %s", status);
                break;
            }
        } else if (cmd >= READ_SCAN_MIN && cmd <= READ_SCAN_MAX) {
            uint8_t num_of_bits = 0;
            uint8_t num_of_bytes = 0;

            get_scan_length(cmd, &num_of_bits, &num_of_bytes);
            if (response_cnt+sizeof(char)+num_of_bytes > MAX_DATA_SIZE) {
                ASD_log(LogType_Error, "Failed to process READ_SCAN. "
                        "Response buffer already full");
                status = ST_ERR;
                break;
            }
            out_msg.buffer[response_cnt++] = cmd;
            status = determine_shift_end_state(ScanType_Read, &packet, &end_state);
            if(status != ST_OK) {
                ASD_log(LogType_Error, "determine_shift_end_state failed, %s", status);
                break;
            }
            status = JTAG_shift(jtag_handler, num_of_bits, 0, NULL,
                                MAX_DATA_SIZE-response_cnt,
                                (unsigned char*)&(out_msg.buffer[response_cnt]),
                                end_state);
            if(status != ST_OK) {
                ASD_log(LogType_Error, "JTAG_shift failed, %s", status);
                break;
            }
            response_cnt += num_of_bytes;
        } else if (cmd >= READ_WRITE_SCAN_MIN && cmd <= READ_WRITE_SCAN_MAX) {
            uint8_t num_of_bits = 0;
            uint8_t num_of_bytes = 0;
            get_scan_length(cmd, &num_of_bits, &num_of_bytes);
            if (response_cnt+sizeof(char)+num_of_bytes > MAX_DATA_SIZE) {
                ASD_log(LogType_Error, "Failed to process READ_WRITE_SCAN. "
                        "Response buffer already full");
                status = ST_ERR;
                break;
            }
            out_msg.buffer[response_cnt++] = cmd;
            data_ptr = get_packet_data(&packet, num_of_bytes);
            if (data_ptr == NULL) {
                ASD_log(LogType_Error, "Failed to read data from buffer: %d", num_of_bytes);
                status = ST_ERR;
                break;
            }
            status = determine_shift_end_state(ScanType_ReadWrite, &packet, &end_state);
            if(status != ST_OK) {
                ASD_log(LogType_Error, "determine_shift_end_state failed, %s", status);
                break;
            }
            status = JTAG_shift(jtag_handler, num_of_bits,
                                MAX_DATA_SIZE - packet.used + num_of_bytes + 1,
                                data_ptr, MAX_DATA_SIZE-response_cnt,
                                (unsigned char*)&(out_msg.buffer[response_cnt]),
                                end_state);
            if(status != ST_OK) {
                ASD_log(LogType_Error, "JTAG_shift failed, %s", status);
                break;
            }
            response_cnt += num_of_bytes;
        } else {
            // Unknown Command
            ASD_log(LogType_Error, "Encountered unknown command 0x%02x", (int)cmd);
            status = ST_ERR;
            break;
        }
    }

    if (status == ST_OK) {
        memcpy_s(&out_msg.header, sizeof(out_msg.header),
                 &s_message->header, sizeof(struct message_header));

        out_msg.header.size_lsb = response_cnt & 0xFF;
        out_msg.header.size_msb = (response_cnt >> 8) & 0x1F;
        out_msg.header.cmd_stat = ASD_SUCCESS;

        status = send_out_msg_on_socket(&out_msg);
        if (status != ST_OK) {
            ASD_log(LogType_Error | LogType_NoRemote, "Failed to send message back on the socket");
        }
    } else {
        // Send error code to client
        extnet_conn_t authd_conn;
        if (session_get_authenticated_conn(&authd_conn) != ST_OK) {
            send_error_message(&authd_conn, s_message, ASD_UNKNOWN_ERROR);
        }
    }

    return status;
}

void process_message(extnet_conn_t *p_extcon, struct spi_message *s_message) {
    if (((s_message->header.type != JTAG_TYPE)
        && (s_message->header.type != I2C_TYPE))
        || (s_message->header.cmd_stat != 0 &&
            s_message->header.cmd_stat != 0x80 &&
            s_message->header.cmd_stat != 1)) {
        ASD_log(LogType_Error, "Received the message that is not supported by this host\n"
                "Message must be in the format:\n"
                "Type = 1 & cmd_stat=0 or 1 or 0x80\n"
                "I got Type=%x, cmd_stat=%x",
                s_message->header.type, s_message->header.cmd_stat);
        send_error_message(p_extcon, s_message, ASD_UNKNOWN_ERROR);
        return;
    }

    if (s_message->header.type == JTAG_TYPE) {
        if (process_jtag_message(s_message) != ST_OK) {
            ASD_log(LogType_Error, "Failed to process JTAG message");
            send_error_message(p_extcon, s_message, ASD_UNKNOWN_ERROR);
            return;
        }
    } else if (s_message->header.type == I2C_TYPE) {
        if (process_i2c_messages(i2c_handler, s_message, &out_msg, &send_out_msg_on_socket) != ST_OK) {
            ASD_log(LogType_Error, "Failed to process I2C message");
            send_error_message(p_extcon, s_message, ASD_UNKNOWN_ERROR);
            return;
        }
    }
}

IPC_LogType ipc_asd_log_map[LogType_MAX];

// This function maps the open ipc log levels to the levels
// we have already defined in this codebase.
void init_logging_map() {
    ipc_asd_log_map[(int)LogType_None] = IPC_LogType_Off;
    ipc_asd_log_map[(int)LogType_IRDR] = IPC_LogType_Trace;
    ipc_asd_log_map[(int)LogType_NETWORK] = IPC_LogType_Trace;
    ipc_asd_log_map[(int)LogType_JTAG] = IPC_LogType_Trace;
    // no log message should be logged as all.
    ipc_asd_log_map[(int)LogType_All] = IPC_LogType_Off;
    ipc_asd_log_map[(int)LogType_Debug] = IPC_LogType_Debug;
    ipc_asd_log_map[(int)LogType_Error] = IPC_LogType_Error;
}

bool shouldRemoteLog(ASD_LogType asd_level) {
    bool result = false;
    if (sg_config.logging.logging_level != IPC_LogType_Off) {
        if (sg_config.logging.logging_level <= ipc_asd_log_map[asd_level])
            result = true;
    }
    return result;
}

void sendRemoteLoggingMessage(ASD_LogType asd_level, const char* message) {
    if (shouldRemoteLog(asd_level)) {
        logging_configuration config_byte;
        config_byte.logging_level = ipc_asd_log_map[asd_level];
        config_byte.logging_stream = sg_config.logging.logging_stream;
        struct spi_message msg = {{0}};
        int message_length = strnlen_s(message, CALLBACK_LOG_MESSAGE_LENGTH);
        if (message_length > (MAX_DATA_SIZE-2))
            message_length = (MAX_DATA_SIZE-2); // minus 2 for the 2 prefixing bytes.
        int buffer_length = message_length + 2;
        msg.buffer = (unsigned char*)malloc(buffer_length);
        if (msg.buffer) {
            msg.header.type = HARDWARE_LOG_EVENT;
            msg.header.tag = 0;
            msg.header.origin_id = 0;
            msg.buffer[0] = AGENT_CONFIGURATION_CMD;
            msg.buffer[1] = config_byte.value;
            // Copy message into remaining buffer space.
            memcpy_s(&msg.buffer[2], message_length, message, message_length);
            // Store the message size into the msb and lsb fields. The size
            // is the length of the message string, plus 2 for the 2 prefix
            // bytes containing the stream and level bits and the cmd type.
            msg.header.size_lsb = buffer_length & 0xFF;
            msg.header.size_msb = (buffer_length >> 8) & 0x1F;
            msg.header.cmd_stat = ASD_SUCCESS;
            send_out_msg_on_socket(&msg);
            free(msg.buffer);
        }
    }
    return;
}

static STATUS sendPinEvent(ASD_EVENT value) {
    STATUS result = ST_OK;
    struct spi_message target_handler_msg = {{0}};
    target_handler_msg.buffer = (unsigned char*)malloc(1);
    if (!target_handler_msg.buffer) {
        ASD_log(LogType_Error, "Failed to allocate pin control event message buffer.");
        result = ST_ERR;
    } else {
        target_handler_msg.header.size_lsb = 1;
        target_handler_msg.header.size_msb = 0;
        target_handler_msg.header.type = JTAG_TYPE;
        target_handler_msg.header.tag = BROADCAST_MESSAGE_ORIGIN_ID;
        target_handler_msg.header.origin_id = BROADCAST_MESSAGE_ORIGIN_ID;
        target_handler_msg.buffer[0] = (value & 0xFF);

        if (send_out_msg_on_socket(&target_handler_msg) != ST_OK) {
            ASD_log(LogType_Error | LogType_NoRemote, "Failed to send pin control event to client.");
            result = ST_ERR;
        }
        free(target_handler_msg.buffer);
    }
    return result;
}

static STATUS TargetHandlerCallback(eventTypes event, ASD_EVENT value) {
    STATUS result = ST_OK;
    if (event == PIN_EVENT) {
        if (sendPinEvent(value) != ST_OK)
            result = ST_ERR;
    }
    else if (event == XDP_PRESENT_EVENT) {
        // send message to network listener thread which is polling for events
        ASD_log(LogType_Debug, "Disabling remote debug.");
        uint64_t event_value = CLOSE_CLIENT_EVENT;
        int i = write(event_fd, &event_value, sizeof(event_value));
        if (i != sizeof(event_value)) {
            ASD_log(LogType_Error, "Failed to send remote debug disable event.");
            result = ST_ERR;
        }
        ASD_log(LogType_Debug, "Disable remote debug event sent.");
    }
    else {
        ASD_log(LogType_Error, "invalid callback event type");
        result = ST_ERR;
    }
    return result;
}

STATUS on_client_connect(extnet_conn_t *p_extcon) {
    STATUS result = ST_OK;
    struct sockaddr_in6 addr;
    size_t addr_sz = sizeof(addr);

    ASD_log(LogType_Debug, "Preparing for client connection");

    if (!getpeername(p_extcon->sockfd, (struct sockaddr *)&addr, &addr_sz)) {
        int offset = sizeof(sg_cl_addr) - sizeof(addr.sin6_addr);

        if (offset < 0) {
            offset = 0;
        }
        memcpy_s(&sg_cl_addr[offset], sizeof(sg_cl_addr)-offset, &addr.sin6_addr, sizeof(addr.sin6_addr));
        ASD_log(LogType_Debug, "client connect "
                "%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x.%02x%02x",
                sg_cl_addr[0], sg_cl_addr[1], sg_cl_addr[2], sg_cl_addr[3],
                sg_cl_addr[4], sg_cl_addr[5], sg_cl_addr[6], sg_cl_addr[7],
                sg_cl_addr[8], sg_cl_addr[9], sg_cl_addr[10], sg_cl_addr[11],
                sg_cl_addr[12], sg_cl_addr[13], sg_cl_addr[14], sg_cl_addr[15]);
    }

    if (result == ST_OK && pthread_mutex_init(&send_buffer_mutex, NULL) != 0) {
        ASD_log(LogType_Error, "Failed to init send buffer mutex");
        result = ST_ERR;
    }

    set_config_defaults(sg_platform_id, &sg_config);

    return result;
}

STATUS on_client_disconnect() {
    STATUS result = ST_OK;
    int ret = 0;
    sg_config.logging.logging_level = IPC_LogType_Off;
    sg_config.logging.logging_stream = 0;
    ASD_log(LogType_Debug, "Cleaning up after client connection");

    ret = pthread_mutex_destroy(&send_buffer_mutex);
    if (ret != 0) {
        ASD_log(LogType_Error, "pthread_mutex_destroy failed, returned: %d", ret);
        result = ST_ERR;
    }

    // Deinitialize the JTAG control handler
    if (JTAG_deinitialize(jtag_handler) != ST_OK) {
        ASD_log(LogType_Error, "Failed to deinitialize the JTAG handler");
        result = ST_ERR;
    }

    // Deinitialize the pin control handler
    if (target_control_handle && target_deinitialize(target_control_handle) != ST_OK) {
        ASD_log(LogType_Error, "Failed to deinitialize the pin control handler");
        result = ST_ERR;
    }
    handlers_initialized = false;
    memset_s(&sg_cl_addr, sizeof(sg_cl_addr), 0);

    return result;
}

void on_message_received(extnet_conn_t *p_extconn,
                         struct spi_message message,
                         const int data_size) {
    if (message.header.enc_bit) {
        ASD_log(LogType_Error, "enc_bit found be we don't support it!");
        send_error_message(p_extconn, &message, ASD_UNKNOWN_ERROR);
        return;
    }
    if (message.header.type == AGENT_CONTROL_TYPE) {
        memcpy_s(&out_msg.header, sizeof(out_msg.header),
                 &message.header, sizeof(struct message_header));
        // The plugin stores the command in the cmd_stat parameter.
        // For the response, the plugin expects the same value in the first
        // byte of the buffer.
        out_msg.buffer[0] = message.header.cmd_stat;

        switch(message.header.cmd_stat) {
            case NUM_IN_FLIGHT_MESSAGES_SUPPORTED_CMD:
                out_msg.buffer[1] = NUM_IN_FLIGHT_BUFFERS_TO_USE;
                out_msg.header.size_lsb = 2;
                out_msg.header.size_msb = 0;
                out_msg.header.cmd_stat = ASD_SUCCESS;
                break;
            case MAX_DATA_SIZE_CMD:
                out_msg.buffer[1] = (MAX_DATA_SIZE) & 0xFF;
                out_msg.buffer[2] = (MAX_DATA_SIZE>>8) & 0xFF;
                out_msg.header.size_lsb = 3;
                out_msg.header.size_msb = 0;
                out_msg.header.cmd_stat = ASD_SUCCESS;
                break;
            case OBTAIN_DOWNSTREAM_VERSION_CMD:
                // The +/-1 references below are to account for the
                // write of cmd_stat to buffer position 0 above
                memcpy_s(&out_msg.buffer[1], MAX_DATA_SIZE-1,
                asd_version, sizeof(asd_version));
                out_msg.header.size_lsb = sizeof(asd_version)+1;
                out_msg.header.size_msb = 0;
                out_msg.header.cmd_stat = ASD_SUCCESS;
                break;
            case AGENT_CONFIGURATION_CMD:
            {
                // An agent configuration command was sent.
                // The next byte is the Agent Config type.
                if(data_size > 0) {
                    int config_type = message.buffer[0];
                    if (config_type == AGENT_CONFIG_TYPE_LOGGING) {
                        // We will store the logging stream only for the sake of sending
                        // it back to the IPC plugin. Technically the protocol is
                        // defined in such a way as where the BMC could emit log
                        // messages for several streams, but since we only have it
                        // implemented with one stream, we wont do much with this
                        // stored stream.
                        sg_config.logging.value = message.buffer[1];
                        ASD_log(LogType_Debug | LogType_NoRemote, "Remote logging command received. Stream: %d Level: %d",
                                sg_config.logging.logging_stream, sg_config.logging.logging_level);
                    } else if (config_type == AGENT_CONFIG_TYPE_GPIO) {
                        ASD_log(LogType_Debug, "Remote config command received. GPIO config: 0x%02x", (uint8_t)message.buffer[1]);
                        if(set_gpio_config(sg_platform_id, message.buffer[1], &sg_config)==ST_ERR) {
                            out_msg.header.size_lsb = 1;
                            out_msg.header.size_msb = 0;
                            out_msg.header.cmd_stat = ASD_UNKNOWN_ERROR;
                            break;
                        }
                        ASD_log(LogType_Debug, "GPIO debug_enable: assert_high=%d is_present=%d pin=%d",
                                sg_config.gpios.debug_enable.assert_high, sg_config.gpios.debug_enable.is_present, sg_config.gpios.debug_enable.pin);
                        ASD_log(LogType_Debug, "GPIO platform_reset: assert_high=%d is_present=%d pin=%d",
                                sg_config.gpios.platform_reset.assert_high, sg_config.gpios.platform_reset.is_present, sg_config.gpios.platform_reset.pin);
                        ASD_log(LogType_Debug, "GPIO power_debug: assert_high=%d is_present=%d pin=%d",
                                sg_config.gpios.power_debug.assert_high, sg_config.gpios.power_debug.is_present, sg_config.gpios.power_debug.pin);
                        ASD_log(LogType_Debug, "GPIO power_good: assert_high=%d is_present=%d pin=%d",
                                sg_config.gpios.power_good.assert_high, sg_config.gpios.power_good.is_present, sg_config.gpios.power_good.pin);
                        ASD_log(LogType_Debug, "GPIO prdy: assert_high=%d is_present=%d pin=%d",
                                sg_config.gpios.prdy.assert_high, sg_config.gpios.prdy.is_present, sg_config.gpios.prdy.pin);
                        ASD_log(LogType_Debug, "GPIO preq: assert_high=%d is_present=%d pin=%d",
                                sg_config.gpios.preq.assert_high, sg_config.gpios.preq.is_present, sg_config.gpios.preq.pin);
                        ASD_log(LogType_Debug, "GPIO rsm_reset: assert_high=%d is_present=%d pin=%d",
                                sg_config.gpios.rsm_reset.assert_high, sg_config.gpios.rsm_reset.is_present, sg_config.gpios.rsm_reset.pin);
                        ASD_log(LogType_Debug, "GPIO sys_pwr_ok: assert_high=%d is_present=%d pin=%d",
                                sg_config.gpios.sys_pwr_ok.assert_high, sg_config.gpios.sys_pwr_ok.is_present, sg_config.gpios.sys_pwr_ok.pin);
                        ASD_log(LogType_Debug, "GPIO tck_mux_select: assert_high=%d is_present=%d pin=%d",
                                sg_config.gpios.tck_mux_select.assert_high, sg_config.gpios.tck_mux_select.is_present, sg_config.gpios.tck_mux_select.pin);
                        ASD_log(LogType_Debug, "GPIO xdp_present: assert_high=%d is_present=%d pin=%d",
                                sg_config.gpios.xdp_present.assert_high, sg_config.gpios.xdp_present.is_present, sg_config.gpios.xdp_present.pin);
                    } else if (config_type == AGENT_CONFIG_TYPE_JTAG_SETTINGS) {
                        if((message.buffer[1] & JTAG_DRIVER_MODE_MASK) == JTAG_DRIVER_MODE_HARDWARE)
                            sg_config.jtag.mode = JTAG_DRIVER_MODE_HARDWARE;
                        else
                            sg_config.jtag.mode = JTAG_DRIVER_MODE_SOFTWARE;
                        if((message.buffer[1] & JTAG_CHAIN_SELECT_MODE_MASK) == JTAG_CHAIN_SELECT_MODE_MULTI)
                            sg_config.jtag.chain_mode = JTAG_CHAIN_SELECT_MODE_MULTI;
                        else
                            sg_config.jtag.chain_mode = JTAG_CHAIN_SELECT_MODE_SINGLE;
                        ASD_log(LogType_Debug, "Remote config command received. JTAG Driver Mode: %s - %s",
                                sg_config.jtag.mode == JTAG_DRIVER_MODE_SOFTWARE ? "Software" : "Hardware",
                                sg_config.jtag.chain_mode == JTAG_CHAIN_SELECT_MODE_SINGLE ? "Single" : "Multi");
                    }
                    out_msg.header.size_lsb = 1;
                    out_msg.header.size_msb = 0;
                    out_msg.header.cmd_stat = ASD_SUCCESS;
                }
                break;
            }
            default:
            {
                ASD_log(LogType_Debug, "Unsupported Agent Control command received %d", message.header.cmd_stat);
            }
        }
        if(send_out_msg_on_socket(&out_msg) != ST_OK) {
            ASD_log(LogType_Error | LogType_NoRemote, "Failed to send agent control message response.");
        }
    } else {
        if (!handlers_initialized) {
            STATUS result = ST_OK;
            // Initialize the pin control handler
            if ((result = target_initialize(target_control_handle, sg_cl_addr)) != ST_OK) {
                ASD_log(LogType_Error, "Failed to initialize the target_control_handle");
            }

            // Initialize the jtag handler
            if (result == ST_OK && (result = JTAG_initialize(jtag_handler, sg_config.jtag.mode == JTAG_DRIVER_MODE_SOFTWARE)) != ST_OK) {
                ASD_log(LogType_Error, "Failed to initialize the jtag_handler");
            }

            // Initialize the i2c handler
            if (result == ST_OK && (result = i2c_initialize(i2c_handler)) != ST_OK) {
                ASD_log(LogType_Error, "Failed to initialize the i2c_handler");
            }

            if (result == ST_OK)
                handlers_initialized = true;
            else {
                send_error_message(p_extconn, &message, ASD_UNKNOWN_ERROR);
                return;
            }
        }
        process_message(p_extconn, &message);
    }
}

//AMI_CHANGE_START
//creating pid file.
#ifdef SPX_BMC
void make_pid_file(void)
 {
     FILE *fp;
     fp = fopen(REMOTE_DEBUG_PID_FILE,"wb+");
     if(fp!=NULL){
         fprintf(fp,"%d",getpid());
         fclose(fp);
     }
 }
#endif

#define EXTNET_DATA sg_options.cp_certkeyfile

int main(int argc, char **argv) {
    int pollfd_cnt;
    struct pollfd poll_fds[CLIENT_FD_INDEX+MAX_SESSIONS] = {{0}};
    struct spi_message s_message = {{0}};
    int data_size = 0;
    SocketReadState read_state = SOCKET_READ_STATE_INITIAL;
    ssize_t read_index = 0;
    init_logging_map();

    set_config_defaults(sg_platform_id, &sg_config);

    print_version();
    process_command_line(argc, argv);

//AMI_CHANGE_START
//creating pid file.
#ifdef SPX_BMC
    make_pid_file();
#endif
//AMI_CHANGE_END
    jtag_handler = JTAGHandler();
    if (!jtag_handler) {
        ASD_log(LogType_Error, "Failed to create JTAGHandler");
        exitAll();
        return 1;
    }

    target_control_handle = TargetHandler(&TargetHandlerCallback, &sg_config.gpios);
    if (!target_control_handle) {
        ASD_log(LogType_Error, "Failed to create TargetHandler");
        exitAll();
        return 1;
    }

    i2c_handler = I2CHandler(&sg_config.i2c);
    if (!i2c_handler) {
        ASD_log(LogType_Error, "Failed to create I2CHandler");
        exitAll();
        return 1;
    }

    extnet_init(sg_options.e_extnet_type, EXTNET_DATA, MAX_SESSIONS);
    auth_init(sg_options.e_auth_type, NULL);
    session_init();

    out_msg.buffer = (unsigned char*) malloc(MAX_DATA_SIZE);
    if (!out_msg.buffer) {
        ASD_log(LogType_Error, "Failed to allocate out_msg.buffer");
        exitAll();
        return 1;
    }

    s_message.buffer = (unsigned char*) malloc(MAX_DATA_SIZE);
    if (!s_message.buffer) {
        ASD_log(LogType_Error, "Failed to allocate s_message.buffer");
        exitAll();
        return 1;
    }

    send_buffer = (char *) malloc(MAX_PACKET_SIZE);
    if (!send_buffer) {
        ASD_log(LogType_Error, "Failed to create message buffer for socket write.");
        if(s_message.buffer)
            free(s_message.buffer);
        exitAll();
        return 1;
    }

    event_fd = eventfd(0, O_NONBLOCK);
    if (event_fd == -1) {
        ASD_log(LogType_Error, "Could not setup event file descriptor.");
        if (s_message.buffer)
            free(s_message.buffer);
        exitAll();
        return 1;
    }

    if (extnet_open_external_socket(sg_options.cp_net_bind_device, sg_options.n_port_number, &host_fd) != ST_OK) {
        ASD_log(LogType_Error, "Could not open the external socket");
        if(s_message.buffer)
            free(s_message.buffer);
        exitAll();
        return 1;
    }
    poll_fds[EVENT_FD_INDEX].fd = event_fd;
    poll_fds[EVENT_FD_INDEX].events = POLLIN;
    poll_fds[HOST_FD_INDEX].fd = host_fd;
    poll_fds[HOST_FD_INDEX].events = POLLIN;

    while (1) {
        session_fdarr_t session_fds = {-1};
        int n_clients = 0, i;
        int n_timeout = -1;

        if (session_getfds(session_fds, &n_clients, &n_timeout) != ST_OK) {
            ASD_log(LogType_Debug, "Cannot get client session fds!");
            if (s_message.buffer)
                free(s_message.buffer);
            exitAll();
            return 1;
        }
        for (i=0; i<n_clients; i++) {
            poll_fds[CLIENT_FD_INDEX+i].fd = session_fds[i];
            poll_fds[CLIENT_FD_INDEX+i].events = POLLIN;
        }
        pollfd_cnt = CLIENT_FD_INDEX + n_clients;
        if (poll(poll_fds, pollfd_cnt, n_timeout) == -1) {
            break;
        }

        if (poll_fds[EVENT_FD_INDEX].revents & POLLIN) {
            ASD_log(LogType_Debug, "Internal event received.");
            uint64_t value;
            int i = read(event_fd, &value, sizeof(value));
            if (i != sizeof(value)) {
                ASD_log(LogType_Error, "Error occurred receiving internal event.");
            } else {
                if (value == CLOSE_CLIENT_EVENT) {
                    extnet_conn_t authd_conn;

                    ASD_log(LogType_Debug, "Internal client close event received.");
                    if (session_get_authenticated_conn(&authd_conn) != ST_OK) {
                        ASD_log(LogType_Error, "Authorized client already disconnected.");
                    } else {
                        ASD_log(LogType_Error, "Remote JTAG disabled, disconnecting client.");
                        if (on_client_disconnect() != ST_OK) {
                            ASD_log(LogType_Error, "Client disconnect cleanup failed.");
                        }
                        session_close(&authd_conn);
                        continue;
                    }
                }
            }
        }
        if (poll_fds[HOST_FD_INDEX].revents & POLLIN) {
            ASD_log(LogType_Debug, "Client Connecting.");
            extnet_conn_t new_extconn;

            if (extnet_accept_connection(host_fd, &new_extconn) != ST_OK) {
                ASD_log(LogType_Error, "Failed to accept incoming connection.");
                continue;
            }
            // Create a session for the new connection
            if (session_open(&new_extconn, SOCKET_READ_STATE_INITIAL) != ST_OK) {
                ASD_log(LogType_Error,
                        "Unable to add session for new connection fd %d",
                        new_extconn.sockfd);
                extnet_close_client(&new_extconn);
                continue;
            }
            if (sg_options.e_auth_type == AUTH_HDLR_NONE) {
                /* Special case where auth is not required. Stuff fd to the
                 * poll_fds array to immediately process the connection.
                 * Otherwise it may be timed out as unathenticated. */
                if (CLIENT_FD_INDEX+n_clients < sizeof(poll_fds)/sizeof(poll_fds[0])) {
                    poll_fds[CLIENT_FD_INDEX+n_clients].fd = new_extconn.sockfd;
                    poll_fds[CLIENT_FD_INDEX+n_clients].revents |= POLLIN;
                    n_clients++;
                }
            }
        }

        session_close_expired_unauth();
        for (i=0; i<n_clients; i++) {
            bool b_data_pending = false;
            extnet_conn_t *p_extconn;
            p_extconn = session_lookup_conn(poll_fds[CLIENT_FD_INDEX+i].fd);
            if (!p_extconn) {
                ASD_log(LogType_Error, "Session for fd %d vanished!",
                        poll_fds[CLIENT_FD_INDEX+i].fd);
                continue;
            }

            if( session_get_data_pending(p_extconn, &b_data_pending) != ST_OK) {
                ASD_log(LogType_Error, "Cannot get session data pending for fd %d!",
                        poll_fds[CLIENT_FD_INDEX+i].fd);
                continue;
            }

            if (b_data_pending || poll_fds[CLIENT_FD_INDEX+i].revents & POLLIN) {
                if (session_already_authenticated(p_extconn) != ST_OK) {
                    // Authenticate the client
                    if (auth_client_handshake(p_extconn) != ST_OK) {
                        session_close(p_extconn);
                    } else if (session_auth_complete(p_extconn) != ST_OK) {
                        session_close(p_extconn);
                    } else {
                        ASD_log(LogType_Debug,
                                "Session on fd %d now authenticated",
                                p_extconn->sockfd);

                        if (on_client_connect(p_extconn) != ST_OK) {
                            ASD_log(LogType_Error, "Connection attempt failed.");
                            if (on_client_disconnect() != ST_OK) {
                                ASD_log(LogType_Error, "Client disconnect cleanup failed.");
                            }
                            session_close(p_extconn);
                            continue;
                        }
                        ASD_log(LogType_Error,
                                "Authenticated client connected on fd %d.",
                                p_extconn->sockfd);
                    }
                    continue;
                }

                if (session_get_data(p_extconn, &read_state) != ST_OK) {
                    ASD_log(LogType_Error, "Cannot get session data for fd %d!",
                            p_extconn->sockfd);
                    session_close(p_extconn);
                    continue;
                }

                switch (read_state) {
                    case SOCKET_READ_STATE_INITIAL: {
                        memset_s(&s_message.header, sizeof(struct message_header), 0);
                        memset_s(s_message.buffer, MAX_DATA_SIZE, 0);
                        read_state = SOCKET_READ_STATE_HEADER;
                        session_set_data(p_extconn, read_state);
                        read_index = 0;
                        // do not 'break' here, continue on and read the header
                    }
                    case SOCKET_READ_STATE_HEADER: {
                        ssize_t cnt = extnet_recv(p_extconn,
                                           (void*)(&(s_message.header)+read_index),
                                           sizeof(s_message.header)-read_index,
                                           &b_data_pending);
                        if (cnt < 1) {
                            if(cnt == 0)
                                ASD_log(LogType_Error, "Client disconnected");
                            else
                                ASD_log(LogType_Error, "Socket header receive failed: %d", cnt);
                            if (on_client_disconnect() != ST_OK) {
                                ASD_log(LogType_Error, "Client disconnect cleanup failed.");
                            }
                            session_close(p_extconn);
                        } else if ((cnt + read_index) == sizeof(s_message.header)) {
                            data_size = get_message_size(&s_message);
                            if (data_size == -1) {
                                ASD_log(LogType_Error, "Failed to read header size.");
                                send_error_message(p_extconn, &s_message, ASD_UNKNOWN_ERROR);
                                if (on_client_disconnect() != ST_OK) {
                                    ASD_log(LogType_Error, "Client disconnect cleanup failed.");
                                }
                                session_close(p_extconn);
                            } else if (data_size > 0) {
                                read_state = SOCKET_READ_STATE_BUFFER;
                                session_set_data(p_extconn, read_state);
                                read_index = 0;
                            } else {
                                // we have finished reading a message and there is no buffer to read.
                                // Set back to initial state for next packet and process message.
                                read_state = SOCKET_READ_STATE_INITIAL;
                                session_set_data(p_extconn, read_state);
                                on_message_received(p_extconn, s_message, data_size);
                            }
                        } else {
                            read_index += cnt;
                            ASD_log(LogType_Debug, "Socket header read not complete (%d of %d)",
                                    read_index, sizeof(s_message.header));
                        }
                        break;
                    }
                    case SOCKET_READ_STATE_BUFFER: {
                        ssize_t cnt = extnet_recv(p_extconn, (void*)(s_message.buffer+read_index),
                                           data_size-read_index, &b_data_pending);
                        if (cnt < 1) {
                            if(cnt == 0)
                                ASD_log(LogType_Error, "Client disconnected");
                            else
                                ASD_log(LogType_Error, "Socket buffer receive failed: %d", cnt);
                            send_error_message(p_extconn, &s_message, ASD_UNKNOWN_ERROR);
                            if (on_client_disconnect() != ST_OK) {
                                ASD_log(LogType_Error, "Client disconnect cleanup failed.");
                            }
                            session_close(p_extconn);
                        } else if ((cnt + read_index) == data_size) {
                            // we have finished reading a packet. Set back to initial state for next packet.
                            read_state = SOCKET_READ_STATE_INITIAL;
                            session_set_data(p_extconn, read_state);
                            on_message_received(p_extconn, s_message, data_size);
                        } else {
                            read_index += cnt;
                            ASD_log(LogType_Debug, "Socket buffer read not complete (%d of %d)",
                                    read_index, data_size);
                        }
                        break;
                    }
                    default:
                    {
                        ASD_log(LogType_Error, "Invalid socket read state: %d", read_state);
                    }
                }
                session_set_data_pending(p_extconn, b_data_pending);
            }
        }
    }
    if(s_message.buffer)
        free(s_message.buffer);
    exitAll();
    ASD_log(LogType_Error, "ASD server closing.");
    return 0;
}
