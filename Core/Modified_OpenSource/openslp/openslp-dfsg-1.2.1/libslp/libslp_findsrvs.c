/***************************************************************************/
/*                                                                         */
/* Project:     OpenSLP - OpenSource implementation of Service Location    */
/*              Protocol Version 2                                         */
/*                                                                         */
/* File:        slplib_findsrvs.c                                          */
/*                                                                         */
/* Abstract:    Implementation for SLPFindSrvs() call.                     */
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

#include "slp.h"
#include "libslp.h"

/*-------------------------------------------------------------------------*/
SLPBoolean ColateSLPSrvURLCallback(SLPHandle hSLP,
                                   const char* pcSrvURL,
                                   unsigned short sLifetime,
                                   SLPError errCode,
                                   void *pvCookie)
/*-------------------------------------------------------------------------*/
{
    SLPSrvUrlColatedItem*   collateditem;
    PSLPHandleInfo          handle;
        
    handle = (PSLPHandleInfo) hSLP;
    handle->callbackcount ++;
    
#ifdef ENABLE_ASYNC_API
    /* Do not colate for async calls */
    if(handle->isAsync)
    {
        return handle->params.findsrvs.callback(hSLP,
                                                pcSrvURL,
                                                sLifetime,
                                                errCode,
                                                pvCookie);
    }
#endif
    
    if(errCode == SLP_LAST_CALL || 
       handle->callbackcount > SLPPropertyAsInteger(SLPGetProperty("net.slp.maxResults")))
    {
        /* We are done so call the caller's callback for each      */
        /* service URL colated item and clean up the colation list */
        handle->params.findsrvs.callback((SLPHandle)handle,
                                         NULL,
                                         0,
                                         SLP_LAST_CALL,
                                         handle->params.findsrvs.cookie);
        goto CLEANUP;
    }
    else if(errCode != SLP_OK)
    {
        return SLP_TRUE;
    }

    /* Add the service URL to the colation list */
    collateditem = (SLPSrvUrlColatedItem*) handle->collatedsrvurls.head;
    while(collateditem)
    {
        if(strcmp(collateditem->srvurl,pcSrvURL) == 0)
        {
            break;
        }
        collateditem = (SLPSrvUrlColatedItem*)collateditem->listitem.next;
    }
                                     
    /* create a new item if none was found */
    if(collateditem == NULL)
    {
        collateditem = (SLPSrvUrlColatedItem*) xmalloc(sizeof(SLPSrvUrlColatedItem) + \
                                                       strlen(pcSrvURL) + 1);
        if(collateditem)
        {
            memset(collateditem,0,sizeof(SLPSrvUrlColatedItem));
            collateditem->srvurl = (char*)(collateditem + 1);
            strcpy(collateditem->srvurl,pcSrvURL);
            collateditem->lifetime = sLifetime;

            /* Add the new item to the collated list */
            SLPListLinkTail(&(handle->collatedsrvurls),
                            (SLPListItem*)collateditem);

            /* Call the caller's callback */
            if(handle->params.findsrvs.callback((SLPHandle)handle,
                                                pcSrvURL,
                                                sLifetime,
                                                SLP_OK,
                                                handle->params.findsrvs.cookie) == SLP_FALSE)
            {
                goto CLEANUP;
            }
        }
    }
    
    return SLP_TRUE;

CLEANUP:
    /* free the collation list */
    while(handle->collatedsrvurls.count)
    {
        collateditem = (SLPSrvUrlColatedItem*)SLPListUnlink(&(handle->collatedsrvurls),
                                                            handle->collatedsrvurls.head);
        xfree(collateditem);
    }   
    handle->callbackcount = 0;

    return SLP_FALSE;
}

/*-------------------------------------------------------------------------*/
SLPBoolean ProcessSrvRplyCallback(SLPError errorcode,
                                  struct sockaddr_in* peerinfo,
                                  SLPBuffer replybuf,
                                  void* cookie)
