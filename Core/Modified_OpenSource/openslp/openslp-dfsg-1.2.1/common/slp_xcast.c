/***************************************************************************/
/*                                                                         */
/* Project:     OpenSLP - OpenSource implementation of Service Location    */
/*              Protocol                                                   */
/*                                                                         */
/* File:        slp_xcast.c                                                */
/*                                                                         */
/* Abstract:    Functions used to multicast and broadcast SLP messages     */
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

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <io.h>
#include <errno.h>
#define ETIMEDOUT 110
#define ENOTCONN  107
#else
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h> 
#include <netdb.h> 
#include <fcntl.h> 
#include <errno.h>
#endif

#ifndef UNICAST_NOT_SUPPORTED
#include "../libslp/slp.h"
#endif

#include "slp_xcast.h"
#include "slp_message.h"
#include "slp_property.h"

/*========================================================================*/
int SLPBroadcastSend(const SLPIfaceInfo* ifaceinfo, 
                     SLPBuffer msg,
                     SLPXcastSockets* socks)
/* Description:
 *    Broadcast a message.
 *
 * Parameters:
 *    ifaceinfo (IN) Pointer to the SLPIfaceInfo structure that contains
 *                   information about the interfaces to send on
 *    msg       (IN) Buffer to send
 *
 *   socks      (OUT) Sockets used broadcast multicast.  May be used to 
 *                    recv() responses.  MUST be close by caller using 
 *                    SLPXcastSocketsClose() 
 *
 * Returns:
 *    Zero on sucess.  Non-zero with errno set on error
 *========================================================================*/
{
    int                 xferbytes;
    int                 flags = 0;  

#ifdef _WIN32
    char    on = 1;
#else
    int     on = 1;
#endif


#if defined(MSG_NOSIGNAL)
    flags = MSG_NOSIGNAL;
#endif

    for (socks->sock_count = 0; 
         socks->sock_count < ifaceinfo->iface_count; 
         socks->sock_count++)
    {
        socks->sock[socks->sock_count] = socket(AF_INET, SOCK_DGRAM, 0);
        if (socks->sock[socks->sock_count] < 0)
        {
            /* error creating socket */
            return -1;
        }
        
        if( (setsockopt(socks->sock[socks->sock_count],
                        SOL_SOCKET, 
                        SO_BROADCAST, 
                        &on, 
                        sizeof(on))) )
        {
            /* Error setting socket option */
            return -1;
        }

        socks->peeraddr[socks->sock_count].sin_family = AF_INET;
        socks->peeraddr[socks->sock_count].sin_port = htons(SLP_RESERVED_PORT);
        socks->peeraddr[socks->sock_count].sin_addr.s_addr = ifaceinfo->bcast_addr[socks->sock_count].sin_addr.s_addr;

        xferbytes = sendto(socks->sock[socks->sock_count], 
                           msg->start,
                           msg->end - msg->start,
                           0,
                           (struct sockaddr *) &(socks->peeraddr[socks->sock_count]),
                           sizeof(struct sockaddr_in));
        if(xferbytes  < 0)
        { 
            /* Error sending to broadcast */
            return -1;
        }
    }  

    return 0;
}


/*========================================================================*/
int SLPMulticastSend(const SLPIfaceInfo* ifaceinfo, 
                     SLPBuffer msg,
                     SLPXcastSockets* socks)
