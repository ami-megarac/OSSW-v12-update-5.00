#------- Comment/uncomment the following line to enable/disable debugging---------
DEBUG = n

#---------------------- Change according to your files ----------------------------
SRC 	= crashDump/Crashdump.c \
			crashDump/Metadata.c \
			crashDump/CoreMca.c \
			crashDump/UncoreMca.c \
			crashDump/TorDump.c \
			crashDump/PowerManagement.c \
			crashDump/AddressMap.c \
			crashDump/SqDump.c \
			crashDump/stubs.c \
			crashDump/UncoreStatus.c \
			crashDump/SysInfo.c \
			crashDump/Plat_dbg.c \
			peciInterface/peci_interface.c
TARGET = Crashdump 			# Output file

LDFLAGS += -lpthread

CFLAGS += -DBUILD_RAW
CFLAGS += -DBUILD_JSON
CFLAGS += -I${SPXINC}/global
CFLAGS += -I${SPXINC}/dbgout
CFLAGS += -I${SPXINC}
CFLAGS += -I${SPXINC}/peci
CFLAGS += -I${SPXINC}/checksum
CFLAGS += -I${SPXINC}/cjson2
CFLAGS += -IpeciInterface
CFLAGS += -IcrashDump
CFLAGS += -DSPX_BMC_ACD
CFLAGS += -Wno-format-truncation
CFLAGS += -Wno-maybe-uninitialized
CFLAGS += -Wno-stringop-truncation
CFLAGS += -I${SPXINC}/peci4


# Add the Corresponding library
#LIBS+= -L${SPXLIB}/libpeci -lpeci
LIBS+= -L${SPXLIB}/checksum -lchecksum
LIBS+= -L${SPXLIB}/libcjson2 -lcjson2
LIBS+= -L$(SPXLIB)/dbgout -Wl,-rpath /usr/local/lib -ldbgout
LIBS+= -L${SPXLIB}/peci4 -lpeci4
#LIBS+= -L${SPXLIB}/gpio -lgpio
LIBS+= -lrt
LIBS+= -ldl

include ${TOOLDIR}/rules/Rules.make.apps

#----------------------------------------------------------------------------------------
