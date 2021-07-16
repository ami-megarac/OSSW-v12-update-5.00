/*
// Copyright (C) 2018 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions
// and limitations under the License.
//
//
// SPDX-License-Identifier: Apache-2.0
*/

//
// Common error returns
//

#ifndef _COMMSTAT_H_
#define _COMMSTAT_H_

#define COMMON_BASE 0x00000000         // Base of our return codes
#define ASIC_BASE 0x00000400           // Base of all ASIC return codes
#define SYSEVENT_BASE 0x00000800       // Base of system event return codes
#define BOOT_BASE 0x00000b00           // Base of boot block codes
#define SENSOR_BASE 0x00000c00         // Base of sensor codes
#define SESSION_BASE 0x00001000        // Base of session codes
#define CONNECT_BASE 0x00001400        // Base of Channel ConnectTo method return codes
#define ALERT_BASE 0x00001800          // Base of Channel Alert method return codes
#define FBM_BASE 0x00003000            // base for FBM codes.
#define VIRTUAL_FLASH_BASE 0x00003100  // Base for Virtual Flash error codes
#define XML_CONFIG_BASE 0x00003200     // Base for XML config error codes

//
// STATUS enumeration is to be predominantly used for error type
// status codes.  It is not to be used for returning general state
// or status information from APIs.  Only error codes.
//
// Enumerate our error returns

