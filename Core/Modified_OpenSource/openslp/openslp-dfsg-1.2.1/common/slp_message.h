/***************************************************************************/
/*                                                                         */
/* Project:     OpenSLP - OpenSource implementation of Service Location    */
/*              Protocol                                                   */
/*                                                                         */
/* File:        slp_message.h                                              */
/*                                                                         */
/* Abstract:    Header file that defines structures and constants that are */
/*              specific to the SLP wire protocol messages.                */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*                                                                         */
/*     Please submit patches to http://www.openslp.org                     */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*                                                                         */
/* Copyright (C) 2000 Caldera Systems, Inc                                 */
/* All rights reserved.                                                    */
/*                                                                         */
/* Redistribution and use in source and binary forms, with or without      */
/* modification, are permitted provided that the following conditions are  */
/* met:                                                                    */ 
/*                                                                         */
/*      Redistributions of source code must retain the above copyright     */
/*      notice, this list of conditions and the following disclaimer.      */
/*                                                                         */
/*      Redistributions in binary form must reproduce the above copyright  */
/*      notice, this list of conditions and the following disclaimer in    */
/*      the documentation and/or other materials provided with the         */
/*      distribution.                                                      */
/*                                                                         */
/*      Neither the name of Caldera Systems nor the names of its           */
/*      contributors may be used to endorse or promote products derived    */
/*      from this software without specific prior written permission.      */
/*                                                                         */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS     */
/* `AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT      */
/* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR   */
/* A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE CALDERA      */
/* SYSTEMS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, */
/* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT        */
/* LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;  LOSS OF USE,  */
/* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON       */
/* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT */
/* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE   */
/* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.    */
/*                                                                         */
/***************************************************************************/

#if(!defined SLP_MESSAGE_H_INCLUDED)
#define SLP_MESSAGE_H_INCLUDED

#include "slp_buffer.h"

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#ifdef _WIN32
# define WIN32_LEAN_AND_MEAN
# include <windows.h>
# include <winsock.h>
# ifndef UINT32_T_DEFINED
# define UINT32_T_DEFINED
typedef unsigned int uint32_t;
# endif
#else
# include <sys/types.h>
# ifdef __FreeBSD__
#  include <sys/param.h>		/* for limits.h - INT_MAX slpd*/
# endif
# include <netinet/in.h>		/* for htonl() routines */
# ifdef HAVE_STDINT_H
#  include <stdint.h>
# else
#  include <inttypes.h>
# endif
#endif

typedef char            CHAR;
typedef unsigned char   UINT8;
typedef unsigned short  UINT16;

#ifndef _WIN32
typedef unsigned long   UINT32;
#endif

typedef CHAR*           PCHAR;
typedef UINT8*          PUINT8;
typedef UINT16*         PUINT16;
typedef UINT32*         PUINT32;


/*=========================================================================*/
/* SLP Important constants                                                 */
/*=========================================================================*/
#define SLP_RESERVED_PORT       427
#define SLP_MCAST_ADDRESS       0xeffffffd  /* 239.255.255.253 */
#define SLP_BCAST_ADDRESS       0xffffffff  /* 255.255.255.255 */
#define SLPv1_DA_MCAST_ADDRESS  0xe0000123  /* 224.0.1.35 */
#define LOOPBACK_ADDRESS        0x7f000001  /* 127.0.0.1 */
#define SLP_MAX_DATAGRAM_SIZE   1400 
#if(!defined SLP_LIFETIME_MAXIMUM) 
#define SLP_LIFETIME_MAXIMUM    0xffff
#endif


/*=========================================================================*/
/* SLP Function ID constants                                               */
/*=========================================================================*/
#define SLP_FUNCT_SRVRQST         1
#define SLP_FUNCT_SRVRPLY         2
#define SLP_FUNCT_SRVREG          3
#define SLP_FUNCT_SRVDEREG        4
#define SLP_FUNCT_SRVACK          5
#define SLP_FUNCT_ATTRRQST        6
#define SLP_FUNCT_ATTRRPLY        7
#define SLP_FUNCT_DAADVERT        8
#define SLP_FUNCT_SRVTYPERQST     9
#define SLP_FUNCT_SRVTYPERPLY     10
#define SLP_FUNCT_SAADVERT        11