/* Description:
 *    Multicast a message.
 *
 * Parameters:
 *    ifaceinfo (IN) Pointer to the SLPIfaceInfo structure that contains
 *                   information about the interfaces to send on
 *    msg       (IN) Buffer to send
 *
 *   socks      (OUT) Sockets used to multicast.  May be used to recv() 
 *                    responses.  MUST be close by caller using 
 *                    SLPXcastSocketsClose() 
 *
 * Returns:
 *    Zero on sucess.  Non-zero with errno set on error
 *========================================================================*/
{
    int             flags = 0;
    int             xferbytes;
    struct in_addr  saddr;
    int		    optarg;


#if defined(MSG_NOSIGNAL)
    flags = MSG_NOSIGNAL;
#endif

    optarg = atoi(SLPPropertyGet("net.slp.multicastTTL"));

    for (socks->sock_count = 0;
         socks->sock_count < ifaceinfo->iface_count;
         socks->sock_count++)
    {
        socks->sock[socks->sock_count] = socket(AF_INET, SOCK_DGRAM, 0);
        if (socks->sock[socks->sock_count] < 0)
        {
            /* error creating socket */
            return -1;
        }
        
        saddr.s_addr = ifaceinfo->iface_addr[socks->sock_count].sin_addr.s_addr;
        if( setsockopt(socks->sock[socks->sock_count], 
                       IPPROTO_IP, 
                       IP_MULTICAST_IF, 
                       (char*)&saddr, 
                       sizeof(struct in_addr)))
        {
            /* error setting socket option */
            return -1;
        }
        if(setsockopt(socks->sock[socks->sock_count],
                      IPPROTO_IP,
                      IP_MULTICAST_TTL,
                      (char*)&optarg,
                      sizeof(optarg)))
        {
            return -1;
        }

        socks->peeraddr[socks->sock_count].sin_family = AF_INET;
        socks->peeraddr[socks->sock_count].sin_port = htons(SLP_RESERVED_PORT);
        socks->peeraddr[socks->sock_count].sin_addr.s_addr = htonl(SLP_MCAST_ADDRESS);

        xferbytes = sendto(socks->sock[socks->sock_count],
                           msg->start,
                           msg->end - msg->start,
                           flags,
                           (struct sockaddr *) &(socks->peeraddr[socks->sock_count]),
                           sizeof(struct sockaddr_in));
        if (xferbytes <= 0)
        {
            /* error sending */
            return -1;
        }
    }
    
    return 0;

}


/*========================================================================*/
int SLPXcastSocketsClose(SLPXcastSockets* socks)
/* Description:
 *    Closes sockets that were opened by calls to SLPMulticastSend() and
 *    SLPBroadcastSend()
 *
 * Parameters:
 *    socks (IN) Pointer to the SLPXcastSockets structure being close
 *
 * Returns:
 *    Zero on sucess.  Non-zero with errno set on error
 *========================================================================*/
{
    while(socks->sock_count)
    {
        socks->sock_count = socks->sock_count - 1;
		#ifdef _WIN32 
			closesocket(socks->sock[socks->sock_count]);
		#else
        close(socks->sock[socks->sock_count]);
		#endif
    }

    return 0;
}



/*=========================================================================*/
int SLPXcastRecvMessage(const SLPXcastSockets* sockets,
                        SLPBuffer* buf,
                        struct sockaddr_in* peeraddr,
                        struct timeval* timeout)
