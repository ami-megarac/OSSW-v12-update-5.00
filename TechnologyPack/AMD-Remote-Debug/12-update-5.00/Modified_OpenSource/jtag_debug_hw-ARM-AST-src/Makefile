DEBUG := n
TARGET := jtag_debug_hw

OBJS := jtag_debug.o 

EXTRA_CFLAGS += -I${SPXINC}/global
EXTRA_CFLAGS += -I${SPXINC}/helper
EXTRA_CFLAGS += -I${SPXINC}/jtag

KBUILD_EXTRA_SYMBOLS += ${SPXINC}/jtag/Module.symvers

-include ${TOOLDIR}/rules/Rules.make.drivers

