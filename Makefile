
TARGET = runner
OBJS = main.o

BUILD_PRX = 1
PSP_FW_VERSION = 660

INCDIR =
CFLAGS = -O2 -G0 -Wall
CXXFLAGS = $(CFLAGS)
ASFLAGS = $(CFLAGS)

LIBS = -lpspgum -lpspgu -lpspdisplay -lpspge -lpspctrl -lpspkernel

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = Runner PSP 3D

include $(PSPSDK)/lib/build.mak
