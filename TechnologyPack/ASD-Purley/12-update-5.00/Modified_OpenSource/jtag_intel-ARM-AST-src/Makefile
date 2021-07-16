DEBUG     := n
TARGET    := jtag_intel
OBJS      := jtag_drv.o

#EXTRA_CFLAGS += -DINTEL_JTAG_ADDITIONS

EXTRA_CFLAGS += -I${SPXINC}/global
EXTRA_CFLAGS += -I${SPXINC}/dbgout
EXTRA_CFLAGS += -I${SPXINC}/helper
EXTRA_CFLAGS += -DSPX_BMC

-include ${TOOLDIR}/rules/Rules.make.drivers