/*-------------------------------------------------------------------------*/
{
    int             i;
    SLPUrlEntry*    urlentry;
    SLPMessage      replymsg;
    PSLPHandleInfo  handle      = (PSLPHandleInfo) cookie;
    SLPBoolean      result      = SLP_TRUE;

#ifdef ENABLE_SLPv2_SECURITY  
    int             securityenabled;
    securityenabled = SLPPropertyAsBoolean(SLPGetProperty("net.slp.securityEnabled"));
#endif

    /*-------------------------------------------*/
    /* Check the errorcode and bail if it is set */
    /*-------------------------------------------*/
    if(errorcode != SLP_OK)
    {
        return ColateSLPSrvURLCallback((SLPHandle)handle,
                                       0,
                                       0,
                                       errorcode,
                                       handle->params.findsrvs.cookie);
    }

    /*--------------------*/
    /* Parse the replybuf */
    /*--------------------*/
    replymsg = SLPMessageAlloc();
    if(replymsg)
    {
        if(SLPMessageParseBuffer(peerinfo,replybuf,replymsg) == 0)
        {
            if(replymsg->header.functionid == SLP_FUNCT_SRVRPLY &&
               replymsg->body.srvrply.errorcode == 0)
            {
                urlentry = replymsg->body.srvrply.urlarray;
            
                for(i=0;i<replymsg->body.srvrply.urlcount;i++)
                {
                    
#ifdef ENABLE_SLPv2_SECURITY
                    /*-------------------------------*/
                    /* Validate the authblocks       */
                    /*-------------------------------*/
                    if(securityenabled &&
                       SLPAuthVerifyUrl(handle->hspi,
                                        1,
                                        &(urlentry[i])))
                    {
                        /* authentication failed skip this URLEntry */
                        continue;
                    }
#endif
                    /*--------------------------------*/
                    /* Send the URL to the API caller */
                    /*--------------------------------*/
                    /* TRICKY: null terminate the url by setting the authcount to 0 */
                    ((char*)(urlentry[i].url))[urlentry[i].urllen] = 0;
    
                    result = ColateSLPSrvURLCallback((SLPHandle)handle,
                                                     urlentry[i].url,
                                                     (unsigned short)urlentry[i].lifetime,
                                                     SLP_OK,
                                                     handle->params.findsrvs.cookie);
                    if(result == SLP_FALSE)
                    {
                        break;
                    }
                } 
            }
            else if(replymsg->header.functionid == SLP_FUNCT_DAADVERT &&
                    replymsg->body.daadvert.errorcode == 0)
            {
#ifdef ENABLE_SLPv2_SECURITY
                if(securityenabled &&
                   SLPAuthVerifyDAAdvert(handle->hspi,
                                         1,
                                         &(replymsg->body.daadvert)))
                {
                    /* Verification failed. Ignore message */
                    SLPMessageFree(replymsg);
                    return SLP_TRUE;
                }
#endif

                ((char*)(replymsg->body.daadvert.url))[replymsg->body.daadvert.urllen] = 0;
                result = ColateSLPSrvURLCallback((SLPHandle)handle,
                                                 replymsg->body.daadvert.url,
                                                 SLP_LIFETIME_MAXIMUM,
                                                 SLP_OK,
                                                 handle->params.findsrvs.cookie);
            }
            else if(replymsg->header.functionid == SLP_FUNCT_SAADVERT)
            {

#ifdef ENABLE_SLPv2_SECURITY
                if(securityenabled &&
                   SLPAuthVerifySAAdvert(handle->hspi,
                                         1,
                                         &(replymsg->body.saadvert)))
                {
                    /* Verification failed. Ignore message */
                    SLPMessageFree(replymsg);
                    return SLP_TRUE;
                }
#endif

                ((char*)(replymsg->body.saadvert.url))[replymsg->body.saadvert.urllen] = 0;
                result = ColateSLPSrvURLCallback((SLPHandle)handle,
                                                 replymsg->body.saadvert.url,
                                                 SLP_LIFETIME_MAXIMUM,
                                                 SLP_OK,
                                                 handle->params.findsrvs.cookie);

            }
        }
        
        SLPMessageFree(replymsg);
    }
    
    return result;
}


