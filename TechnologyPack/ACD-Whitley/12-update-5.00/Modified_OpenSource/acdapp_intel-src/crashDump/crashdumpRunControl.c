/****************************************************************
 ****************************************************************
 **                                                            **
 **    (C)Copyright 2009-2015, American Megatrends Inc.        **
 **                                                            **
 **            All Rights Reserved.                            **
 **                                                            **
 **        5555, Oakbrook Parkway, Norcross,                   **
 **                                                            **
 **        Georgia - 30093, USA. Phone-(770)-246-8600.         **
 **                                                            **
 ****************************************************************
 ****************************************************************/
/*****************************************************************
 *
 * parseargs.c
 * Source for parsing arguments for ACD app
 *
 * Author: Aruna Venkataraman <arunav@ami.com>
 *
 *****************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "crashdumpRunControl.h"

int dumpNow = -1;
crashdumpSections dumpSec = -1;
//int prnt_dbg = 0;

/* Path to only_one_instance() lock. */
static char ooi_path[DIR_NAME_LEN];
void *dl_handle = NULL;

static char *arglist[] =
{
        "?",
        "--dump-section",
        "--dump-now",
//        "--verbose-dbg",
        NULL
};


static int ShowUsage( int argc, char **argv, int index );
static int DumpSection( int argc, char **argv, int index );
static int DumpNow( int argc, char **argv, int index );
//static int VerboseDbgEnable( int argc, char **argv, int index );

static int ( *handlerList[] )( int, char **, int ) =
{
        ShowUsage,
        DumpSection,
        DumpNow,
//        VerboseDbgEnable,
    NULL
};

static int ShowUsage(int argc, char **argv, int index)
{
        if (0)
        {
                argc = argc;
                argv = argv;
                index = index;
        }

        printf ("\nAutonomous Crash Dump Tool:\n");
        printf ("\tThis tool collects the crash dump data when system failure events are detected.\n");
        printf ("\tOptions are available to collects the crash dump data manually.\n\n");
        printf( "Usage : Crashdump [arguments]\n" );
        printf( "Arguments: (optional)\n" );
        printf( "\t?:       \t\t\tPrints this help\n" );
        printf( "\t--dump-section <value>:       \tCollects and dumps only the specified section details\n" );
        printf( "\t\t Value: \n");
        printf( "\t\t\t 0: BigCore Crashdump\n");
        printf( "\t\t\t 1: BigCore SqDump\n");
        printf( "\t\t\t 2: Core MCA\n");
        printf( "\t\t\t 3: Uncore MCA\n");
        printf( "\t\t\t 4: Uncore Status\n");
        printf( "\t\t\t 5: TOR Dump\n");
        printf( "\t\t\t 6: Power Management Info\n");
        printf( "\t\t\t 7: Address Map\n");
        printf( "\t\t\t 8: Metadata\n");
        printf( "\t--dump-now: \t\t\tCollects the crashdump details right now and then the application exits\n" );
        printf( "\t\t\t\t\tThis argument can be used individually or in conjunction with --dump-section option\n" );
//        printf( "\t--verbose-dbg: \t\t\tPrints the debug logs on console\n" );
//        printf( "\t\t\t\t\tThis argument can be used individually or in conjunction with --dump-section or --dump-now option\n" );
        printf( "\n" );

        return 0;
}

static int DumpSection(int argc, char **argv, int index)
{
    if( index + 1 < argc )
        dumpSec = atoi(argv[index + 1]);
    else
    {
        fprintf( stderr, "Missing argument to --dump-section\n" );
        return 0;
    }

    return 1;
}

static int DumpNow(int argc, char **argv, int index)
{
        if (0)
        {
                argc = argc;
                argv = argv;
                index = index;
        }

    dumpNow = 1;
    return 0;
}

#if 0
static int VerboseDbgEnable(int argc, char **argv, int index)
{
        if (0)
        {
                argc = argc;
                argv = argv;
                index = index;
        }

    prnt_dbg = 1;
    return 0;
}
#endif

void parse_arguments( int argc, char **argv )
{
    int i, j;

    for( i = 1; i < argc; i++ )
    {
        j = 0;
        while( arglist[ j ] != NULL )
        {
            if( strcmp( argv[ i ], arglist[ j ] ) == 0 )
            {
                int retval;

                /* Match! Handle this argument (and skip the specified
                   number of arguments that were just handled) */
                retval = handlerList[ j ]( argc, argv, i );
                if( retval >= 0 )
                    i += retval;
                else
                {
                    fprintf( stderr, "Cannot handle argument: %s\n", arglist[ j ] );
                    return;
                }
            }
            j++;
        }
    }
}

static void ooi_unlink(void)
{
        unlink(ooi_path);
}

/* Exit if another instance of this program is running. */
void only_one_instance(const char *crashdump_json_dir)
{
        struct flock fl;
        int fd;

        snprintf(ooi_path, sizeof(ooi_path)-1, "%s/%s", crashdump_json_dir, ACD_INSTANCE_LOCK);
        fd = open(ooi_path, O_RDWR | O_CREAT, 0600);
        if (fd < 0)
        {
                printf("only_one_instance: open failed!\n");
                exit(1);
        }

        fl.l_start = 0;
        fl.l_len = 0;
        fl.l_type = F_WRLCK;
        fl.l_whence = SEEK_SET;
        if (fcntl(fd, F_SETLK, &fl) < 0)
        {
                printf("Another instance of this program is running.\n");
                exit(1);
        }

        /*
         * Run unlink(ooi_path) when the program exits. The program
         * always releases locks when it exits.
         */
        atexit(ooi_unlink);
}

