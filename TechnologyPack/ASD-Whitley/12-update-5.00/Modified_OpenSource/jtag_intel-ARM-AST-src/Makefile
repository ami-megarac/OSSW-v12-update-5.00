DEBUG     := n
ifeq ($(apptype),jtagcore)
OBJS      := jtag.o
TARGET    := jtag_intel_core
endif

ifeq ($(apptype),jtagaspeed)
OBJS      := jtag-aspeed.o
TARGET    := jtag_intel_aspeed
endif

#EXTRA_CFLAGS += -DINTEL_JTAG_ADDITIONS

EXTRA_CFLAGS += -I$(PWD)/include
EXTRA_CFLAGS += -I${SPXINC}/global
EXTRA_CFLAGS += -I${SPXINC}/dbgout
EXTRA_CFLAGS += -I${SPXINC}/helper
EXTRA_CFLAGS += -DSPX_BMC

-include ${TOOLDIR}/rules/Rules.make.drivers

