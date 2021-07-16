#------- Comment/uncomment the following line to enable/disable debugging---------
DEBUG = n

#---------------------- Change according to your files ----------------------------
SRC    = jtagtest.c

TARGET = jtag_test 			# Output file

CFLAGS += -I${SPXINC}/global
CFLAGS += -I${SPXINC}/gpio
CFLAGS += -I${SPXINC}/jtag_intel
CFLAGS += -DSPX_BMC -Wno-sign-compare -std=c99 -Wno-unused-parameter

# Add the Corresponding library
LIBS+= -L${SPXLIB}/gpio -lgpio
LIBS+= -L${SPXLIB}/dbgout -ldbgout

include ${TOOLDIR}/rules/Rules.make.apps

#----------------------------------------------------------------------------------------
