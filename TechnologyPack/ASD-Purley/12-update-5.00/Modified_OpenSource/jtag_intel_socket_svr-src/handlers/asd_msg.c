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

//AMI_CHANGE_START
//safe_lib library not available
#ifndef SPX_BMC
#include <safe_lib.h>
#endif
//AMI_CHANGE_END
#include "asd_msg.h"
#include "logging.h"
#include "i2c_msg_builder.h"


STATUS do_bus_select_command(I2C_Handler *i2c_handler, struct packet_data *packet);
STATUS do_set_sclk_command(I2C_Handler *i2c_handler, struct packet_data *packet);
STATUS do_read_command(uint8_t cmd, I2C_Msg_Builder *builder, struct packet_data *packet, bool *force_stop);
STATUS do_write_command(uint8_t cmd, I2C_Msg_Builder *builder, struct packet_data *packet, bool *force_stop);
STATUS build_responses(struct spi_message *out_msg, int* response_cnt,
                       I2C_Msg_Builder *builder, bool ack, SendFunctionPtr send_ptr);

void *get_packet_data(struct packet_data *packet, int bytes_wanted) {
    void *p;

    if (packet == NULL || packet->used + bytes_wanted > packet->total)
        return NULL;

    p = packet->next_data;

    packet->next_data += bytes_wanted;
    packet->used += bytes_wanted;

    return p;
}

int get_message_size(struct spi_message *s_message) {
    int result = (s_message->header.size_lsb & 0xFF) |
        ((s_message->header.size_msb & 0x1F) << 8);
    if (result > MAX_DATA_SIZE)
        result = -1;
    return result;
}

STATUS process_i2c_messages(I2C_Handler* i2c_handler, struct spi_message* in_msg,
                            struct spi_message* out_msg, SendFunctionPtr send_ptr) {
    int response_cnt = 0;
    STATUS status;
    int size;
    struct packet_data packet;
    unsigned char *data_ptr;
    uint8_t cmd;
    bool i2c_command_pending = false;
    bool force_stop = false;
    I2C_Msg_Builder* builder = I2CMsgBuilder();

    if (!i2c_handler || !in_msg || !out_msg || !send_ptr) {
        ASD_log(LogType_Error, "Invalid process_i2c_messages input.");
        status = ST_ERR;
    } else {
        size = get_message_size(in_msg);
        if (size == -1) {
            ASD_log(LogType_Error, "Failed to process i2c message because "
                "get message size failed.");
            status = ST_ERR;
        } else {
            memset_s(&out_msg->header, sizeof(struct message_header), 0);
            memset_s(out_msg->buffer, MAX_DATA_SIZE, 0);
            memcpy_s(&out_msg->header, sizeof(out_msg->header),
                     &in_msg->header, sizeof(struct message_header));

            status = i2c_msg_initialize(builder);
            if (status != ST_OK) {
                ASD_log(LogType_Error, "Failed to initialize i2c msg builder.");
            } else {
                ASD_log(LogType_JTAG, "NetReq tag: %d size: %d", in_msg->header.tag, size);
                ASD_log_buffer(LogType_JTAG, in_msg->buffer, (size_t)size, "NetReq");

                packet.next_data = in_msg->buffer;
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
                    if (cmd == I2C_WRITE_CFG_BUS_SELECT) {
                        status = do_bus_select_command(i2c_handler, &packet);
                    }
                    else if (cmd == I2C_WRITE_CFG_SCLK) {
                        status = do_set_sclk_command(i2c_handler, &packet);
                    } else if (cmd >= I2C_READ_MIN && cmd <= I2C_READ_MAX) {
                        status = do_read_command(cmd, builder, &packet, &force_stop);
                        i2c_command_pending = true;
                    }
                    else if (cmd >= I2C_WRITE_MIN && cmd <= I2C_WRITE_MAX) {
                        status = do_write_command(cmd, builder, &packet, &force_stop);
                        i2c_command_pending = true;
                    } else {
                        // Unknown Command
                        ASD_log(LogType_Error, "Encountered unknown i2c command 0x%02x", (int)cmd);
                        status = ST_ERR;
                        break;
                    }

                    if (status != ST_OK)
                        break;

                    // if a i2c command is pending and either
                    // we are done looping through all commands,
                    // or the force stop bit was set,
                    // then we need to also flush the commands
                    if (i2c_command_pending && ((packet.used == packet.total) || force_stop)) {
                        i2c_command_pending = false;
                        force_stop = false;
                        status = i2c_read_write(i2c_handler, builder->msg_set);
                        if (status != ST_OK) {
                            ASD_log(LogType_Debug, "i2c_read_write failed, %d, assuming NAK", status);
                        }
                        status = build_responses(out_msg, &response_cnt, builder, (status == ST_OK), send_ptr);
                        if (status != ST_OK) {
                            ASD_log(LogType_Error, "i2c_read_write failed to parse i2c responses, %d", status);
                            break;
                        }
                        status = i2c_msg_reset(builder);
                        if (status != ST_OK) {
                            ASD_log(LogType_Error, "i2c_msg_reset failed to reset response builder, %d", status);
                            break;
                        }
                    }
                }
            }
        }
    }

    if (status == ST_OK) {
        out_msg->header.size_lsb = (uint32_t) (response_cnt & 0xFF);
        out_msg->header.size_msb = (uint32_t) ((response_cnt >> 8) & 0x1F);
        out_msg->header.cmd_stat = ASD_SUCCESS;
        if (send_ptr(out_msg) != ST_OK) {
            ASD_log(LogType_Error | LogType_NoRemote, "Failed to send message back on the socket");
        }
    }
    if (builder) {
        i2c_msg_deinitialize(builder);
        free(builder);
    }
    return status;
}

