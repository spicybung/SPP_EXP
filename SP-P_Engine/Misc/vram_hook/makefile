TARGET = ../../../vramext
OBJS = main.o exports.o sceGe_driver.o

INCDIR = ../include
CFLAGS = -Os -G0 -Wall -fshort-wchar -fno-pic -mno-check-zero-division -DF_sceGe_driver_0013 -Wno-discarded-array-qualifiers
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

BUILD_PRX = 1
PRX_EXPORTS = exports.exp

USE_KERNEL_LIBC = 1
USE_KERNEL_LIBS = 1

LIBDIR = ../../../libs/precompiled
LIBS = -lpspdve_driver -lpsphprm_driver -lpspge_driver -lpspimpose_driver

PSP_FW_VERSION=500

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build_prx.mak

EXTRA_TARGETS = VramExt.S VramExt_driver.S

VramExt.S : $(PRX_EXPORTS)
	psp-build-exports --build-stubs $(PRX_EXPORTS)

VramExt_driver.S : $(PRX_EXPORTS)
	psp-build-exports --build-stubs $(PRX_EXPORTS)

all: $(FINAL_TARGET) VramExt.S VramExt_driver.S
