/***************************************************************************/
/*                                                                         */
/* Project:     OpenSLP - OpenSource implementation of Service Location    */
/*              Protocol                                                   */
/*                                                                         */
/* File:        slp_database.h                                             */
/*                                                                         */
/* Abstract:    An SLP message database.  The SLP message database holds   */
/*              actual SLP "wire" message buffers as well as structures    */
/*              that interpret the message buffer.  The database exposes   */
/*              an interface suitable linked-list based implementation     */
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

#ifndef SLP_DATABASE_H_INCLUDED
#define SLP_DATABASE_H_INCLUDED

#include "slp_message.h"
#include "slp_buffer.h"
#include "slp_linkedlist.h"


/*=========================================================================*/
typedef struct _SLPDatabaseEntry
/*=========================================================================*/
{
    SLPListItem listitem;
    SLPMessage  msg;
    SLPBuffer   buf;
}SLPDatabaseEntry;


/*=========================================================================*/
typedef SLPList SLPDatabase;
/*=========================================================================*/


/*=========================================================================*/
typedef struct _SLPDatabaseHandle
/*=========================================================================*/
{
    SLPDatabase*      database;
    SLPDatabaseEntry* current;
}*SLPDatabaseHandle;


/*=========================================================================*/
int SLPDatabaseInit(SLPDatabase* database);
/* Initialize a SLPDatabase.                                               */
/*                                                                         */
/* Parameters: database (IN) pointer to the database to initialize         */
/*                                                                         */
/*                                                                         */
/* Returns: zero on success. Non-zero on error                             */
/*=========================================================================*/


/*=========================================================================*/
void SLPDatabaseDeinit(SLPDatabase* database);
/* Deinitialze a SLPDatabase                                               */
/*                                                                         */
/* Parameters: database (IN) pointer to the database to de-initialize      */
/*                                                                         */
/* Returns: none                                                           */
/*=========================================================================*/


/*=========================================================================*/
SLPDatabaseEntry* SLPDatabaseEntryCreate(SLPMessage msg,
                                         SLPBuffer buf);
/* Create a SLPDatabaseEntry.                                              */
/*                                                                         */
/* Parameters: msg (IN) The interpreting message structure for buf         */
/*             buf (IN) The SLP message buffer                             */
/*                                                                         */
/* Returns: Pointer to a new SLPDatabaseEntry.  NULL if out of memory      */
/*                                                                         */
/* Note:  VERY IMPORTANT.  The msg and especially the buf are owned by the */
/*        returned SLPDatabaseEntry and MUST NOT be freed by the caller    */
/*        via SLPMessageFree() or SLPBufferFree()!   Instead, the caller   */
/*        should use SLPDatabaseEntryDestroy() only to free memory         */
/*=========================================================================*/

/*=========================================================================*/
void SLPDatabaseEntryDestroy(SLPDatabaseEntry* entry);
/* Free resources associated with the specified entry                      */
/*                                                                         */
/* Parameters: entry (IN) pointer to the entry to destroy                  */
/*                                                                         */
/* Returns: none                                                           */
/*=========================================================================*/


/*=========================================================================*/
SLPDatabaseHandle SLPDatabaseOpen(SLPDatabase* database);
/* Open a handle that is used with subsequent calls to SLPDatabaseEnum(),  */
/* SLPDatabaseAdd() and SLPDatabaseRemove()                                */
/*                                                                         */
/* Parameters (IN) pointer an initialized SLPDatabase                      */
/*                                                                         */
/* Returns: Valid handle.  NULL on error.                                  */
/*                                                                         */
/* Note:  It is important to make sure that handles returned by this       */
/*        function are used and closed as quickly as possible. Future      */
/*        may use handles to ensure syncronized access to the database     */
/*        in threaded environments                                         */
/*=========================================================================*/


/*=========================================================================*/
SLPDatabaseEntry* SLPDatabaseEnum(SLPDatabaseHandle dh);
/* Used to enumerate through entries of a SLPDatabase                      */
/*                                                                         */
/* Parameters: dh (IN) A handle obtained via SLPDatabaseOpen()             */
/*                                                                         */
/* Returns: Pointer to a SLPDatabase entry or NULL if end of enumeration   */
/*          has been reached.                                              */
/*=========================================================================*/


/*=========================================================================*/
void SLPDatabaseRewind(SLPDatabaseHandle dh);
/* Reset handle so SLPDatabaseEnum starts at the beginning again           */
/*                                                                         */
/* Parameters: eh (IN) A handle obtained via SLPDatabaseOpen()             */
/*                                                                         */
/* Returns: None                                                           */
/*=========================================================================*/


/*=========================================================================*/
void SLPDatabaseClose(SLPDatabaseHandle dh);
/* Closes a handle obtained from SLPDatabaseOpen()                         */
/*                                                                         */
/* Parameters: dh (IN) a handle obtained from SLPDatabaseOpenEnum()        */
/*                                                                         */
/* Returns: None                                                           */
/*=========================================================================*/

/*=========================================================================*/
void SLPDatabaseRemove(SLPDatabaseHandle dh, 
                       SLPDatabaseEntry* entry);
/* Removes the specified entry                                             */
/*                                                                         */
/* Parameters: dh         (IN) The SLPDatabaseEnumHandle used to obtain    */
/*                             the entry in the first place                */
/*             entry      (IN) The entry to remove                         */
/*                                                                         */
/* Returns: None                                                           */
/*                                                                         */
/* Note: During removal SLPDatabaseEntryDestroy() is called on entry.      */
/*       This means that you MUST NOT use entry after it is removed        */
/*=========================================================================*/


/*=========================================================================*/
void SLPDatabaseAdd(SLPDatabaseHandle dh,
                    SLPDatabaseEntry* entry);
/* Add the specified entry                                                 */
/*                                                                         */
/* Parameters: dh    (IN) handle obtained from SLPDatabseOpen()            */
/*             entry (IN) the entry to add                                 */
/*                                                                         */
/* Return: None                                                            */
/*                                                                         */
/* Note: DO NOT call SLPDatabaseEntryDestroy() on an entry that has been   */
/*       added to the database.  Instead call SLPDatabaseDeinit() or       */
/*       SLPDatabaseRemove() to free resources associated with an added    */
/*       entry                                                             */
/*=========================================================================*/


/*=========================================================================*/
int SLPDatabaseCount(SLPDatabaseHandle dh);
/* Returns the number of entries that are in the database                  */
/*=========================================================================*/


#endif