/*=========================================================================*/
/* SLP Protocol Error codes                                                */
/*=========================================================================*/
#define SLP_ERROR_OK                       0
#define SLP_ERROR_LANGUAGE_NOT_SUPPORTED   1
#define SLP_ERROR_PARSE_ERROR              2
#define SLP_ERROR_INVALID_REGISTRATION     3
#define SLP_ERROR_SCOPE_NOT_SUPPORTED      4
#define SLP_ERROR_CHARSET_NOT_UNDERSTOOD   5 /* valid only for SLPv1 */
#define SLP_ERROR_AUTHENTICATION_UNKNOWN   5
#define SLP_ERROR_AUTHENTICATION_ABSENT    6
#define SLP_ERROR_AUTHENTICATION_FAILED    7
#define SLP_ERROR_VER_NOT_SUPPORTED        9
#define SLP_ERROR_INTERNAL_ERROR           10
#define SLP_ERROR_DA_BUSY_NOW              11
#define SLP_ERROR_INVALID_UPDATE           13
#define SLP_ERROR_MESSAGE_NOT_SUPPORTED    14
#define SLP_ERROR_REFRESH_REJECTED         15

/*=========================================================================*/
/* SLP Flags                                                               */
/*=========================================================================*/
#define SLP_FLAG_OVERFLOW         0x8000
#define SLP_FLAG_FRESH            0x4000
#define SLP_FLAG_MCAST            0x2000
#ifndef UNICAST_NOT_SUPPORTED
#define SLP_FLAG_UCAST            0x0000
#endif

/*=========================================================================*/
/* SLP Constants                                                           */
/*=========================================================================*/
/* Max time to wait for a complete multicast query response        */
#define CONFIG_MC_MAX             15    

/* Wait interval to give up on a unicast request retransmission  */
#define CONFIG_RETRY_MAX          15    

/* Default wait between retransmits*/
#define CONFIG_RETRY_INTERVAL      3    

#define SLP_DA_SERVICE_TYPE       "service:directory-agent"
#define SLP_SA_SERVICE_TYPE       "service:service-agent"


/*=========================================================================*/
/* SLP Registration Sources                                                */
/*=========================================================================*/
#define SLP_REG_SOURCE_UNKNOWN  0
#define SLP_REG_SOURCE_REMOTE   1  /* from a remote host    */
#define SLP_REG_SOURCE_LOCAL    2  /* from localhost or IPC */
#define SLP_REG_SOURCE_STATIC   3  /* from the slp.reg file */


/*=========================================================================*/
/* SLP Extension IDs                                                       */
/*=========================================================================*/
#define SLP_EXTENSION_ID_REG_PID   0x9799


/*=========================================================================*/
/* SLPHeader structure and associated functions                            */
/*=========================================================================*/
typedef struct _SLPHeader
{
    int             version;
    int             functionid;
    int             length;       
    int             flags;
    int             encoding;   /* language encoding, valid only for SLPv1 */
    int             extoffset;    
    int             xid;
    int             langtaglen;
    const char*     langtag; /* points into the translated message */
}SLPHeader;


/*=========================================================================*/
/* SLPAuthBlock structure and associated functions                         */
/*=========================================================================*/
typedef struct _SLPAuthBlock
{
    unsigned int            bsd;
    int                     length;
    unsigned int            timestamp;
    int                     spistrlen;
    const char*             spistr;
    const unsigned char*    authstruct;
    /* The following are not part of the RFC protocol.  They are used by   */
    /* the OpenSLP implementation for convenience                          */
    int                     opaquelen;
    char*                   opaque;
}SLPAuthBlock;


/*=========================================================================*/
/* SLPUrlEntry structure and associated functions                          */
/*=========================================================================*/
typedef struct _SLPUrlEntry
{
    char                    reserved;       /* This will always be 0 */
    int                     lifetime;
    int                     urllen;
    const char*             url;
    int                     authcount;
    SLPAuthBlock*           autharray; 
    /* The following are not part of the RFC protocol.  They are used by   */
    /* the OpenSLP implementation for convenience                          */
    int                     opaquelen;
    char*                   opaque;
}SLPUrlEntry;