/*-------------------------------------------------------------------------*/
SLPError ProcessSrvRqst(PSLPHandleInfo handle)
/*-------------------------------------------------------------------------*/
{
    struct sockaddr_in  peeraddr;
    int                 sock        = -1;
    int                 bufsize     = 0;
    char*               buf         = 0;
    char*               curpos      = 0;
    SLPError            result      = 0;

#ifdef ENABLE_SLPv2_SECURITY
    int                 spistrlen   = 0;
    char*               spistr      = 0;
#endif

    /*------------------------------------------*/
    /* Is this a special attempt to locate DAs? */
    /*------------------------------------------*/
    if(strncasecmp(handle->params.findsrvs.srvtype,
                   SLP_DA_SERVICE_TYPE,
                   handle->params.findsrvs.srvtypelen) == 0)
    {
        KnownDAProcessSrvRqst(handle);
        goto FINISHED;
    }

#ifdef ENABLE_SLPv2_SECURITY
    if(SLPPropertyAsBoolean(SLPGetProperty("net.slp.securityEnabled")))
    {
        SLPSpiGetDefaultSPI(handle->hspi,
                            SLPSPI_KEY_TYPE_PUBLIC,
                            &spistrlen,
                            &spistr);
    }
#endif

    /*-------------------------------------------------------------------*/
    /* determine the size of the fixed portion of the SRVRQST            */
    /*-------------------------------------------------------------------*/
    bufsize  = handle->params.findsrvs.srvtypelen + 2;   /*  2 bytes for len field */
    bufsize += handle->params.findsrvs.scopelistlen + 2; /*  2 bytes for len field */
    bufsize += handle->params.findsrvs.predicatelen + 2; /*  2 bytes for len field */
    bufsize += 2;    /*  2 bytes for spistr len*/
#ifdef ENABLE_SLPv2_SECURITY
    bufsize += spistrlen;
#endif
    
    buf = curpos = (char*)xmalloc(bufsize);
    if(buf == 0)
    {
        result = SLP_MEMORY_ALLOC_FAILED;
        goto FINISHED;
    }

    /*------------------------------------------------------------*/
    /* Build a buffer containing the fixed portion of the SRVRQST */
    /*------------------------------------------------------------*/
    /* service type */
    ToUINT16(curpos,handle->params.findsrvs.srvtypelen);
    curpos = curpos + 2;
    memcpy(curpos,
           handle->params.findsrvs.srvtype,
           handle->params.findsrvs.srvtypelen);
    curpos = curpos + handle->params.findsrvs.srvtypelen;
    /* scope list */
    ToUINT16(curpos,handle->params.findsrvs.scopelistlen);
    curpos = curpos + 2;
    memcpy(curpos,
           handle->params.findsrvs.scopelist,
           handle->params.findsrvs.scopelistlen);
    curpos = curpos + handle->params.findsrvs.scopelistlen;
    /* predicate */
    ToUINT16(curpos,handle->params.findsrvs.predicatelen);
    curpos = curpos + 2;
    memcpy(curpos,
           handle->params.findsrvs.predicate,
           handle->params.findsrvs.predicatelen);
    curpos = curpos + handle->params.findsrvs.predicatelen;
#ifdef ENABLE_SLPv2_SECURITY
    ToUINT16(curpos,spistrlen);
    curpos = curpos + 2;
    memcpy(curpos,spistr,spistrlen);
    curpos = curpos + spistrlen;
#else
    ToUINT16(curpos,0);
#endif

    /*--------------------------*/
    /* Call the RqstRply engine */
    /*--------------------------*/
    do
    {

        #ifndef UNICAST_NOT_SUPPORTED
	if ( handle->dounicast == 1 ) 
	{
	    void *cookie = (PSLPHandleInfo) handle;
	    result = NetworkUcastRqstRply(handle,
                                          buf,
                                          SLP_FUNCT_SRVRQST,
					  bufsize,
					  ProcessSrvRplyCallback,
                                          cookie);
            break;
	}
	else
	#endif
	if(strncasecmp(handle->params.findsrvs.srvtype,
                       SLP_SA_SERVICE_TYPE,
                       handle->params.findsrvs.srvtypelen))
        {
            sock = NetworkConnectToDA(handle,
                                      handle->params.findsrvs.scopelist,
                                      handle->params.findsrvs.scopelistlen,
                                      &peeraddr);
        }

        if(sock == -1)
        {
            /* use multicast as a last resort */
            #ifndef MI_NOT_SUPPORTED
	    result = NetworkMcastRqstRply(handle,
					  buf,
					  SLP_FUNCT_SRVRQST,
					  bufsize,
					  ProcessSrvRplyCallback,
					  NULL);
	    #else		
	    result = NetworkMcastRqstRply(handle->langtag,
                                          buf,
                                          SLP_FUNCT_SRVRQST,
                                          bufsize,
                                          ProcessSrvRplyCallback,
                                          handle);
            #endif /* MI_NOT_SUPPORTED */
	    break;
        }

        result = NetworkRqstRply(sock,
                                 &peeraddr,
                                 handle->langtag,
                                 0,
                                 buf,
                                 SLP_FUNCT_SRVRQST,
                                 bufsize,
                                 ProcessSrvRplyCallback,
                                 handle);
        if(result)
        {
            NetworkDisconnectDA(handle);
        }

    }while(result == SLP_NETWORK_ERROR);

    FINISHED:
    if(buf) xfree(buf);
#ifdef ENABLE_SLPv2_SECURITY
    if(spistr) xfree(spistr);
#endif

    return result;
}   


