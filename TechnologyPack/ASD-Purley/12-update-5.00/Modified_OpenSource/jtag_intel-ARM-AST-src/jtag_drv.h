/********************************************************************************
* File Name     : driver/char/asped/ast_jtag.c
* Author         : Ryan Chen
* Description   : AST JTAG driver
*
* Copyright (c) 2018 Intel Corporation
* Copyright (C) 2012-2020  ASPEED Technology Inc.
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by the Free Software Foundation;
* either version 2 of the License, or (at your option) any later version.
* This program is distributed in the hope that it will be useful,  but WITHOUT ANY WARRANTY;
* without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#ifndef __JTAG_DRV_H__
#define __JTAG_DRV_H__

typedef enum xfer_mode {
    HW_MODE = 0,
    SW_MODE
} xfer_mode;

struct tck_bitbang {
    unsigned char     tms;
    unsigned char     tdi;        // TDI bit value to write
    unsigned char     tdo;        // TDO bit value to read
};

struct scan_xfer {
    xfer_mode        mode;        // Hardware or software mode
    unsigned int     tap_state;   // Current tap state
    unsigned int     length;      // number of bits to clock
    unsigned char    *tdi;        // data to write to tap (optional)
    unsigned int     tdi_bytes;
    unsigned char    *tdo;        // data to read from tap (optional)
    unsigned int     tdo_bytes;
    unsigned int     end_tap_state;
};

struct set_tck_param {
    xfer_mode        mode;        // Hardware or software mode
    unsigned int     tck;         // This can be treated differently on systems as needed. It can be a divisor or actual frequency as needed.
};

struct get_tck_param {
    xfer_mode        mode;        // Hardware or software mode
    unsigned int     tck;         // This can be treated differently on systems as needed. It can be a divisor or actual frequency as needed.
};

struct tap_state_param {
    xfer_mode        mode;        // Hardware or software mode
    unsigned int     from_state;
    unsigned int     to_state;
};

struct controller_mode_param {
    xfer_mode        mode;        // Hardware or software mode
    unsigned int     controller_mode;
};

typedef enum {
    JtagTLR,
    JtagRTI,
    JtagSelDR,
    JtagCapDR,
    JtagShfDR,
    JtagEx1DR,
    JtagPauDR,
    JtagEx2DR,
    JtagUpdDR,
    JtagSelIR,
    JtagCapIR,
    JtagShfIR,
    JtagEx1IR,
    JtagPauIR,
    JtagEx2IR,
    JtagUpdIR
} JtagStates;

#define JTAGIOC_BASE    'T'

#define AST_JTAG_SET_TCK          _IOW( JTAGIOC_BASE, 3, struct set_tck_param)
#define AST_JTAG_GET_TCK          _IOR( JTAGIOC_BASE, 4, struct get_tck_param)
#define AST_JTAG_BITBANG          _IOWR(JTAGIOC_BASE, 5, struct tck_bitbang)
#define AST_JTAG_SET_TAPSTATE     _IOW( JTAGIOC_BASE, 6, struct tap_state_param)
#define AST_JTAG_READWRITESCAN    _IOWR(JTAGIOC_BASE, 7, struct scan_xfer)
#define AST_JTAG_SLAVECONTLR      _IOW( JTAGIOC_BASE, 8, struct controller_mode_param)

#endif
