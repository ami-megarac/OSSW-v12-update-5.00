DEBUG := n
TARGET := i2c_mux
OBJS := mux_if.o

# Kernel build switch
EXTRA_CFLAGS += -DLINUX_KERNEL_BUILD

EXTRA_CFLAGS += -I${SPXINC}/global
EXTRA_CFLAGS += -I${SPXINC}/i2cmux

EXTRA_CFLAGS += -I${SPXINC}/helper

KBUILD_EXTRA_SYMBOLS += ${SPXINC}/i2cmux/i2c_mux_hw.symvers
KBUILD_EXTRA_SYMBOLS += ${SPXINC}/i2cmux/i2c_mux_PCA954.symvers

-include ${TOOLDIR}/rules/Rules.make.drivers
