/***************************************************************************/
/*                                                                         */
/* Project:     OpenSLP - OpenSource implementation of Service Location    */
/*              Protocol                                                   */
/*                                                                         */
/* File:        slp_auth.c                                                 */
/*                                                                         */
/* Abstract:    Common for OpenSLP's SLPv2 authentication implementation   */
/*              Currently only bsd 0x0002 (DSA-SHA1) is supported          */
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

#include <time.h>

#include "slp_xmalloc.h"
#include "slp_auth.h"
#include "slp_crypto.h"

/*-------------------------------------------------------------------------*/
int SLPAuthDigestString(int spistrlen,
                        const char* spistr,
                        int stringlen,
                        const char* string,
                        unsigned long timestamp,
                        unsigned char* digest)
/*-------------------------------------------------------------------------*/
{
    int                 result;
    int                 tmpbufsize;
    unsigned char*      tmpbuf;
    unsigned char*      curpos;

    /* assume success */
    result = 0;

    /*-------------------------------------------------------*/
    /* Allocate temporary buffer for contiguous data         */
    /*-------------------------------------------------------*/
    /* +8 makes room for:  stringlen (2 bytes)               */
    /*                     autharray[i].spistrlen (2 bytes)  */
    /*                     timestamp 4 bytes                 */
    tmpbufsize = stringlen + spistrlen + 8;
    tmpbuf = xmalloc(tmpbufsize);
    if(tmpbuf == 0)
    {
        return SLP_ERROR_INTERNAL_ERROR;
    }
    
    /*-----------------------------------*/
    /* Copy data into continguous buffer */
    /*-----------------------------------*/
    curpos = tmpbuf;
    
    ToUINT16(curpos,spistrlen);
    curpos = curpos + 2;
    memcpy(curpos, spistr, spistrlen);
    curpos = curpos + spistrlen;

    ToUINT16(curpos,stringlen);
    curpos = curpos + 2;
    memcpy(curpos, string, stringlen);
    curpos = curpos + stringlen; 

    ToUINT32(curpos, timestamp);

    /*---------------------*/
    /* Generate the digest */
    /*---------------------*/
    if(SLPCryptoSHA1Digest(tmpbuf,
                           tmpbufsize,
                           digest))
    {
        result = SLP_ERROR_INTERNAL_ERROR;
    }

    /*------------------------------*/
    /* Cleanup the temporary buffer */
    /*------------------------------*/
    xfree(tmpbuf);

    return result;
}

/*-------------------------------------------------------------------------*/
int SLPAuthDigestDAAdvert(unsigned short spistrlen,
                          const char* spistr,
                          unsigned long timestamp,
                          unsigned long bootstamp,
                          unsigned short urllen,
                          const char* url,
                          unsigned short attrlistlen,
                          const char* attrlist,
                          unsigned short scopelistlen,
                          const char* scopelist,
                          unsigned short daspistrlen,
                          const char* daspistr,
                          unsigned char* digest)
