#------- Comment/uncomment the following line to enable/disable debugging---------
DEBUG = n
TARGET = acdbafigenerator
#---------------------- Change according to your files ----------------------------
SRC += dummy.c
SRCCPP += main.cpp
#SRCCPP += HarddiskImgFileReader.cpp
#SRCCPP += comm.cpp
#SRCCPP += server.cpp
#SRC += cdromimagereader.c
#SRC += harddiskimagereader.c




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

CFLAGS += -DFIX_ALIGN
CFLAGS +=-D_FILE_OFFSET_BITS=64
CFLAGS +=-D_GNU_SOURCE

CXXFLAGS +=  -Wno-missing-field-initializers
CXXFLAGS +=  -Wno-unused-but-set-variable
CXXFLAGS +=  -Wno-unused-parameter
CXXFLAGS +=  -Wno-unused-function
CXXFLAGS +=  -Wno-sign-compare
CXXFLAGS +=  -Wno-unused-variable
CXXFLAGS +=  -Wno-write-strings
CXXFLAGS +=  -Wno-missing-braces
CXXFLAGS +=  -Wno-ignored-qualifiers
CXXFLAGS += -std=c++17
CXXFLAGS +=  -Wno-switch-default
CXXFLAGS +=  -Wno-return-type

# Add the Corresponding library
#LIBS+= -L$(SPXLIB)/dbgout -Wl,-rpath /usr/local/lib -ldbgout
LIBS+= -L${SPXLIB}/libcjson2 -lcjson2
LIBS+= -L${SPXLIB}/peci4 -lpeci4
LIBS+= -L${SPXLIB}/checksum -lchecksum
LIBS+= -L$(SPXLIB)/dbgout -Wl,-rpath /usr/local/lib -ldbgout
#LIBS+= -L${SPXLIB}/gpio -lgpio
LIBS+= -lrt
LIBS+= -ldl
LIBS +=  -lpthread 
LIBS +=  -lstdc++


include ${TOOLDIR}/rules/Rules.make.apps

#----------------------------------------------------------------------------------------
