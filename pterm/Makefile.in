include ../version.mk
include ../config.mk

COBJS = pterm.o
CFLAGS  = -DBSD_COMP -g
TARGET = pterm

ifeq ($(shell uname),Darwin)
	# Add a few things for the Mac
	CFLAGS += -D_XOPEN_SOURCE
	CFLAGS += -Wno-int-to-void-pointer-cast
	OS = macosx
else
	OS = linux
endif


$(TARGET): $(COBJS)
	$(CC) -o $(TARGET) $(COBJS)

clean:
	rm -f $(COBJS) $(TARGET)
	
distclean: clean
	rm -rf Makefile config.h config.log config.status config.mk autom4te.cache

install: $(TARGET)
	mkdir -p $(BIN_DIR)
	$(INSTALL_PROGRAM) $(TARGET) $(BIN_DIR)