/*-------------------------------------------------------------------------*/
{
    int                 result;
    int                 tmpbufsize;
    unsigned char*      tmpbuf;
    unsigned char*      curpos;

    /* assume success */
    result = 0;

    /*-------------------------------------------------------*/
    /* Allocate temporary buffer for contiguous data         */
    /*-------------------------------------------------------*/
    /* +18 makes room for:  spistrlen   (2 bytes)            */
    /*                      bootstamp   (4 bytes)            */
    /*                      urllen      (2 bytes)            */
    /*                      scopelistlen(2 bytes)            */
    /*                      attrlistlen (2 bytes)            */
    /*                      daspistrlen (2 bytes)            */
    /*                      timestamp   (4 bytes)            */
    tmpbufsize = spistrlen + urllen + scopelistlen + attrlistlen + daspistrlen + 18;
    tmpbuf = xmalloc(tmpbufsize);
    if(tmpbuf == 0)
    {
        return SLP_ERROR_INTERNAL_ERROR;
    }
    
    /*-----------------------------------*/
    /* Copy data into continguous buffer */
    /*-----------------------------------*/
    curpos = tmpbuf;

    ToUINT16(curpos,spistrlen);
    curpos += 2;
    memcpy(curpos, spistr, spistrlen);
    curpos += spistrlen;

    ToUINT32(curpos,bootstamp);
    curpos += 4;

    ToUINT16(curpos,urllen);
    curpos += 2;
    memcpy(curpos, url, urllen);
    curpos += urllen;      

    ToUINT16(curpos,scopelistlen);
    curpos += 2;
    memcpy(curpos, scopelist, scopelistlen);
    curpos += scopelistlen;

    ToUINT16(curpos,attrlistlen);
    curpos += 2;
    memcpy(curpos, attrlist, attrlistlen);
    curpos += attrlistlen;

    ToUINT16(curpos,daspistrlen);
    curpos += 2;
    memcpy(curpos, daspistr, daspistrlen);
    curpos += daspistrlen;

    ToUINT32(curpos,timestamp);

    /*---------------------*/
    /* Generate the digest */
    /*---------------------*/
    if(SLPCryptoSHA1Digest(tmpbuf,
                           tmpbufsize,
                           digest))
    {
        result = SLP_ERROR_INTERNAL_ERROR;
    }

    /*------------------------------*/
    /* Cleanup the temporary buffer */
    /*------------------------------*/
    xfree(tmpbuf);

    return result;
}


/*-------------------------------------------------------------------------*/
int SLPAuthSignDigest(int spistrlen,
                      const char* spistr,
                      SLPCryptoDSAKey* key,
                      unsigned char* digest,
                      int* authblocklen,
                      unsigned char** authblock)
/*-------------------------------------------------------------------------*/
{
    int                 signaturelen;
    int                 result;
    unsigned char*      curpos;
    
    /*----------------------------------------------*/
    /* Allocate memory for the authentication block */
    /*----------------------------------------------*/
    /* +10 makes room for:                          */
    /*     - the bsd (2 bytes)                      */
    /*     - the the authblock length (2 bytes)     */
    /*     - the spi string length (2 bytes)        */
    /*     - the timestamp (4 bytes)                */
    signaturelen = SLPCryptoDSASignLen(key);
    *authblocklen = spistrlen + signaturelen + 10;
    *authblock = (unsigned char*)xmalloc(*authblocklen);
    if(*authblock == 0)
    {
        result = SLP_ERROR_INTERNAL_ERROR;
        goto ERROR;
    }
    
    /*---------------------------------------------------------*/
    /* Fill in the Authblock with everything but the signature */
    /*---------------------------------------------------------*/
    curpos = *authblock;
    ToUINT16(curpos,0x0002); /* the BSD for DSA-SHA1 */
    curpos += 2;
    ToUINT16(curpos,*authblocklen);
    curpos += 2;
    ToUINT32(curpos,0xffffffff); /* very long expiration (for now) */
    curpos += 4;
    ToUINT16(curpos, spistrlen);
    curpos += 2;
    memcpy(curpos, spistr, spistrlen);
    curpos += spistrlen;

    /*---------------------------------------------*/
    /* Sign the digest and put it in the authblock */
    /*---------------------------------------------*/
    if( SLPCryptoDSASign(key,
                         digest,
                         SLPAUTH_SHA1_DIGEST_SIZE,
                         curpos,
                         &signaturelen))
    {
        result = SLP_ERROR_INTERNAL_ERROR;
        goto ERROR;
    }

    /*---------*/
    /* Success */
    /*---------*/ 
    
    return 0;


ERROR:
    
    /*-------------------------------*/
    /* Clean up and return errorcode */
    /*-------------------------------*/ 
    if(authblock) xfree(*authblock);
    *authblock = 0;
    *authblocklen = 0;
    
    return result;
}

/*-------------------------------------------------------------------------*/
int SLPVerifyDigest(SLPSpiHandle hspi,
                    int emptyisfail,
                    SLPCryptoDSAKey* key,
                    unsigned char* digest,
                    int authcount,
                    const SLPAuthBlock* autharray)