#ifdef ENABLE_ASYNC_API
/*-------------------------------------------------------------------------*/ 
SLPError AsyncProcessSrvRqst(PSLPHandleInfo handle)
/*-------------------------------------------------------------------------*/
{
    SLPError result = ProcessSrvRqst(handle);
    xfree((void*)handle->params.findsrvs.srvtype);
    xfree((void*)handle->params.findsrvs.scopelist);
    xfree((void*)handle->params.findsrvs.predicate);
    handle->inUse = SLP_FALSE;
    return result;
}
#endif


/*=========================================================================*/
SLPError SLPAPI SLPFindSrvs(SLPHandle  hSLP,
                     const char *pcServiceType,
                     const char *pcScopeList,
                     const char *pcSearchFilter,
                     SLPSrvURLCallback callback,
                     void *pvCookie)
/*                                                                         */
/* Issue the query for services on the language specific SLPHandle and     */
/* return the results through the callback.  The parameters determine      */
/* the results                                                             */
/*                                                                         */
/* hSLP             The language specific SLPHandle on which to search for */
/*                  services.                                              */
/*                                                                         */
/* pcServiceType    The Service Type String, including authority string if */
/*                  any, for the request, such as can be discovered using  */
/*                  SLPSrvTypes(). This could be, for example              */
/*                  "service:printer:lpr" or "service:nfs".  May not be    */
/*                  the empty string or NULL.                              */
/*                                                                         */
/*                                                                         */
/* pcScopeList      A pointer to a char containing comma separated list of */
/*                  scope names.  Pass in the NULL or the empty string ""  */
/*                  to find services in all the scopes the local host is   */
/*                  configured query.                                      */
/*                                                                         */
/* pcSearchFilter   A query formulated of attribute pattern matching       */
/*                  expressions in the form of a LDAPv3 Search Filter.     */
/*                  If this filter is NULL or empty, i.e.  "", all         */
/*                  services of the requested type in the specified scopes */
/*                  are returned.                                          */
/*                                                                         */
/* callback         A callback function through which the results of the   */
/*                  operation are reported. May not be NULL                */
/*                                                                         */
/* pvCookie         Memory passed to the callback code from the client.    */
/*                  May be NULL.                                           */
/*                                                                         */
/* Returns:         If an error occurs in starting the operation, one of   */
/*                  the SLPError codes is returned.                        */
/*                                                                         */
/*=========================================================================*/
{
    PSLPHandleInfo      handle;
    SLPError            result;

    /*------------------------------*/
    /* check for invalid parameters */
    /*------------------------------*/
    if(hSLP            == 0 ||
       *(unsigned int*)hSLP != SLP_HANDLE_SIG ||
       pcServiceType   == 0 ||
       *pcServiceType  == 0 ||  /* srvtype can't be empty string */
       callback        == 0)
    {
        return SLP_PARAMETER_BAD;
    }


    /*-----------------------------------------*/
    /* cast the SLPHandle into a SLPHandleInfo */
    /*-----------------------------------------*/
    handle = (PSLPHandleInfo)hSLP; 

    /*-----------------------------------------*/
    /* Check to see if the handle is in use    */
    /*-----------------------------------------*/
    if(handle->inUse == SLP_TRUE)
    {
        return SLP_HANDLE_IN_USE;
    }
    handle->inUse = SLP_TRUE;


    /*-------------------------------------------*/
    /* Set the handle up to reference parameters */
    /*-------------------------------------------*/
    handle->params.findsrvs.srvtypelen   = strlen(pcServiceType);
    handle->params.findsrvs.srvtype      = pcServiceType;
    if(pcScopeList && *pcScopeList)
    {
        handle->params.findsrvs.scopelistlen = strlen(pcScopeList);
        handle->params.findsrvs.scopelist    = pcScopeList;
    }
    else
    {
        handle->params.findsrvs.scopelist    = SLPGetProperty("net.slp.useScopes");
        handle->params.findsrvs.scopelistlen = strlen(handle->params.findsrvs.scopelist);
    }

    if(pcSearchFilter)
    {
        handle->params.findsrvs.predicatelen = strlen(pcSearchFilter);
        handle->params.findsrvs.predicate    = pcSearchFilter;
    }
    else
    {
        handle->params.findsrvs.predicatelen = 0;
        handle->params.findsrvs.predicate  = (char*)&handle->params.findsrvs.predicatelen;
    }
    handle->params.findsrvs.callback     = callback;
    handle->params.findsrvs.cookie       = pvCookie; 


    /*----------------------------------------------*/
    /* Check to see if we should be async or sync   */
    /*----------------------------------------------*/
#ifdef ENABLE_ASYNC_API
    if(handle->isAsync)
    {
        /* COPY all the referenced parameters */
        handle->params.findsrvs.srvtype = xstrdup(handle->params.findsrvs.srvtype);
        handle->params.findsrvs.scopelist = xstrdup(handle->params.findsrvs.scopelist);
        handle->params.findsrvs.predicate = xstrdup(handle->params.findsrvs.predicate);

        /* make sure strdups did not fail */
        if(handle->params.findsrvs.srvtype &&
           handle->params.findsrvs.scopelist &&
           handle->params.findsrvs.predicate)
        {
            result = ThreadCreate((ThreadStartProc)AsyncProcessSrvRqst,handle);
        }
        else
        {
            result = SLP_MEMORY_ALLOC_FAILED;    
        }

        if(result)
        {
            if(handle->params.findsrvs.srvtype) xfree((void*)handle->params.findsrvs.srvtype);
            if(handle->params.findsrvs.scopelist) xfree((void*)handle->params.findsrvs.scopelist);
            if(handle->params.findsrvs.predicate) xfree((void*)handle->params.findsrvs.predicate);
            handle->inUse = SLP_FALSE;
        }
    }
    else
#endif /* ifdef ENABLE_ASYNC_API */
    {
        /* Leave all parameters REFERENCED */

        result = ProcessSrvRqst(handle);

        handle->inUse = SLP_FALSE;
    }

    return result;
}

