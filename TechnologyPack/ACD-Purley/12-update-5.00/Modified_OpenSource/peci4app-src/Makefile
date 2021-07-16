#------- Comment/uncomment the following line to enable/disable debugging---------
DEBUG = n

#---------------------- Change according to your files ----------------------------
SRC 		= peci4app.c			# Source File
TARGET = peci4app 			# Output file

CFLAGS += -I${SPXINC}/peci
CFLAGS += -I${SPXINC}/peci4
CFLAGS += -I${SPXINC}/global
CFLAGS += -DSPX_BMC_ACD

# Add the Corresponding library
LIBS+= -L${SPXLIB}/peci4 -lpeci4
LIBS+= -L${SPXLIB}/checksum -lchecksum

include ${TOOLDIR}/rules/Rules.make.apps

#----------------------------------------------------------------------------------------