/*=========================================================================*/
/* SLPSrvRqst structure and associated functions                           */
/*=========================================================================*/
typedef struct _SLPSrvRqst
{
    int             prlistlen;
    const char*     prlist;
    int             srvtypelen;
    const char*     srvtype;
    int             scopelistlen;
    const char*     scopelist;
    int             predicatever;
    int             predicatelen;
    const char*     predicate;
    int             spistrlen;
    const char*     spistr;
}SLPSrvRqst;


/*=========================================================================*/
typedef struct _SLPSrvRply                                                 
/*=========================================================================*/
{
    int             errorcode;
    int             urlcount;
    SLPUrlEntry*    urlarray;
}SLPSrvRply;


/*=========================================================================*/
typedef struct _SLPSrvReg
/*=========================================================================*/
{
    SLPUrlEntry         urlentry;
    int                 srvtypelen;
    const char*         srvtype;
    int                 scopelistlen;
    const char*         scopelist;
    int                 attrlistlen;
    const char*         attrlist;
    int                 authcount;
    SLPAuthBlock*       autharray;
    /* The following are used for OpenSLP specific extensions */
    uint32_t            pid;
    /* The following are not part of the RFC protocol.  They are used by   */
    /* the OpenSLP implementation for convenience                          */
    int                 source;
}SLPSrvReg;


/*=========================================================================*/
typedef struct _SLPSrvDeReg
/*=========================================================================*/
{
    int                 scopelistlen;
    const char*         scopelist;
    SLPUrlEntry         urlentry;
    int                 taglistlen;
    const char*         taglist;
}SLPSrvDeReg;


/*=========================================================================*/
typedef struct _SLPSrvAck
/*=========================================================================*/
{
    int errorcode;
}SLPSrvAck;


/*=========================================================================*/
typedef struct _SLPDAAdvert
/*=========================================================================*/
{
    int                 errorcode;
    unsigned int        bootstamp;
    int                 urllen;
    const char*         url;
    int                 scopelistlen;
    const char*         scopelist;
    int                 attrlistlen;
    const char*         attrlist;
    int                 spilistlen;
    const char*         spilist;
    int                 authcount;
    SLPAuthBlock*       autharray;
}SLPDAAdvert;


/*=========================================================================*/
typedef struct _SLPAttrRqst
/*=========================================================================*/
{
    int             prlistlen;
    const char*     prlist;
    int             urllen;
    const char*     url;
    int             scopelistlen;
    const char*     scopelist;
    int             taglistlen;
    const char*     taglist;
    int             spistrlen;
    const char*     spistr;
}SLPAttrRqst;


/*=========================================================================*/
typedef struct _SLPAttrRply
/*=========================================================================*/
{
    int             errorcode;
    int             attrlistlen;
    const char*     attrlist;
    int             authcount;
    SLPAuthBlock*   autharray;
}SLPAttrRply;


/*=========================================================================*/
typedef struct _SLPSrvTypeRqst
/*=========================================================================*/
{
    int             prlistlen;
    const char*     prlist;
    int             namingauthlen;
    const char*     namingauth;
    int             scopelistlen;
    const char*     scopelist;
}SLPSrvTypeRqst;



/*=========================================================================*/
typedef struct _SLPSrvTypeRply
/*=========================================================================*/
{
    int             errorcode;
    int             srvtypelistlen;
    const char*     srvtypelist;
}SLPSrvTypeRply;



/*=========================================================================*/
typedef struct _SLPSAAdvert
/*=========================================================================*/
{
    int             urllen;
    const char*     url;
    int             scopelistlen;
    const char*     scopelist;
    int             attrlistlen;
    const char*     attrlist;
    int             authcount;
    SLPAuthBlock*   autharray;
}SLPSAAdvert;