/*-------------------------------------------------------------------------*/
{
    int                 i;
    int                 signaturelen;
    int                 result;
    unsigned long       timestamp;
    
    /*-----------------------------------*/
    /* Should we fail on emtpy authblock */
    /*-----------------------------------*/
    if(emptyisfail)
    {
        result = SLP_ERROR_AUTHENTICATION_FAILED;
    }
    else
    {
        result = SLP_ERROR_OK;
    }  
    
    /*-----------------*/
    /* Get a timestamp */
    /*-----------------*/
    timestamp = time(NULL);

    /*------------------------------------------------------*/
    /* Iterate and check all authentication blocks          */
    /*------------------------------------------------------*/
    /* If any one of the authblocks can be verified then we */
    /* accept it                                            */
    for(i=0;i<authcount;i++)
    {
        /*-------------------------------*/
        /* Get a public key for the SPI  */
        /*-------------------------------*/
        key = SLPSpiGetDSAKey(hspi,
                              SLPSPI_KEY_TYPE_PUBLIC,
                              autharray[i].spistrlen,
                              autharray[i].spistr,
                              &key);
        
        /* Continue if we have a key and if the authenticator is not */
        /* timed out                                                 */
        if(key && timestamp <= autharray[i].timestamp)
        {
        
            /*------------------------------------------------------------*/
            /* Calculate the size of the DSA signature from the authblock */
            /*------------------------------------------------------------*/
            /* we have to calculate the signature length since            */
            /* autharray[i].length is (stupidly) the length of the entire */
            /* authblock                                                  */
            signaturelen = autharray[i].length - (autharray[i].spistrlen + 10);
    
            /*----------------------*/
            /* Verify the signature */
            /*----------------------*/
            if(SLPCryptoDSAVerify(key,
                                  digest,
                                  SLPAUTH_SHA1_DIGEST_SIZE,
                                  autharray[i].authstruct,
                                  signaturelen))
            {
                break;
            }

            result = SLP_ERROR_AUTHENTICATION_FAILED;
        }
    }
    
    return result;
}



/*=========================================================================*/
int SLPAuthVerifyString(SLPSpiHandle hspi,
                        int emptyisfail,
                        unsigned short stringlen,
                        const char* string,
                        int authcount,
                        const SLPAuthBlock* autharray)
/* Verify authenticity of  the specified attribute list                    */
/*                                                                         */
/* Parameters: hspi        (IN) open SPI handle                            */
/*             emptyisfail (IN) if non-zero, messages without authblocks   */
/*                              will fail                                  */
/*             stringlen   (IN) the length of string to verify             */
/*             string      (IN) the list to verify                         */
/*             authcount   (IN) the number of blocks in autharray          */
/*             autharray   (IN) array of authblocks                        */
/*                                                                         */
/* Returns: 0 on success or SLP_ERROR_xxx code on failure                  */
/*=========================================================================*/
{
    int                 i;
    int                 signaturelen;
    int                 result;
    unsigned long       timestamp;
    SLPCryptoDSAKey*    key = 0;
    unsigned char       digest[SLPAUTH_SHA1_DIGEST_SIZE];
    
    /*-----------------------------------*/
    /* Should we fail on emtpy authblock */
    /*-----------------------------------*/
    if(emptyisfail)
    {
        result = SLP_ERROR_AUTHENTICATION_FAILED;
    }
    else
    {
        result = SLP_ERROR_OK;
    }  
    
    /*-----------------*/
    /* Get a timestamp */
    /*-----------------*/
    timestamp = time(NULL);

    /*------------------------------------------------------*/
    /* Iterate and check all authentication blocks          */
    /*------------------------------------------------------*/
    /* If any one of the authblocks can be verified then we */
    /* accept it                                            */
    for(i=0;i<authcount;i++)
    {
        /*-------------------------------*/
        /* Get a public key for the SPI  */
        /*-------------------------------*/
        key = SLPSpiGetDSAKey(hspi,
                              SLPSPI_KEY_TYPE_PUBLIC,
                              autharray[i].spistrlen,
                              autharray[i].spistr,
                              &key);
        
        /* Continue if we have a key and if the authenticator is not */
        /* timed out                                                 */
        if(key && timestamp <= autharray[i].timestamp)
        {
        
            /*--------------------------*/
            /* Generate the SHA1 digest */
            /*--------------------------*/
            result = SLPAuthDigestString(autharray[i].spistrlen,
                                         autharray[i].spistr,
                                         stringlen,
                                         string,
                                         autharray[i].timestamp,
                                         digest);
            if(result == 0)
            {
                /*------------------------------------------------------------*/
                /* Calculate the size of the DSA signature from the authblock */
                /*------------------------------------------------------------*/
                /* we have to calculate the signature length since            */
                /* autharray[i].length is (stupidly) the length of the entire */
                /* authblock                                                  */
                signaturelen = autharray[i].length - (autharray[i].spistrlen + 10);
        
                /*----------------------*/
                /* Verify the signature */
                /*----------------------*/
                if(SLPCryptoDSAVerify(key,
                                      digest,
                                      sizeof(digest),
                                      autharray[i].authstruct,
                                      signaturelen))
                {
                    break;
                }
    
                result = SLP_ERROR_AUTHENTICATION_FAILED;
            }
        }
    }
    
    if(key) SLPCryptoDSAKeyDestroy(key);
   
    return result;
}


