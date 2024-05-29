/***************************************************************************/
/*                                                                         */
/* Project:     OpenSLP - OpenSource implementation of Service Location    */
/*              Protocol                                                   */
/*                                                                         */
/* File:        slp_pid.c                                                  */
/*                                                                         */
/* Abstract:    Common code to obtain process identifier information       */
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

#ifndef SLP_PID_H_INCLUDED
#define SLP_PID_H_INCLUDED

#ifdef _WIN32
# ifndef UINT32_T_DEFINED
#  define UINT32_T_DEFINED
typedef unsigned int uint32_t;
# endif
#else
# ifdef HAVE_CONFIG_H
#  include "config.h"
# endif
# ifdef HAVE_STDINT_H
#  include <stdint.h>
# else
#  include <inttypes.h>
# endif
#endif    

/*=========================================================================*/
uint32_t SLPPidGet();
/* Description:
 *    Get a process 32 bit integer identifier for the current process
 *    loopback interface
 *
 * Parameters:
 *
 * Returns:
 *     32 bit integer identifier for the current process
 *=========================================================================*/


/*=========================================================================*/
int SLPPidExists(uint32_t pid);
/* Description:
 *    (quickly) determine whether or not the process with the specified
 *    identifier exists (is alive)
 *
 * Parameters:
 *    pid (IN) 32 bit integer identifier for the process to check for
 *
 * Returns:
 *    Boolean value.  Zero if process does not exist, non-zero if it does
 *=========================================================================*/

#endif