/*=========================================================================*/
typedef struct _SLPMessage
/*=========================================================================*/
{
    struct sockaddr_in    peer;
    SLPHeader             header;
    union _body
    {
        SLPSrvRqst        srvrqst;
        SLPSrvRply        srvrply;
        SLPSrvReg         srvreg;
        SLPSrvDeReg       srvdereg;
        SLPSrvAck         srvack;
        SLPDAAdvert       daadvert;
        SLPAttrRqst       attrrqst;
        SLPAttrRply       attrrply;
        SLPSrvTypeRqst    srvtyperqst;
        SLPSrvTypeRply    srvtyperply;
        SLPSAAdvert       saadvert;
    }body; 

}*SLPMessage;


/*=========================================================================*/
void SLPMessageFreeInternals(SLPMessage message);
/*=========================================================================*/

/*=========================================================================*/
SLPMessage SLPMessageAlloc();
/* Allocates memory for a SLP message descriptor                           */
/*                                                                         */
/* Returns   - A newly allocated SLPMessage pointer of NULL on ENOMEM      */
/*=========================================================================*/


/*=========================================================================*/
SLPMessage SLPMessageRealloc(SLPMessage msg);
/* Reallocates memory for a SLP message descriptor                         */
/*                                                                         */
/* Returns   - A newly allocated SLPMessage pointer of NULL on ENOMEM      */
/*=========================================================================*/


/*=========================================================================*/
void SLPMessageFree(SLPMessage message);
/* Frees memory that might have been allocated by the SLPMessage for       */
/* UrlEntryLists or AuthBlockLists.                                        */
/*                                                                         */
/* message  - (IN) the SLPMessage to free                                  */
/*=========================================================================*/


/*=========================================================================*/
int SLPMessageParseHeader(SLPBuffer buffer, SLPHeader* header);
/* Fill out a header structure with what ever is in the buffer             */
/*                                                                         */
/* buffer (IN) the buffer to be parsed                                     */
/*                                                                         */
/* header (IN/OUT) pointer to the header structure to fill out             */
/*=========================================================================*/


/*=========================================================================*/
int SLPMessageParseBuffer(struct sockaddr_in* peerinfo,
                          SLPBuffer buffer, 
                          SLPMessage message);
/* Initializes a message descriptor by parsing the specified buffer.       */
/*                                                                         */
/* peerinfo - (IN) pointer to the network address information that sent    */ 
/*                 buffer                                                  */
/*                                                                         */
/* buffer   - (IN) pointer the SLPBuffer to parse                          */
/*                                                                         */
/* message  - (OUT) set to describe the message from the buffer            */
/*                                                                         */
/* Returns  - Zero on success, SLP_ERROR_PARSE_ERROR, or                   */
/*            SLP_ERROR_INTERNAL_ERROR if out of memory.  SLPMessage is    */
/*            invalid return is not successful.                            */
/*                                                                         */
/* WARNING  - If successful, pointers in the SLPMessage reference memory in*/ 
/*            the parsed SLPBuffer.  If SLPBufferFree() is called then the */
/*            pointers in SLPMessage will be invalidated.                  */
/*=========================================================================*/

/*=========================================================================*/
/* Functions used to parse buffers                                         */
unsigned short AsUINT16(const char *charptr);
unsigned int AsUINT24(const char *charptr);
unsigned int AsUINT32(const char *charptr);
/*=========================================================================*/

/*=========================================================================*/
/* Functions used to set buffers                                           */
void ToUINT16(char *charptr, unsigned int val);
void ToUINT24(char *charptr, unsigned int val);
void ToUINT32(char *charptr, unsigned int val);
/*=========================================================================*/


#ifdef i386

/*=========================================================================*/
/* Macros to check in_addr                                                 */
#define ISLOCAL(addr) ((ntohl((addr).s_addr) & 0xff000000) == 0x7f000000)
#define ISMCAST(addr) ((ntohl((addr).s_addr) & 0xff000000) >= 0xef000000)
/*=========================================================================*/

#else 

/*=========================================================================*/
/* Macros to check in_addr                                                 */
#define ISLOCAL(addr) (((addr).s_addr & 0xff000000) == 0x7f000000)
#define ISMCAST(addr) (((addr).s_addr & 0xff000000) >= 0xef000000)
/*=========================================================================*/
#endif


#endif
