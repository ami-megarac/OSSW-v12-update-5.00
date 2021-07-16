#
# Copyright (c) 2017, Intel Corporation
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
#    * Redistributions of source code must retain the above copyright notice,
#      this list of conditions and the following disclaimer.
#    * Redistributions in binary form must reproduce the above copyright
#      notice, this list of conditions and the following disclaimer in the
#      documentation and/or other materials provided with the distribution.
#    * Neither the name of Intel Corporation nor the names of its contributors
#      may be used to endorse or promote products derived from this software
#      without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

SRC =      socket_main.c \
          ext_network.c \
          authenticate.c \
          session.c \
          logging.c \
          config.c \
          handlers/asd_msg.c \
          handlers/JTAG_handler.c \
          handlers/debug_enable.c \
          handlers/gpio.c \
          handlers/i2c_handler.c \
          handlers/i2c_msg_builder.c \
          handlers/ipmi.c \
          handlers/platform_reset.c \
          handlers/power_btn.c \
          handlers/power_debug.c \
          handlers/power_good.c \
          handlers/prdy.c \
          handlers/preq.c \
          handlers/tck_mux_select.c \
          handlers/xdp_present.c \
          handlers/sys_pwr_ok.c \
          handlers/reset_btn.c \
          handlers/rsm_reset.c \
          handlers/target_handler.c \
          handlers/ext_tcp.c \
          handlers/ext_tls.c \
          handlers/auth_none.c \
          handlers/auth_pam.c

TARGET = RemoteDebugServer 			# Output file

ADTM_VERSION := $(shell date +"%c" | tr ' :' '__')

#LDFLAGS +=  -std=c++11 -lstdc++

CFLAGS += -I.
CFLAGS += -Ihandlers
CFLAGS += -I${TARGETDIR}/usr/include
CFLAGS += -I${SPXINC}/global
CFLAGS += -I${SPXINC}/ipmihalapi
CFLAGS += -I${SPXINC}/featuredefine
CFLAGS += -I${SPXINC}/ipmi
CFLAGS += -I${SPXINC}/pdk
CFLAGS += -I${SPXINC}/unix
CFLAGS += -I${SPXINC}/pdk
CFLAGS += -I${SPXINC}/hostname
CFLAGS += -I${SPXINC}/network
CFLAGS += -I${SPXINC}/dbgout
CFLAGS += -I${SPXINC}
CFLAGS += -I${SPXINC}/libipmi
CFLAGS += -I${SPXINC}/jtag_intel

#LDFLAGS += -lpthread -lld\
 #          -znoexecstack -zrelro -znow -pie

#LDFLAGS += -lpthread -ldl -fPIC
CFLAGS += -D_GNU_SOURCE -DSPX_BMC -Wno-type-limits -Wno-missing-field-initializers -Wno-sign-compare -Wno-unused-parameter -fno-strict-aliasing -std=c99
CPPFLAGS += -std=gnu++0x -O3

LIBS += -lpthread -ldl
LIBS += -L${SPXLIB}/dbgout -ldbgout
LIBS += -L$(SPXLIB)/ipmiparams -lipmiparams
LIBS += -L$(SPXLIB)/ipmi -lipmi
LIBS += -L$(SPXLIB)/gpio -lgpio
LIBS += ${IMAGE_TREE}/lib/${MULTI_ARCH_LIB}/libpam.so.0
LIBS    += ${IMAGE_TREE}/usr/lib/${MULTI_ARCH_LIB}/libcrypto.so
LIBS    += ${IMAGE_TREE}/usr/lib/${MULTI_ARCH_LIB}/libssl.so
LIBS    += ${IMAGE_TREE}/usr/lib/${MULTI_ARCH_LIB}/libz.so


include ${TOOLDIR}/rules/Rules.make.apps

#ifeq ($(CONFIG_SPX_FEATURE_NO_STRICT_CC_ANY),NO)
CFLAGS += -Wno-error
#endif