/*=========================================================================*/
int SLPAuthVerifyUrl(SLPSpiHandle hspi,
                     int emptyisfail,
                     const SLPUrlEntry* urlentry)
/* Verify authenticity of  the specified url entry                         */
/*                                                                         */
/* Parameters: hspi         (IN) open SPI handle                            */
/*             emptyisfail  (IN) if non-zero, messages without authblocks  */
/*                               will fail                                 */
/*             urlentry     (IN) the url entry to verify                   */
/*                                                                         */
/* Returns: 0 on success or SLP_ERROR_xxx code on failure                  */
/*=========================================================================*/
{
    return SLPAuthVerifyString(hspi,
                               emptyisfail,
                               urlentry->urllen,
                               urlentry->url,
                               urlentry->authcount,
                               urlentry->autharray);
}


/*=========================================================================*/
int SLPAuthVerifyDAAdvert(SLPSpiHandle hspi,
                          int emptyisfail,
                          const SLPDAAdvert* daadvert)
/* Verify authenticity of  the specified DAAdvert                          */
/*                                                                         */
/* Parameters: hspi        (IN) open SPI handle                            */
/*                         (IN) if non-zero, messages without authblocks   */
/*                              will fail                                  */
/*             spistrlen   (IN) length of the spi string                   */
/*             sprstr      (IN) the spi string                             */
/*             daadvert    (IN) the DAAdvert to verify                     */
/*                                                                         */
/* Returns: 0 on success or SLP_ERROR_xxx code on failure                  */
/*=========================================================================*/
{
    int                 i;
    int                 signaturelen;
    int                 result;
    unsigned long       timestamp;
    const SLPAuthBlock* autharray;
    int                 authcount;
    SLPCryptoDSAKey*    key = 0;
    unsigned char       digest[SLPAUTH_SHA1_DIGEST_SIZE];
    
    /*-----------------------------------*/
    /* Should we fail on emtpy authblock */
    /*-----------------------------------*/
    if(emptyisfail)
    {
        result = SLP_ERROR_AUTHENTICATION_FAILED;
    }
    else
    {
        result = SLP_ERROR_OK;
    }  
    
    /*-----------------*/
    /* Get a timestamp */
    /*-----------------*/
    timestamp = time(NULL);

    /*------------------------------------------------------*/
    /* Iterate and check all authentication blocks          */
    /*------------------------------------------------------*/
    /* If any one of the authblocks can be verified then we */
    /* accept it                                            */
    authcount = daadvert->authcount;
    autharray = daadvert->autharray;
    for(i=0;i<authcount;i++)
    {
        /*-------------------------------*/
        /* Get a public key for the SPI  */
        /*-------------------------------*/
        key = SLPSpiGetDSAKey(hspi,
                              SLPSPI_KEY_TYPE_PUBLIC,
                              autharray[i].spistrlen,
                              autharray[i].spistr,
                              &key);
        
        /* Continue if we have a key and if the authenticator is not */
        /* timed out                                                 */
        if(key && timestamp <= autharray[i].timestamp)
        {
        
            /*--------------------------*/
            /* Generate the SHA1 digest */
            /*--------------------------*/
            result = SLPAuthDigestDAAdvert(autharray[i].spistrlen,
                                           autharray[i].spistr,
                                           autharray[i].timestamp,
                                           daadvert->bootstamp,
                                           daadvert->urllen,
                                           daadvert->url,
                                           daadvert->attrlistlen,
                                           daadvert->attrlist,
                                           daadvert->scopelistlen,
                                           daadvert->scopelist,
                                           daadvert->spilistlen,
                                           daadvert->spilist,
                                           digest);
            if(result == 0)
            {
                /*------------------------------------------------------------*/
                /* Calculate the size of the DSA signature from the authblock */
                /*------------------------------------------------------------*/
                /* we have to calculate the signature length since            */
                /* autharray[i].length is (stupidly) the length of the entire */
                /* authblock                                                  */
                signaturelen = autharray[i].length - (autharray[i].spistrlen + 10);
        
                /*----------------------*/
                /* Verify the signature */
                /*----------------------*/
                if(SLPCryptoDSAVerify(key,
                                      digest,
                                      sizeof(digest),
                                      autharray[i].authstruct,
                                      signaturelen))
                {
                    break;
                }
    
                result = SLP_ERROR_AUTHENTICATION_FAILED;
            }
        }
    }
    
    if(key) SLPCryptoDSAKeyDestroy(key);
   
    return result;
}


