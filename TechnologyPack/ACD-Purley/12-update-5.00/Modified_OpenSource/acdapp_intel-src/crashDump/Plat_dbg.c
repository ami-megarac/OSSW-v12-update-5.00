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

typedef unsigned long uint32_t;

#include "common.h"
#include "Commstat.h"
#ifndef SPX_BMC_ACD
#include <cJSON.h>
#else
#include <dlfcn.h>
#include <unistd.h>
#endif
#include <time.h>

#include "Plat_dbg.h"
#include "Crashdump.h"
#include "SysInfo.h"
#include "Metadata.h"
#include "UncoreMca.h"
#include "CoreMca.h"
#include "TorDump.h"
#include "PowerManagement.h"
#include "AddressMap.h"
#include "SqDump.h"
#include "UncoreStatus.h"

#include <sys/stat.h>
#include <semaphore.h>
#include <fcntl.h>
#include <errno.h>

#include <sys/time.h>
#include <sys/resource.h>

#include <pthread.h>
#include <dbgout.h>

#define CPU_TIME (getrusage(RUSAGE_SELF,&ruse), ruse.ru_utime.tv_sec + \
      ruse.ru_stime.tv_sec + 1e-6 * \
      (ruse.ru_utime.tv_usec + ruse.ru_stime.tv_usec))

#define TIME_SECTION

/* Interprocess semaphore for ACD PLATRST signal
 */
sem_t *ACD_PLATRST_sem = SEM_FAILED;
bool ACD_PLATRST_Abort = FALSE;

#ifndef SPX_BMC_ACD
/* Interprocess semaphore for CRASHDUMP signal
 */
sem_t *CRASHDUMP_sem = SEM_FAILED;
#define CRASHDUMP_SEM "/CRASHDUMP"

/* Interprocess semaphore for ACD PLATRST signal
 */
#define ACD_PLATRST_SEM "/ACD_PLATRST"
#else
int dumpNow = -1;
void *dl_handle = NULL;
int prnt_dbg = 0;

/* Path to only_one_instance() lock. */
static char ooi_path[DIR_NAME_LEN];
/* Filename for only_one_instance() lock. */
#define ACD_INSTANCE_LOCK "crashdump-lock"
#define ACDPDK_LIB "/usr/local/lib/libacdcfgpdk.so"
#define ERRCHECKMAXCOUNT	300 //30 seconds - (delay of 100ms)*300

/*Platform Reset Semaphore Name */
static char plt_rst_sem_name[DIR_NAME_LEN];
#endif

int saveArg = -1;

/******************************************************************************
*
*  Global Variables
*
******************************************************************************/
typedef struct ST {
    UINT32 DumpStatus;
} SYSTEMSTATUS;

SYSTEMSTATUS gDump;

static SDebugLogSection const sDebugLogTable[] =
{
    { "metadata", logMetadata},
    { "sys_info", logSysInfo},
    { "uncore_MCA", logUncoreMca},
    { "uncore_status_regs", logUncoreStatus},
    { "addr_map", logAddressMap},
    { "power_man", logPowerManagement},
    { "TOR_dump", logTorDump},
    { "core_MCA", logCoreMca},
    { "core_MLC_SQ", logSqDump},
    { "crashdump", logCrashdump},
};

/******************************************************************************
*
*  GetClockTime
*
******************************************************************************/
#ifdef TIME_SECTION
void GetClockTime(char* buffer, int iSize)
{
    time_t curtime;
    struct tm* loctime;

    curtime = time(NULL);
    loctime = localtime(&curtime);
    if (NULL != loctime) {
        strftime(buffer, iSize, "%H:%M:%S  %m/%d/%Y\n", loctime);
    }
}
#endif

#define MIN_STACKSIZE (0)

static pthread_t *sg_psLogThread = NULL;
static pthread_t thread_id;

void OSThreadDelete(pthread_t *Pthread_id)
{
    pthread_cancel(*Pthread_id);
}
ESTATUS CreateThread(UINT8 * name, size_t stacksize, void * (*Thread)(void *), void * Arg, void * pErr)
{
#ifdef SPX_BMC_ACD
    UN_USED(name);
    UN_USED(pErr);
#endif
    pthread_attr_t  threadAttr;
    pthread_attr_init( &threadAttr );
    pthread_attr_setdetachstate( &threadAttr, PTHREAD_CREATE_DETACHED );
    pthread_attr_setstacksize(&threadAttr, stacksize);
    if (0 != pthread_create (&thread_id, &threadAttr, Thread, Arg))
    {
        printf("Error creating threadn");
        pthread_attr_destroy(&threadAttr);
        return 1;
    }
    sg_psLogThread = &thread_id;
    pthread_attr_destroy(&threadAttr);
    return ST_OK;
}

