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
STREAM_SCRIPT_SRC	= src/stream.sh
AUTORUN_SCRIPT_SRC	= src/autorun_terminal.sh

STREAM_SCRIPT		= stream.sh
AUTORUN_SCRIPT  	= autorun_terminal.sh
AUTORUN_SCIPT_LINK	= S98autorun_terminal.sh

SRCS			= src/utils/media.h src/utils/media.c src/utils/comm.h src/utils/comm.c src/utils/filesys.h src/utils/filesys.c src/terminal.c
OBJS			= $(SRCS:.c=.o)
INCLUDES		= -I $(LIBJSON_INCLUDE)
LIBS			= -L $(LIBJSON_LIB) -l json -lm -lpthread

build: 
	$(V)$(CC) $(CFLAGS) $(INCLUDES) -o $(BIN) $(SRCS) $(LIBS) $(QOUT)
install: 
	$(V)install -D -m 755 $(BIN) $(FSROOT)/opt/$(BIN) $(QOUT)
	$(V)cp -f -r $(STREAM_SCRIPT_SRC) $(FSROOT)/opt/
	$(V)chmod +x $(FSROOT)/opt/$(STREAM_SCRIPT)
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