/*=========================================================================*/
int SLPAuthVerifySAAdvert(SLPSpiHandle hspi,
                          int emptyisfail,
                          const SLPSAAdvert* saadvert)
/* Verify authenticity of  the specified SAAdvert                          */
/*                                                                         */
/* Parameters: hspi        (IN) open SPI handle                            */
/*             emptyisfail (IN) if non-zero, messages without authblocks   */
/*                              will fail                                  */
/*             spistrlen   (IN) length of the spi string                   */
/*             sprstr      (IN) the spi string                             */
/*             saadvert    (IN) the SAADVERT to verify                     */
/*                                                                         */
/* Returns: 0 on success or SLP_ERROR_xxx code on failure                  */
/*=========================================================================*/
{
    return 0;
}


/*=========================================================================*/
int SLPAuthSignString(SLPSpiHandle hspi,
                      int spistrlen,
                      const char* spistr,
                      unsigned short stringlen,
                      const char* string,
                      int* authblocklen,
                      unsigned char** authblock)
/* Generate an authblock signature for an attribute list                   */
/*                                                                         */
/* Parameters: hspi         (IN) open SPI handle                           */
/*             spistrlen    (IN) length of the SPI string                  */
/*             spistr       (IN) SPI to sign with                          */
/*             attrlistlen  (IN) the length of the URL to sign             */
/*             attrlist     (IN) the url to sign                           */
/*             authblocklen (OUT) the length of the authblock signature    */
/*             authblock    (OUT) buffer containing authblock signature    */
/*                                must be freed by the caller              */ 
/*                                                                         */
/* Returns: 0 on success or SLP_ERROR_xxx code on failure                  */
/*=========================================================================*/
{
    int                 result;
    SLPCryptoDSAKey*    key;
    unsigned char       digest[20];
    int                 defaultspistrlen    = 0;
    char*               defaultspistr       = 0;
    
    /* NULL out the authblock and spistr just to be safe */
    key = 0;
    *authblock = 0;
    *authblocklen = 0;
    spistr = 0;
    spistrlen = 0;

    /*--------------------------------*/
    /* Get a private key for the SPI  */
    /*--------------------------------*/
    if(spistr)
    {
        key = SLPSpiGetDSAKey(hspi,
                              SLPSPI_KEY_TYPE_PRIVATE,
                              spistrlen, 
                              spistr, 
                              &key);
    }
    else
    {
        if(SLPSpiGetDefaultSPI(hspi,
                               SLPSPI_KEY_TYPE_PRIVATE,
                               &defaultspistrlen,
                               &defaultspistr))
        {
            spistr = defaultspistr;
            spistrlen = defaultspistrlen;

            key = SLPSpiGetDSAKey(hspi,
                                  SLPSPI_KEY_TYPE_PRIVATE,
                                  spistrlen,
                                  spistr,
                                  &key);
        }
    }

    if(key == 0)
    {
        result = SLP_ERROR_AUTHENTICATION_UNKNOWN;
        goto ERROR;
    }
     
    /*--------------------------*/
    /* Generate the SHA1 digest */
    /*--------------------------*/
    result = SLPAuthDigestString(spistrlen,
                                 spistr,
                                 stringlen,
                                 string,
                                 0xffffffff, /* very long expiration (for now) */
                                 digest);
    
    /*---------------------------------------------*/
    /* Sign the digest and put it in the authblock */
    /*---------------------------------------------*/
    if(result == 0)
    {
        result = SLPAuthSignDigest(spistrlen,
                                   spistr,
                                   key,
                                   digest,
                                   authblocklen,
                                   authblock);
    }

ERROR:
    /*---------*/
    /* Cleanup */
    /*---------*/ 
    if(defaultspistr) xfree(defaultspistr);
    if(key) SLPCryptoDSAKeyDestroy(key);
    return result;
}