STATUS do_bus_select_command(I2C_Handler *i2c_handler, struct packet_data *packet) {
    STATUS status;
    unsigned char *data_ptr = get_packet_data(packet, 1);
    if (data_ptr == NULL) {
        ASD_log(LogType_Error, "Failed to read data for I2C_WRITE_CFG_BUS_SELECT, short packet");
        status = ST_ERR;
    } else {
        status = i2c_bus_select(i2c_handler, (uint8_t)*data_ptr);
        if(status != ST_OK) {
            ASD_log(LogType_Error, "i2c_bus_select failed, %d", status);
        }
    }
    return status;
}

STATUS do_set_sclk_command(I2C_Handler *i2c_handler, struct packet_data *packet) {
    STATUS status;
    unsigned char *data_ptr = get_packet_data(packet, 2);
    if (data_ptr == NULL) {
        ASD_log(LogType_Error, "Failed to read data for I2C_WRITE_CFG_SCLK, short packet");
        status = ST_ERR;
    } else {
        uint8_t lsb = *data_ptr;
        uint8_t msb = *(data_ptr+1);
        status = i2c_set_sclk(i2c_handler, (uint16_t) ((msb<<8) + lsb));
        if (status != ST_OK) {
            ASD_log(LogType_Error, "i2c_set_sclk failed, %d", status);
        }
    }
    return status;
}

STATUS do_read_command(uint8_t cmd, I2C_Msg_Builder *builder, struct packet_data *packet, bool *force_stop) {
    STATUS status;
    asd_i2c_msg msg;
    unsigned char *data_ptr;
    *force_stop = false;
    msg.length = (uint8_t) (cmd & I2C_LENGTH_MASK);
    msg.read = true;
    //ASD_log(LogType_Debug, "i2c read, length: %d", msg.length);

    data_ptr = get_packet_data(packet, 1);
    if (data_ptr == NULL) {
        ASD_log(LogType_Error, "Failed to read data for I2C_READ, short packet");
        status = ST_ERR;
    } else {
        msg.address = (uint8_t) (*data_ptr & I2C_ADDRESS_MASK) >> 1;
        *force_stop = ((*data_ptr & I2C_FORCE_STOP_MASK) == I2C_FORCE_STOP_MASK);

        status = i2c_msg_add(builder, &msg);
        if (status != ST_OK) {
            ASD_log(LogType_Error, "i2c_msg_add failed, %d", status);
        }
    }
    return status;
}