// A separate thread of generating a platform debug log file.
static void * ACD_PLATRST_semaphore_monitor_Thread(void *pvArg)
{
    ACD_PLATRST_Abort = FALSE;
#ifdef SPX_BMC_ACD
    UN_USED(pvArg);
#endif
    while(1) {
        /* Initialize pseudo ACD_PLATRST signal using interprocess semaphore.
         * The named semaphore should have been created and initialized by
         * Error Interupt Handler funtion
         */
#ifndef SPX_BMC_ACD
        ACD_PLATRST_sem = sem_open(ACD_PLATRST_SEM, O_RDWR);
#else
        ACD_PLATRST_sem = sem_open(plt_rst_sem_name, O_RDWR);
#endif
        if (SEM_FAILED == ACD_PLATRST_sem) {
#ifndef SPX_BMC_ACD
            fprintf(stderr, "failed to open ACD_PLATRST interprocess semaphore \"%s\"\n", ACD_PLATRST_SEM);
#else
            fprintf(stderr, "failed to open ACD_PLATRST interprocess semaphore \"%s\"\n", plt_rst_sem_name);
#endif
            return NULL;
        }
        do {
            /* Flush any outstanding ACD_PLATRST events encountered (semaphore posted
             * via Error Interupt Handler funtion).
             */
            while(sem_trywait(ACD_PLATRST_sem) == 0) {
                ;
            }
            /* if it returns an error (EAGAIN), it should be because there was nothing
             * to decrement (already at 0).
             */
        } while (EINTR == errno);

        if (SEM_FAILED == ACD_PLATRST_sem) {
            return NULL;
        }
        do {
            /* Check to see if a ACD_PLATRST signal was encountered (semaphore posted in PDKInit.c).
             */
            while(sem_wait(ACD_PLATRST_sem) == 0) {
                PRINT(PRINT_DBG, PRINT_INFO, "Platform Debug - ACD Platform Reset Semaphore woke up!\n");
                ACD_PLATRST_Abort = TRUE;
                break;
            }

            /* if it returns an error (EAGAIN), it should be because there was nothing
             * to decrement (already at 0).
             */
        } while (EINTR == errno);
    }

    // ready for the log file retrieve
    sg_psLogThread = NULL;
    return NULL;
}