/* Description: 
 * 	Receives datagram messages from one of the sockets in the specified 
 * 	SLPXcastsSockets structure
 * 
 * Parameters:
 * 	sockets (IN) Pointer to the SOPXcastSockets structure that describes
 * 		       which sockets to read messages from.
 * 	buf (OUT) Pointer to SLPBuffer that will contain the message upon
 * 		       successful return.
 * 	peeraddr (OUT) Pointer to struc sockaddr_in that will contain the
 * 		       address of the peer that sent the received message.
 * 	timeout (IN/OUT) pointer to the struct timeval that indicates how much
 * 		       time to wait for a message to arrive
 *
 * Returns:
 * 	Zero on success, non-zero with errno set on failure.
 *==========================================================================*/ 
{
    fd_set  readfds;
    int     highfd;
    int     i;
    int     readable;
    size_t  bytesread;
    int     recvloop;
    int     peeraddrlen = sizeof(struct sockaddr_in);
    char    peek[16];
    int     result;

    /* recv loop */
    result = -1;
    recvloop = 1;
    while(recvloop)
    {
        /* Set the readfds */
        FD_ZERO(&readfds);
        highfd = 0;
        for (i=0; i<sockets->sock_count; i++)
        {
            FD_SET(sockets->sock[i],&readfds);
            if(sockets->sock[i] > highfd)
            {
                highfd = sockets->sock[i];
            }
        }
    
        /* Select */
        readable = select(highfd + 1,&readfds,NULL,NULL,timeout);
        if(readable > 0)
        {
            /* Read the datagram */
            for (i=0; i<sockets->sock_count; i++)
            {
                if(FD_ISSET(sockets->sock[i],&readfds))
                {
                    /* Peek at the first 16 bytes of the header */
                    bytesread = recvfrom(sockets->sock[i],
                                         peek,
                                         16,
                                         MSG_PEEK,
                                         (struct sockaddr *)peeraddr,
                                         &peeraddrlen);
                    if(bytesread == 16
#ifdef _WIN32
                    /* Win32 returns WSAEMSGSIZE if the message is larger than
                     * the requested size, even with MSG_PEEK. But if this is the
                     * error code we can be sure that the message is at least 16
                     * bytes */
                       || (bytesread == (size_t)-1 && WSAGetLastError() == WSAEMSGSIZE)
#endif
                       )
                    {
                        if(AsUINT24(peek + 2) <=  SLP_MAX_DATAGRAM_SIZE)
                        {
                            *buf = SLPBufferRealloc(*buf, AsUINT24(peek + 2));
                            bytesread = recv(sockets->sock[i],
                                             (*buf)->curpos,
                                             (*buf)->end - (*buf)->curpos,
                                             0);
                            if(bytesread != AsUINT24(peek + 2))
                            {
                                /* This should never happen but we'll be paranoid*/
                                (*buf)->end = (*buf)->curpos + bytesread;
                            }
                            
                            /* Message read. We're done! */
                            result = 0; 
                            recvloop = 0;
                            break;
                        }
                        else
                        {
                            /* we got a bad message, or one that is too big! */
#ifndef UNICAST_NOT_SUPPORTED
                            /* Reading SLP_MAX_DATAGRAM_SIZE bytes on the socket */
		            *buf = SLPBufferRealloc(*buf, SLP_MAX_DATAGRAM_SIZE);
		            bytesread = recv(sockets->sock[i],
		                              (*buf)->curpos,
					      (*buf)->end - (*buf)->curpos,
					      0);
			    if(bytesread != SLP_MAX_DATAGRAM_SIZE)
			    {
			        /* This should never happen but we'll be paranoid*/
			        (*buf)->end = (*buf)->curpos + bytesread;
			    }
			    result = SLP_RETRY_UNICAST;
			    recvloop = 0;
			    return result;
#endif
                        }
                    }
                    else
                    {
                        /* Not even 16 bytes available */
                    }
                }
            }   
        }
        else if(readable == 0)
        {
            result = -1;
            errno = ETIMEDOUT;
            recvloop = 0;
        }
        else
        {
            result = -1;
            recvloop = 0;
        }
    }

    return result;
}

/*===========================================================================
 * TESTING CODE may be compiling with the following command line:
 *
 * $ gcc -g -DDEBUG -DSLP_XMIT_TEST slp_xcast.c slp_iface.c slp_buffer.c 
 *   slp_linkedlist.c slp_compare.c slp_xmalloc.c
 *==========================================================================*/ 
#ifdef SLP_XMIT_TEST
main()
{
    SLPIfaceInfo    ifaceinfo;
    SLPXcastSockets      socks;
    SLPBuffer           buffer;
    #ifdef _WIN32
    WSADATA wsadata;
    WSAStartup(MAKEWORD(2,2), &wsadata);
    #endif

    buffer = SLPBufferAlloc(SLP_MAX_DATAGRAM_SIZE);
    if(buffer)
    {
        
        strcpy(buffer->start,"testdata");
    
        SLPIfaceGetInfo(NULL,&ifaceinfo);
    
        if (SLPBroadcastSend(&ifaceinfo, buffer,&socks) !=0)
            printf("\n SLPBroadcastSend failed \n");
        SLPXcastSocketsClose(&socks);
    
        if (SLPMulticastSend(&ifaceinfo, buffer, &socks) !=0)
            printf("\n SLPMulticast failed \n");
        SLPXcastSocketsClose(&socks);

        printf("Success\n");

        SLPBufferFree(buffer);
    }
    #ifdef _WIN32
    WSACleanup();
    #endif
}

#endif

