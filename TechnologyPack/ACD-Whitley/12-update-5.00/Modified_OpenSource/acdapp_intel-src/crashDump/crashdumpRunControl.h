/****************************************************************
 ****************************************************************
 **                                                            **
 **    (C)Copyright 2009-2020, American Megatrends Inc.        **
 **                                                            **
 **            All Rights Reserved.                            **
 **                                                            **
 **        5555, Oakbrook Parkway, Norcross,                   **
 **                                                            **
 **        Georgia - 30093, USA. Phone-(770)-246-8600.         **
 **                                                            **
 ****************************************************************
 ****************************************************************/

#ifndef _PARSEARGS_H_
#define _PARSEARGS_H_

#include <dlfcn.h>
#include <unistd.h>
#include <sys/file.h>

#define ACDPDK_LIB 		"/usr/local/lib/libacdcfgpdk.so"
/* Filename for only_one_instance() lock. */
#define ACD_INSTANCE_LOCK 	"crashdump-lock"
#define ERRCHECKMAXCOUNT        300 //30 seconds - (delay of 100ms)*300
#define DIR_NAME_LEN 		128

typedef enum
{
    bigcoreCrashdump,
    bigcoreSqdump,
    coreMca,
    uncoreMca,
    uncoreStatus,
    torDump,
    pmInfo,
    addrMap,
#ifdef CONFIG_SPX_FEATURE_OEMDATA_SECTION
    oemData,
#endif
    metaData,
} crashdumpSections;

extern crashdumpSections dumpSec;
extern void *dl_handle;
extern int dumpNow;

void parse_arguments( int argc, char **argv );
void only_one_instance(const char *crashdump_json_dir);

#endif