/******************************************************************************
*
*    doPlatformDebugLog
*
*    Dump the debug info into a log file.
*
******************************************************************************/
#ifndef SPX_BMC_ACD
static void doPlatformDebugLog(SPlatDBG* pSPlatDBG)
#else
static void doPlatformDebugLog(SPlatDBG* pSPlatDBG, char *crashdump_dir, char *crashdump_json_dir)
#endif
{
    FILE* fpRaw = NULL;
    FILE* fp = NULL;
    FILE* fpJson = NULL;
#ifdef TIME_SECTION
    char tempr[100];
#endif //TIME_SECTION
#ifdef BUILD_RAW
    char rawFilename[256];
#endif //BUILD_RAW
#ifdef BUILD_JSON
    char * out = NULL;
    cJSON * root = NULL;
    char jsonFilename[256];
#ifndef SPX_BMC_ACD
    char timeString[SECTION_NAME_LEN];
#else
    char timeString[DEBUG_SECTION_NAME_LEN];
#endif
#endif //BUILD_JSON
    cJSON * logSectionJson = NULL;
#ifndef SPX_BMC_ACD
    UINT32 i = 0;
#else
    int i = 0;
#endif
    int retVal =0;

    ESTATUS eStatus;

    switch (pSPlatDBG->ePlatDBGCmd) {
        case PLAT_DBG_ON_IERR:
        case PLAT_DBG_ON_ERR2:
        case PLAT_DBG_ON_SMI:
            // start the JSON tree for CPU dump
#ifdef BUILD_JSON
            root = cJSON_CreateObject();
#ifdef TIME_SECTION
            time_t totalStart, totalEnd;
            time(&totalStart);
            cJSON_AddStringToObject(root, DBG_TIME_ITEM_NAME, "");
#endif //TIME_SECTION
#endif //BUILD_JSON

            // open the text file for CPU dump
#ifdef BUILD_TXT
            if (pSPlatDBG->u8Arg[0x0f] == 0x61) {
#ifndef SPX_BMC_ACD
                fp = fopen(CRASHDUMP_DIRECTORY "/" RAM_CPUDUMP_FILE, "ae");
#else
                fp = fopen(crashdump_dir "/" RAM_CPUDUMP_FILE, "ae");
#endif
            } else {
#ifndef SPX_BMC_ACD
                fp = fopen(CRASHDUMP_DIRECTORY "/" RAM_CPUDUMP_FILE, "we");
#else
                fp = fopen(crashdump_dir "/" RAM_CPUDUMP_FILE, "we");
#endif
            }
#endif //BUILD_TXT

            // open the raw file for CPU dump
#ifdef BUILD_RAW
#ifndef SPX_BMC_ACD
       retVal=snprintf(rawFilename,sizeof(rawFilename), "%s/%s", CRASHDUMP_DIRECTORY, RAM_CPUDUMP_RAW_FILE);
       if((retVal < 0) || (retVal >= (signed)sizeof(rawFilename)))
                {
                        TCRIT(" Buffer over flow..\n");
                }
#else
       retVal=snprintf(rawFilename,sizeof(rawFilename), "%s/%s", crashdump_dir, RAM_CPUDUMP_RAW_FILE);
       if((retVal < 0) || (retVal >= (signed)sizeof(rawFilename)))
                {
                        TCRIT(" Buffer over flow..\n");
                }

#endif
            fpRaw = fopen(rawFilename, "wb");
#endif //BUILD_RAW

            // Loop through the list of log sections and generate each one
#ifndef SPX_BMC_ACD
            for (i = 0; i < (sizeof(sDebugLogTable) / sizeof(SDebugLogSection)); i++) {
#else
			for (i = 0; i < (int)((sizeof(sDebugLogTable) / sizeof(SDebugLogSection))); i++) {
#endif
                if ((-1 != saveArg) && (i != saveArg))
                {
                    continue;
                }
                // Print the starting time for this section
#ifdef TIME_SECTION
				double first, second;
				struct rusage ruse;
				time_t start, end;
				time(&start);
				first = CPU_TIME;

                memset_s(tempr, sizeof(tempr), 0);
                GetClockTime(tempr, 63);
                PRINT(PRINT_DBG, PRINT_ERROR, "Logging %s : %s\n", sDebugLogTable[i].sectionName, tempr);
                fflush(stdout);
#endif //TIME_SECTION

                // Create an empty JSON object for this section
#ifdef BUILD_JSON
                logSectionJson = cJSON_CreateObject();
                cJSON_AddStringToObject(logSectionJson, DBG_ABORTED_ITEM_NAME, "Valid");
                cJSON_AddStringToObject(logSectionJson, DBG_TIME_ITEM_NAME, "");
#endif //BUILD_JSON

                // Get the log for this section
                eStatus = ST_OK;
                if ((TRUE != ACD_PLATRST_Abort) && ((eStatus = sDebugLogTable[i].GetSectionLog(fpRaw, fp, logSectionJson)) != ST_OK)) {
                    PRINT(PRINT_DBG, PRINT_ERROR, "Error %d during %s log\n", eStatus, sDebugLogTable[i].sectionName);
                    gDump.DumpStatus |= (1 << i);
                }

#ifdef BUILD_JSON
                if (TRUE == ACD_PLATRST_Abort) {
                    // Add the aborted item to the JSON structure
                    cJSON_ReplaceItemInObject(logSectionJson, DBG_ABORTED_ITEM_NAME, cJSON_CreateString(DBG_ABORTED_STATUS));
                }

                // Add the _time item to the JSON structure
#ifdef TIME_SECTION
                // Save end time
                time(&end);
#ifndef SPX_BMC_ACD
                snprintf(timeString, SECTION_NAME_LEN, "%ds", (int)(end - start));
#else
                snprintf(timeString, DEBUG_SECTION_NAME_LEN, "%ds", (int)(end - start));
#endif
                cJSON_ReplaceItemInObject(logSectionJson, DBG_TIME_ITEM_NAME, cJSON_CreateString(timeString));
#endif //TIME_SECTION

                // If any child data was added to the JSON section, add it to the root
                if (logSectionJson->child != NULL) {
                    cJSON_AddItemToObject(root, sDebugLogTable[i].sectionName, logSectionJson);
                } else {
                    // Otherwise, since it won't be added to the root, delete it
                    cJSON_Delete(logSectionJson);
                }
                out = cJSON_Print(root);
                if (out != NULL) {
#ifndef SPX_BMC_ACD
                    sprintf(jsonFilename, "%s/%s", CRASHDUMP_DIRECTORY, RAM_CPUJSON_FILE);
#else
                    sprintf(jsonFilename, "%s/%s", crashdump_json_dir, RAM_CPUJSON_FILE);
#endif
                    fpJson = fopen(jsonFilename, "we");
                    if (fpJson != NULL) {
                        fprintf(fpJson, "%s", out);
                    }
                    cJSON_free(out);
                } else {
                    PRINT(PRINT_DBG, PRINT_ERROR, "cJSON_Print Failed\n");
                }
		if(fpJson != NULL) {
			fclose(fpJson);
			fpJson = NULL;
		}
#endif //BUILD_JSON

#ifdef TIME_SECTION
                // Print the completed time for this section
				// Save end time
				time(&end);
				second = CPU_TIME;
				printf("%s %s  cpu: %.6f secs  user: %d secs\n",
#ifdef BUILD_RAW
					"BUILD_RAW",
#else //BUILD_RAW
					" - ",
#endif //BUILD_RAW
#ifdef BUILD_JSON
					"BUILD_JSON",
#else //BUILD_JSON
					" - ",
#endif //BUILD_JSON
					(second - first), (int)(end - start));

                memset_s(tempr, sizeof(tempr), 0);
                GetClockTime(tempr, 63);
                PRINT(PRINT_DBG, PRINT_ERROR, "Finished logging %s : %s\n", sDebugLogTable[i].sectionName, tempr);
                fflush(stdout);
#endif //TIME_SECTION
            }

#ifdef BUILD_JSON
#ifdef TIME_SECTION
            // Save totalEnd time
            time(&totalEnd);
#ifndef SPX_BMC_ACD
            snprintf(timeString, SECTION_NAME_LEN, "%ds", (int)(totalEnd - totalStart));
#else
            snprintf(timeString, DEBUG_SECTION_NAME_LEN, "%ds", (int)(totalEnd - totalStart));
#endif
            cJSON_ReplaceItemInObject(root, DBG_TIME_ITEM_NAME, cJSON_CreateString(timeString));
#endif //TIME_SECTION
            // open the JSON file for CPU dump
#ifndef SPX_BMC_ACD
            sprintf(jsonFilename, "%s/%s", CRASHDUMP_DIRECTORY, RAM_CPUJSON_FILE);
#else
            sprintf(jsonFilename, "%s/%s", crashdump_json_dir, RAM_CPUJSON_FILE);
#endif
            fpJson = fopen(jsonFilename, "w");
            if (fpJson != NULL) {
                out = cJSON_Print(root);
                if (out != NULL) {
                    fprintf(fpJson, "%s", out);
                    cJSON_free(out);
                } else {
                    PRINT(PRINT_DBG, PRINT_ERROR, "cJSON_Print Failed\n");
                }
            }

            cJSON_Delete(root);
#endif //BUILD_JSON
            if (NULL != fpJson) {
                fclose(fpJson);
            }
            if (NULL != fp) {
                fclose(fp);
            }
            if (NULL != fpRaw) {
                fclose(fpRaw);
            }
            break;
        default:
            break;
    }
}

#ifdef SPX_BMC_ACD
static char *arglist[] =
{
	"?",
	"--dump-section",
	"--dump-now",
	"--verbose-dbg",
	NULL
};


static int ShowUsage( int argc, char **argv, int index );
static int DumpSection( int argc, char **argv, int index );
static int DumpNow( int argc, char **argv, int index );
static int VerboseDbgEnable( int argc, char **argv, int index );

static int ( *handlerList[] )( int, char **, int ) =
{
	ShowUsage,
	DumpSection,
	DumpNow,
	VerboseDbgEnable,
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
	printf( "\t\t\t 0: metadata\n");
	printf( "\t\t\t 1: sys_info\n");
	printf( "\t\t\t 2: power_man\n");
	printf( "\t\t\t 3: uncore_MCA\n");
	printf( "\t\t\t 4: uncore_status_regs\n");
	printf( "\t\t\t 5: TOR_dump\n");
	printf( "\t\t\t 6: addr_map\n");
	printf( "\t\t\t 7: core_MCA\n");
	printf( "\t\t\t 8: core_MLC_SQ\n");
	printf( "\t\t\t 9: crashdump\n");
	printf( "\t--dump-now: \t\t\tCollects the crashdump details right now and then the application exits\n" );
	printf( "\t\t\t\t\tThis argument can be used individually or in conjunction with --dump-section option\n" );
	printf( "\t--verbose-dbg: \t\t\tPrints the debug logs on console\n" );
	printf( "\t\t\t\t\tThis argument can be used individually or in conjunction with --dump-section or --dump-now option\n" );
	printf( "\n" );
	
	exit(0);
}

static int DumpSection(int argc, char **argv, int index)
{
    if( index + 1 < argc )
    	saveArg = atoi(argv[index + 1]);
    else
    {
        fprintf( stderr, "Missing argument to --dump-section\n" );
        exit(0);
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

static void parse_arguments( int argc, char **argv )
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
static void only_one_instance(char *crashdump_json_dir)
{
	struct flock fl;
	int fd = -1,ret = 0;

	ret = snprintf(ooi_path, sizeof(ooi_path), "%s/%s", crashdump_json_dir, ACD_INSTANCE_LOCK);
	if(ret < 0 || ret >= (signed)sizeof(ooi_path))
	{
		TCRIT("Buffer Overflow\n");
		exit(1);
	}
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
		close(fd);
		exit(1);
	}
 
	/*
	 * Run unlink(ooi_path) when the program exits. The program
	 * always releases locks when it exits.
	 */
	atexit(ooi_unlink);

	if(fd != -1) {
		close(fd);
	}
}

static void ReadMcaErrSrcLogReg(UINT8 u8Cpu, UINT32 * p32PeciData)
{
	SRdPkgConfigReq sRdPkgConfigReq;
	SRdPkgConfigRes sRdPkgConfigRes;

	// Get the MCA_ERR_SRC_LOG
	memset(&sRdPkgConfigReq, 0 , sizeof(SRdPkgConfigReq));
	memset(&sRdPkgConfigRes, 0 , sizeof(SRdPkgConfigRes));
	sRdPkgConfigReq.sHeader.u8ClientAddr = PECI_BASE_ADDR + u8Cpu;
	sRdPkgConfigReq.sHeader.u8WriteLength = 0x05;
	sRdPkgConfigReq.sHeader.u8ReadLength = sizeof(UINT32) + 1;
	sRdPkgConfigReq.u8CmdCode = 0xA1;
	sRdPkgConfigReq.u8HostID_Retry = 0x02;
	sRdPkgConfigReq.u8Index = MBX_INDEX_CPU_ID;
	sRdPkgConfigReq.u16Parameter = PKG_ID_MACHINE_CHECK_STATUS;
	if (0 == PECI_RdPkgConfig (&sRdPkgConfigReq, &sRdPkgConfigRes))
	{
		memcpy(p32PeciData, sRdPkgConfigRes.u8Data, sizeof(UINT32));
	}
	else
	{
		sRdPkgConfigRes.u8CompletionCode = 0x00;
	}
	if (sRdPkgConfigRes.u8CompletionCode != PECI_CC_SUCCESS) 
	{
		PRINT(PRINT_DBG, PRINT_ERROR, "[DBG] - CPU #%d MCA_ERR_SRC_LOG Failed\n", u8Cpu);
		return;
        }
	printf("MCA_ERR_SRC_LOG regsiter = 0x%x, cpu = %d\n", *p32PeciData, u8Cpu);

	return;
}

static int IsValidError()
{
	UINT32 u32PeciData;
	UINT8 u8Cpu;
	int (*IsError)(UINT32);

	PRINT(PRINT_DBG, PRINT_INFO, "Platform Debug - Read MCA_ERR_SRC_LOG\n");

	// Get the CPUID info
	for (u8Cpu = 0; u8Cpu < MAX_CPU; u8Cpu++) 
	{
		if (FALSE == IsCpuPresent(u8Cpu)) 
		{
			continue;
		}

		memset(&u32PeciData, 0x0, sizeof(UINT32));
		ReadMcaErrSrcLogReg(u8Cpu, &u32PeciData);

		IsError = dlsym(dl_handle,"ACDPDK_IsValidError");
		if (IsError != NULL)
		{
			if (IsError(u32PeciData))
				return 1;
		}
		else
			return 1;
	}

	return 0;
}

#endif

int main (int argc, char **argv)
{
	SPlatDBG sPlatDBG;
	ESTATUS eStatus = ST_OK;
#ifdef SPX_BMC_ACD
	int (*IsACDTriggered)();
	int (*IsValidTrigger)();
	int (*GetCrashdumpPath)(char *, int);
	int (*GetCrashdumpJSONPath)(char *, int);
	int (*PostDumpAction)();
	int (*GetPltRstSemName)(char *, int);
	int validTrigger = 0, checkErrCount = 0;
	char crashdump_dir[MAX_DIR_LEN] = {0};
	char crashdump_json_dir[MAX_DIR_LEN] = {0};
	int ret = 0;
#endif

	if (0)
	{
		IsCpuPresent(0);
		PECI_RdPkgConfig(NULL, NULL);
		PECI_WrPkgConfig(NULL, NULL);
	}

#ifdef SPX_BMC_ACD
	//Default CRASHDUMP directories (if PDK not available) 
	ret = snprintf(crashdump_dir, sizeof(crashdump_dir),"%s",CRASHDUMP_DIRECTORY);
	if(ret < 0 || ret >= (signed)sizeof(crashdump_dir))
	{
		TCRIT("Buffer Overflow\n");
		return 1;
	}		
	ret = snprintf(crashdump_json_dir,sizeof(crashdump_json_dir),"%s",CRASHDUMP_JSON_DIRECTORY);
	if(ret < 0 || ret >= (signed)sizeof(crashdump_json_dir))
	{
		TCRIT("Buffer Overflow\n");
		return 1;
	}
	dl_handle = dlopen((char *)ACDPDK_LIB,RTLD_NOW);
	if(!dl_handle)
	{
		printf("Error in loading ACDPDK_LIB library %s\n",dlerror());
		return 1;
	}

	GetCrashdumpPath = dlsym(dl_handle,"ACDPDK_GetCrashdumpPath");
	if (GetCrashdumpPath != NULL)
	{
		GetCrashdumpPath(crashdump_dir, sizeof(crashdump_dir));
	}
	
	GetCrashdumpJSONPath = dlsym(dl_handle,"ACDPDK_GetCrashdumpJSONPath");
	if (GetCrashdumpJSONPath != NULL)
	{
		GetCrashdumpJSONPath(crashdump_json_dir, sizeof(crashdump_json_dir));
	}

	GetPltRstSemName = dlsym(dl_handle,"ACDPDK_GetPltRstSemName");
	if (GetPltRstSemName != NULL)
	{
		GetPltRstSemName(plt_rst_sem_name, sizeof(plt_rst_sem_name));
	}

	/* Check only if one instance of Crashdump running */
	only_one_instance(crashdump_dir);
#endif

	struct stat st = {0};

#ifndef SPX_BMC_ACD
	if (stat(CRASHDUMP_DIRECTORY, &st) == -1) {
		mkdir(CRASHDUMP_DIRECTORY, 0700);
	}

	if (1 < argc)
	{
		saveArg = atoi(argv[1]);
	}
#else
	/* Fortify [Race Condition: File System Access]:: False Positive */
	/* Reason for False Positive – The file does not change between the calls of stat() and mkdir()*/
	if (stat(crashdump_dir, &st) == -1) {
		mkdir(crashdump_dir, 0700);
	}
	/* Fortify [Race Condition: File System Access]: False Positive */
        /* Reason for False Positive – The file does not change between the calls of stat() and mkdir()*/
	if (stat(crashdump_json_dir, &st) == -1) {
		mkdir(crashdump_json_dir, 0700);
	}

	/* Process arguments */
	parse_arguments(argc, argv);

	if (dumpNow == 1)
	{
		sPlatDBG.ePlatDBGCmd = PLAT_DBG_ON_IERR;
		// start the thread to monitor ACD PLATRST semaphore
		ACD_PLATRST_Abort = FALSE;
		eStatus = CreateThread((UINT8 *)"ACD PLATRST semaphore monitor Thread",
		                       MIN_STACKSIZE + 2048,
		                       ACD_PLATRST_semaphore_monitor_Thread,
		                       0,
		                       NULL);
		if (eStatus != ST_OK)
		{
		}
		doPlatformDebugLog(&sPlatDBG, crashdump_dir, crashdump_json_dir);
		system("/etc/init.d/crashdump_compress.sh &");
#ifdef CONFIG_SPX_FEATURE_GLOBAL_AUTONOMOUS_CRASH_DUMP_DECODE_SUPPORT
		system("/etc/init.d/crashdump_decode.sh &");
#endif

#ifdef CONFIG_SPX_FEATURE_ACD_BAFI_SUPPORT
        // create BAFI.json using acdbafigenerator binary app	
        if( access( "/var/crashdump/json/sysdebug2.json", F_OK ) != -1 ) {
	         system("/usr/local/bin/acdbafigenerator /var/crashdump/json/sysdebug2.json > /var/crashdump/json/bafi_decoder.json");
	} else {
	         system("/usr/local/bin/acdbafigenerator /var/crashdump/json/sysdebug1.json > /var/crashdump/json/bafi_decoder.json");
	}
#endif

		if (SEM_FAILED != ACD_PLATRST_sem) {
			sem_close(ACD_PLATRST_sem);
			ACD_PLATRST_sem = SEM_FAILED;
		}
		return 0;
	}
	
	IsACDTriggered = dlsym(dl_handle,"ACDPDK_IsACDTriggered");
	if (IsACDTriggered == NULL)
	{
		dlclose(dl_handle);
                return 1;
	}
#endif

	while(1) {
#ifdef SPX_BMC_ACD
		if (IsACDTriggered() != 1)
		{
			dlclose(dl_handle);
			return 1;
		}

		validTrigger = 1;
		checkErrCount = 0;
		while (!IsValidError())
		{
			//Check for error for about 30 seconds (delay of 100ms * 300 iterations)
			if (checkErrCount >= ERRCHECKMAXCOUNT)
			{
				PRINT(PRINT_DBG, PRINT_ERROR, "CRASHDUMP error is not valid!\n");
				break;
			}

			IsValidTrigger = dlsym(dl_handle,"ACDPDK_IsValidTrigger");
			if (IsValidTrigger != NULL)
			{
				//If not a valid CATERR, continue to wait for next valid CATERR
				if(IsValidTrigger() != TRUE)
				{
					validTrigger = 0;
					break;
				}
				checkErrCount++;
				usleep(100000);
			}
			else
				break;
		}
		if ((validTrigger == 0) || (checkErrCount >= ERRCHECKMAXCOUNT))
		{
			continue;
		}
#else
		/* Initialize pseudo CRASHDUMP signal using interprocess semaphore.
		 * The named semaphore should have been created and initialized by
		 * Error Interupt Handler funtion
		 */
		CRASHDUMP_sem = sem_open(CRASHDUMP_SEM, O_RDWR);
		if (SEM_FAILED == CRASHDUMP_sem) {
			fprintf(stderr, "failed to open CRASHDUMP interprocess semaphore \"%s\"\n", CRASHDUMP_SEM);
			return 1;
		}
		do {
			/* Flush any outstanding CRASHDUMP events encountered (semaphore posted
			 * via Error Interupt Handler funtion).
			 */
			while(sem_trywait(CRASHDUMP_sem) == 0) {
				;
			}
			/* if it returns an error (EAGAIN), it should be because there was nothing
			 * to decrement (already at 0).
			 */
		} while (EINTR == errno);

		if (SEM_FAILED == CRASHDUMP_sem)
			return 1;
		do {
			/* Check to see if a CRASHDUMP signal was encountered (semaphore posted in PDKInit.c).
			 */
			while(sem_wait(CRASHDUMP_sem) == 0) {
				break;
			}

			/* if it returns an error (EAGAIN), it should be because there was nothing
			 * to decrement (already at 0).
			 */
		} while (EINTR == errno);
#endif

#ifdef SPX_BMC_ACD
		printf("CRASHDUMP signal was encountered!\n");
#endif
		sPlatDBG.ePlatDBGCmd = PLAT_DBG_ON_IERR;
		time_t start, end;
		double first, second;
		struct rusage ruse;
		time(&start);
		first = CPU_TIME;

		// start the thread to monitor ACD PLATRST semaphore
		ACD_PLATRST_Abort = FALSE;
		eStatus = CreateThread((UINT8 *)"ACD PLATRST semaphore monitor Thread",
		                       MIN_STACKSIZE + 2048,
		                       ACD_PLATRST_semaphore_monitor_Thread,
		                       0,
		                       NULL);
		if (eStatus != ST_OK)
		{
		}

#ifdef SPX_BMC_ACD
		doPlatformDebugLog(&sPlatDBG, crashdump_dir, crashdump_json_dir);
#else
		doPlatformDebugLog(&sPlatDBG);
#endif

		// Save end time
		time(&end);
		second = CPU_TIME;
		printf("%s %s  cpu: %.6f secs  user: %d secs\n",
#ifdef BUILD_RAW
		"BUILD_RAW",
#else //BUILD_RAW
		" - ",
#endif //BUILD_RAW
#ifdef BUILD_JSON
		"BUILD_JSON",
#else //BUILD_JSON
		" - ",
#endif //BUILD_JSON
		(second - first), (int)(end - start));

#ifdef SPX_BMC_ACD
		system("/etc/init.d/crashdump_compress.sh &");

#ifdef CONFIG_SPX_FEATURE_GLOBAL_AUTONOMOUS_CRASH_DUMP_DECODE_SUPPORT
		system("/etc/init.d/crashdump_decode.sh &");
#endif

#ifdef CONFIG_SPX_FEATURE_ACD_BAFI_SUPPORT
        // create BAFI.json using acdbafigenerator binary app	
        if( access( "/var/crashdump/json/sysdebug2.json", F_OK ) != -1 ) {
	         system("/usr/local/bin/acdbafigenerator /var/crashdump/json/sysdebug2.json > /var/crashdump/json/bafi_decoder.json");
	} else {
	         system("/usr/local/bin/acdbafigenerator /var/crashdump/json/sysdebug1.json > /var/crashdump/json/bafi_decoder.json");
	}
#endif
		/* PDK for post crasdump actions */
		PostDumpAction = dlsym(dl_handle,"ACDPDK_PostDumpAction");
		if (PostDumpAction != NULL)
		{
			PostDumpAction();
		}
#else
#ifdef BUILD_JSON
		if (stat(CRASHDUMP_DIRECTORY "/" ARCHIVE_FILE_2, &st) == 0) {
			remove(CRASHDUMP_DIRECTORY "/" ARCHIVE_FILE_2);
		}

		if (stat(CRASHDUMP_DIRECTORY "/" ARCHIVE_FILE_1, &st) == 0) {
			rename(CRASHDUMP_DIRECTORY "/" ARCHIVE_FILE_1, CRASHDUMP_DIRECTORY "/" ARCHIVE_FILE_2);
		}

		char cmd[256];
		//sprintf(cmd, "CURDIR=$(pwd); cd %s ; tar -cPzvf %s %s/%s ; cd $CURDIR", CRASHDUMP_DIRECTORY, ARCHIVE_FILE_1, CRASHDUMP_DIRECTORY, RAM_CPUJSON_FILE);
		sprintf(cmd, "CURDIR=$(pwd); cd %s ; tar -czvf %s %s ; cd $CURDIR", CRASHDUMP_DIRECTORY, ARCHIVE_FILE_1, RAM_CPUJSON_FILE);
		system(cmd);
#endif //BUILD_JSON
#endif

		//sg_eLogXferState = LOGFILE_XFER_LOG_END;
	}

#ifdef SPX_BMC_ACD
	if(IsACDTriggered != NULL)
	{
		dlclose(dl_handle);
	}

#else
	if (SEM_FAILED != CRASHDUMP_sem) {
		sem_close(CRASHDUMP_sem);
		CRASHDUMP_sem = SEM_FAILED;
	}
#endif

	if (SEM_FAILED != ACD_PLATRST_sem) {
		sem_close(ACD_PLATRST_sem);
		ACD_PLATRST_sem = SEM_FAILED;
	}

	return 0;
}