/*=========================================================================*/
int SLPAuthSignUrl(SLPSpiHandle hspi,
                   int spistrlen,
                   const char* spistr,
                   unsigned short urllen,
                   const char* url,
                   int* authblocklen,
                   unsigned char** authblock)
/* Generate an authblock signature for a Url                               */
/*                                                                         */
/* Parameters: hspi         (IN) open SPI handle                           */
/*             spistrlen    (IN) length of the SPI string                  */
/*             spistr       (IN) SPI to sign with                          */
/*             urllen       (IN) the length of the URL to sign             */
/*             url          (IN) the url to sign                           */
/*             authblocklen (OUT) the length of the authblock signature    */
/*             authblock    (OUT) buffer containing authblock signature    */
/*                                must be freed by the caller              */
/*                                                                         */
/* Returns: 0 on success or SLP_ERROR_xxx code on failure                  */
/*=========================================================================*/
{
    return  SLPAuthSignString(hspi,
                              spistrlen,
                              spistr,
                              urllen,
                              url,
                              authblocklen,
                              authblock);
}


/*=========================================================================*/
int SLPAuthSignDAAdvert(SLPSpiHandle hspi,
                        unsigned short spistrlen,
                        const char* spistr,
                        unsigned long bootstamp,
                        unsigned short urllen,
                        const char* url,
                        unsigned short attrlistlen,
                        const char* attrlist,
                        unsigned short scopelistlen,
                        const char* scopelist,
                        unsigned short daspistrlen,
                        const char* daspistr,
                        int* authblocklen,
                        unsigned char** authblock)
