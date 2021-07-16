#------- Comment/uncomment the following line to enable/disable debugging---------
DEBUG = n

#---------------------- Change according to your files ----------------------------
SRC	+=	crashDump/crashdumpRunControl.c
SRCCPP +=	crashDump/crashdump.cpp \
		crashDump/utils.cpp \
		crashDump/CrashdumpSections/AddressMap.cpp \
     		crashDump/CrashdumpSections/TorDump.cpp \
		crashDump/CrashdumpSections/BigCore.cpp \
     		crashDump/CrashdumpSections/MetaData.cpp \
		crashDump/CrashdumpSections/CoreMca.cpp \
     		crashDump/CrashdumpSections/SqDump.cpp \
		crashDump/CrashdumpSections/Uncore.cpp \
		crashDump/CrashdumpSections/PowerManagement.cpp \
		crashDump/CrashdumpSections/UncoreMca.cpp

#ifdef CONFIG_SPX_FEATURE_OEMDATA_SECTION
SRCCPP +=	crashDump/CrashdumpSections/OemData.cpp
#endif

TARGET = Crashdump 			# Output file

LDFLAGS += -lpthread

CFLAGS += -DBUILD_RAW
CFLAGS += -DBUILD_JSON
CFLAGS += -I${SPXINC}/global
CFLAGS += -I${SPXINC}/dbgout
CFLAGS += -I${SPXINC}
CFLAGS += -I${SPXINC}/peci
CFLAGS += -I${SPXINC}/peci4
CFLAGS += -I${SPXINC}/checksum
CFLAGS += -I${SPXINC}/cjson2
CFLAGS += -I${SPXINC}/cjson2/cjson
CFLAGS += -I${SPXINC}/safesystem
CFLAGS += -IcrashDump
CFLAGS += -DSPX_BMC_ACD
CFLAGS +=  -Wno-missing-field-initializers
CFLAGS +=  -Wno-unused-but-set-variable
CFLAGS +=  -Wno-unused-parameter
CFLAGS +=  -Wno-unused-function
CFLAGS +=  -Wno-sign-compare
CFLAGS +=  -Wno-unused-variable
CFLAGS +=  -Wno-write-strings
CFLAGS +=  -Wno-psabi
#CFLAGS += -std=gnu99

CXXFLAGS += -std=c++11
CXXFLAGS += -std=gnu++17
CXXFLAGS += -Wno-missing-braces
CXXFLAGS += -Wno-switch-default
#CXXFLAGS += -std=c++1z

# Add the Corresponding library
LIBS+= -L${SPXLIB}/peci4 -lpeci4
LIBS+= -L${SPXLIB}/checksum -lchecksum
LIBS+= -L${SPXLIB}/libcjson2 -lcjson2
LIBS+= -L$(SPXLIB)/dbgout -Wl,-rpath /usr/local/lib -ldbgout
LIBS+= -L${SPXLIB}/safesystem -lsafesystem
#LIBS+= -L${SPXLIB}/gpio -lgpio
LIBS+= -lrt
LIBS+= -ldl
LIBS+= -lstdc++ -lstdc++fs

include ${TOOLDIR}/rules/Rules.make.apps

#----------------------------------------------------------------------------------------