STATUS do_write_command(uint8_t cmd, I2C_Msg_Builder *builder, struct packet_data *packet, bool *force_stop) {
    STATUS status;
    asd_i2c_msg msg;
    unsigned char *data_ptr;
    *force_stop = false;
    msg.length = (uint8_t) (cmd & I2C_LENGTH_MASK);
    msg.read = false;
    //ASD_log(LogType_Debug, "i2c write, length: %d", msg.length);

    data_ptr = get_packet_data(packet, 1);
    if (data_ptr == NULL) {
        ASD_log(LogType_Error, "Failed to read data for I2C_WRITE address, short packet");
        status = ST_ERR;
    } else {
        msg.address = (uint8_t) (*data_ptr & I2C_ADDRESS_MASK) >> 1;
        *force_stop = ((*data_ptr & I2C_FORCE_STOP_MASK) == I2C_FORCE_STOP_MASK);
        data_ptr = get_packet_data(packet, msg.length);
        if (data_ptr == NULL) {
            ASD_log(LogType_Error, "Failed to read data for I2C_WRITE buffer, short packet");
            status = ST_ERR;
        } else {
            for (int i = 0; i < msg.length; i++) {
                msg.buffer[i] = data_ptr[i];
            }
            status = i2c_msg_add(builder, &msg);
            if (status != ST_OK) {
                ASD_log(LogType_Error, "i2c_msg_add failed, %d", status);
            }
        }
    }
    return status;
}

STATUS build_responses(struct spi_message *out_msg, int* response_cnt,
                       I2C_Msg_Builder *builder, bool ack, SendFunctionPtr send_ptr) {
    STATUS status;
    asd_i2c_msg msg;
    u_int32_t num_i2c_messages = 0;
    status = i2c_msg_get_count(builder, &num_i2c_messages);

    if (status == ST_OK) {
        for (int i=0; i<num_i2c_messages; i++) {
            status = i2c_msg_get_asd_i2c_msg(builder, i, &msg);
            if (status != ST_OK)
                break;

            if((msg.length + (*response_cnt) + 2) > MAX_DATA_SIZE) { // 2 for command header and ack bytes
                // buffer is too full for this next packet
                // send response on socket with packet continuation bit set.
                out_msg->header.size_lsb = (uint32_t) ((*response_cnt) & 0xFF);
                out_msg->header.size_msb = (uint32_t) (((*response_cnt) >> 8) & 0x1F);
                out_msg->header.cmd_stat = ASD_SUCCESS | ASD_PACKET_CONTINUATION;
                status = send_ptr(out_msg);
                if (status != ST_OK) {
                    ASD_log(LogType_Error | LogType_NoRemote, "Failed to send message back on the socket");
                    break;
                }
                memset_s(out_msg->buffer, MAX_DATA_SIZE, 0);
                (*response_cnt) = 0;
            }

            uint8_t cmd_byte = (uint8_t) (msg.read ? I2C_READ_MIN : I2C_WRITE_MIN) + msg.length;
            out_msg->buffer[(*response_cnt)++] = cmd_byte;
            // Linux driver does not return the address or read ACKs, but Open IPC and the Aurora
            // SDK do. So we will just fake it here.
            if (ack)
                out_msg->buffer[(*response_cnt)++] = (unsigned char) (I2C_ADDRESS_ACK + msg.length);
            else
                out_msg->buffer[(*response_cnt)++] = (unsigned char) (msg.length);

            if (msg.read) {
                for (int j = 0; j < msg.length; j++) {
                    out_msg->buffer[(*response_cnt)++] = msg.buffer[j];  // data
                }
            }
        }
    }

    return ST_OK;
}