/* Generate an authblock signature for a DAADVERT                          */
/*                                                                         */
/* Parameters: hspi         (IN) open SPI handle                           */
/*             spistrlen (IN) length of the spi string                     */
/*             sprstr (IN) the spi string                                  */
/*             bootstamp (IN) the statless DA boot timestamp               */
/*             urllen (IN) the length of the URL to sign                   */
/*             url (IN) the url to sign                                    */
/*             attrlistlen (IN) the length of the URL to sign              */
/*             attrlist (IN) the url to sign                               */
/*             scopelistlen (IN) the length of the DA's scope list         */
/*             scopelist (IN) the DA's scope list                          */
/*             daspistrlen (IN) the length of the list of DA's SPIs        */
/*             daspistr (IN) the list of the DA's SPI's                    */
/*             authblocklen (OUT) the length of the authblock signature    */
/*             authblock (OUT) buffer containing authblock signature must  */
/*                             be freed by the caller                      */
/*                                                                         */
/* Returns: 0 on success or SLP_ERROR_xxx code on failure                  */
/*=========================================================================*/
{
    int                 result;
    SLPCryptoDSAKey*    key;
    unsigned char       digest[20];
    int                 defaultspistrlen    = 0;
    char*               defaultspistr       = 0;
    
    /* NULL out the authblock and spistr just to be safe */
    key = 0;
    *authblock = 0;
    *authblocklen = 0;
    spistr = 0;
    spistrlen = 0;

    /*--------------------------------*/
    /* Get a private key for the SPI  */
    /*--------------------------------*/
    if(spistr)
    {
        key = SLPSpiGetDSAKey(hspi,
                              SLPSPI_KEY_TYPE_PRIVATE,
                              spistrlen, 
                              spistr, 
                              &key);
    }
    else
    {
        if(SLPSpiGetDefaultSPI(hspi,
                               SLPSPI_KEY_TYPE_PRIVATE,
                               &defaultspistrlen,
                               &defaultspistr))
        {
            spistr = defaultspistr;
            spistrlen = defaultspistrlen;

            key = SLPSpiGetDSAKey(hspi,
                                  SLPSPI_KEY_TYPE_PRIVATE,
                                  spistrlen,
                                  spistr,
                                  &key);
        }
    }

    if(key == 0)
    {
        result = SLP_ERROR_AUTHENTICATION_UNKNOWN;
        goto ERROR;
    }
     
    /*--------------------------*/
    /* Generate the SHA1 digest */
    /*--------------------------*/
    result = SLPAuthDigestDAAdvert(spistrlen,
                                   spistr,
                                   0xffffffff,
                                   bootstamp,
                                   urllen,
                                   url,
                                   attrlistlen,
                                   attrlist,
                                   scopelistlen,
                                   scopelist,
                                   daspistrlen,
                                   daspistr,
                                   digest);

    /*---------------------------------------------*/
    /* Sign the digest and put it in the authblock */
    /*---------------------------------------------*/
    if(result == 0)
    {
        result = SLPAuthSignDigest(spistrlen,
                                   spistr,
                                   key,
                                   digest,
                                   authblocklen,
                                   authblock);
    }

ERROR:
    /*---------*/
    /* Cleanup */
    /*---------*/ 
    if(defaultspistr) xfree(defaultspistr);
    if(key) SLPCryptoDSAKeyDestroy(key);
    return result;
}


/*=========================================================================*/
int SLPAuthSignSAAdvert(unsigned short spistrlen,
                        const char* spistr,
                        unsigned short urllen,
                        const char* url,
                        unsigned short attrlistlen,
                        const char* attrlist,
                        unsigned short scopelistlen,
                        const char* scopelist,
                        int* authblocklen,
                        unsigned char** authblock)
/* Generate an authblock signature for a SAADVERT                          */
/*                                                                         */
/* Parameters: spistrlen (IN) length of the spi string                     */
/*             sprstr (IN) the spi string                                  */
/*             urllen (IN) the length of the URL to sign                   */
/*             url (IN) the url to sign                                    */
/*             attrlistlen (IN) the length of the URL to sign              */
/*             attrlist (IN) the url to sign                               */
/*             scopelistlen (IN) the length of the DA's scope list         */
/*             scopelist (IN) the DA's scope list                          */
/*             authblocklen (OUT) the length of the authblock signature    */
/*             authblock (OUT) buffer containing authblock signature       */
/*                                                                         */
/* Returns: 0 on success or SLP_ERROR_xxx code on failure                  */
/*=========================================================================*/
{
    *authblocklen = 0;
    *authblock = 0;
    return 0;
}