typedef enum {
    // Common status base

    ST_OK = COMMON_BASE,       // 00h - Function completed
    ST_ILLEGAL_IO_CLASS,       // 01h - Class out of range
    ST_ILLEGAL_IO_INSTANCE,    // 02h - Instance out of range
    ST_NO_FUNCTION,            // 03h - Requested function not supplied
    ST_INVALID_GLOBAL_EVENT,   // 04h - Unsupported global event
    ST_NO_RESOURCE,            // 05h - Requested resource unavailable
    ST_OS_ERROR,               // 06h - Got a ThreadX OS error
    ST_IO_DONE,                // 07h - IO is done
    ST_ENTRY_NOTFOUND,         // 08h - Couldn't find entry
    ST_DRIVER_ABORT,           // 09h - Driver is aborting
    ST_IOCTL_INVALID,          // 0ah - Invalid IOCTL
    ST_DEVICE_UNAVAILABLE,     // 0bh - Device not available
    ST_IOCTL_BAD_PARAM,        // 0ch - Invalid IOCTL parameter
    ST_IOCTL_UNIMPLEMENTED,    // 0dh - Unimplemented IOCTL
    ST_HW_FAILURE,             // 0eh - It's a hardware failure!
    ST_IOCTL_BAD_CONTEXT,      // 0fh - Can't do this in this context
    ST_NO_DATA,                // 10h - No more data!
    ST_INVALID_PARAM,          // 11h - Invalid parameter
    ST_UART_BUSY,              // 12h - UART Can't be released!
    ST_STORAGE_UNINIT,         // 13h - Storage uninitialized
    ST_STORAGE_LIMIT,          // 14h - Reading off end of storage
    ST_STORAGE_PARM,           // 15h - Invalid storage parameter
    ST_STORAGE_BAD_AREA,       // 16h - Invalid storage area
    ST_STORAGE_AREA_LIMIT,     // 17h - Out of storage area range
    ST_STORAGE_INVALID,        // 18h - Invalid operation for work area
    ST_BUFFER_SIZE,            // 19h - Invalid buffer size
    ST_IPMB_ERROR,             // 1ah - IPMB command timed out
    ST_INIT_FAILURE,           // 1bh - object failed initialization
    ST_DEFER_INIT,             // 1ch - object can't be initialized now
    ST_DEFER_READ,             // 1dh - object can't be read now
    ST_RESOURCE_IN_USE,        // 1eh - Resource is in use
    ST_PORT_IN_USE,            // 1fh - Port (UART) is in use
    ST_NO_MATCH,               // 20h - Match not found (string find)
    ST_AMBIGUOUS,              // 21h - Match is ambugious
    ST_BADFRAME,               // 22h - Bad frame
    ST_USE_GATEWAY,            // 23h - Use gateway for MAC
    ST_DO_ARP,                 // 24h - ARP on this interface
    ST_NO_PATH,                // 25h - No path to this address
    ST_FAN_OVERWRITE,          // 26h - Fan overwrite bit set.
    ST_PARTIAL_UPDATE,         // 27h - Object partially updated
    ST_INVALID_PWRMODE,        // 28h - invalid power mode requested
    ST_BAD_POINTER,            // 29h - Pointer passed in is invalid!
    ST_BLOCK_NOT_ALLOCATED,    // 2ah - Tried to free a deallocated buffer
    ST_LINKDOWN,               // 2bh - Transport is down
    ST_INVALID_IPMB_PROTOCOL,  // 2ch - Error in IPMB protocol fields

    // DO NOT COMBINE THE FOLLOWING STATUS TYPES WITH OTHER STATUS TYPES OR
    // USE THESE STATUS TYPES IN ANY CODE BUT THE PARAMETER SETTING CODE! THEY NEED
    // TO BE GLOBALLY UNIQUE TO THAT MODULE ONLY!

    ST_CONFIG_LEN_BAD,        // 2dh - Configuration length bad.
    ST_CONFIG_READ_ONLY,      // 2eh - Configuration data read only!
    ST_CONFIG_WRITE_ONLY,     // 2fh - Configuration data write only!
    ST_CONFIG_RESERVED_ONLY,  // 30h - Configuration reserved bits
    ST_PARAM_NOT_FOUND,       // 31h - Couldn't find parameter in config block
    ST_CONFIG_PARAM_LOCKED,   // 32h - Param is locked or invalid

    // Error codes for speaker driver functions

    ST_SPEAKER_BUSY,    // 33h
    ST_INVALID_MODE,    // 34h
    ST_INVALID_BEEPID,  // 35h

    // Error codes for LAN configuration
    ST_SET_IN_PROGRESS_ACTIVE,  // 36h - LAN param modify in progress

    // Misc common
    ST_INVALID_LENGTH,  // 37h
    ST_OUT_OF_RANGE,    // 38h - Parameter out of range
    ST_CMD_FAILURE,     // 39h - Command Processor failure

    ST_ILLEGAL_HOSTNAME,  // 3ah - Hostname has bad/unprintable chars
    ST_PEC_ERROR,         // PMBus/SMBus had invalid PEC data byte.

    ST_COMMON_MAX,

    // ASIC Status base

    ST_I2CS_BUS_ARB = ASIC_BASE,       // I2C lost bus arbitration
    ST_I2CS_WRITE_NAK,                 // Write operation nak'ed
    ST_I2CS_READ_NAK,                  // Read operation nak'ed
    ST_I2CS_TIMEOUT,                   // I2C operation timedout
    ST_I2CS_ILLEGAL,                   // Illegal I/O
    ST_I2CS_ERROR,                     // I2C error
    ST_I2CS_ARB_FAIL,                  // I2C got arbitration fail
    ST_I2CS_SEMAPHORE_ERROR,           // Driver got a semaphore timeout error
    ST_I2CS_SLAVEX,                    // Driver got a slave request
    ST_I2CS_SHORT_WRITE,               // Master write stopped too soon
    ST_I2CS_BUS_HUNG,                  // One of the bus line hung
    ST_I2CS_BUS_LOCKED,                // Someone has locked the bus
    ST_I2CS_BUS_UNAVAILABLE,           // Someone has locked the bus
    ST_I2CS_INIT_ERROR,                // Error initializing I2C bus
    ST_I2CS_NB_PROCESSING,             // Nonblocking master transaction
                                       // parameters still being processed
    ST_I2CS_NB_PENDING,                // NB master transaction waiting for bus
    ST_I2CS_NB_INITIATED,              // NB master transaction initiated on bus
    ST_I2CS_INVALID_BYTE_COUNT,        // Incoming BC not valid for block type
    ST_I2CS_READ_OVERFLOW,             // Master buffer too small for incoming BC
    ST_I2CS_INVALID_TRANSACTION_TYPE,  // Bad transaction type read in FML Op
    ST_GPIO_INIT_ERROR,                // Redefined a GPIO pin
    ST_GPIO_BAD_TYPE,                  // Invalid GPIO type
    ST_GPIO_BAD_DEVICE,                // Invalid GPIO device
    ST_I2CS_SELF_ADDR,                 // I2C master operation to our own addr
    ST_EPORT_NO_EXPANSION_PORT,        // No expansion ports exist
    ST_INVALID_INSTANCE,               // instance out of range
    ST_ASIC_MAX,

    // System event statuses

    ST_SYSEVENT_NOPENDING = SYSEVENT_BASE,  // no events in progress (timed out?)
    ST_SYSEVENT_INVALID_ID,                 // acked with incorrect event id
    ST_SYSEVENT_BUSY,                       // already event in progress
    ST_SYSEVENT_INVALID_KEY,                // bogus ack key
    ST_SYSEVENT_NOT_PENDING,                // acked for no reason, or double-ack
    ST_SYSEVENT_ACK_TIMEOUT,                // someone forgot to ack
    ST_SYSEVENT_INVALID_CONTEXT,            // called from ISR or before OS, and someone acked later
    ST_SYSEVENT_DEFER,

    // Boot Block error codes

    ST_NO_DEVICE_REGISTERED = BOOT_BASE,  // 0x0B00, no device was registered during discovery
    ST_METHOD_NOT_SUPPORTED,              // 0x0B01, flash method is not supported by device module
    ST_DEVICE_NOT_SUPPORTED,              // 0x0B02, flash device is not supported by device module
    ST_DEVICE_BUSY,                       // 0x0B03, flash device is busy erase in progress (e.g., suspended)
    ST_REGISTRATION_COLLISION,            // 0x0B04, 2nd device attempted to register
    ST_SET_REGION_FAILED,                 // 0x0B05, set ram region failed
    ST_RELOCATION_FAILED,                 // 0x0B06, flash method failed to relocate to ram
    ST_DISCOVERY_FAILED,                  // 0x0B07, flash discovery failed
    ST_INVALID_COMMAND,                   // 0x0B08, invalid command
    ST_INVALID_INTERFACE,                 // 0x0B09, invalid interface for fw update module
    ST_MISCOMPARE,                        // 0x0B0A, compare on write failed
    ST_TIMEOUT,                           // 0x0B0B, timeout on erase
    ST_UNLOCK_TIMEOUT,                    // 0x0B0C, timeout on block unlock
    ST_UNLOCK_FAILED,                     // 0x0B0D, failed to unlock block
    ST_LOCK_FAILED,                       // 0x0B0E, failed to lock block
    ST_LOCK_TIMEOUT,                      // 0x0B0F, timeout on block lock
    ST_LOCKED_BLOCK,                      // 0x0B10, attempted to write to a locked block
    ST_BLOCK_COLLISION,                   // 0x0B11, collision between SFlashBlock[] vs. usage bits (FLASHBLOCKFLAG_GP)
    ST_ISRWRAPPER_INST_FAILED,            // 0x0B12, failed to install ISR wrapper function
    ST_INIT_ERROR,                        // 0x0B13, failed to initialize block
    ST_ERASE_SUSPENDED,                   // 0x0B14, erase was suspended
    ST_ERASE_ERROR,                       // 0x0B15, failed to erase
    ST_SUSPEND_ERROR,                     // 0x0B16, failed to suspend erase
    ST_PROGRAM_ERROR,                     // 0x0B17, an error occurred during a write operation
    ST_BLOCK_NOT_FOUND,                   // 0x0B18, unable to determine address to block mapping
    ST_EXCEEDED_BLOCKS,                   // 0x0B19, exceeded the number of physical pamater blocks for device

    // Sensor error codes

    ST_SENSOR_NOT_FOUND = SENSOR_BASE,
    ST_SENSOR_NOT_ENABLED,
    ST_SENSOR_TYPE_NOT_FOUND,
    ST_SENSOR_NOT_INITIALIZED,
    ST_INVALID_SCANNER_FN,
    ST_DEVICE_WRITE_FAIL,
    ST_NO_IPMI_BUFFER,
    ST_SELADD_ENTRY_FAILED,
    ST_ILLEGAL_SENSORTYPE,
    ST_INVALID_SETMASK,
    ST_INVALID_MASK,
    ST_FAILED_INIT,

    // SEI API return codes
    ST_SEI_RDR_NOT_FOUND,
    ST_SEI_EOF,
    ST_SEI_RDR_BUSY,
    ST_SEI_RDR_NOT_AVAILABLE,

    // Session and user error codes

    ST_SESSION_NOT_AVAIL = SESSION_BASE,
    ST_DUPLICATE_SESSSION_MESSAGE,
    ST_SESSION_OUT_OF_ORDER,
    ST_INVALID_AUTHTYPE,          // invalid authentication type
    ST_INVALID_AUTHCODE,          // invalid authentication code
    ST_IPMI_MESSAGING_DISABLED,   // IPMI message disabled for user
    ST_SESSION_SLOTS_FULL,        // no more slots available
    ST_ACCESS_MODE_INVALID,       // Channel access mode invalid
    ST_USER_DISABLED_ON_CHANNEL,  // User disabled on the channel

    // OEM SDR error codes
    ST_RECFOUND_NOPENDING,
    ST_RECFOUND_INVALID_ID,
    ST_RECFOUND_BAD_RECORD,
    ST_RECFOUND_ACK_TIMEOUT,

    // Channel ConnectTo method return codes

    ST_CONN_SUCCESS = CONNECT_BASE,  // on success
    ST_CONN_FAILURE,                 // on failure
    ST_CONN_DEFERRED,                // Destination unavailable, need to retry
    ST_CONN_BUSY,                    // Channel busy, try again
    ST_CONN_BLACKOUT,                // Dial Page is in blackout interval

    // Channel Alert method return codes

    ST_ALERT_SUCCESS = ALERT_BASE,  // on success
    ST_ALERT_FAILURE,               // on failure
    ST_ALERT_DEFERRED,              // Wait for alert acknowledge

    // PIA I/O return codes

    ST_SIGNAL_SUBST,
    ST_UNAVAILABLE_ERROR,
    ST_WRONG_SYSTEM_STATE,
    ST_INVALID_SIGNAL_ACCESS,

    // DB error codes
    ST_INVALID_DB_HANDLE,
    ST_INVALID_DB_OPS,
    ST_DB_RECORD_ALREADY_EXISTS,
    ST_DB_RECID_INVALID,
    ST_DB_FULL,
    ST_DB_INVALID_OPERATION,

    // Crypto shim return codes
    ST_INVALID_COOKIE,

    // Error codes for the FBM module
    ST_FBM_SUCCESS = FBM_BASE,  // on success Do not use this use ST_OK
    ST_FBM_VALUE,
    ST_FBM_INIT,
    ST_FBM_INUSE,
    ST_FBM_FLASH_FULL,
    ST_FBM_ALLOCATE,
    ST_FBM_BAD_HANDLE,
    ST_FBM_OUTOFBOUNDS,

    // TLS error codes
    ST_TLS_PORT_INUSE,           // The specified port is already registered.
    ST_TLS_NO_LISTEN_SOCKET,     // All listen sockets already allocated.
    ST_TLS_SEND_FAIL,            // Failed to write data.
    ST_TLS_UNPROVISIONED,        // Not provisioned.
    ST_TLS_INVALID_KEY,          // Key not valid
    ST_TLS_INVALID_CERTIFICATE,  // Certificate not valid
    ST_TLS_INVALID_ENFORCER,     // Enforcer not valid, encoding not valid
    ST_TLS_BAD_SETTINGS,         // Corrupted key or certificate
    ST_TLS_VALIDITY,             // Certificate invalid
    // OEM Hook subscriber function return codes
    ST_HOOK_ABORT,
    ST_HOOK_SKIP_NEXT,

    ST_INVALID_STATE,

    ST_INVALID_POLICYID,
    ST_INVALID_DOMAINID,
    ST_POLICY_IS_ENABLED,
    ST_PSTATE_UNCHANGED,
    ST_HDMA_FAILED,

    // Virtual flash module errors
    ST_VIRTUAL_FLASH_ALREADY_INITIALIZED = VIRTUAL_FLASH_BASE,
    ST_VIRTUAL_FLASH_NOT_INITIALIZED,
    ST_VIRTUAL_FLASH_FILENAME_IS_NULL,
    ST_VIRTUAL_FLASH_UNLINK_FAILURE,
    ST_VIRTUAL_FLASH_OPEN_FAILURE,
    ST_VIRTUAL_FLASH_READ_FAILURE,
    ST_VIRTUAL_FLASH_WRITE_FAILURE,
    ST_VIRTUAL_FLASH_INVALID_ADDRESS,
    ST_VIRTUAL_FLASH_SHORT_READ_RESTORING_DATA,

    // XML Config errors
    ST_XML_TAG_START_EXPECTED = XML_CONFIG_BASE,
    ST_XML_COMMENTSTART1_EXPECTED,
    ST_XML_COMMENTSTART2_EXPECTED,
    ST_XML_PRIMARY_EXPECTED,
    ST_XML_SECONDARYEQUALS_EXPECTED,
    ST_XML_TAG_END_EXPECTED,
    ST_XML_ID_INSTANCE_BEFORE_CLASS,
    ST_XML_ID_SIGNAL_BEFORE_INSTANCE,
    ST_XML_ID_NOT_FOUND,
    ST_XML_ID_ALREADY_EXISTS,
    ST_XML_NO_SRC_DATA,
    ST_XML_STRUCTURE_NOT_ALLOCATED,
    ST_XML_INCOMPATIBLE_PROPERTY_WRITE,
    ST_XML_EXPECTED_MASK_TYPE,
    ST_XML_EXPECTED_TRUE_OR_FALSE,
    ST_XML_BAD_BIT_SHIFT,
    ST_XML_UNEXPECTED_INSTANCE_PARAMETER,
    ST_XML_UNEXPECTED_PREDEFINED_ID,
    ST_XML_UNEXPECTED_PRIMARY_PARAMETER,
    ST_XML_UNEXPECTED_SECONDARY_PARAMETER,
    ST_XML_EXPECTED_TOKEN_CONSTANT,
    ST_XML_EXPECTED_TOKEN_STRING,

    // Just a terminator
    ST_MAX
} STATUS;

typedef STATUS ESTATUS;

#endif  // _COMMSTAT_H_
