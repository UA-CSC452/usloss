include ../version.mk
include ../config.mk

COBJS = makedisk.o
CFLAGS += -I../src -std=gnu99
TARGET = makedisk
LIBRARY = libdisk$(VERSION).a
LIBOBJS = disk.o
LIBS = -ldisk
HDRS = libdisk.h


ifeq ($(shell uname),Darwin)
	# Add a few things for the Mac
	CFLAGS += -D_XOPEN_SOURCE
	CFLAGS += -Wno-int-to-void-pointer-cast
	OS = macosx
else
	OS = linux
endif

all: $(TARGET) $(LIBRARY)

$(TARGET): $(COBJS) $(LIBRARY)
	$(CC) $(CFLAGS) -o $@ $^

$(LIBRARY) : $(LIBOBJS)
	$(AR) -r $@ $^

clean:
	rm -f $(COBJS) $(LIBOBJS) $(TARGET) $(LIBRARY) *.a

distclean: clean
	rm -rf Makefile config.h config.log config.status config.mk autom4te.cache

install: $(TARGET) $(LIBRARY)
	mkdir -p $(BIN_DIR) $(LIB_DIR)
	$(INSTALL_PROGRAM) $(TARGET) $(BIN_DIR)
	$(INSTALL_DATA) $(HDRS) $(INC_DIR)
	$(INSTALL_DATA) $(LIBRARY) $(LIB_DIR)
