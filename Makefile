#
# myapps/terminal/Makefile
#

.PHONY: build install clean

ifeq ($(DEVDIR),)
   $(error ====== DEVDIR variable is empty, invoke this Makefile from the BSP root, or provide the path to it =====)
endif
include $(DEVDIR)/bsp/mach/Make.conf
include $(DEVDIR)/fs/Apps.defs


BIN			= terminal
AUTORUN_SCRIPT_SRC	= src/autorun_terminal.sh
AUTORUN_SCRIPT  	= autorun_terminal.sh
AUTORUN_SCIPT_LINK	= S98autorun_terminal.sh

SRCS			= src/utils/streaming.h src/utils/streaming.c src/utils/comm.h src/utils/comm.c src/utils/filesys.h src/utils/filesys.c src/terminal.c
OBJS			= $(SRCS:.c=.o)

LDFLAGS = -Wl,--rpath-link -Wl,$(FSDEVROOT)/usr/lib:$(FSDEVROOT)/lib
EXTRA_CFLAGS = -pthread -I$(FSDEVROOT)/usr/include -I$(FSDEVROOT)/usr/include/gstreamer-0.10 -I$(FSDEVROOT)/usr/include/glib-2.0 -I$(FSDEVROOT)/usr/lib/glib-2.0/include -I$(FSDEVROOT)/usr/include/libxml2   
EXTRA_LIBS = -pthread -L$(FSDEVROOT)/usr/lib -ljson -lgstreamer-0.10 -lgobject-2.0 -lgmodule-2.0 -lxml2 -lgthread-2.0 -lrt -lglib-2.0  -ljson

build: 
	$(V)$(CC) $(CFLAGS) $(EXTRA_CFLAGS) -o $(BIN) $(SRCS) $(LDFLAGS) $(EXTRA_LIBS) $(QOUT)
install: 
	$(V)install -D -m 755 $(BIN) $(FSROOT)/opt/$(BIN) $(QOUT)
	$(V)cp -f -r $(AUTORUN_SCRIPT_SRC) $(FSROOT)/etc/init.d
	$(V)chmod +x $(FSROOT)/etc/init.d/$(AUTORUN_SCRIPT)
	$(V)ln -f $(FSROOT)/etc/init.d/$(AUTORUN_SCRIPT) $(FSROOT)/etc/rc.d/$(AUTORUN_SCIPT_LINK)
clean: 
	$(V)rm -f $(BIN) *.debug *.o core *~ $(QOUT)

# Empty but required targets
sim:
chkconfig:
preconfig:
buildfs:
